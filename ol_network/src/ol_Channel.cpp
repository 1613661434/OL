#include "ol_net/ol_Channel.h"

// #define DEBUG

namespace ol
{

#ifdef __unix__
    Channel::Channel(EventLoop* eventLoop, int fd) : m_eventLoop(eventLoop), m_fd(fd)
    {
    }

    // 析构函数。
    Channel::~Channel()
    {
        // 在析构函数中，不要销毁m_eventLoop，也不能关闭m_fd，因为这两个东西不属于Channel类，Channel类只是需要它们，使用它们而已。
    }

    // 返回m_fd成员。
    int Channel::getFd()
    {
        return m_fd;
    }

    // 返回m_inEpoll成员。
    bool Channel::getInEpoll()
    {
        return m_inEpoll;
    }

    // 返回m_events成员。
    uint32_t Channel::getEvents()
    {
        return m_events;
    }

    // 返回m_revents成员。
    uint32_t Channel::getRevents()
    {
        return m_revents;
    }

    // 采用边缘触发。
    void Channel::useET()
    {
        m_events |= EPOLLET;
    }

    // 让epoll_wait()监视m_fd的读事件。
    void Channel::enableReading()
    {
        m_events |= EPOLLIN;
        m_eventLoop->updateChnl(this);
    }

    // 取消读事件。
    void Channel::disableReading()
    {
        m_events &= ~EPOLLIN;
        m_eventLoop->updateChnl(this);
    }

    // 注册写事件。
    void Channel::enableWriting()
    {
        m_events |= EPOLLOUT;
        m_eventLoop->updateChnl(this);
    }

    // 取消写事件。
    void Channel::disableWriting()
    {
        m_events &= ~EPOLLOUT;
        m_eventLoop->updateChnl(this);
    }

    // 取消全部的事件。
    void Channel::disableAll()
    {
        m_events = 0;
        m_eventLoop->updateChnl(this);
    }

    // 从事件循环中删除Channel。
    void Channel::remove()
    {
        m_eventLoop->removeChnl(this);
    }

    // 把m_inEpoll成员的值设置为true。
    void Channel::setInEpoll()
    {
        m_inEpoll = true;
    }

    // 设置m_revents成员的值为参数ev。
    void Channel::setRevents(uint32_t ev)
    {
        m_revents = ev;
    }

    // 设置m_fd读事件的回调函数。
    void Channel::setReadCb(std::function<void()> func)
    {
        m_readCb = func;
    }

    // 设置关闭m_fd的回调函数。
    void Channel::setCloseCb(std::function<void()> func)
    {
        m_closeCb = func;
    }

    // 设置m_fd发生了错误的回调函数。
    void Channel::setErrorCb(std::function<void()> func)
    {
        m_errorCb = func;
    }

    // 设置写事件的回调函数。
    void Channel::setWriteCb(std::function<void()> func)
    {
        m_writeCb = func;
    }

    // 事件处理函数，epoll_wait()返回的时候，执行它。
    void Channel::handleEvent()
    {
        if (m_revents & EPOLLRDHUP) // 对方已关闭，有些系统检测不到，可以使用EPOLLIN，recv()返回0。
        {
#ifdef DEBUG
            printf("EPOLLRDHUP\n");
#endif
            // 回调Connection::closeCb()。
            m_closeCb();
        }
        else if (m_revents & (EPOLLIN | EPOLLPRI)) // 接收缓冲区中有数据可以读。
        {
#ifdef DEBUG
            printf("EPOLLIN | EPOLLPRI\n");
#endif
            // 如果是servChnl，将回调Acceptor::newConnection()；
            // 如果是cliChnl，将回调Connection::onMessage()；
            // 如果是wakeUpChnl，将回调EventLoop::handleWakeUp()。
            m_readCb();
        }
        else if (m_revents & EPOLLOUT) // 有数据需要写。
        {
#ifdef DEBUG
            printf("EPOLLOUT\n");
#endif
            // 回调Connection::writeCb()。
            m_writeCb();
        }
        else // 其它事件，都视为错误。
        {
#ifdef DEBUG
            printf("EPOLLELSE\n");
#endif
            // 回调Connection::errorCb()。
            m_errorCb();
        }
    }
#endif // __unix__

} // namespace ol