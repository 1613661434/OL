#include "ol_net/ol_SocketFd.h"

namespace ol
{

#ifdef __unix__
    // 创建一个非阻塞的socketFd。
    int createFdNonblocking()
    {
        // 创建fd。
        int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
        if (fd < 0)
        {
            fprintf(stderr, "%s:%s:%d listen socket create error:%d\n", __FILE__, __FUNCTION__, __LINE__, errno);
            exit(-1);
        }
        return fd;
    }

    SocketFd::SocketFd(int in_fd) : m_fd(in_fd) {};
    SocketFd::~SocketFd()
    {
        ::close(m_fd);
    }

    // 返回m_fd成员。
    int SocketFd::getFd() const
    {
        return m_fd;
    }

    // 返回ip。
    const char* SocketFd::getIp() const
    {
        return m_addr.getIp();
    }

    // 返回port。
    uint16_t SocketFd::getPort() const
    {
        return m_addr.getPort();
    }

    // 设置地址（用于客户端连接的SocketFd）
    void SocketFd::setAddr(const InetAddr& addr)
    {
        m_addr = addr;
    }

    void SocketFd::setTcpnodelay(bool on)
    {
        int optval = on ? 1 : 0;
        ::setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)); // TCP_NODELAY包含头文件 <netinet/tcp.h>
    }

    void SocketFd::setReuseaddr(bool on)
    {
        int optval = on ? 1 : 0;
        ::setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    }

    void SocketFd::setReuseport(bool on)
    {
        int optval = on ? 1 : 0;
        ::setsockopt(m_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    }

    void SocketFd::setKeepalive(bool on)
    {
        int optval = on ? 1 : 0;
        ::setsockopt(m_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
    }

    void SocketFd::bind(const InetAddr& servAddr)
    {
        if (::bind(m_fd, servAddr.getAddr(), servAddr.getAddrLen()) < 0)
        {
            perror("bind() failed.");
            close(m_fd);
            exit(-1);
        }
        m_addr = servAddr;
    }

    void SocketFd::listen(int n)
    {
        if (::listen(m_fd, n) < 0)
        {
            perror("listen() failed.");
            close(m_fd);
            exit(-1);
        }
    }

    int SocketFd::accept(InetAddr& cliAddr)
    {
        sockaddr_in peerAddr;
        socklen_t len = sizeof(peerAddr);
        int clifd = accept4(m_fd, (sockaddr*)&peerAddr, &len, SOCK_NONBLOCK);

        cliAddr.setAddr((sockaddr*)&peerAddr, sizeof(peerAddr)); // 客户端的地址和协议。

        return clifd;
    }
#endif // __unix__

} // namespace ol