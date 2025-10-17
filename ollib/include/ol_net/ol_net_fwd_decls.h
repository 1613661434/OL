/****************************************************************************************/
/*
 * 程序名：ol_net_fwd_decls.h
 * 功能描述：OL网络库前置声明，避免头文件循环依赖。
 * 作者：ol
 * 适用标准：C++11及以上
 */
/****************************************************************************************/

#ifndef OL_NET_FWD_DECLS_H
#define OL_NET_FWD_DECLS_H 1

#include <memory>

namespace ol
{

#ifdef __linux__
    class Buffer;

    class InetAddr;

    class SocketFd;
    using SocketFdPtr = std::unique_ptr<SocketFd>;

    class EpollChnl;
    using EpollChnlPtr = std::unique_ptr<EpollChnl>;

    class Channel;
    using ChannelPtr = std::unique_ptr<Channel>;

    class Acceptor;

    class Connection;
    using ConnectionPtr = std::shared_ptr<Connection>;

    class EventLoop;
    using EventLoopPtr = std::unique_ptr<EventLoop>;

    class TcpServer;
#endif // __linux__

} // namespace ol

#endif // !OL_NET_FWD_DECLS_H