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
#include <queue>
#include <mutex>
#include <memory>
#include <type_traits>

namespace ol
{
    // 数据库连接抽象基类
    class IDBConn
    {
    public:
        virtual ~IDBConn() = default;

        virtual bool connect() = 0;
        virtual void disconnect() = 0;
        virtual bool isConnected() const = 0;
        virtual void reset() = 0;
        virtual void* getHandle() = 0;
    };

    // 模板数据库连接池
    template <typename T>
    class DBPool : public TypeNonCopyableMovable
    {
        static_assert(std::is_base_of<IDBConn, T>::value, "DBPool: DBConn Must inherit from IDBConn");

    public:
        using ConnPtr = std::shared_ptr<T>;

    private:
        std::queue<ConnPtr> m_queue;
        mutable std::mutex m_mtx;
        size_t m_max;

    public:
        explicit DBPool(size_t max_conn) : m_max(max_conn)
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            for (size_t i = 0; i < m_max; ++i)
            {
                ConnPtr conn = std::make_shared<T>();
                if (conn->connect()) m_queue.push(conn);
            }
        }
        ~DBPool() { destroy(); }

        ConnPtr get()
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            if (m_queue.empty()) return nullptr;
            ConnPtr conn = m_queue.front();
            m_queue.pop();
            if (!conn->isConnected() && !conn->connect()) return nullptr;
            return conn;
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