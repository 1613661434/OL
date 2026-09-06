/****************************************************************************************/
/*
 * 程序名：ol_database.h
 * 功能描述：数据库连接抽象基类 IDBConn + 模板连接池 DBPool，支持以下特性：
 *          - 线程安全：通过互斥锁和条件变量保证多线程环境下连接获取/释放安全
 *          - 阻塞等待：无空闲连接时线程自动阻塞，不占用CPU资源
 *          - 超时获取：支持带默认超时的连接获取，避免无限阻塞
 *          - 自动重连：获取连接时自动检测有效性，失效则重新连接
 *          - 资源管理：连接池销毁时自动关闭所有连接，无资源泄漏
 *          - 单例模式：基于CRTP单例基类，每种数据库类型对应一个全局单例
 * 规范：所有数据库 DBConn 必须继承 IDBConn
 * 作者：ol
 * 适用标准：C++17及以上
 */
/****************************************************************************************/

#ifndef OL_DATABASE_H
#define OL_DATABASE_H 1

#include "ol_type_traits.h"
#include <functional>
#include <stdexcept>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <type_traits>
#include <chrono>
#include <utility>

namespace ol
{
    /**
     * @brief 数据库连接抽象基类
     * @note 所有数据库连接实现类必须继承该抽象类
     * @note 禁止拷贝/移动，保证连接资源独占性
     */
    class IDBConn : public TypeNonCopyableMovable
    {
    public:
        virtual ~IDBConn() = default;

        /**
         * @brief 建立数据库连接
         * @return 连接成功返回true，失败返回false
         */
        virtual bool connect() = 0;

        /**
         * @brief 断开数据库连接
         */
        virtual void disconnect() = 0;

        /**
         * @brief 检查连接是否有效
         * @return 已连接返回true，未连接返回false
         */
        virtual bool isConnected() const = 0;

        /**
         * @brief 重置连接状态
         * @note 释放连接回池前调用，清理上下文状态
         */
        virtual void reset() = 0;
    };

    /**
     * @brief 模板数据库连接池（全局单例模式）
     * @tparam T 数据库连接类型，必须继承自IDBConn
     * @note 线程安全设计，支持多线程并发获取/释放连接
     * @note 继承单例基类，每种数据库类型对应唯一全局单例
     * @note 禁止外部构造/拷贝/移动，保证单例唯一性
     */
    template <typename T>
    class DBPool : public TypeSingleton<DBPool<T>>
    {
        // 编译期校验：连接类型必须继承自IDBConn
        static_assert(std::is_base_of<IDBConn, T>::value, "DBPool: DBConn Must inherit from IDBConn");
        // 单例基类友元，允许访问私有构造函数
        friend class TypeSingleton<DBPool<T>>;

    public:
        using ConnConfigCallback = std::function<void(T& conn)>;
        using TimeoutMs = std::chrono::milliseconds;

    private:
        using StoragePtr = std::unique_ptr<T>;

        enum class State : char
        {
            Uninitialized,
            Initializing,
            Running,
            Stopped
        };

    public:
        /**
         * @brief 独占连接租约，离开作用域时自动将连接归还连接池
         * @note 不可复制但可以移动，从类型上避免重复归还和归还外部连接
         */
        class ConnectionLease
        {
            friend class DBPool<T>;

        private:
            DBPool* m_pool{nullptr};
            StoragePtr m_conn;

            ConnectionLease(DBPool* pool, StoragePtr conn) noexcept
                : m_pool(pool), m_conn(std::move(conn))
            {
            }

        public:
            ConnectionLease() noexcept = default;
            ~ConnectionLease() { reset(); }

            ConnectionLease(const ConnectionLease&) = delete;
            ConnectionLease& operator=(const ConnectionLease&) = delete;

            ConnectionLease(ConnectionLease&& other) noexcept
                : m_pool(std::exchange(other.m_pool, nullptr)),
                  m_conn(std::move(other.m_conn))
            {
            }

            ConnectionLease& operator=(ConnectionLease&& other) noexcept
            {
                if (this == &other) return *this;
                reset();
                m_pool = std::exchange(other.m_pool, nullptr);
                m_conn = std::move(other.m_conn);
                return *this;
            }

            T* get() const noexcept { return m_conn.get(); }
            T& operator*() const noexcept { return *m_conn; }
            T* operator->() const noexcept { return m_conn.get(); }
            explicit operator bool() const noexcept { return static_cast<bool>(m_conn); }

            void reset() noexcept
            {
                if (!m_conn)
                {
                    m_pool = nullptr;
                    return;
                }

                StoragePtr conn = std::move(m_conn);
                DBPool* pool = std::exchange(m_pool, nullptr);
                if (pool) pool->returnConnection(std::move(conn));
            }
        };

        // 保留原名称以兼容使用auto/ConnPtr的现有代码；实际类型已改为RAII租约。
        using ConnPtr = ConnectionLease;

    private:
        std::queue<StoragePtr> m_queue; ///< 空闲连接队列
        mutable std::mutex m_mtx;       ///< 互斥锁
        std::condition_variable m_cv;   ///< 条件变量：实现空闲连接阻塞等待
        size_t m_max_conn{0};           ///< 最大连接数
        ConnConfigCallback m_config_cb; ///< 连接初始化配置回调
        State m_state{State::Uninitialized}; ///< 连接池状态（由m_mtx保护）

    private:
        // 单例模式：构造函数私有化
        DBPool() = default;

        // 析构函数私有化
        ~DBPool() { destroy(); }

        static void disconnectNoexcept(StoragePtr& conn) noexcept
        {
            if (!conn) return;
            try
            {
                conn->disconnect();
            }
            catch (...)
            {
            }
        }

        static void disconnectAll(std::queue<StoragePtr>& connections) noexcept
        {
            while (!connections.empty())
            {
                StoragePtr conn = std::move(connections.front());
                connections.pop();
                disconnectNoexcept(conn);
            }
        }

        void returnConnection(StoragePtr conn) noexcept
        {
            if (!conn) return;

            try
            {
                conn->reset();
            }
            catch (...)
            {
                disconnectNoexcept(conn);
                return;
            }

            bool returned = false;
            {
                std::lock_guard<std::mutex> lock(m_mtx);
                if (m_state == State::Running)
                {
                    try
                    {
                        m_queue.push(std::move(conn));
                        returned = true;
                    }
                    catch (...)
                    {
                    }
                }
            }

            if (returned)
                m_cv.notify_one();
            else
                disconnectNoexcept(conn);
        }

        ConnPtr prepareLease(StoragePtr conn, const ConnConfigCallback& config_cb)
        {
            try
            {
                if (!conn->isConnected())
                {
                    config_cb(*conn);
                    if (!conn->connect())
                    {
                        returnConnection(std::move(conn));
                        return {};
                    }
                }
            }
            catch (...)
            {
                returnConnection(std::move(conn));
                throw;
            }

            return ConnPtr(this, std::move(conn));
        }

    public:
        /**
         * @brief 初始化单例连接池（全局仅需调用1次）
         * @param max_conn 最大连接数（必须大于0）
         * @param config_cb 连接配置回调（设置地址、账号、密码、字符集等参数）
         * @throw std::invalid_argument 回调为空或最大连接数为0时抛出
         * @throw std::runtime_error 连接初始化失败时抛出
         */
        void init(size_t max_conn, ConnConfigCallback config_cb)
        {
            if (!config_cb) throw std::invalid_argument("DBPool: Connection configuration callback cannot be empty!");
            if (max_conn == 0) throw std::invalid_argument("DBPool: Maximum connection count cannot be zero!");

            {
                std::lock_guard<std::mutex> lock(m_mtx);
                if (m_state != State::Uninitialized)
                    throw std::runtime_error("DBPool: Singleton pool has been initialized or stopped!");
                m_state = State::Initializing;
            }

            std::queue<StoragePtr> connections;
            try
            {
                for (size_t i = 0; i < max_conn; ++i)
                {
                    StoragePtr conn = std::make_unique<T>();
                    config_cb(*conn);
                    if (!conn->connect())
                    {
                        disconnectNoexcept(conn);
                        throw std::runtime_error("DBPool: Database connection initialization failed!");
                    }
                    connections.push(std::move(conn));
                }
            }
            catch (...)
            {
                disconnectAll(connections);
                {
                    std::lock_guard<std::mutex> lock(m_mtx);
                    if (m_state == State::Initializing) m_state = State::Uninitialized;
                }
                m_cv.notify_all();
                throw;
            }

            bool initialized = false;
            {
                std::lock_guard<std::mutex> lock(m_mtx);
                if (m_state == State::Initializing)
                {
                    m_max_conn = max_conn;
                    m_config_cb = std::move(config_cb);
                    m_queue.swap(connections);
                    m_state = State::Running;
                    initialized = true;
                }
            }

            m_cv.notify_all();
            if (initialized) return;

            disconnectAll(connections);
            throw std::runtime_error("DBPool: Pool was stopped during initialization!");
        }

        /**
         * @brief 阻塞获取连接（无限等待，直到获取到可用连接）
         * @return 成功返回连接租约；连接池已停止时返回空租约
         * @note 无空闲连接时线程阻塞，获取后自动检查连接有效性
         * @note 必须先调用init()初始化连接池
         */
        ConnPtr get()
        {
            StoragePtr conn;
            ConnConfigCallback config_cb;
            std::unique_lock<std::mutex> lock(m_mtx);

            if (m_state == State::Uninitialized)
                throw std::logic_error("DBPool::get: Pool has not been initialized!");

            if (m_state == State::Initializing)
                m_cv.wait(lock, [this]() { return m_state != State::Initializing; });

            if (m_state != State::Running) return {};

            m_cv.wait(lock, [this]()
                      { return !m_queue.empty() || m_state != State::Running; });

            if (m_state != State::Running) return {};

            conn = std::move(m_queue.front());
            m_queue.pop();
            config_cb = m_config_cb;
            lock.unlock();

            return prepareLease(std::move(conn), config_cb);
        }

        /**
         * @brief 超时获取连接（带默认超时时间）
         * @param timeout_ms 超时时间，默认3秒
         * @return 成功返回连接租约；超时或连接池已停止时返回空租约
         * @note 超时时间可自定义，避免线程无限阻塞
         * @note 必须先调用init()初始化连接池
         */
        ConnPtr getTimeout(TimeoutMs timeout_ms = TimeoutMs(3000))
        {
            if (timeout_ms < TimeoutMs::zero())
                throw std::invalid_argument("DBPool::getTimeout: Timeout cannot be negative!");

            const auto deadline = std::chrono::steady_clock::now() + timeout_ms;
            StoragePtr conn;
            ConnConfigCallback config_cb;
            std::unique_lock<std::mutex> lock(m_mtx);

            if (m_state == State::Uninitialized)
                throw std::logic_error("DBPool::getTimeout: Pool has not been initialized!");

            if (m_state == State::Initializing &&
                !m_cv.wait_until(lock, deadline, [this]() { return m_state != State::Initializing; }))
                return {};

            if (m_state != State::Running) return {};

            if (!m_cv.wait_until(lock, deadline, [this]()
                                 { return !m_queue.empty() || m_state != State::Running; }))
                return {};

            if (m_state != State::Running) return {};

            conn = std::move(m_queue.front());
            m_queue.pop();
            config_cb = m_config_cb;
            lock.unlock();

            return prepareLease(std::move(conn), config_cb);
        }

        /**
         * @brief 提前释放连接租约（通常无需调用，租约析构时会自动归还）
         * @param conn 由当前连接池获取的连接租约
         * @note 重复调用安全；外部连接无法构造成ConnectionLease
         */
        void release(ConnPtr& conn) noexcept
        {
            conn.reset();
        }

        /**
         * @brief 销毁连接池，关闭所有数据库连接
         * @note 唤醒所有等待线程，避免销毁时阻塞卡死
         */
        void destroy() noexcept
        {
            {
                std::lock_guard<std::mutex> lock(m_mtx);
                if (m_state == State::Stopped) return;
                m_state = State::Stopped;
            }

            m_cv.notify_all();

            std::queue<StoragePtr> connections;
            {
                std::lock_guard<std::mutex> lock(m_mtx);
                connections.swap(m_queue);
            }
            disconnectAll(connections);
        }

        /**
         * @brief 获取当前空闲连接数量
         * @return 空闲连接数
         * @note 线程安全，通过互斥锁保护队列访问
         */
        size_t idle() const
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            return m_queue.size();
        }

        /**
         * @brief 获取连接池最大连接数
         * @return 最大连接数量
         */
        size_t maxConn() const
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            return m_max_conn;
        }

        /**
         * @brief 检查连接池是否仍接受连接获取和归还
         */
        bool isRunning() const
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            return m_state == State::Running;
        }
    };

} // namespace ol

#endif // OL_DATABASE_H
