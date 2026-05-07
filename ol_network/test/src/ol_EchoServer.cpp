#include "ol_EchoServer.h"

#ifdef __unix__
#include <sys/syscall.h>
#include <unistd.h>
#endif // __unix__

// #define DEBUG
// #define DEBUG2

namespace ol
{

#ifdef __unix__
    EchoServer::EchoServer(const std::string& ip, const uint16_t port, size_t workThreadNum, size_t subThreadNum, size_t MainMaxEvents, size_t SubMaxEvents, int epWaitTimeout, int timerTimetvl, int timerTimeout)
        : m_tcpServ(ip, port, subThreadNum, MainMaxEvents, SubMaxEvents, epWaitTimeout, timerTimetvl, timerTimeout), m_threadPool(workThreadNum, 0)
    {
        // 以下代码不是必须的，业务关心什么事件，就指定相应的回调函数。
        m_tcpServ.setNewConnCb(std::bind(&EchoServer::handleNewConn, this, std::placeholders::_1));
        m_tcpServ.setCloseCb(std::bind(&EchoServer::handleClose, this, std::placeholders::_1));
        m_tcpServ.setErrorCb(std::bind(&EchoServer::handleError, this, std::placeholders::_1));
        m_tcpServ.setOnMessageCb(std::bind(&EchoServer::handleMessage, this, std::placeholders::_1, std::placeholders::_2));
        m_tcpServ.setSendCompleteCb(std::bind(&EchoServer::handleSendComplete, this, std::placeholders::_1));
        // m_tcpServ.setTimeoutCb(std::bind(&EchoServer::handleTimeOut, this, std::placeholders::_1));
    }

    EchoServer::~EchoServer()
    {
    }

    // 启动服务。
    void EchoServer::start(int newConnTimeout)
    {
        m_tcpServ.start(newConnTimeout);
    }

    // 停止服务。
    void EchoServer::stop()
    {
        // 停止工作线程。
        m_threadPool.stop();
#ifdef DEBUG
        printf("工作线程已停止。\n");
#endif

        // 停止TcpServer。
        m_tcpServ.stop();
    }

    // 处理新客户端连接请求，在TcpServer类中回调此函数。
    void EchoServer::handleNewConn(Connection::Ptr conn)
    {
#ifdef DEBUG
        printf("EchoServer::handleNewConn(%ld).\n", syscall(SYS_gettid));
#endif // DEBUG

        printf("%s new connection(fd=%d,ip=%s,port=%d) ok.\n", TimeStamp::now().toString().c_str(), conn->getFd(), conn->getIp(), conn->getPort());

        // std::cout << "New Connection Come in." << std::endl;

        // 根据业务的需求，在这里可以增加其它的代码。
    }

    // 关闭客户端的连接，在TcpServer类中回调此函数。
    void EchoServer::handleClose(Connection::Ptr conn)
    {
        // std::cout << "EchoServer conn closed." << std::endl;

        printf("%s connection closed(fd=%d,ip=%s,port=%d).\n", TimeStamp::now().toString().c_str(), conn->getFd(), conn->getIp(), conn->getPort());

        // 根据业务的需求，在这里可以增加其它的代码。
    }

    // 客户端的连接错误，在TcpServer类中回调此函数。
    void EchoServer::handleError(Connection::Ptr conn)
    {
        // std::cout << "EchoServer conn error." << std::endl;

        // 根据业务的需求，在这里可以增加其它的代码。
    }

    // 处理客户端的请求报文，在TcpServer类中回调此函数。
    void EchoServer::handleMessage(Connection::Ptr conn, std::string& message)
    {
#ifdef DEBUG
        printf("EchoServer::handleMessage(%ld).\n", syscall(SYS_gettid));
#endif // DEBUG

        if (m_threadPool.getWorkerNum() == 0)
        {
            // 如果没有工作线程，表示在IO线程中计算
            onMessage(conn, message);
        }
        else
        {
            // 把业务添加到线程池的任务队列中。
            m_threadPool.addTask(std::bind(&EchoServer::onMessage, this, conn, message));
        }
    }

    // 数据发送完成后，在TcpServer类中回调此函数。
    void EchoServer::handleSendComplete(Connection::Ptr conn)
    {
        // std::cout << "Message send complete." << std::endl;

        // 根据业务的需求，在这里可以增加其它的代码。
    }

    // epoll_wait()超时，在TcpServer类中回调此函数。
    void EchoServer::handleTimeOut(EventLoop* eventLoop)
    {
        // std::cout << "EchoServer timeout." << std::endl;

        // 根据业务的需求，在这里可以增加其它的代码。
    }

    // 处理客户端的请求报文，用于添加给线程池。
    void EchoServer::onMessage(Connection::Ptr conn, std::string& message)
    {
#ifdef DEBUG
        printf("EchoServer::onMessage(%ld).\n", syscall(SYS_gettid));
#endif // DEBUG

#ifdef DEBUG2
        printf("%s message (eventfd=%d):%s\n", TimeStamp::now().toString().c_str(), conn->getFd(), message.c_str());
#endif
        // 在这里，将经过若干步骤的运算。
        message = "reply:" + message; // 回显业务。

        conn->send(message.data(), message.size()); // 把数据发送出去。
    }
#endif // __unix__

} // namespace ol