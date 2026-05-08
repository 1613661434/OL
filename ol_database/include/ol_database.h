/****************************************************************************************/
/*
 * 程序名：ol_database.h
 * 功能描述：数据库连接抽象基类 IDBConn + 模板连接池 DBPool
 * 规范：所有数据库 DBConn 必须继承 IDBConn
 * 作者：ol
 */
/****************************************************************************************/
#ifndef OL_DATABASE_H
#define OL_DATABASE_H

#include "ol_type_traits.h"
#include <functional>
#include <stdexcept>
#include <queue>
#include <mutex>
#include <memory>
#include <type_traits>

namespace ol
{
    // 数据库连接抽象基类
    class IDBConn : public TypeNonCopyableMovable
    {
    public:
        virtual ~IDBConn() = default;

        virtual bool connect() = 0;
        virtual void disconnect() = 0;
        virtual bool isConnected() const = 0;
        virtual void reset() = 0;
    };

    // 模板数据库连接池
    template <typename T>
    class DBPool : public TypeNonCopyableMovable
    {
        static_assert(std::is_base_of<IDBConn, T>::value, "DBPool: DBConn Must inherit from IDBConn");

    public:
        using ConnPtr = std::shared_ptr<T>;
        using ConnConfigCallback = std::function<void(T& conn)>;

    private:
        std::queue<ConnPtr> m_queue;
        mutable std::mutex m_mtx;
        size_t m_max;
        ConnConfigCallback m_config_cb; // 配置回调

    public:
        explicit DBPool(size_t max_conn, ConnConfigCallback config_cb)
            : m_max(max_conn), m_config_cb(std::move(config_cb))
        {
            if (!m_config_cb)
            {
                throw std::invalid_argument("DBPool: The connection configuration callback (m_config_cb) cannot be empty!");
            }
            if (m_max == 0)
            {
                throw std::invalid_argument("DBPool: The maximum number of connections (m_max_conn) cannot be 0!");
            }

            for (size_t i = 0; i < m_max; ++i)
            {
                ConnPtr conn = std::make_shared<T>();
                m_config_cb(*conn);
                if (!conn->connect()) throw std::runtime_error("DBPool: Database connection initialization failed!");
                m_queue.push(conn);
            }
        }
        ~DBPool() { destroy(); }

        ConnPtr get()
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            if (m_queue.empty()) return nullptr;
            ConnPtr conn = m_queue.front();
            m_queue.pop();

            // 重连逻辑：先重新配置 → 再连接
            if (!conn->isConnected())
            {
                m_config_cb(*conn);
                conn->connect();
            }

            return conn->isConnected() ? conn : nullptr;
        }

        void release(ConnPtr conn)
        {
            if (!conn) return;
            std::lock_guard<std::mutex> lock(m_mtx);
            conn->reset();
            m_queue.push(conn);
        }

        void destroy()
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            while (!m_queue.empty())
            {
                m_queue.front()->disconnect();
                m_queue.pop();
            }
        }

        size_t idle() const
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            return m_queue.size();
        }
    };

} // namespace ol

#endif // OL_DATABASE_H