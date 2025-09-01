/****************************************************************************************/
/*
 * 程序名：ol_inetAddr.h
 * 功能描述：套接字地址封装类，提供网络地址的统一管理，支持以下特性：
 *          - 兼容IPv4（sockaddr_in）和IPv6（sockaddr_in6）地址存储
 *          - 线程安全的IP地址与端口转换（避免静态缓冲区竞争）
 *          - 提供原生套接字地址访问接口，便于系统调用（bind/connect等）
 *          - 支持地址类型判断（IPv4/IPv6）和格式化输出（IP:端口）
 * 作者：ol
 * 适用标准：C++11及以上（需支持异常处理、reinterpret_cast等特性）
 */
/****************************************************************************************/

#ifndef __OL_INETADDR_H
#define __OL_INETADDR_H 1

#include <cstring>
#include <stdexcept>
#include <string>

#ifdef __linux__
#include <netinet/in.h>
#include <arpa/inet.h>
#endif // __linux__

namespace ol
{

#ifdef __linux__
    class inetAddr
    {
    private:
        /**
         * @brief 通用地址结构体，兼容IPv4和IPv6
         * @note sockaddr_storage是系统定义的通用地址类型，可容纳任何套接字地址
         */
        sockaddr_storage m_addr;

        /**
         * @brief 地址长度（区分IPv4和IPv6）
         * @note IPv4为sizeof(sockaddr_in)，IPv6为sizeof(sockaddr_in6)
         */
        socklen_t m_addrLen;

        /**
         * @brief IP地址字符串缓存缓冲区（线程安全）
         * @note 用mutable修饰，允许const成员函数修改；大小为INET6_ADDRSTRLEN（IPv6最大长度）
         */
        mutable char m_ipBuf[INET6_ADDRSTRLEN];

    public:
        /**
         * @brief 默认构造函数：初始化空地址
         */
        inetAddr();

        /**
         * @brief 从IP字符串和端口构造（自动识别IPv4/IPv6）
         * @param ip 字符串格式的IP地址（如"192.168.1.1"或"::1"）
         * @param port 主机字节序的端口号（如80、8080）
         * @throw std::invalid_argument 当IP地址格式无效时抛出
         */
        inetAddr(const std::string& ip, uint16_t port);

        /**
         * @brief 仅从端口构造（绑定到所有接口）
         * @param port 主机字节序的端口号
         * @param isIpv6 是否使用IPv6（默认false，即IPv4）
         * @note IPv4绑定到INADDR_ANY（0.0.0.0），IPv6绑定到in6addr_any（::）
         */
        explicit inetAddr(uint16_t port, bool isIpv6 = false);

        /**
         * @brief 从原生sockaddr构造（用于accept等场景）
         * @param addr 原生套接字地址指针
         * @param addrLen 地址长度
         * @throw std::invalid_argument 当地址长度超出范围时抛出
         */
        inetAddr(const sockaddr* addr, socklen_t addrLen);

        /**
         * @brief 复制构造函数
         * @param other 待复制的inetAddr对象
         */
        inetAddr(const inetAddr& other);

        /**
         * @brief 赋值运算符
         * @param other 待赋值的inetAddr对象
         * @return 自身引用
         */
        inetAddr& operator=(const inetAddr& other);

        /**
         * @brief 析构函数（默认即可，无动态资源）
         */
        ~inetAddr() = default;

        /**
         * @brief 获取IP地址字符串（线程安全）
         * @return 指向IP字符串的指针（如"192.168.1.1"或"::1"）
         * @throw std::runtime_error 当地址转换失败时抛出
         */
        const char* ip() const;

        /**
         * @brief 获取端口号（主机字节序）
         * @return 端口号（如80、8080）
         * @throw std::runtime_error 当地址族不支持时抛出
         */
        uint16_t port() const;

        /**
         * @brief 获取原生sockaddr指针（用于系统调用）
         * @return 指向sockaddr的const指针
         */
        const sockaddr* addr() const
        {
            return reinterpret_cast<const sockaddr*>(&m_addr);
        }

        /**
         * @brief 获取地址长度（用于系统调用）
         * @return 地址长度（socklen_t类型）
         */
        socklen_t addrLen() const
        {
            return m_addrLen;
        }

        /**
         * @brief 判断是否为IPv4地址
         * @return 是IPv4则返回true，否则返回false
         */
        bool isIpv4() const
        {
            return m_addr.ss_family == AF_INET;
        }

        /**
         * @brief 判断是否为IPv6地址
         * @return 是IPv6则返回true，否则返回false
         */
        bool isIpv6() const
        {
            return m_addr.ss_family == AF_INET6;
        }

        /**
         * @brief 获取地址族（AF_INET/AF_INET6）
         * @return 地址族类型
         */
        sa_family_t family() const
        {
            return m_addr.ss_family;
        }

        /**
         * @brief 生成"IP:端口"格式的字符串
         * @return 格式化的地址字符串（如"192.168.1.1:8080"或"[::1]:80"）
         */
        std::string toString() const;
    };
#endif // __linux__

} // namespace ol

#endif // !__OL_INETADDR_H