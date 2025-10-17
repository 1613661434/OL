/****************************************************************************************/
/*
 * 程序名：ol_InetAddr.h
 * 功能描述：套接字地址封装类，提供网络地址的统一管理，支持以下特性：
 *          - 兼容IPv4（sockaddr_in）和IPv6（sockaddr_in6）地址存储
 *          - 线程安全的IP地址与端口转换（避免静态缓冲区竞争）
 *          - 提供原生套接字地址访问接口，便于系统调用（bind/connect等）
 *          - 支持地址类型判断（IPv4/IPv6）和格式化输出（IP:端口）
 * 作者：ol
 * 适用标准：C++11及以上（需支持异常处理等特性）
 */
/****************************************************************************************/

#ifndef OL_INETADDR_H
#define OL_INETADDR_H 1

#include <cstring>
#include <stdexcept>
#include <string>

#ifdef __linux__
#include <arpa/inet.h>
#include <netinet/in.h>
#endif // __linux__

namespace ol
{

#ifdef __linux__
    class InetAddr
    {
    private:
        union AddrUnion
        {
            sockaddr_in ipv4;                   // IPv4地址结构体（16字节）
            sockaddr_in6 ipv6;                  // IPv6地址结构体（28字节）
        } m_addr;                               ///< 联合体：存储IPv4/IPv6地址
        sa_family_t m_family;                   ///< 地址族（AF_INET/AF_INET6）
        socklen_t m_addrLen;                    ///< 地址长度（区分IPv4和IPv6）
        mutable char m_ipBuf[INET6_ADDRSTRLEN]; ///< IP地址字符串缓存缓冲区（线程安全）

        /**
         * @brief 检查m_ipBuf是否全为0（缓存是否失效）
         * @return 全0返回true（缓存失效），否则返回false（缓存有效）
         */
        inline bool isIpBufZero() const
        {
            static const char zeroBuf[INET6_ADDRSTRLEN] = {0}; // 全0参照缓冲区
            return std::memcmp(m_ipBuf, zeroBuf, INET6_ADDRSTRLEN) == 0;
        }

    public:
        /**
         * @brief 默认构造函数：初始化空地址（默认IPv4）
         */
        InetAddr();

        /**
         * @brief 从IP字符串和端口构造（自动识别IPv4/IPv6）
         * @param ip 字符串格式的IP地址（如"192.168.1.1"或"::1"）
         * @param port 主机字节序的端口号（如80、8080）
         * @throw std::invalid_argument 当IP地址格式无效时抛出
         */
        InetAddr(const std::string& ip, uint16_t port);

        /**
         * @brief 仅从端口构造（绑定到所有接口）
         * @param port 主机字节序的端口号
         * @param isIpv6 是否使用IPv6（默认false，即IPv4）
         * @note IPv4绑定到INADDR_ANY（0.0.0.0），IPv6绑定到in6addr_any（::）
         */
        explicit InetAddr(uint16_t port, bool isIpv6 = false);

        /**
         * @brief 从原生sockaddr构造（用于accept等场景）
         * @param addr 原生套接字地址指针
         * @param addrLen 地址长度
         * @throw std::invalid_argument 当地址长度超出范围时抛出
         */
        InetAddr(const sockaddr* addr, socklen_t addrLen);

        /**
         * @brief 复制构造函数
         * @param other 待复制的InetAddr对象
         */
        InetAddr(const InetAddr& other);

        /**
         * @brief 赋值运算符
         * @param other 待赋值的InetAddr对象
         * @return 自身引用
         */
        InetAddr& operator=(const InetAddr& other);

        /**
         * @brief 析构函数（默认即可，无动态资源）
         */
        ~InetAddr() = default;

        /**
         * @brief 获取IP地址字符串（线程安全）
         * @return 指向IP字符串的指针（如"192.168.1.1"或"::1"）
         * @throw std::runtime_error 当地址转换失败时抛出
         */
        const char* getIp() const;

        /**
         * @brief 获取端口号（主机字节序）
         * @return 端口号（如80、8080）
         * @throw std::runtime_error 当地址族不支持时抛出
         */
        uint16_t getPort() const;

        /**
         * @brief 生成"IP:端口"格式的字符串
         * @return 格式化的地址字符串（如"192.168.1.1:8080"或"[::1]:80"）
         */
        std::string getAddrStr() const;

        /**
         * @brief 获取原生sockaddr指针（用于系统调用）
         * @return 指向sockaddr的const指针
         */
        const sockaddr* getAddr() const
        {
            return reinterpret_cast<const sockaddr*>(&m_addr);
        }

        /**
         * @brief 获取地址长度（用于系统调用）
         * @return 地址长度（socklen_t类型）
         */
        socklen_t getAddrLen() const
        {
            return m_addrLen;
        }

        /**
         * @brief 获取地址族（AF_INET/AF_INET6）
         * @return 地址族类型
         */
        sa_family_t getFamily() const
        {
            return m_family;
        }

        /**
         * @brief 判断是否为IPv4地址
         * @return 是IPv4则返回true，否则返回false
         */
        bool isIpv4() const
        {
            return m_family == AF_INET;
        }

        /**
         * @brief 判断是否为IPv6地址
         * @return 是IPv6则返回true，否则返回false
         */
        bool isIpv6() const
        {
            return m_family == AF_INET6;
        }

        /**
         * @brief 修改IP地址（保持端口不变）
         * @param ip 新的IP地址字符串
         * @throw std::invalid_argument 无效IP或地址族不匹配时抛出
         */
        void setIp(const std::string& ip);

        /**
         * @brief 修改端口号（保持IP不变）
         * @param port 新的端口号（主机字节序）
         * @throw std::runtime_error 不支持的地址族时抛出
         */
        void setPort(uint16_t port);

        /**
         * @brief 同时修改IP和端口
         * @param ip 新的IP地址字符串
         * @param port 新的端口号（主机字节序）
         * @throw std::invalid_argument 无效IP时抛出
         */
        void setAddr(const std::string& ip, uint16_t port);

        /**
         * @brief 从原生sockaddr修改地址
         * @param addr 原生套接字地址指针
         * @param addrLen 地址长度
         * @throw std::invalid_argument 地址长度超限时抛出
         */
        void setAddr(const sockaddr* addr, socklen_t addrLen);
    };
#endif // __linux__

} // namespace ol

#endif // !OL_INETADDR_H