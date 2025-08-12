#include "../include/ol_tcp.h"

namespace ol
{

#ifdef __linux__
    bool ctcpclient::connect(const std::string& ip, const int port)
    {
        // 如果已连接到服务端，则断开，这种处理方法没有特别的原因，不要纠结。
        if (m_connfd != -1)
        {
            ::close(m_connfd);
            m_connfd = -1;
        }

        // 忽略SIGPIPE信号，防止程序异常退出。
        // 如果send到一个disconnected socket上，内核就会发出SIGPIPE信号。这个信号
        // 的缺省处理方法是终止进程，大多数时候这都不是我们期望的。我们重新定义这
        // 个信号的处理方法，大多数情况是直接屏蔽它。
        signal(SIGPIPE, SIG_IGN);

        m_ip = ip;
        m_port = port;

        struct hostent* h;
        struct sockaddr_in servaddr;

        if ((m_connfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) return false;

        if (!(h = gethostbyname(m_ip.c_str())))
        {
            ::close(m_connfd);
            m_connfd = -1;
            return false;
        }

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(m_port); // 指定服务端的通讯端口
        memcpy(&servaddr.sin_addr, h->h_addr, h->h_length);

        if (::connect(m_connfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0)
        {
            ::close(m_connfd);
            m_connfd = -1;
            return false;
        }

        return true;
    }

    void ctcpclient::close()
    {
        if (m_connfd >= 0) ::close(m_connfd);

        m_connfd = -1;
        m_port = 0;
    }

    ctcpclient::~ctcpclient()
    {
        close();
    }

    bool ctcpserver::initserver(const unsigned int port, const int backlog)
    {
        // 如果服务端的socket>0，关掉它，这种处理方法没有特别的原因，不要纠结。
        if (m_listenfd > 0)
        {
            ::close(m_listenfd);
            m_listenfd = -1;
        }

        if ((m_listenfd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) return false;

        // 忽略SIGPIPE信号，防止程序异常退出。
        // 如果往已关闭的socket继续写数据，会产生SIGPIPE信号，它的缺省行为是终止程序，所以要忽略它。
        signal(SIGPIPE, SIG_IGN);

        // 打开SO_REUSEADDR选项，当服务端连接处于TIME_WAIT状态时可以再次启动服务器，
        // 否则bind()可能会不成功，报：Address already in use。
        int opt = 1;
        setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        memset(&m_servaddr, 0, sizeof(m_servaddr));
        m_servaddr.sin_family = AF_INET;
        m_servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 任意ip地址。
        m_servaddr.sin_port = htons(port);
        if (bind(m_listenfd, (struct sockaddr*)&m_servaddr, sizeof(m_servaddr)) != 0)
        {
            closelisten();
            return false;
        }

        if (listen(m_listenfd, backlog) != 0)
        {
            closelisten();
            return false;
        }

        return true;
    }

    bool ctcpserver::accept()
    {
        if (m_listenfd == -1) return false;

        int m_socklen = sizeof(struct sockaddr_in);
        if ((m_connfd = ::accept(m_listenfd, (struct sockaddr*)&m_clientaddr, (socklen_t*)&m_socklen)) < 0)
            return false;

        return true;
    }

    char* ctcpserver::getip()
    {
        return (inet_ntoa(m_clientaddr.sin_addr));
    }

    bool ctcpserver::read(void* buffer, const int ibuflen, const int itimeout) // 接收二进制数据。
    {
        if (m_connfd == -1) return false;

        return (tcpread(m_connfd, buffer, ibuflen, itimeout));
    }

    bool ctcpserver::read(std::string& buffer, const int itimeout) // 接收文本数据。
    {
        if (m_connfd == -1) return false;

        return (tcpread(m_connfd, buffer, itimeout));
    }

    bool ctcpclient::read(void* buffer, const int ibuflen, const int itimeout) // 接收二进制数据。
    {
        if (m_connfd == -1) return false;

        return (tcpread(m_connfd, buffer, ibuflen, itimeout));
    }

    bool ctcpclient::read(std::string& buffer, const int itimeout) // 接收文本数据。
    {
        if (m_connfd == -1) return false;

        return (tcpread(m_connfd, buffer, itimeout));
    }

    bool ctcpserver::write(const void* buffer, const int ibuflen) // 发送二进制数据。
    {
        if (m_connfd == -1) return false;

        return (tcpwrite(m_connfd, (char*)buffer, ibuflen));
    }

    bool ctcpserver::write(const std::string& buffer)
    {
        if (m_connfd == -1) return false;

        return (tcpwrite(m_connfd, buffer));
    }

    bool ctcpclient::write(const void* buffer, const int ibuflen)
    {
        if (m_connfd == -1) return false;

        return (tcpwrite(m_connfd, (char*)buffer, ibuflen));
    }

    bool ctcpclient::write(const std::string& buffer)
    {
        if (m_connfd == -1) return false;

        return (tcpwrite(m_connfd, buffer));
    }

    void ctcpserver::closelisten()
    {
        if (m_listenfd >= 0)
        {
            ::close(m_listenfd);
            m_listenfd = -1;
        }
    }

    void ctcpserver::closeclient()
    {
        if (m_connfd >= 0)
        {
            ::close(m_connfd);
            m_connfd = -1;
        }
    }

    ctcpserver::~ctcpserver()
    {
        closelisten();
        closeclient();
    }

    bool tcpread(const int sockfd, void* buffer, const int ibuflen, const int itimeout) // 接收二进制数据。
    {
        if (sockfd == -1) return false;

        // 如果itimeout>0，表示需要等待itimeout秒，如果itimeout秒后还没有数据到达，返回false。
        if (itimeout > 0)
        {
            struct pollfd fds;
            fds.fd = sockfd;
            fds.events = POLLIN;
            if (poll(&fds, 1, itimeout * 1000) <= 0) return false;
        }

        // 如果itimeout==-1，表示不等待，立即判断socket的缓冲区中是否有数据，如果没有，返回false。
        if (itimeout == -1)
        {
            struct pollfd fds;
            fds.fd = sockfd;
            fds.events = POLLIN;
            if (poll(&fds, 1, 0) <= 0) return false;
        }

        // 读取报文内容。
        if (readn(sockfd, (char*)buffer, ibuflen) == false) return false;

        return true;
    }

    bool tcpread(const int sockfd, std::string& buffer, const int itimeout) // 接收文本数据。
    {
        if (sockfd == -1) return false;

        // 如果itimeout>0，表示等待itimeout秒，如果itimeout秒后接收缓冲区中还没有数据，返回false。
        if (itimeout > 0)
        {
            struct pollfd fds;
            fds.fd = sockfd;
            fds.events = POLLIN;
            if (poll(&fds, 1, itimeout * 1000) <= 0) return false;
        }

        // 如果itimeout==-1，表示不等待，立即判断socket的接收缓冲区中是否有数据，如果没有，返回false。
        if (itimeout == -1)
        {
            struct pollfd fds;
            fds.fd = sockfd;
            fds.events = POLLIN;
            if (poll(&fds, 1, 0) <= 0) return false;
        }

        int buflen = 0;

        // 先读取报文长度，4个字节。
        if (readn(sockfd, (char*)&buflen, 4) == false) return false;

        buffer.resize(buflen); // 设置buffer的大小。

        // 再读取报文内容。
        if (readn(sockfd, &buffer[0], buflen) == false) return false;

        return true;
    }

    bool tcpwrite(const int sockfd, const void* buffer, const int ibuflen) // 发送二进制数据。
    {
        if (sockfd == -1) return false;

        if (writen(sockfd, (char*)buffer, ibuflen) == false) return false;

        return true;
    }

    bool tcpwrite(const int sockfd, const std::string& buffer) // 发送文本数据。
    {
        if (sockfd == -1) return false;

        int buflen = buffer.size();

        // 先发送报头。
        if (writen(sockfd, (char*)&buflen, 4) == false) return false;

        // 再发送报文体。
        if (writen(sockfd, buffer.c_str(), buflen) == false) return false;

        return true;
    }

    // 从已经准备好的socket中读取数据。
    // sockfd：已经准备好的socket连接。
    // buffer：接收数据缓冲区的地址。
    // n：本次接收数据的字节数。
    // 返回值：成功接收到n字节的数据后返回true，socket连接不可用返回false。
    bool readn(const int sockfd, char* buffer, const size_t n)
    {
        int nleft = n; // 剩余需要读取的字节数。
        int idx = 0;   // 已成功读取的字节数。
        int nread;     // 每次调用recv()函数读到的字节数。

        while (nleft > 0)
        {
            if ((nread = recv(sockfd, buffer + idx, nleft, 0)) <= 0) return false;

            idx = idx + nread;
            nleft = nleft - nread;
        }

        return true;
    }

    // 向已经准备好的socket中写入数据。
    // sockfd：已经准备好的socket连接。
    // buffer：待发送数据缓冲区的地址。
    // n：待发送数据的字节数。
    // 返回值：成功发送完n字节的数据后返回true，socket连接不可用返回false。
    bool writen(const int sockfd, const char* buffer, const size_t n)
    {
        int nleft = n; // 剩余需要写入的字节数。
        int idx = 0;   // 已成功写入的字节数。
        int nwritten;  // 每次调用send()函数写入的字节数。

        while (nleft > 0)
        {
            if ((nwritten = send(sockfd, buffer + idx, nleft, 0)) <= 0) return false;

            nleft = nleft - nwritten;
            idx = idx + nwritten;
        }

        return true;
    }
#endif // __linux__

} // namespace ol