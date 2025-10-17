#ifndef OL_SOCKETFd_H
#define OL_SOCKETFd_H 1

#include "ol_net/ol_InetAddr.h"
#include <errno.h>
#include <memory>
#include <string.h>

#ifdef __linux__
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif // __linux__

namespace ol
{

#ifdef __linux__
    // 创建一个非阻塞的socketFd。
    int createFdNonblocking();

    class SocketFd
    {
    public:
        using Ptr = std::unique_ptr<SocketFd>;

    private:
        const int m_fd;  ///< SocketFd持有的fd，在构造函数中传进来。
        InetAddr m_addr; ///< 如果是服务端的fd，存放服务端监听的Addr。

    public:
        SocketFd(int in_fd); // 构造函数，传入一个已准备好的fd。
        ~SocketFd();         // 在析构函数中，将关闭m_fd。

        int getFd() const;         // 返回m_fd成员。
        const char* getIp() const; // 返回ip。
        uint16_t getPort() const;  // 返回port。

        void setAddr(const InetAddr& addr); // 设置地址（用于客户端连接的SocketFd）
        void setReuseaddr(bool on);         // 设置SO_REUSEADDR选项，true-打开，false-关闭。
        void setReuseport(bool on);         // 设置SO_REUSEPORT选项。
        void setTcpnodelay(bool on);        // 设置TCP_NODELAY选项。
        void setKeepalive(bool on);         // 设置SO_KEEPALIVE选项。

        void bind(const InetAddr& servAddr); // 服务端的SocketFd将调用此函数。
        void listen(int n = 128);            // 服务端的SocketFd将调用此函数。
        int accept(InetAddr& cliAddr);       // 服务端的SocketFd将调用此函数。
    };
#endif // __linux__

} // namespace ol

#endif // !OL_SOCKETFd_H