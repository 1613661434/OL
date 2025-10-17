#ifndef OL_CONNECTION_H
#define OL_CONNECTION_H 1

#include "ol_TimeStamp.h"
#include "ol_net/ol_Buffer.h"
#include "ol_net/ol_Channel.h"
#include "ol_net/ol_EventLoop.h"
#include "ol_net/ol_InetAddr.h"
#include "ol_net/ol_SocketFd.h"
#include "ol_net/ol_net_fwd_decls.h"
#include <atomic>
#include <functional>
#include <memory>

#ifdef __linux__
#include <sys/syscall.h>
#endif // __linux__

namespace ol
{

#ifdef __linux__
    class Connection : public std::enable_shared_from_this<Connection>
    {
    public:
        using Ptr = std::shared_ptr<Connection>;

    private:
        EventLoop* m_eventLoop;          ///< Connection对应的事件循环，在构造函数中传入。
        SocketFdPtr m_cliFd;             ///< 与客户端通讯的Socket。
        ChannelPtr m_cliChnl;            ///< Connection对应的Channel，在构造函数中创建。
        Buffer m_inputBuf;               ///< 接收缓冲区
        Buffer m_outputBuf;              ///< 发送缓冲区
        std::atomic_bool m_disconnected; ///< 客户端连接是否已断开，如果已断开，则设置为true。
        TimeStamp m_lastATime;           ///< 时间戳，创建Connection对象时为当前时间，每接收到一个报文，把时间戳更新为当前时间。

        std::function<void(ConnectionPtr)> m_closeCb;                   ///< 关闭fd_的回调函数，将回调TcpServer::closeConnection()。
        std::function<void(ConnectionPtr)> m_errorCb;                   ///< fd_发生了错误的回调函数，将回调TcpServer::errorConnection()。
        std::function<void(ConnectionPtr, std::string&)> m_onMessageCb; ///< 处理报文的回调函数，将回调TcpServer::onMessage()。
        std::function<void(ConnectionPtr)> m_sendCompleteCb;            ///< 发送数据完成后的回调函数，将回调TcpServer::sendComplete()。
    public:
        Connection(EventLoop* eventLoop, SocketFdPtr cliFd);
        ~Connection();

        int getFd() const;         // 返回fd。
        const char* getIp() const; // 返回ip。
        uint16_t getPort() const;  // 返回port。

        void setCloseCb(std::function<void(ConnectionPtr)> func);                   // 设置关闭m_fd的回调函数。
        void setErrorCb(std::function<void(ConnectionPtr)> func);                   // 设置m_fd发生了错误的回调函数。
        void setOnMessageCb(std::function<void(ConnectionPtr, std::string&)> func); // 设置处理报文的回调函数。
        void setSendCompleteCb(std::function<void(ConnectionPtr)> func);            // 发送数据完成后的回调函数。

        void closeCb(); // TCP连接关闭（断开）的回调函数，供Channel回调。
        void errorCb(); // TCP连接错误的回调函数，供Channel回调。
        void writeCb(); // 处理写事件的回调函数，供Channel回调。

        void onMessage();                         // 处理对端发送过来的消息。
        void send(const char* data, size_t size); // 发送数据，不管在任何线程中，都是调用此函数发送数据。
    private:
        void _sendInLoop(const char* data, size_t size); // 发送数据，如果当前线程是IO线程，直接调用此函数，如果是工作线程，将把此函数传给IO线程去执行。

    public:
        bool timeout(time_t now, int val); // 判断TCP连接是否超时（空闲太久）。
    };
#endif // __linux__

} // namespace ol

#endif // !OL_CONNECTION_H