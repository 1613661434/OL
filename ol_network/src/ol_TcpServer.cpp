#include "ol_net/ol_TcpServer.h"

#include <cstdint>
#include <limits>
#include <stdexcept>

// #define OL_DEBUG

namespace ol
{

#ifdef __unix__
    namespace
    {
        size_t checkedThreadNum(size_t threadNum)
        {
            if (threadNum == 0)
                throw std::invalid_argument("The number of sub event threads must be greater than zero");
            return threadNum;
        }

        size_t checkedMaxFrameSize(size_t maxFrameSize)
        {
            if (maxFrameSize == 0 || maxFrameSize > std::numeric_limits<uint32_t>::max())
                throw std::invalid_argument("Maximum frame size must be between 1 and UINT32_MAX");
            return maxFrameSize;
        }
    } // namespace

    TcpServer::TcpServer(const std::string& ip, const uint16_t port, size_t threadNum,
                         size_t MainMaxEvents, size_t SubMaxEvents, int epWaitTimeout,
                         int timerTimetvl, int timerTimeout, size_t maxFrameSize)
        : m_mainEventLoop(std::make_unique<EventLoop>(true, MainMaxEvents)),
          m_threadNum(checkedThreadNum(threadNum)), m_threadPool(m_threadNum, 0),
          m_acceptor(m_mainEventLoop.get(), ip, port),
          m_maxFrameSize(checkedMaxFrameSize(maxFrameSize))
    {
        m_mainEventLoop->setEpollTimeoutCb(std::bind(&TcpServer::epollTimeout, this, std::placeholders::_1));

        m_acceptor.setNewConnCb(std::bind(&TcpServer::newConn, this, std::placeholders::_1));

        // 创建从事件循环。
        m_subEventLoops.resize(m_threadNum);
        for (size_t i = 0; i < m_threadNum; ++i)
        {
            m_subEventLoops[i] = std::make_unique<EventLoop>(false, SubMaxEvents, timerTimetvl, timerTimeout);          // 创建从事件循环，存入m_subEventLoops容器中。
            m_subEventLoops[i]->setEpollTimeoutCb(std::bind(&TcpServer::epollTimeout, this, std::placeholders::_1));    // 设置timeout超时的回调函数。
            m_subEventLoops[i]->setRemoveTimeoutConnCb(std::bind(&TcpServer::removeConn, this, std::placeholders::_1)); // 设置清理空闲TCP连接的回调函数。
            m_threadPool.addTask(std::bind(&EventLoop::run, m_subEventLoops[i].get(), epWaitTimeout));                  // 在线程池中运行从事件循环。
        }
    }

    TcpServer::~TcpServer()
    {
        stop();
    }

    // 运行事件循环。
    void TcpServer::start(int newConnTimeout)
    {
        m_mainEventLoop->run(newConnTimeout);
    }

    // 停止IO线程和事件循环。
    void TcpServer::stop()
    {
        if (m_stopped.exchange(true, std::memory_order_acq_rel)) return;

        // 停止主事件循环。
        m_mainEventLoop->stop();
#ifdef OL_DEBUG
        printf("主事件循环已停止。\n");
#endif

        // 停止从事件循环。
        for (size_t i = 0; i < m_threadNum; ++i)
        {
            m_subEventLoops[i]->stop();
        }
#ifdef OL_DEBUG
        printf("从事件循环已停止。\n");
#endif

        // 停止IO线程。
        m_threadPool.stop();
#ifdef OL_DEBUG
        printf("IO线程池停止。\n");
#endif
    }

    // 处理新客户端连接请求。
    void TcpServer::newConn(SocketFd::Ptr cliFd)
    {
        if (m_stopped.load(std::memory_order_acquire)) return;

        const size_t loopIndex = static_cast<size_t>(cliFd->getFd()) % m_threadNum;
        EventLoop* eventLoop = m_subEventLoops[loopIndex].get();

        // 把新建的conn分配给从事件循环。
        ConnectionPtr conn = std::make_shared<Connection>(eventLoop, std::move(cliFd), m_maxFrameSize);
        conn->setCloseCb(std::bind(&TcpServer::closeConn, this, std::placeholders::_1));
        conn->setErrorCb(std::bind(&TcpServer::errorConn, this, std::placeholders::_1));
        conn->setOnMessageCb(std::bind(&TcpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
        conn->setSendCompleteCb(std::bind(&TcpServer::sendComplete, this, std::placeholders::_1));

#ifdef OL_DEBUG
        printf("TcpServer::newConn(fd=%d,ip=%s,port=%d)\n", conn->getFd(), conn->getIp(), conn->getPort());
#endif
        {
            std::lock_guard<std::mutex> lock(m_connsMutex);
            m_conns[conn->getFd()] = conn; // 把conn存放unordered_map容器中。
        }
        eventLoop->newConn(conn); // 把conn存放到EventLoop的map容器中。

        // 先排队建立连接，确保业务回调中send()产生的任务排在Channel生命周期绑定之后。
        eventLoop->pushToQueue([conn]() { conn->connectEstablished(); });

        if (m_newConnCb) m_newConnCb(conn); // 回调上层业务类的handleNewConn()。
    }

    // 关闭客户端的连接，在Connection类中回调此函数。
    void TcpServer::closeConn(ConnectionPtr conn)
    {
        if (m_closeCb) m_closeCb(conn); // 回调上层业务类的handleClose()。
#ifdef OL_DEBUG
        printf("TcpServer::closeConn(%d)\n", conn->getFd());
#endif
        m_subEventLoops[conn->getFd() % m_threadNum]->closeConn(conn); // 删除从事件中的conn。
        std::lock_guard<std::mutex> lock(m_connsMutex);
        m_conns.erase(conn->getFd()); // 从unordered_map中删除conn。
    }

    // 客户端的连接错误，在Connection类中回调此函数。
    void TcpServer::errorConn(ConnectionPtr conn)
    {
        if (m_errorCb) m_errorCb(conn); // 回调上层业务类的handleError()。
#ifdef OL_DEBUG
        printf("TcpServer::errorConn(%d)\n", conn->getFd());
#endif
        m_subEventLoops[conn->getFd() % m_threadNum]->closeConn(conn); // 删除从事件中的conn。
        std::lock_guard<std::mutex> lock(m_connsMutex);
        m_conns.erase(conn->getFd()); // 从unordered_map中删除conn。
    }

    // 处理客户端的请求报文，在Connection类中回调此函数。
    void TcpServer::onMessage(ConnectionPtr conn, std::string& message)
    {
        if (m_onMessageCb) m_onMessageCb(conn, message); // 回调上层业务类的handleMessage()。
    }

    // 数据发送完成后，在Connection类中回调此函数。
    void TcpServer::sendComplete(ConnectionPtr conn)
    {
        if (m_sendCompleteCb) m_sendCompleteCb(conn); // 回调上层业务类的handleSendComplete()。
    }

    // epoll_wait()超时，在EventLoop类中回调此函数。
    void TcpServer::epollTimeout(EventLoop* eventLoop)
    {
        if (m_timeoutCb) m_timeoutCb(eventLoop); // 回调上层业务类的handleTimeOut()。
    }

    // 删除m_conns中的Connection对象，在EventLoop::handleTimer()中将回调此函数。
    void TcpServer::removeConn(int fd)
    {
        {
            std::lock_guard<std::mutex> lock(m_connsMutex);
            m_conns.erase(fd); // 从unordered_map中删除conn。
        }

        if (m_timerTimeoutCb) m_timerTimeoutCb(fd);
    }

    void TcpServer::setNewConnCb(std::function<void(ConnectionPtr)> func)
    {
        m_newConnCb = func;
    }

    void TcpServer::setCloseCb(std::function<void(ConnectionPtr)> func)
    {
        m_closeCb = func;
    }

    void TcpServer::setErrorCb(std::function<void(ConnectionPtr)> func)
    {
        m_errorCb = func;
    }

    void TcpServer::setOnMessageCb(std::function<void(ConnectionPtr, std::string& message)> func)
    {
        m_onMessageCb = func;
    }

    void TcpServer::setSendCompleteCb(std::function<void(ConnectionPtr)> func)
    {
        m_sendCompleteCb = func;
    }

    void TcpServer::setTimeoutCb(std::function<void(EventLoop*)> func)
    {
        m_timeoutCb = func;
    }

    void TcpServer::setTimerTimeoutCb(std::function<void(int)> func)
    {
        m_timerTimeoutCb = func;
    }
#endif // __unix__

} // namespace ol
