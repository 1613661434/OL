#ifndef OL_ACCEPTOR_H
#define OL_ACCEPTOR_H 1

#include "ol_net/ol_Channel.h"
#include "ol_net/ol_Connection.h"
#include "ol_net/ol_EventLoop.h"
#include "ol_net/ol_InetAddr.h"
#include "ol_net/ol_SocketFd.h"
#include "ol_net/ol_net_fwd_decls.h"
#include <functional>
#include <memory>

namespace ol
{

#ifdef __linux__
    class Acceptor
    {
    private:
        EventLoop* m_eventLoop;                       ///< Acceptor对应的事件循环，在构造函数中传入。
        SocketFd m_servFd;                            ///< 服务端用于监听的Socket，在构造函数中创建。
        Channel m_acceptChnl;                         ///< Acceptor对应的Channel，在构造函数中创建。
        std::function<void(SocketFdPtr)> m_newConnCb; ///< 处理新客户端连接请求的回调函数，将指向TcpServer::newConnection()

    public:
        Acceptor(EventLoop* eventLoop, const std::string& ip, const uint16_t port);
        ~Acceptor();

        void setNewConnCb(std::function<void(SocketFdPtr)> func); // 设置处理新客户端连接请求的回调函数，将在创建Acceptor对象的时候（TcpServer类的构造函数中）设置。
        void newConn();                                           // 处理新客户端连接请求。
    };
#endif // __linux__

} // namespace ol

#endif // !OL_ACCEPTOR_H