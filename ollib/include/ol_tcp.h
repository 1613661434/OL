/****************************************************************************************/
/*
 * 程序名：ol_tcp.h
 * 功能描述：TCP网络通信工具类，提供客户端和服务端的 socket 通信实现，支持以下特性：
 *          - 客户端类（ctcpclient）：负责与服务端建立连接、发送/接收数据
 *          - 服务端类（ctcpserver）：负责初始化监听、接收客户端连接、数据交互
 *          - 通用 TCP 读写函数：支持文本/二进制数据，带超时控制
 *          - 仅支持 Linux 平台（依赖 POSIX socket 接口）
 * 作者：ol
 * 适用标准：C++11及以上（需支持Linux系统调用）
 */
/****************************************************************************************/

#ifndef OL_TCP_H
#define OL_TCP_H 1

#include "ol_fstream.h"
#include <iostream>
#include <signal.h>
#include <string>

#ifdef __linux__
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/sem.h> // 定义 SEM_UNDO 常量和信号量相关函数
#include <sys/shm.h>
#include <sys/socket.h>
#endif // __linux__

namespace ol
{

    // socket通讯的函数和类
    // ===========================================================================
#ifdef __linux__
    // TCP客户端类，用于与服务端建立连接并进行数据通信
    class ctcpclient
    {
    private:
        int m_connfd;     // 客户端的socket描述符
        std::string m_ip; // 服务端的ip地址
        int m_port;       // 服务端通讯的端口
    public:
        // 构造函数，初始化成员变量
        ctcpclient() : m_connfd(-1), m_port(0)
        {
        }

        /**
         * @brief 向服务端发起连接请求
         * @param ip 服务端IP地址（如"127.0.0.1"）
         * @param port 服务端端口号（1-65535）
         * @return true-连接成功，false-连接失败
         */
        bool connect(const std::string& ip, const int port);

        /**
         * @brief 接收服务端发送的数据
         * @param buffer 存储接收数据的缓冲区（字符串或二进制指针）
         * @param ibuflen 计划接收的字节数（仅二进制版本需要）
         * @param itimeout 超时时间（秒）：-1-不等待，0-无限等待，>0-指定秒数
         * @return true-接收成功，false-失败（超时或连接不可用）
         */
        bool read(std::string& buffer, const int itimeout = 0);             // 文本数据版本
        bool read(void* buffer, const int ibuflen, const int itimeout = 0); // 二进制数据版本

        /**
         * @brief 向服务端发送数据
         * @param buffer 待发送数据（字符串或二进制指针）
         * @param ibuflen 待发送的字节数（仅二进制版本需要）
         * @return true-发送成功，false-失败（连接不可用）
         */
        bool write(const std::string& buffer);             // 文本数据版本
        bool write(const void* buffer, const int ibuflen); // 二进制数据版本

        // 断开与服务端的连接
        void close();

        // 析构函数，自动关闭socket释放资源
        ~ctcpclient();
    };

    // TCP服务端类，用于监听端口、接收客户端连接并进行数据通信
    class ctcpserver
    {
    private:
        int m_socklen;                   // 结构体struct sockaddr_in的大小
        struct sockaddr_in m_clientaddr; // 客户端的地址信息
        struct sockaddr_in m_servaddr;   // 服务端的地址信息
        int m_listenfd;                  // 监听socket描述符
        int m_connfd;                    // 客户端连接socket描述符
    public:
        // 构造函数，初始化成员变量
        ctcpserver() : m_listenfd(-1), m_connfd(-1)
        {
        }

        /**
         * @brief 初始化服务端（创建监听socket并绑定端口）
         * @param port 服务端监听端口（1-65535）
         * @param backlog 未完成连接队列的最大长度（默认5）
         * @return true-初始化成功，false-失败（端口被占用等）
         */
        bool initserver(const unsigned int port, const int backlog = 5);

        /**
         * @brief 接收客户端连接（从连接队列中获取）
         * @return true-获取连接成功，false-失败（可重试）
         * @note 若连接队列为空，将阻塞等待
         */
        bool accept();

        /**
         * @brief 获取当前连接的客户端IP地址
         * @return 客户端IP地址字符串（如"192.168.1.100"）
         */
        char* getip();

        /**
         * @brief 接收客户端发送的数据
         * @param buffer 存储接收数据的缓冲区（字符串或二进制指针）
         * @param ibuflen 计划接收的字节数（仅二进制版本需要）
         * @param itimeout 超时时间（秒）：-1-不等待，0-无限等待，>0-指定秒数
         * @return true-接收成功，false-失败（超时或连接不可用）
         */
        bool read(std::string& buffer, const int itimeout = 0);             // 文本数据版本
        bool read(void* buffer, const int ibuflen, const int itimeout = 0); // 二进制数据版本

        /**
         * @brief 向客户端发送数据
         * @param buffer 待发送数据（字符串或二进制指针）
         * @param ibuflen 待发送的字节数（仅二进制版本需要）
         * @return true-发送成功，false-失败（连接不可用）
         */
        bool write(const std::string& buffer);             // 文本数据版本
        bool write(const void* buffer, const int ibuflen); // 二进制数据版本

        /**
         * @brief 关闭监听socket（m_listenfd）
         * @note 常用于多进程服务的子进程中
         */
        void closelisten();

        /**
         * @brief 关闭客户端连接socket（m_connfd）
         * @note 常用于多进程服务的父进程中
         */
        void closeclient();

        // 析构函数，自动关闭socket释放资源
        ~ctcpserver();
    };

    /**
     * @brief 从TCP socket读取数据
     * @param sockfd 已连接的socket描述符
     * @param buffer 存储接收数据的缓冲区（字符串或二进制指针）
     * @param ibuflen 计划接收的字节数（仅二进制版本需要）
     * @param itimeout 超时时间（秒）
     * @return true-接收成功，false-失败（超时或连接不可用）
     */
    bool tcpread(const int sockfd, std::string& buffer, const int itimeout = 0);             // 文本数据版本
    bool tcpread(const int sockfd, void* buffer, const int ibuflen, const int itimeout = 0); // 二进制数据版本

    /**
     * @brief 向TCP socket发送数据
     * @param sockfd 已连接的socket描述符
     * @param buffer 待发送数据（字符串或二进制指针）
     * @param ibuflen 待发送的字节数（仅二进制版本需要）
     * @return true-发送成功，false-失败（连接不可用）
     */
    bool tcpwrite(const int sockfd, const std::string& buffer);             // 文本数据版本
    bool tcpwrite(const int sockfd, const void* buffer, const int ibuflen); // 二进制数据版本

    /**
     * @brief 从就绪的socket读取指定长度的二进制数据
     * @param sockfd 已就绪的socket描述符
     * @param buffer 存储数据的缓冲区
     * @param n 计划读取的字节数
     * @return true-成功读取n字节，false-连接不可用
     */
    bool readn(const int sockfd, char* buffer, const size_t n);

    /**
     * @brief 向就绪的socket写入指定长度的二进制数据
     * @param sockfd 已就绪的socket描述符
     * @param buffer 待写入的数据缓冲区
     * @param n 待写入的字节数
     * @return true-成功写入n字节，false-连接不可用
     */
    bool writen(const int sockfd, const char* buffer, const size_t n);
    // ===========================================================================
#endif // __linux__

} // namespace ol

#endif // !OL_TCP_H