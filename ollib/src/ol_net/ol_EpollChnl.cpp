#include "ol_net/ol_EpollChnl.h"

// #define DEBUG

namespace ol
{

#ifdef __unix__
    EpollChnl::EpollChnl(size_t MaxEvents)
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

    EpollChnl::~EpollChnl()
    {
        close(m_epollFd); // 在析构函数中关闭m_epollFd。
        delete[] m_events;
    }

    // 把fd和它需要监视的事件添加到红黑树上。
    void EpollChnl::updateChnl(Channel* chnl)
    {
        epoll_event ev{};              // 声明事件的数据结构。
        ev.data.ptr = chnl;            // 指定channel。
        ev.events = chnl->getEvents(); // 指定事件。

        if (chnl->getInEpoll()) // 如果channel已经在树上了。
        {
            if (epoll_ctl(m_epollFd, EPOLL_CTL_MOD, chnl->getFd(), &ev) == -1)
            {
                perror("epoll_ctl() failed.\n");
                exit(-1);
            }
        }
        else // 如果channel不在树上。
        {
            if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, chnl->getFd(), &ev) == -1)
            {
                perror("epoll_ctl() failed.\n");
                exit(-1);
            }
            chnl->setInEpoll(); // 把channel的m_inEpoll成员设置为true。
        }
    }

    // 从红黑树上删除channel。
    void EpollChnl::removeChnl(Channel* chnl)
    {
        if (chnl->getInEpoll()) // 如果channel已经在树上了。
        {
#ifdef DEBUG
            printf("removeChnl(%d)\n", chnl->getFd());
#endif
            if (epoll_ctl(m_epollFd, EPOLL_CTL_DEL, chnl->getFd(), 0) == -1)
            {
                perror("epoll_ctl() failed.\n");
                exit(-1);
            }
        }
    }

    // 运行epoll_wait()，等待事件的发生，已发生的事件用vector容器返回。
    std::vector<Channel*> EpollChnl::loop(int timeout)
    {
        std::vector<Channel*> chnls; // 存放epoll_wait()返回的事件。

        int infds;

        while (true)
        {
            memset(m_events, 0, sizeof(epoll_event) * m_MaxEvents);
            infds = epoll_wait(m_epollFd, m_events, m_MaxEvents, timeout); // 等待监视的fd有事件发生。

            if (infds < 0)
            {
                if (errno == EINTR)
                {
                    // 被信号中断，重试epoll_wait
                    continue;
                }
                else
                {
                    // 其他致命错误，退出
                    perror("epoll_wait() failed");
                    exit(-1);
                }
            }
            break; // 成功获取事件，退出循环
        }

        // 超时。
        if (infds == 0)
        {
            // printf("epoll_wait() timeout.\n");
            return chnls;
        }

        // 如果infds>0，表示有事件发生的fd的数量。
        for (int i = 0; i < infds; ++i) // 遍历epoll返回的数组m_events。
        {
            Channel* chnl = (Channel*)m_events[i].data.ptr;
            chnl->setRevents(m_events[i].events);
            chnls.push_back(chnl);
        }

        return chnls;
    }

    size_t EpollChnl::getMaxEvents()
    {
        return m_MaxEvents;
    }
#endif // __unix__

} // namespace ol