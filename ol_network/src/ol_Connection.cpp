#include "ol_net/ol_Connection.h"

#include <cerrno>
#include <stdexcept>

// #define OL_DEBUG

namespace ol
{

#ifdef __unix__
    Connection::Connection(EventLoop* eventLoop, SocketFd::Ptr cliFd, size_t maxFrameSize)
        : m_eventLoop(eventLoop), m_cliFd(std::move(cliFd)),
          m_cliChnl(std::make_unique<Channel>(m_eventLoop, m_cliFd->getFd())),
          m_inputBuf(1, maxFrameSize), m_outputBuf(1, maxFrameSize),
          m_maxFrameSize(maxFrameSize), m_disconnected(false)
    {
        m_cliChnl->setReadCb(std::bind(&Connection::onMessage, this));
        m_cliChnl->setCloseCb(std::bind(&Connection::closeCb, this));
        m_cliChnl->setErrorCb(std::bind(&Connection::errorCb, this));
        m_cliChnl->setWriteCb(std::bind(&Connection::writeCb, this));
        m_cliChnl->useET(); // 客户端连上来的fd采用边缘触发。
    }

    void Connection::connectEstablished()
    {
        if (m_disconnected.load(std::memory_order_acquire)) return;
        m_cliChnl->tie(shared_from_this());
        m_cliChnl->enableReading();
    }

    void Connection::disconnect()
    {
        if (m_disconnected.exchange(true, std::memory_order_acq_rel)) return;
        m_cliChnl->remove();
    }

    Connection::~Connection()
    {
#ifdef OL_DEBUG
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
        if (m_disconnected.exchange(true, std::memory_order_acq_rel)) return;
        auto self = shared_from_this();
        m_cliChnl->remove(); // 从事件循环中删除Channel。
        if (m_closeCb) m_closeCb(self);
    }

    // TCP连接错误的回调函数，供Channel回调。
    void Connection::errorCb()
    {
        if (m_disconnected.exchange(true, std::memory_order_acq_rel)) return;
        auto self = shared_from_this();
        m_cliChnl->remove(); // 从事件循环中删除Channel。
        if (m_errorCb) m_errorCb(self);
    }

    // 处理写事件的回调函数，供Channel回调。
    void Connection::writeCb()
    {
#ifdef OL_DEBUG
        printf("Connection::writeCb(%ld).\n", syscall(SYS_gettid));
#endif // OL_DEBUG

        while (!m_outputBuf.empty())
        {
            const ssize_t written = ::send(getFd(), m_outputBuf.data(), m_outputBuf.size(), MSG_NOSIGNAL);
            if (written > 0)
            {
                m_outputBuf.erase(0, static_cast<size_t>(written));
                continue;
            }
            if (written < 0 && errno == EINTR) continue;
            if (written < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) break;

            errorCb();
            return;
        }

        // 如果发送缓冲区中没有数据了，表示数据已发送完成，不再关注写事件。
        if (m_outputBuf.empty())
        {
            m_cliChnl->disableWriting();
            if (m_sendCompleteCb) m_sendCompleteCb(shared_from_this());
        }
    }

    // 处理对端发送过来的消息。
    void Connection::onMessage()
    {
        auto self = shared_from_this();
        while (true)
        {
            const ssize_t count = m_inputBuf.recvFd(getFd());
            if (count > 0)
            {
                try
                {
                    std::string message;
                    while (m_inputBuf.pickMessage(message))
                    {
                        m_lastATime = TimeStamp::now();
#ifdef OL_DEBUG
                        std::cout << "lastATime=" << m_lastATime.toString() << std::endl;
#endif
                        if (m_onMessageCb) m_onMessageCb(self, message);
                        if (m_disconnected.load(std::memory_order_acquire)) return;
                    }
                }
                catch (const std::length_error&)
                {
                    errorCb();
                    return;
                }
                continue;
            }
            if (count == 0)
            {
                closeCb();
                return;
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
            if (errno == EINTR) continue;

            errorCb();
            return;
        }
    }

    // 发送数据。
    void Connection::send(const char* data, size_t size)
    {
        if (data == nullptr && size != 0) throw std::invalid_argument("Message data must not be null");
        if (size > m_maxFrameSize) throw std::length_error("Message exceeds the maximum frame size");

        if (m_disconnected.load(std::memory_order_acquire))
        {
#ifdef OL_DEBUG
            printf("send() return.\n");
#endif
            return;
        }

        if (m_eventLoop->isInLoopThread()) // 判断当前线程是否为事件循环线程（IO线程）。
        {
// 如果当前线程是IO线程，直接调用_sendInLoop()发送数据。
#ifdef OL_DEBUG
            printf("send() 在事件循环的线程中。\n");
#endif
            _sendInLoop(data, size);
        }
        else
        {
// 如果当前线程不是IO线程，调用EventLoop::queueinloop()，把_sendInLoop()交给事件循环线程去执行。
#ifdef OL_DEBUG
            printf("send() 不在事件循环的线程中。\n");
#endif
            // 拷贝数据，避免调用方buffer释放后IO线程访问已释放内存
            auto msg = data == nullptr
                           ? std::make_shared<std::string>()
                           : std::make_shared<std::string>(data, size);
            auto self = shared_from_this();
            m_eventLoop->pushToQueue([self, msg]()
                                     { self->_sendInLoop(msg->data(), msg->size()); });
        }
    }

    // 发送数据，如果当前线程是IO线程，直接调用此函数，如果是工作线程，将把此函数传给IO线程去执行。
    void Connection::_sendInLoop(const char* data, size_t size)
    {
        if (m_disconnected.load(std::memory_order_acquire)) return;
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
