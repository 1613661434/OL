#include "ol_net/ol_EpollFd.h"

namespace ol
{

#ifdef __linux__
    EpollFd::EpollFd(size_t MaxEvents)
    {
        // 创建epoll句柄：设置EPOLL_CLOEXEC标志，避免进程替换时泄露文件描述符
        if ((m_epollFd = epoll_create1(EPOLL_CLOEXEC)) == -1) // 创建epoll句柄（红黑树）。
        {
            printf("epoll_create() failed(%d).\n", errno);
            exit(-1);
        }

        m_events = new epoll_event[MaxEvents];
        m_MaxEvents = MaxEvents;
    }

    EpollFd::~EpollFd()
    {
        close(m_epollFd); // 在析构函数中关闭m_epollFd。
        delete[] m_events;
    }

    // 把fd和它需要监视的事件添加到红黑树上。
    void EpollFd::addFd(int fd, uint32_t op)
    {
        epoll_event ev;  // 声明事件的数据结构。
        ev.data.fd = fd; // 指定事件的自定义数据，会随着epoll_wait()返回的事件一并返回。
        ev.events = op;  // 让epoll监视fd的。

        if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, fd, &ev) == -1) // 把需要监视的fd和它的事件加入epollfd中。
        {
            printf("epoll_ctl() failed(%d).\n", errno);
            exit(-1);
        }
    }

    // 运行epoll_wait()，等待事件的发生，已发生的事件用vector容器返回。
    std::vector<epoll_event> EpollFd::loop(int timeout)
    {
        std::vector<epoll_event> evs; // 存放epoll_wait()返回的事件。

        memset(m_events, 0, sizeof(epoll_event) * m_MaxEvents);
        int infds = epoll_wait(m_epollFd, m_events, m_MaxEvents, timeout); // 等待监视的fd有事件发生。

        // 返回失败。
        if (infds < 0)
        {
            perror("epoll_wait() failed");
            exit(-1);
        }

        // 超时。
        if (infds == 0)
        {
            printf("epoll_wait() timeout.\n");
            return evs;
        }

        // 如果infds>0，表示有事件发生的fd的数量。
        for (int i = 0; i < infds; ++i) // 遍历epoll返回的数组m_events。
        {
            evs.push_back(m_events[i]);
        }

        return evs;
    }

    size_t EpollFd::getMaxEvents()
    {
        return m_MaxEvents;
    }
#endif // __linux__

} // namespace ol