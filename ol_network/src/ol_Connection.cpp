#include "ol_net/ol_Connection.h"

// #define DEBUG

namespace ol
{

#ifdef __unix__
    Connection::Connection(EventLoop* eventLoop, SocketFd::Ptr cliFd)
        : m_eventLoop(eventLoop), m_cliFd(std::move(cliFd)), m_disconnected(false), m_cliChnl(std::make_unique<Channel>(m_eventLoop, m_cliFd->getFd()))
    {
        // 为新客户端连接准备读事件，并添加到epoll中。
        m_cliChnl->setReadCb(std::bind(&Connection::onMessage, this));
        m_cliChnl->setCloseCb(std::bind(&Connection::closeCb, this));
        m_cliChnl->setErrorCb(std::bind(&Connection::errorCb, this));
        m_cliChnl->setWriteCb(std::bind(&Connection::writeCb, this));
        m_cliChnl->useET();         // 客户端连上来的fd采用边缘触发。
        m_cliChnl->enableReading(); // 让epoll_wait()监视m_cliChnl的读事件。
    }

    Connection::~Connection()
    {
#ifdef DEBUG
        printf("Conn对象已被析构\n");
#endif
    }

    // 返回fd。
    int Connection::getFd() const
    {
        return m_cliFd->getFd();
    }

    // 返回ip。
    const char* Connection::getIp() const
    {
        return m_cliFd->getIp();
    }

    // 返回port。
    uint16_t Connection::getPort() const
    {
        return m_cliFd->getPort();
    }

    // 设置关闭m_fd的回调函数。
    void Connection::setCloseCb(std::function<void(ConnectionPtr)> func)
    {
        m_closeCb = func;
    }

    // 设置m_fd发生了错误的回调函数。
    void Connection::setErrorCb(std::function<void(ConnectionPtr)> func)
    {
        m_errorCb = func;
    }

    // 设置处理报文的回调函数。
    void Connection::setOnMessageCb(std::function<void(ConnectionPtr, std::string&)> func)
    {
        m_onMessageCb = func;
    }

    // 发送数据完成后的回调函数。
    void Connection::setSendCompleteCb(std::function<void(ConnectionPtr)> func)
    {
        m_sendCompleteCb = func;
    }

    // TCP连接关闭（断开）的回调函数，供Channel回调。
    void Connection::closeCb()
    {
        m_disconnected = true;
        m_cliChnl->remove(); // 从事件循环中删除Channel。
        m_closeCb(shared_from_this());
    }

    // TCP连接错误的回调函数，供Channel回调。
    void Connection::errorCb()
    {
        m_disconnected = true;
        m_cliChnl->remove(); // 从事件循环中删除Channel。
        m_errorCb(shared_from_this());
    }

    // 处理写事件的回调函数，供Channel回调。
    void Connection::writeCb()
    {
#ifdef DEBUG
        printf("Connection::writeCb(%ld).\n", syscall(SYS_gettid));
#endif // DEBUG

        // 尝试把m_outputBuf中的数据全部发送出去。
        int writen = ::send(getFd(), m_outputBuf.data(), m_outputBuf.size(), 0);

        if (writen > 0)
        {
            // 从m_outputBuf中删除已成功发送的字节数。
            m_outputBuf.erase(0, writen);
        }

        // 如果发送缓冲区中没有数据了，表示数据已发送完成，不再关注写事件。
        if (m_outputBuf.empty())
        {
            m_cliChnl->disableWriting();
            m_sendCompleteCb(shared_from_this());
        }
    }

    // 处理对端发送过来的消息。
    void Connection::onMessage()
    {
        // 直接调用recvFd从fd读取数据到m_inputBuf，内部已处理非阻塞循环
        ssize_t nread_total = m_inputBuf.recvFd(getFd());

        if (nread_total < 0)
        {
            // 读取错误：区分可恢复错误和致命错误
            if (errno == EINTR)
            {
                // 被信号中断，重试读取（理论上recvFd内部已处理，此处冗余保险）
                return;
            }
            else
            {
                // 致命错误（如连接重置、关闭），触发错误回调
                errorCb();
                return;
            }
        }
        else if (nread_total == 0)
        {
            // 读取到0字节，客户端已断开连接
            closeCb();
            return;
        }

        // 读取成功，从m_inputBuf中拆分完整报文并处理
        std::string message;
        while (m_inputBuf.pickMessage(message))
        {
            m_lastATime = TimeStamp::now(); // 更新最后活动时间
#ifdef DEBUG
            std::cout << "lastATime=" << m_lastATime.toString() << std::endl;
#endif
            m_onMessageCb(shared_from_this(), message); // 回调业务处理
        }
    }

    // 发送数据。
    void Connection::send(const char* data, size_t size)
    {

        if (m_disconnected == true)
        {
#ifdef DEBUG
            printf("send() return.\n");
#endif
            return;
        }

        if (m_eventLoop->isInLoopThread()) // 判断当前线程是否为事件循环线程（IO线程）。
        {
// 如果当前线程是IO线程，直接调用_sendInLoop()发送数据。
#ifdef DEBUG
            printf("send() 在事件循环的线程中。\n");
#endif
            _sendInLoop(data, size);
        }
        else
        {
// 如果当前线程不是IO线程，调用EventLoop::queueinloop()，把_sendInLoop()交给事件循环线程去执行。
#ifdef DEBUG
            printf("send() 不在事件循环的线程中。\n");
#endif
            m_eventLoop->pushToQueue(std::bind(&Connection::_sendInLoop, this, data, size));
        }
    }

    // 发送数据，如果当前线程是IO线程，直接调用此函数，如果是工作线程，将把此函数传给IO线程去执行。
    void Connection::_sendInLoop(const char* data, size_t size)
    {
        m_outputBuf.appendWithSep(data, size); // 把需要发送的数据保存到Connection的发送缓冲区中。
        m_cliChnl->enableWriting();            // 注册写事件。
    }

    // 判断TCP连接是否超时（空闲太久）。
    bool Connection::timeout(time_t now, int val)
    {
        return now - m_lastATime.toInt() > val;
    }
#endif // __unix__

} // namespace ol