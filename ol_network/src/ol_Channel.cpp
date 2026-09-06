#include "ol_net/ol_Channel.h"

// #define OL_DEBUG

namespace ol
{

#ifdef __unix__
    Channel::Channel(EventLoop* eventLoop, int fd) : m_fd(fd), m_eventLoop(eventLoop)
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

    void Channel::setNotInEpoll()
    {
        m_inEpoll = false;
    }

    // 设置m_revents成员的值为参数ev。
    void Channel::setRevents(uint32_t ev)
    {
        m_revents = ev;
    }

    void Channel::tie(const std::shared_ptr<void>& owner)
    {
        m_tie = owner;
        m_tied = true;
    }

    std::shared_ptr<void> Channel::lockTie() const
    {
        return m_tied ? m_tie.lock() : std::shared_ptr<void>{};
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
        if (m_tied)
        {
            if (auto guard = m_tie.lock()) handleEventWithGuard();
            return;
        }

        handleEventWithGuard();
    }

    void Channel::handleEventWithGuard()
    {
        // 同一批epoll事件中，定时器回调可能已经移除了此Channel。
        if (!m_inEpoll) return;

        bool handled = false;

        // RDHUP可能与最后一批可读数据同时出现，必须先把数据读完。
        if (m_revents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
        {
#ifdef OL_DEBUG
            printf("EPOLLIN | EPOLLPRI | EPOLLRDHUP\n");
#endif
            handled = true;
            if (m_readCb) m_readCb();
            if (!m_inEpoll) return;
        }

        // 读写事件可以同时到达，不能使用else if丢掉写事件。
        if (m_revents & EPOLLOUT)
        {
#ifdef OL_DEBUG
            printf("EPOLLOUT\n");
#endif
            handled = true;
            if (m_writeCb) m_writeCb();
            if (!m_inEpoll) return;
        }

        if (m_revents & EPOLLERR)
        {
#ifdef OL_DEBUG
            printf("EPOLLERR\n");
#endif
            if (m_errorCb) m_errorCb();
            return;
        }

        if (m_revents & EPOLLHUP)
        {
#ifdef OL_DEBUG
            printf("EPOLLHUP\n");
#endif
            if (m_closeCb) m_closeCb();
            return;
        }

        if (!handled && m_errorCb) m_errorCb();
    }
#endif // __unix__

} // namespace ol
