#include "ol_inetAddr.h"

namespace ol
{

#ifdef __linux__
    // 默认构造函数：初始化空地址
    inetAddr::inetAddr()
    {
        std::memset(&m_addr, 0, sizeof(m_addr));
        m_addrLen = sizeof(m_addr);
    }

    // 从IP字符串和端口构造（自动识别IPv4/IPv6）
    inetAddr::inetAddr(const std::string& ip, uint16_t port)
    {
        std::memset(&m_addr, 0, sizeof(m_addr));
        m_addrLen = sizeof(m_addr);

        // 尝试解析为IPv4
        if (inet_pton(AF_INET, ip.c_str(), &(reinterpret_cast<sockaddr_in*>(&m_addr)->sin_addr)) == 1)
        {
            m_addr.ss_family = AF_INET;
            reinterpret_cast<sockaddr_in*>(&m_addr)->sin_port = htons(port);
            m_addrLen = sizeof(sockaddr_in);
            return;
        }

        // 尝试解析为IPv6
        if (inet_pton(AF_INET6, ip.c_str(), &(reinterpret_cast<sockaddr_in6*>(&m_addr)->sin6_addr)) == 1)
        {
            m_addr.ss_family = AF_INET6;
            reinterpret_cast<sockaddr_in6*>(&m_addr)->sin6_port = htons(port);
            m_addrLen = sizeof(sockaddr_in6);
            return;
        }

        // 解析失败，抛出异常
        throw std::invalid_argument("Invalid IP address: " + ip);
    }

    // 仅从端口构造（绑定到所有接口）
    inetAddr::inetAddr(uint16_t port, bool isIpv6)
    {
        std::memset(&m_addr, 0, sizeof(m_addr));
        if (isIpv6)
        {
            m_addr.ss_family = AF_INET6;
            auto* addr6 = reinterpret_cast<sockaddr_in6*>(&m_addr);
            addr6->sin6_addr = in6addr_any; // IPv6的"所有接口"地址
            addr6->sin6_port = htons(port);
            m_addrLen = sizeof(sockaddr_in6);
        }
        else
        {
            m_addr.ss_family = AF_INET;
            auto* addr4 = reinterpret_cast<sockaddr_in*>(&m_addr);
            addr4->sin_addr.s_addr = INADDR_ANY; // IPv4的"所有接口"地址
            addr4->sin_port = htons(port);
            m_addrLen = sizeof(sockaddr_in);
        }
    }

    // 从原生sockaddr构造
    inetAddr::inetAddr(const sockaddr* addr, socklen_t addrLen)
    {
        std::memset(&m_addr, 0, sizeof(m_addr));
        if (addrLen > sizeof(m_addr))
        {
            throw std::invalid_argument("Address length too large");
        }
        std::memcpy(&m_addr, addr, addrLen);
        m_addrLen = addrLen;
    }

    // 复制构造函数
    inetAddr::inetAddr(const inetAddr& other)
    {
        std::memcpy(&m_addr, &other.m_addr, sizeof(m_addr));
        m_addrLen = other.m_addrLen;
        std::memcpy(m_ipBuf, other.m_ipBuf, sizeof(m_ipBuf));
    }

    // 赋值运算符
    inetAddr& inetAddr::operator=(const inetAddr& other)
    {
        if (this != &other)
        {
            std::memcpy(&m_addr, &other.m_addr, sizeof(m_addr));
            m_addrLen = other.m_addrLen;
            std::memcpy(m_ipBuf, other.m_ipBuf, sizeof(m_ipBuf));
        }
        return *this;
    }

    // 获取IP地址字符串（线程安全）
    const char* inetAddr::ip() const
    {
        const void* addrPtr = nullptr;
        if (isIpv4())
        {
            addrPtr = &(reinterpret_cast<const sockaddr_in*>(&m_addr)->sin_addr);
        }
        else if (isIpv6())
        {
            addrPtr = &(reinterpret_cast<const sockaddr_in6*>(&m_addr)->sin6_addr);
        }
        else
        {
            throw std::runtime_error("Unsupported address family");
        }

        if (inet_ntop(m_addr.ss_family, addrPtr, m_ipBuf, sizeof(m_ipBuf)) == nullptr)
        {
            throw std::runtime_error("Failed to convert address to string");
        }
        return m_ipBuf;
    }

    // 获取端口号（主机字节序）
    uint16_t inetAddr::port() const
    {
        if (isIpv4())
        {
            return ntohs(reinterpret_cast<const sockaddr_in*>(&m_addr)->sin_port);
        }
        else if (isIpv6())
        {
            return ntohs(reinterpret_cast<const sockaddr_in6*>(&m_addr)->sin6_port);
        }
        throw std::runtime_error("Unsupported address family");
    }

    // 生成"IP:端口"格式的字符串
    std::string inetAddr::toString() const
    {
        std::string res;
        if (isIpv6())
        {
            res += "["; // IPv6地址需用[]包裹，避免与端口混淆
        }
        res += ip();
        if (isIpv6())
        {
            res += "]";
        }
        res += ":" + std::to_string(port());
        return res;
    }
#endif // __linux__

} // namespace ol