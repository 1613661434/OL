#include "ol_net/ol_InetAddr.h"

namespace ol
{

#ifdef __unix__
    // 默认构造函数：初始化空地址（默认IPv4）
    InetAddr::InetAddr()
        : m_family(AF_INET), m_addrLen(sizeof(sockaddr_in))
    {
        std::memset(&m_addr, 0, sizeof(m_addr));  // 初始化地址联合体
        std::memset(m_ipBuf, 0, sizeof(m_ipBuf)); // 初始缓存失效
    }

    // 从IP字符串和端口构造（自动识别IPv4/IPv6）
    InetAddr::InetAddr(const std::string& ip, uint16_t port)
        : m_family(AF_UNSPEC), m_addrLen(0)
    {
        std::memset(&m_addr, 0, sizeof(m_addr));  // 初始化地址联合体
        std::memset(m_ipBuf, 0, sizeof(m_ipBuf)); // 初始缓存失效

        // 尝试解析为IPv4
        if (inet_pton(AF_INET, ip.c_str(), &m_addr.ipv4.sin_addr) == 1)
        {
            m_family = AF_INET;
            m_addr.ipv4.sin_family = AF_INET;
            m_addr.ipv4.sin_port = htons(port);
            m_addrLen = sizeof(sockaddr_in);
            return;
        }

        // 尝试解析为IPv6
        if (inet_pton(AF_INET6, ip.c_str(), &m_addr.ipv6.sin6_addr) == 1)
        {
            m_family = AF_INET6;
            m_addr.ipv6.sin6_family = AF_INET6;
            m_addr.ipv6.sin6_port = htons(port);
            m_addrLen = sizeof(sockaddr_in6);
            return;
        }

        // 解析失败，抛出异常
        throw std::invalid_argument("Invalid IP address: " + ip);
    }

    // 仅从端口构造（绑定到所有接口）
    InetAddr::InetAddr(uint16_t port, bool isIpv6)
        : m_family(isIpv6 ? AF_INET6 : AF_INET)
    {
        std::memset(&m_addr, 0, sizeof(m_addr));  // 初始化地址联合体
        std::memset(m_ipBuf, 0, sizeof(m_ipBuf)); // 初始缓存失效

        if (isIpv6)
        {
            m_addr.ipv6.sin6_family = AF_INET6;
            m_addr.ipv6.sin6_addr = in6addr_any; // IPv6的"所有接口"地址
            m_addr.ipv6.sin6_port = htons(port);
            m_addrLen = sizeof(sockaddr_in6);
        }
        else
        {
            m_addr.ipv4.sin_family = AF_INET;
            m_addr.ipv4.sin_addr.s_addr = INADDR_ANY; // IPv4的"所有接口"地址
            m_addr.ipv4.sin_port = htons(port);
            m_addrLen = sizeof(sockaddr_in);
        }
    }

    // 从原生sockaddr构造
    InetAddr::InetAddr(const sockaddr* addr, socklen_t addrLen)
        : m_family(addr->sa_family), m_addrLen(addrLen)
    {
        std::memset(&m_addr, 0, sizeof(m_addr));  // 初始化地址联合体
        std::memset(m_ipBuf, 0, sizeof(m_ipBuf)); // 初始缓存失效

        if (addr == nullptr || addrLen == 0)
        {
            throw std::invalid_argument("Null address or zero length");
        }

        // 根据地址族复制对应类型的地址
        switch (m_family)
        {
        case AF_INET:
            if (addrLen < sizeof(sockaddr_in))
            {
                throw std::invalid_argument("IPv4 address length too small");
            }
            std::memcpy(&m_addr.ipv4, addr, sizeof(sockaddr_in));
            m_addrLen = sizeof(sockaddr_in);
            break;
        case AF_INET6:
            if (addrLen < sizeof(sockaddr_in6))
            {
                throw std::invalid_argument("IPv6 address length too small");
            }
            std::memcpy(&m_addr.ipv6, addr, sizeof(sockaddr_in6));
            m_addrLen = sizeof(sockaddr_in6);
            break;
        default:
            throw std::invalid_argument("Unsupported address family");
        }
    }

    // 复制构造函数
    InetAddr::InetAddr(const InetAddr& other)
        : m_family(other.m_family), m_addrLen(other.m_addrLen)
    {
        std::memcpy(&m_addr, &other.m_addr, sizeof(m_addr));  // 复制地址联合体
        std::memcpy(m_ipBuf, other.m_ipBuf, sizeof(m_ipBuf)); // 复制IP缓存
    }

    // 赋值运算符
    InetAddr& InetAddr::operator=(const InetAddr& other)
    {
        if (this != &other)
        {
            m_family = other.m_family;
            m_addrLen = other.m_addrLen;
            std::memcpy(&m_addr, &other.m_addr, sizeof(m_addr));  // 复制地址联合体
            std::memcpy(m_ipBuf, other.m_ipBuf, sizeof(m_ipBuf)); // 复制IP缓存
        }
        return *this;
    }

    // 获取IP地址字符串（线程安全）
    const char* InetAddr::getIp() const
    {
        // 缓存有效：直接返回
        if (!isIpBufZero())
        {
            return m_ipBuf;
        }

        // 缓存失效：根据地址族获取地址指针
        const void* addrPtr = nullptr;
        switch (m_family)
        {
        case AF_INET:
            addrPtr = &m_addr.ipv4.sin_addr;
            break;
        case AF_INET6:
            addrPtr = &m_addr.ipv6.sin6_addr;
            break;
        default:
            throw std::runtime_error("Unsupported address family: " + std::to_string(m_family));
        }

        // 转换为字符串并更新缓存
        if (inet_ntop(m_family, addrPtr, m_ipBuf, sizeof(m_ipBuf)) == nullptr)
        {
            throw std::runtime_error("Failed to convert address to string");
        }
        return m_ipBuf;
    }

    // 获取端口号（主机字节序）
    uint16_t InetAddr::getPort() const
    {
        switch (m_family)
        {
        case AF_INET:
            return ntohs(m_addr.ipv4.sin_port);
        case AF_INET6:
            return ntohs(m_addr.ipv6.sin6_port);
        default:
            throw std::runtime_error("Unsupported address family: " + std::to_string(m_family));
        }
    }

    // 生成"IP:端口"格式的字符串
    std::string InetAddr::getAddrStr() const
    {
        std::string res;
        if (isIpv6())
        {
            res += "["; // IPv6地址需用[]包裹，避免与端口混淆
        }
        res += getIp();
        if (isIpv6())
        {
            res += "]";
        }
        res += ":" + std::to_string(getPort());
        return res;
    }

    /**
     * 修改IP地址（保持端口和地址族不变）
     */
    void InetAddr::setIp(const std::string& ip)
    {
        std::memset(m_ipBuf, 0, sizeof(m_ipBuf)); // 缓存失效

        // 根据当前地址族解析新IP
        switch (m_family)
        {
        case AF_INET:
            if (inet_pton(AF_INET, ip.c_str(), &m_addr.ipv4.sin_addr) != 1)
            {
                throw std::invalid_argument("Invalid IPv4 address: " + ip);
            }
            break;
        case AF_INET6:
            if (inet_pton(AF_INET6, ip.c_str(), &m_addr.ipv6.sin6_addr) != 1)
            {
                throw std::invalid_argument("Invalid IPv6 address: " + ip);
            }
            break;
        default:
            throw std::runtime_error("Unsupported address family for setIp");
        }
    }

    /**
     * 修改端口号（保持IP和地址族不变）
     */
    void InetAddr::setPort(uint16_t port)
    {
        uint16_t netPort = htons(port);
        switch (m_family)
        {
        case AF_INET:
            m_addr.ipv4.sin_port = netPort;
            break;
        case AF_INET6:
            m_addr.ipv6.sin6_port = netPort;
            break;
        default:
            throw std::runtime_error("Unsupported address family for setPort");
        }
    }

    /**
     * 同时修改IP和端口（可能改变地址族）
     */
    void InetAddr::setAddr(const std::string& ip, uint16_t port)
    {
        // 复用构造函数逻辑，通过赋值运算符更新当前对象
        *this = InetAddr(ip, port);
    }

    /**
     * 从原生sockaddr修改地址（更新地址族和长度）
     */
    void InetAddr::setAddr(const sockaddr* addr, socklen_t addrLen)
    {
        std::memset(&m_addr, 0, sizeof(m_addr));  // 清空地址联合体
        std::memset(m_ipBuf, 0, sizeof(m_ipBuf)); // 缓存失效

        if (addr == nullptr || addrLen == 0)
        {
            throw std::invalid_argument("Null address or zero length");
        }

        m_family = addr->sa_family;
        // 根据地址族复制对应类型的地址
        switch (m_family)
        {
        case AF_INET:
            if (addrLen < sizeof(sockaddr_in))
            {
                throw std::invalid_argument("IPv4 address length too small");
            }
            std::memcpy(&m_addr.ipv4, addr, sizeof(sockaddr_in));
            m_addrLen = sizeof(sockaddr_in);
            break;
        case AF_INET6:
            if (addrLen < sizeof(sockaddr_in6))
            {
                throw std::invalid_argument("IPv6 address length too small");
            }
            std::memcpy(&m_addr.ipv6, addr, sizeof(sockaddr_in6));
            m_addrLen = sizeof(sockaddr_in6);
            break;
        default:
            throw std::invalid_argument("Unsupported address family");
        }
    }
#endif // __unix__

} // namespace ol