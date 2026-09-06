#include "ol_tcp.h"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <climits>
#include <cstdint>
#include <cstring>
#include <limits>
#include <new>
#include <stdexcept>

#ifdef __unix__
#include <unistd.h>
#endif

namespace ol
{

#ifdef __unix__
    namespace
    {
        using SteadyClock = std::chrono::steady_clock;

        int remainingPollTimeout(int timeoutSeconds, const SteadyClock::time_point& deadline)
        {
            if (timeoutSeconds == 0) return -1;
            if (timeoutSeconds == -1) return 0;

            const auto now = SteadyClock::now();
            if (now >= deadline) return 0;

            auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count();
            if (milliseconds <= 0) milliseconds = 1;
            return static_cast<int>(std::min<long long>(milliseconds, INT_MAX));
        }

        bool waitForSocket(int sockfd, short events, int timeoutSeconds,
                           const SteadyClock::time_point& deadline)
        {
            while (true)
            {
                if (timeoutSeconds > 0 && SteadyClock::now() >= deadline)
                {
                    errno = ETIMEDOUT;
                    return false;
                }

                pollfd fd{};
                fd.fd = sockfd;
                fd.events = events;

                const int result = ::poll(&fd, 1, remainingPollTimeout(timeoutSeconds, deadline));
                if (result > 0)
                {
                    if (fd.revents & (POLLERR | POLLNVAL)) return false;
                    if (fd.revents & (events | POLLHUP)) return true;
                    continue;
                }
                if (result == 0)
                {
                    errno = timeoutSeconds == -1 ? EAGAIN : ETIMEDOUT;
                    return false;
                }
                if (errno != EINTR) return false;
            }
        }

        bool readExactUntil(int sockfd, char* buffer, size_t size, int timeoutSeconds,
                            const SteadyClock::time_point& deadline)
        {
            if (sockfd < 0 || (buffer == nullptr && size != 0) || timeoutSeconds < -1)
            {
                errno = EINVAL;
                return false;
            }

            size_t received = 0;

            while (received < size)
            {
                if (!waitForSocket(sockfd, POLLIN, timeoutSeconds, deadline)) return false;

                const ssize_t count = ::recv(sockfd, buffer + received, size - received, 0);
                if (count > 0)
                {
                    received += static_cast<size_t>(count);
                    continue;
                }
                if (count == 0) return false;
                if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) continue;
                return false;
            }

            return true;
        }

        bool readExact(int sockfd, char* buffer, size_t size, int timeoutSeconds)
        {
            const auto deadline = timeoutSeconds > 0
                                      ? SteadyClock::now() + std::chrono::seconds(timeoutSeconds)
                                      : SteadyClock::time_point{};
            return readExactUntil(sockfd, buffer, size, timeoutSeconds, deadline);
        }
    } // namespace

    bool ctcpclient::connect(const std::string& ip, const int port)
    {
        // 如果已连接到服务端，则断开，这种处理方法没有特别的原因，不要纠结。
        if (m_connfd != -1)
        {
            ::close(m_connfd);
            m_connfd = -1;
        }

        if (port <= 0 || port > 65535) return false;

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
        if (m_listenfd >= 0)
        {
            ::close(m_listenfd);
            m_listenfd = -1;
        }

        if (port == 0 || port > 65535) return false;
        if ((m_listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) return false;

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

        socklen_t socklen = sizeof(struct sockaddr_in);
        if ((m_connfd = ::accept(m_listenfd, (struct sockaddr*)&m_clientaddr, &socklen)) < 0)
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

    bool ctcpserver::read(std::string& buffer, const int itimeout, size_t maxFrameSize) // 接收文本数据。
    {
        if (m_connfd == -1) return false;

        return tcpread(m_connfd, buffer, itimeout, maxFrameSize);
    }

    bool ctcpclient::read(void* buffer, const int ibuflen, const int itimeout) // 接收二进制数据。
    {
        if (m_connfd == -1) return false;

        return (tcpread(m_connfd, buffer, ibuflen, itimeout));
    }

    bool ctcpclient::read(std::string& buffer, const int itimeout, size_t maxFrameSize) // 接收文本数据。
    {
        if (m_connfd == -1) return false;

        return tcpread(m_connfd, buffer, itimeout, maxFrameSize);
    }

    bool ctcpserver::write(const void* buffer, const int ibuflen) // 发送二进制数据。
    {
        if (m_connfd == -1) return false;

        return (tcpwrite(m_connfd, (char*)buffer, ibuflen));
    }

    bool ctcpserver::write(const std::string& buffer, size_t maxFrameSize)
    {
        if (m_connfd == -1) return false;

        return tcpwrite(m_connfd, buffer, maxFrameSize);
    }

    bool ctcpclient::write(const void* buffer, const int ibuflen)
    {
        if (m_connfd == -1) return false;

        return (tcpwrite(m_connfd, (char*)buffer, ibuflen));
    }

    bool ctcpclient::write(const std::string& buffer, size_t maxFrameSize)
    {
        if (m_connfd == -1) return false;

        return tcpwrite(m_connfd, buffer, maxFrameSize);
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
        if (ibuflen < 0 || (buffer == nullptr && ibuflen != 0))
        {
            errno = EINVAL;
            return false;
        }

        return readExact(sockfd, static_cast<char*>(buffer), static_cast<size_t>(ibuflen), itimeout);
    }

    bool tcpread(const int sockfd, std::string& buffer, const int itimeout, size_t maxFrameSize) // 接收文本数据。
    {
        buffer.clear();
        if (maxFrameSize == 0 || maxFrameSize > std::numeric_limits<uint32_t>::max())
        {
            errno = EINVAL;
            return false;
        }

        if (itimeout < -1)
        {
            errno = EINVAL;
            return false;
        }

        const auto deadline = itimeout > 0
                                  ? SteadyClock::now() + std::chrono::seconds(itimeout)
                                  : SteadyClock::time_point{};
        uint32_t encodedLength = 0;
        if (!readExactUntil(sockfd, reinterpret_cast<char*>(&encodedLength), sizeof(encodedLength),
                            itimeout, deadline))
            return false;

        const size_t length = ntohl(encodedLength);
        if (length > maxFrameSize)
        {
            errno = EMSGSIZE;
            return false;
        }

        try
        {
            buffer.resize(length);
        }
        catch (const std::bad_alloc&)
        {
            errno = ENOMEM;
            return false;
        }
        catch (const std::length_error&)
        {
            errno = EMSGSIZE;
            return false;
        }
        if (length == 0) return true;
        if (readExactUntil(sockfd, buffer.data(), length, itimeout, deadline)) return true;

        buffer.clear();
        return false;
    }

    bool tcpwrite(const int sockfd, const void* buffer, const int ibuflen) // 发送二进制数据。
    {
        if (sockfd < 0 || ibuflen < 0 || (buffer == nullptr && ibuflen != 0))
        {
            errno = EINVAL;
            return false;
        }

        return writen(sockfd, static_cast<const char*>(buffer), static_cast<size_t>(ibuflen));
    }

    bool tcpwrite(const int sockfd, const std::string& buffer, size_t maxFrameSize) // 发送文本数据。
    {
        if (sockfd < 0 || maxFrameSize == 0 ||
            maxFrameSize > std::numeric_limits<uint32_t>::max())
        {
            errno = EINVAL;
            return false;
        }
        if (buffer.size() > maxFrameSize ||
            buffer.size() > std::numeric_limits<uint32_t>::max())
        {
            errno = EMSGSIZE;
            return false;
        }

        const uint32_t encodedLength = htonl(static_cast<uint32_t>(buffer.size()));
        if (!writen(sockfd, reinterpret_cast<const char*>(&encodedLength), sizeof(encodedLength)))
            return false;

        return buffer.empty() || writen(sockfd, buffer.data(), buffer.size());
    }

    // 从已经准备好的socket中读取数据。
    // sockfd：已经准备好的socket连接。
    // buffer：接收数据缓冲区的地址。
    // n：本次接收数据的字节数。
    // 返回值：成功接收到n字节的数据后返回true，socket连接不可用返回false。
    bool readn(const int sockfd, char* buffer, const size_t n)
    {
        return readExact(sockfd, buffer, n, 0);
    }

    // 向已经准备好的socket中写入数据。
    // sockfd：已经准备好的socket连接。
    // buffer：待发送数据缓冲区的地址。
    // n：待发送数据的字节数。
    // 返回值：成功发送完n字节的数据后返回true，socket连接不可用返回false。
    bool writen(const int sockfd, const char* buffer, const size_t n)
    {
        if (sockfd < 0 || (buffer == nullptr && n != 0))
        {
            errno = EINVAL;
            return false;
        }

        size_t written = 0;
        while (written < n)
        {
            const ssize_t count = ::send(sockfd, buffer + written, n - written, MSG_NOSIGNAL);
            if (count > 0)
            {
                written += static_cast<size_t>(count);
                continue;
            }
            if (count < 0 && errno == EINTR) continue;
            if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            {
                if (!waitForSocket(sockfd, POLLOUT, 0, SteadyClock::time_point{})) return false;
                continue;
            }
            return false;
        }

        return true;
    }
#endif // __unix__

} // namespace ol
