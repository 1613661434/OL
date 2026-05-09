/****************************************************************************************/
/*
 * 程序名：ol_database.h
 * 功能描述：数据库连接抽象基类 IDBConn + 模板连接池 DBPool，支持以下特性：
 *          - 线程安全：通过互斥锁和条件变量保证多线程环境下连接获取/释放安全
 *          - 阻塞等待：无空闲连接时线程自动阻塞，不占用CPU资源
 *          - 超时获取：支持带默认超时的连接获取，避免无限阻塞
 *          - 自动重连：获取连接时自动检测有效性，失效则重新连接
 *          - 资源管理：连接池销毁时自动关闭所有连接，无资源泄漏
 * 规范：所有数据库 DBConn 必须继承 IDBConn
 * 作者：ol
 * 适用标准：C++17及以上
 */
/****************************************************************************************/

#ifndef OL_DATABASE_H
#define OL_DATABASE_H

#include "ol_type_traits.h"
#include <functional>
#include <stdexcept>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <type_traits>
#include <chrono>

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
     * @brief 模板数据库连接池
     * @tparam T 数据库连接类型，必须继承自IDBConn
     * @note 线程安全设计，支持多线程并发获取/释放连接
     * @note 禁止拷贝/移动，保证连接池单例唯一性
     */
    template <typename T>
    class DBPool : public TypeNonCopyableMovable
    {
        // 编译期校验：连接类型必须继承自IDBConn
        static_assert(std::is_base_of<IDBConn, T>::value, "DBPool: DBConn Must inherit from IDBConn");

    public:
        using ConnPtr = std::shared_ptr<T>;
        using ConnConfigCallback = std::function<void(T& conn)>;
        using TimeoutMs = std::chrono::milliseconds;

    private:
        std::queue<ConnPtr> m_queue;    ///< 空闲连接队列
        mutable std::mutex m_mtx;       ///< 互斥锁
        std::condition_variable m_cv;   ///< 条件变量：实现空闲连接阻塞等待
        size_t m_max_conn;              ///< 最大连接数
        ConnConfigCallback m_config_cb; ///< 连接初始化配置回调

    public:
        /**
         * @brief 构造数据库连接池
         * @param max_conn 最大连接数（必须大于0）
         * @param config_cb 连接配置回调（设置地址、账号、密码、字符集等参数）
         * @throw std::invalid_argument 回调为空或最大连接数为0时抛出
         * @throw std::runtime_error 连接初始化失败时抛出
         */
        explicit DBPool(size_t max_conn, ConnConfigCallback config_cb)
            : m_max_conn(max_conn), m_config_cb(std::move(config_cb))
        {
            if (!m_config_cb) throw std::invalid_argument("DBPool: Connection configuration callback cannot be empty!");
            if (m_max_conn == 0) throw std::invalid_argument("DBPool: Maximum connection count cannot be zero!");

            // 初始化所有连接
            for (size_t i = 0; i < m_max_conn; ++i)
            {
                ConnPtr conn = std::make_shared<T>();
                m_config_cb(*conn);
                if (!conn->connect()) throw std::runtime_error("DBPool: Database connection initialization failed!");
                m_queue.push(conn);
            }
        }

        /**
         * @brief 析构函数
         * @note 自动调用destroy()，关闭所有连接并释放资源
         */
        ~DBPool()
        {
            destroy();
        }

        /**
         * @brief 阻塞获取连接（无限等待，直到获取到可用连接）
         * @return 有效的数据库连接智能指针
         * @note 无空闲连接时线程阻塞，获取后自动检查连接有效性
         */
        ConnPtr get()
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            // 等待队列非空
            m_cv.wait(lock, [this]()
                      { return !m_queue.empty(); });

            // 取出连接
            ConnPtr conn = m_queue.front();
            m_queue.pop();
            lock.unlock();

            // 自动重连：保证连接有效性
            if (!conn->isConnected())
            {
                m_config_cb(*conn);
                conn->connect();
            }

            return conn->isConnected() ? conn : nullptr;
        }

        /**
         * @brief 超时获取连接（带默认超时时间）
         * @param timeout_ms 超时时间，默认3秒
         * @return 成功返回有效连接，超时返回nullptr
         * @note 超时时间可自定义，避免线程无限阻塞
         */
        ConnPtr getTimeout(TimeoutMs timeout_ms = TimeoutMs(3000))
        {
            std::unique_lock<std::mutex> lock(m_mtx);

            // 带超时等待空闲连接
            if (!m_cv.wait_for(lock, timeout_ms, [this]()
                               { return !m_queue.empty(); })) return nullptr;

            ConnPtr conn = m_queue.front();
            m_queue.pop();
            lock.unlock();

            // 自动重连
            if (!conn->isConnected())
            {
                m_config_cb(*conn);
                conn->connect();
            }

            return conn->isConnected() ? conn : nullptr;
        }

        /**
         * @brief 释放连接（归还到连接池）
         * @param conn 待归还的数据库连接
         * @note 归还前自动重置连接状态，唤醒一个等待线程
         */
        void release(ConnPtr conn)
        {
            if (!conn) return;

            std::lock_guard<std::mutex> lock(m_mtx);
            conn->reset();      // 重置连接状态
            m_queue.push(conn); // 归还至空闲队列
            m_cv.notify_one();  // 唤醒一个等待的工作线程
        }

        /**
         * @brief 销毁连接池，关闭所有数据库连接
         * @note 唤醒所有等待线程，避免销毁时阻塞卡死
         */
        void destroy()
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            while (!m_queue.empty())
            {
                m_queue.front()->disconnect();
                m_queue.pop();
            }
            m_cv.notify_all(); // 唤醒所有等待线程
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
            return m_max_conn;
        }
    };

} // namespace ol

#endif // OL_DATABASE_H