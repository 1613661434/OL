#ifndef __OL_TCP_H
#define __OL_TCP_H 1

#include "../include/ol_fstream.h"
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

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // ======================
    // socket通讯的函数和类
    // ======================
#ifdef __linux__
    // socket通讯的客户端类
    class ctcpclient
    {
    private:
        int m_connfd;     // 客户端的socket.
        std::string m_ip; // 服务端的ip地址。
        int m_port;       // 服务端通讯的端口。
    public:
        ctcpclient() : m_connfd(-1), m_port(0)
        {
        } // 构造函数。

        // 向服务端发起连接请求。
        // ip：服务端的ip地址。
        // port：服务端通讯的端口。
        // 返回值：true-成功；false-失败。
        bool connect(const std::string& ip, const int port);

        // 接收对端发送过来的数据。
        // buffer：存放接收数据缓冲区。
        // ibuflen: 打算接收数据的大小。
        // itimeout：等待数据的超时时间（秒）：-1-不等待；0-无限等待；>0-等待的秒数。
        // 返回值：true-成功；false-失败，失败有两种情况：1）等待超时；2）socket连接已不可用。
        bool read(std::string& buffer, const int itimeout = 0);             // 接收文本数据。
        bool read(void* buffer, const int ibuflen, const int itimeout = 0); // 接收二进制数据。

        // 向对端发送数据。
        // buffer：待发送数据缓冲区。
        // ibuflen：待发送数据的大小。
        // 返回值：true-成功；false-失败，如果失败，表示socket连接已不可用。
        bool write(const std::string& buffer);             // 发送文本数据。
        bool write(const void* buffer, const int ibuflen); // 发送二进制数据。

        // 断开与服务端的连接
        void close();

        ~ctcpclient(); // 析构函数自动关闭socket，释放资源。
    };

    // socket通讯的服务端类
    class ctcpserver
    {
    private:
        int m_socklen;                   // 结构体struct sockaddr_in的大小。
        struct sockaddr_in m_clientaddr; // 客户端的地址信息。
        struct sockaddr_in m_servaddr;   // 服务端的地址信息。
        int m_listenfd;                  // 服务端用于监听的socket。
        int m_connfd;                    // 客户端连接上来的socket。
    public:
        ctcpserver() : m_listenfd(-1), m_connfd(-1)
        {
        } // 构造函数。

        // 服务端初始化。
        // port：指定服务端用于监听的端口。
        // backlog：指定未完成连接队列的最大长度，默认为5。
        // 返回值：true-成功；false-失败，一般情况下，只要port设置正确，没有被占用，初始化都会成功。
        bool initserver(const unsigned int port, const int backlog = 5);

        // 从已连接队列中获取一个客户端连接，如果已连接队列为空，将阻塞等待。
        // 返回值：true-成功的获取了一个客户端连接，false-失败，如果accept失败，可以重新accept。
        bool accept();

        // 获取客户端的ip地址。
        // 返回值：客户端的ip地址，如"192.168.1.100"。
        char* getip();

        // 接收对端发送过来的数据。
        // buffer：存放接收数据的缓冲区。
        // ibuflen: 打算接收数据的大小。
        // itimeout：等待数据的超时时间（秒）：-1-不等待；0-无限等待；>0-等待的秒数。
        // 返回值：true-成功；false-失败，失败有两种情况：1）等待超时；2）socket连接已不可用。
        bool read(std::string& buffer, const int itimeout = 0);             // 接收文本数据。
        bool read(void* buffer, const int ibuflen, const int itimeout = 0); // 接收二进制数据。

        // 向对端发送数据。
        // buffer：待发送数据缓冲区。
        // ibuflen：待发送数据的大小。
        // 返回值：true-成功；false-失败，如果失败，表示socket连接已不可用。
        bool write(const std::string& buffer);             // 发送文本数据。
        bool write(const void* buffer, const int ibuflen); // 发送二进制数据。

        // 关闭监听的socket，即m_listenfd，常用于多进程服务程序的子进程代码中。
        void closelisten();

        // 关闭客户端的socket，即m_connfd，常用于多进程服务程序的父进程代码中。
        void closeclient();

        ~ctcpserver(); // 析构函数自动关闭socket，释放资源。
    };

    // 接收socket的对端发送过来的数据。
    // sockfd：可用的socket连接。
    // buffer：接收数据缓冲区的地址。
    // ibuflen：本次成功接收数据的字节数。
    // itimeout：读取数据超时的时间，单位：秒，-1-不等待；0-无限等待；>0-等待的秒数。
    // 返回值：true-成功；false-失败，失败有两种情况：1）等待超时；2）socket连接已不可用。
    bool tcpread(const int sockfd, std::string& buffer, const int itimeout = 0);             // 读取文本数据。
    bool tcpread(const int sockfd, void* buffer, const int ibuflen, const int itimeout = 0); // 读取二进制数据。

    // 向socket的对端发送数据。
    // sockfd：可用的socket连接。
    // buffer：待发送数据缓冲区的地址。
    // ibuflen：待发送数据的字节数。
    // 返回值：true-成功；false-失败，如果失败，表示socket连接已不可用。
    bool tcpwrite(const int sockfd, const std::string& buffer);             // 写入文本数据。
    bool tcpwrite(const int sockfd, const void* buffer, const int ibuflen); // 写入二进制数据。

    // 从已经准备好的socket中读取数据。
    // sockfd：已经准备好的socket连接。
    // buffer：存放数据的地址。
    // n：本次打算读取数据的字节数。
    // 返回值：成功接收到n字节的数据后返回true，socket连接不可用返回false。
    bool readn(const int sockfd, char* buffer, const size_t n);

    // 向已经准备好的socket中写入数据。
    // sockfd：已经准备好的socket连接。
    // buffer：待写入数据的地址。
    // n：待写入数据的字节数。
    // 返回值：成功写入完n字节的数据后返回true，socket连接不可用返回false。
    bool writen(const int sockfd, const char* buffer, const size_t n);
    ///////////////////////////////////// /////////////////////////////////////
#endif // __linux__

} // namespace ol

#endif // !__OL_TCP_H