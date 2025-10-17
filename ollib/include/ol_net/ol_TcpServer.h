#ifndef OL_TCPSERVER_H
#define OL_TCPSERVER_H 1

#include "ol_ThreadPool.h"
#include "ol_net/ol_Acceptor.h"
#include "ol_net/ol_Channel.h"
#include "ol_net/ol_Connection.h"
#include "ol_net/ol_EpollChnl.h"
#include "ol_net/ol_EventLoop.h"
#include "ol_net/ol_SocketFd.h"
#include "ol_net/ol_net_fwd_decls.h"
#include <cassert>
#include <unordered_map>

namespace ol
{

#ifdef __linux__
    class TcpServer
    {
    private:
        EventLoopPtr m_mainEventLoop;                   ///< 主事件循环。
        std::vector<EventLoopPtr> m_subEventLoops;      ///< 存放从事件循环的容器。
        size_t m_threadNum;                             ///< 线程池的大小，即从事件循环的个数。
        ThreadPool<false> m_threadPool;                 ///< 线程池。
        Acceptor m_acceptor;                            ///< 一个TcpServer只有一个Acceptor对象。
        std::mutex m_connsMutex;                        ///< 保护m_conns的互斥锁。
        std::unordered_map<int, ConnectionPtr> m_conns; ///< 一个TcpServer有多个Connection对象，存放在unordered_map容器中。

        std::function<void(ConnectionPtr)> m_newConnCb;                         ///< 回调上层业务类的handleNewConn()。
        std::function<void(ConnectionPtr)> m_closeCb;                           ///< 回调上层业务类的handleClose()。
        std::function<void(ConnectionPtr)> m_errorCb;                           ///< 回调上层业务类的handleError()。
        std::function<void(ConnectionPtr, std::string& message)> m_onMessageCb; ///< 回调上层业务类的handleMessage()。
        std::function<void(ConnectionPtr)> m_sendCompleteCb;                    ///< 回调上层业务类的handleSendComplete()。
        std::function<void(EventLoop*)> m_timeoutCb;                            ///< 回调上层业务类的handleTimeOut()。
        std::function<void(int)> m_timerTimeoutCb;                              ///< 回调上层业务类的handleTimerTimeOut()。
    public:
        TcpServer(const std::string& ip, const uint16_t port, size_t threadNum = 3, size_t MainMaxEvents = 100, size_t SubMaxEvents = 100, int epWaitTimeout = 10000, int timerTimetvl = 30, int timerTimeout = 80);
        ~TcpServer();

        void start(int newConnTimeout = 10000); // 运行事件循环。
        void stop();                            // 停止IO线程和事件循环。

        void newConn(SocketFd::Ptr cliFd);                        // 处理新客户端连接请求。
        void closeConn(ConnectionPtr conn);                       // 关闭客户端的连接，在Connection类中回调此函数。
        void errorConn(ConnectionPtr conn);                       // 客户端的连接错误，在Connection类中回调此函数。
        void onMessage(ConnectionPtr conn, std::string& message); // 处理客户端的请求报文，在Connection类中回调此函数。
        void sendComplete(ConnectionPtr conn);                    // 数据发送完成后，在Connection类中回调此函数。
        void epollTimeout(EventLoop* eventLoop);                  // epoll_wait()超时，在EventLoop类中回调此函数。
        void removeConn(int fd);                                  // 删除m_conns中的Connection对象，在EventLoop::handleTimer()中将回调此函数。

        void setNewConnCb(std::function<void(ConnectionPtr)> func);
        void setCloseCb(std::function<void(ConnectionPtr)> func);
        void setErrorCb(std::function<void(ConnectionPtr)> func);
        void setOnMessageCb(std::function<void(ConnectionPtr, std::string& message)> func);
        void setSendCompleteCb(std::function<void(ConnectionPtr)> func);
        void setTimeoutCb(std::function<void(EventLoop*)> func);
        void setTimerTimeoutCb(std::function<void(int)> func);
    };
#endif // __linux__

} // namespace ol

#endif // !OL_TCPSERVER_H