#ifndef OL_ECHOSERVER_H
#define OL_ECHOSERVER_H 1

#include "ol_ThreadPool.h"
#include "ol_net/ol_Connection.h"
#include "ol_net/ol_EventLoop.h"
#include "ol_net/ol_TcpServer.h"

namespace ol
{

#ifdef __unix__
    /**
     * @brief   EchoServer类：回显服务器
     */
    class EchoServer
    {
    private:
        TcpServer m_tcpServ;            ///< TcpServer成员
        ThreadPool<false> m_threadPool; ///< 线程池成员

    public:
        EchoServer(const std::string& ip, const uint16_t port, size_t workThreadNum = 2, size_t subThreadNum = 3, size_t MainMaxEvents = 100, size_t SubMaxEvents = 100, int epWaitTimeout = 10000, int timerTimetvl = 5, int timerTimeout = 10);
        ~EchoServer();

        void start(int newConnTimeout = 10000); // 启动服务。
        void stop();                            // 停止服务。

        void handleNewConn(Connection::Ptr conn);                       // 处理新客户端连接请求，在TcpServer类中回调此函数。
        void handleClose(Connection::Ptr conn);                         // 关闭客户端的连接，在TcpServer类中回调此函数。
        void handleError(Connection::Ptr conn);                         // 客户端的连接错误，在TcpServer类中回调此函数。
        void handleMessage(Connection::Ptr conn, std::string& message); // 处理客户端的请求报文，在TcpServer类中回调此函数。
        void handleSendComplete(Connection::Ptr conn);                  // 数据发送完成后，在TcpServer类中回调此函数。
        void handleTimeOut(EventLoop* eventLoop);                       // epoll_wait()超时，在TcpServer类中回调此函数。

        void onMessage(Connection::Ptr conn, std::string& message); // 处理客户端的请求报文，用于添加给线程池。
    };
#endif // __unix__

} // namespace ol

#endif //! OL_ECHOSERVER_H