#ifndef OL_BANKSERVER_H
#define OL_BANKSERVER_H 1

#include "ol_ThreadPool.h"
#include "ol_net/ol_Connection.h"
#include "ol_net/ol_EventLoop.h"
#include "ol_net/ol_TcpServer.h"
#include "ol_string.h"

namespace ol
{

#ifdef __unix__
    // 用户/客户端的信息（状态机）。
    class UserInfo
    {
    public:
        using Ptr = std::shared_ptr<UserInfo>;

    private:
        int m_fd;             ///< 客户端的fd。
        std::string m_ip;     ///< 客户端的ip地址。
        bool m_login = false; ///< 客户端登录的状态：true-已登录；false-未登录。
        std::string m_name;   ///< 客户端的用户名。
    public:
        UserInfo(int fd, const std::string& ip) : m_fd(fd), m_ip(ip)
        {
        }
        void setLogin(bool login)
        {
            m_login = login;
        }
        bool login()
        {
            return m_login;
        }
    };

    /**
     * @brief   BankServer类：网上银行服务器
     */
    class BankServer
    {
    private:
        TcpServer m_tcpServ;                                    ///< TcpServer成员
        ThreadPool<false> m_threadPool;                         ///< 线程池成员
        std::mutex m_userInfoMutex;                             ///< m_userInfo_umap的互斥锁
        std::unordered_map<int, UserInfo::Ptr> m_userInfo_umap; ///< 用户的状态机

    public:
        BankServer(const std::string& ip, const uint16_t port, size_t workThreadNum = 2, size_t subThreadNum = 3, size_t MainMaxEvents = 100, size_t SubMaxEvents = 100, int epWaitTimeout = 10000, int timerTimetvl = 15, int timerTimeout = 30);
        ~BankServer();

        void start(int newConnTimeout = 10000); // 启动服务。
        void stop();                            // 停止服务。

        void handleNewConn(Connection::Ptr conn);                       // 处理新客户端连接请求，在TcpServer类中回调此函数。
        void handleClose(Connection::Ptr conn);                         // 关闭客户端的连接，在TcpServer类中回调此函数。
        void handleError(Connection::Ptr conn);                         // 客户端的连接错误，在TcpServer类中回调此函数。
        void handleMessage(Connection::Ptr conn, std::string& message); // 处理客户端的请求报文，在TcpServer类中回调此函数。
        void handleSendComplete(Connection::Ptr conn);                  // 数据发送完成后，在TcpServer类中回调此函数。
        void handleTimeOut(EventLoop* eventLoop);                       // epoll_wait()超时，在TcpServer类中回调此函数。
        void handleTimerTimeOut(int fd);                                // 客户端的连接超时，在TcpServer类中回调此函数。

        void onMessage(Connection::Ptr conn, std::string& message); // 处理客户端的请求报文，用于添加给线程池。
    };
#endif // __unix__

} // namespace ol

#endif //! OL_BANKSERVER_H