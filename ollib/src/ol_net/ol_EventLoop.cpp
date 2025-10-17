#include "ol_net/ol_EventLoop.h"

// #define DEBUG

namespace ol
{
#ifdef __linux__
    static inline int createTimerFd(int sec = 30)
    {
        int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK); // 创建timerfd。
        struct itimerspec timetvl;                                             // 定时时间的数据结构。
        memset(&timetvl, 0, sizeof(struct itimerspec));
        timetvl.it_value.tv_sec = sec; // 定时时间。
        timetvl.it_value.tv_nsec = 0;
        timerfd_settime(tfd, 0, &timetvl, 0);
        return tfd;
    }

    // 在构造函数中创建EpollChnl对象m_epChnl。
    EventLoop::EventLoop(bool mainEventLoop, size_t MaxEvents, int timetvl, int timeout)
        : m_mainEventLoop(mainEventLoop), m_stop(false),
          m_timetvl(timetvl), m_timeout(timeout),
          m_epChnl(std::make_unique<EpollChnl>(MaxEvents)),
          m_wakeUpFd(eventfd(0, EFD_NONBLOCK)), m_wakeUpChnl(std::make_unique<Channel>(this, m_wakeUpFd)),
          m_timerFd(createTimerFd(m_timetvl)), m_timerChnl(std::make_unique<Channel>(this, m_timerFd)), m_threadId(0)
    {
        m_wakeUpChnl->setReadCb(std::bind(&EventLoop::handleWakeUp, this));
        m_wakeUpChnl->enableReading();

        m_timerChnl->setReadCb(std::bind(&EventLoop::handleTimer, this));
        m_timerChnl->enableReading();
    }

    // 析构函数
    EventLoop::~EventLoop()
    {
    }

    // 设置epoll_wait()超时的回调函数。
    void EventLoop::setEpollTimeoutCb(std::function<void(EventLoop*)> func)
    {
        m_epollTimeoutCb = func;
    }

    // 运行事件循环。
    void EventLoop::run(int timeout)
    {
// 事件循环时的 ID 输出
#ifdef DEBUG
        printf("EventLoop::run(%ld).\n", syscall(SYS_gettid));
#endif // DEBUG

        m_threadId = syscall(SYS_gettid);

        while (!m_stop) // 事件循环。
        {
            std::vector<Channel*> channels = m_epChnl->loop(timeout); // 等待监视的fd有事件发生。

            // 如果channels为空，表示超时，回调TcpServer::epollTimeout()。
            if (channels.empty())
            {
                if (m_epollTimeoutCb)
                {
                    m_epollTimeoutCb(this);
                }
            }
            else
            {
                for (auto& chnl : channels)
                {
                    chnl->handleEvent(); // 处理epoll_wait()返回的事件。
                }
            }
        }
    }

    // 停止事件循环。
    void EventLoop::stop()
    {
        m_stop = true;
        wakeUp(); // 唤醒事件循环，如果没有这行代码，事件循环将在下次闹钟响时或epoll_wait()超时时才会停下来。
    }

    // 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
    void EventLoop::updateChnl(Channel* ch)
    {
        m_epChnl->updateChnl(ch);
    }

    // 从红黑树上删除channel。
    void EventLoop::removeChnl(Channel* ch)
    {
        m_epChnl->removeChnl(ch);
    }

    // 把任务添加到队列中。
    void EventLoop::pushToQueue(std::function<void()> func)
    {
        {
            std::lock_guard<std::mutex> lock(m_taskQueueMutex); // 给任务队列加锁。
            m_taskQueue.push(func);                             // 任务入队。
        }

        // 唤醒事件循环。
        wakeUp();
    }

    // 用eventfd唤醒事件循环线程。
    void EventLoop::wakeUp()
    {
        uint64_t val = 1;
        write(m_wakeUpFd, &val, sizeof(val));
    }

    // 事件循环线程被eventfd唤醒后执行的函数。
    void EventLoop::handleWakeUp()
    {
#ifdef DEBUG
        printf("EventLoop::handleWakeUp(%ld).\n", syscall(SYS_gettid));
#endif
        uint64_t val;
        read(m_wakeUpFd, &val, sizeof(val)); // 从eventfd中读取出数据，如果不读取，eventfd的读事件会一直触发。

        std::function<void()> func;
        std::lock_guard<std::mutex> lock(m_taskQueueMutex); // 给任务队列加锁。

        while (m_taskQueue.size() > 0)
        {
            func = std::move(m_taskQueue.front()); // 出队一个元素。
            m_taskQueue.pop();
            func(); // 执行任务。
        }
    }

    // 闹钟响时执行的函数。
    void EventLoop::handleTimer()
    {
        // 重新计时。
        struct itimerspec timetvl; // 定时时间的数据结构。
        memset(&timetvl, 0, sizeof(struct itimerspec));
        timetvl.it_value.tv_sec = m_timetvl; // 定时时间
        timetvl.it_value.tv_nsec = 0;
        timerfd_settime(m_timerFd, 0, &timetvl, 0);

        if (m_mainEventLoop)
        {
#ifdef DEBUG
// printf("主事件循环的闹钟时间到了。\n");
#endif
        }
        else
        {
#ifdef DEBUG
            // printf("从事件循环的闹钟时间到了。\n");
            printf("EventLoop::handleTimer(%ld). Fd:", syscall(SYS_gettid));
#endif
            time_t now = time(nullptr); // 获取当前时间

            std::lock_guard<std::mutex> lock(m_connsMutex); // 遍历时加锁
            auto it = m_conns.begin();
            while (it != m_conns.end())
            {
#ifdef DEBUG
                printf("%d ", it->first);
#endif
                if (it->second->timeout(now, m_timeout))
                {
                    if (m_removeTimeoutConnCb)
                    {
                        m_removeTimeoutConnCb(it->first); // 从TcpServer的unordered_map中删除超时的conn。
                    }

                    it = m_conns.erase(it); // 从EventLoop的unordered_map中删除超时的conn。
                }
                else
                {
                    ++it;
                }
            }
#ifdef DEBUG
            printf("\n");
#endif
        }
    }

    // 把Connection对象保存在m_conns中。
    void EventLoop::newConn(ConnectionPtr conn)
    {
        std::lock_guard<std::mutex> lock(m_connsMutex);
        m_conns[conn->getFd()] = conn;
    }

    // 把Connection对象从m_conns中删除。
    void EventLoop::closeConn(ConnectionPtr conn)
    {
#ifdef DEBUG
        printf("EventLoop::closeConn(%d)\n", conn->getFd());
#endif
        std::lock_guard<std::mutex> lock(m_connsMutex);
        m_conns.erase(conn->getFd());
    }

    // 将被设置为TcpServer::removeConn()
    void EventLoop::setRemoveTimeoutConnCb(std::function<void(int)> func)
    {
        m_removeTimeoutConnCb = func;
    }
#endif // __linux__

} // namespace ol