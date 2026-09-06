#include "ol_net/ol_EventLoop.h"

#include <cerrno>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <vector>

// #define OL_DEBUG

namespace ol
{
#ifdef __unix__
    static inline int createTimerFd(int sec = 30)
    {
        if (sec <= 0) throw std::invalid_argument("Timer interval must be greater than zero");

        const int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
        if (tfd < 0) throw std::system_error(errno, std::generic_category(), "timerfd_create failed");

        struct itimerspec timetvl{};
        timetvl.it_value.tv_sec = sec;
        timetvl.it_interval.tv_sec = sec;
        if (timerfd_settime(tfd, 0, &timetvl, nullptr) < 0)
        {
            const int error = errno;
            ::close(tfd);
            throw std::system_error(error, std::generic_category(), "timerfd_settime failed");
        }
        return tfd;
    }

    // 在构造函数中创建EpollChnl对象m_epChnl。
    EventLoop::EventLoop(bool mainEventLoop, size_t MaxEvents, int timetvl, int timeout)
        : m_mainEventLoop(mainEventLoop), m_stop(false),
          m_timetvl(timetvl), m_timeout(timeout),
          m_epChnl(std::make_unique<EpollChnl>(MaxEvents)),
          m_threadId(0), m_wakeUpFd(-1), m_timerFd(-1)
    {
        try
        {
            m_wakeUpFd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
            if (m_wakeUpFd < 0)
                throw std::system_error(errno, std::generic_category(), "eventfd failed");

            m_wakeUpChnl = std::make_unique<Channel>(this, m_wakeUpFd);
            m_wakeUpChnl->setReadCb(std::bind(&EventLoop::handleWakeUp, this));
            m_wakeUpChnl->enableReading();

            m_timerFd = createTimerFd(m_timetvl);
            m_timerChnl = std::make_unique<Channel>(this, m_timerFd);
            m_timerChnl->setReadCb(std::bind(&EventLoop::handleTimer, this));
            m_timerChnl->enableReading();
        }
        catch (...)
        {
            if (m_wakeUpFd >= 0) ::close(m_wakeUpFd);
            if (m_timerFd >= 0) ::close(m_timerFd);
            throw;
        }
    }

    // 析构函数
    EventLoop::~EventLoop()
    {
        if (m_wakeUpFd >= 0) ::close(m_wakeUpFd);
        if (m_timerFd >= 0) ::close(m_timerFd);
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
#ifdef OL_DEBUG
        printf("EventLoop::run(%ld).\n", syscall(SYS_gettid));
#endif // OL_DEBUG

        m_threadId.store(static_cast<pid_t>(syscall(SYS_gettid)), std::memory_order_release);

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
                // 先锁住整批Channel的所属对象，避免前一个回调删除后一个事件中的连接。
                std::vector<std::shared_ptr<void>> activeOwners;
                activeOwners.reserve(channels.size());
                for (Channel* chnl : channels)
                {
                    if (auto owner = chnl->lockTie()) activeOwners.push_back(std::move(owner));
                }

                for (auto& chnl : channels)
                {
                    chnl->handleEvent(); // 处理epoll_wait()返回的事件。
                }
            }
        }

        m_threadId.store(0, std::memory_order_release);
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
        const uint64_t val = 1;
        while (::write(m_wakeUpFd, &val, sizeof(val)) < 0)
        {
            if (errno == EINTR) continue;
            break;
        }
    }

    // 事件循环线程被eventfd唤醒后执行的函数。
    void EventLoop::handleWakeUp()
    {
#ifdef OL_DEBUG
        printf("EventLoop::handleWakeUp(%ld).\n", syscall(SYS_gettid));
#endif
        uint64_t val = 0;
        while (::read(m_wakeUpFd, &val, sizeof(val)) < 0)
        {
            if (errno == EINTR) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
            return;
        }

        std::queue<std::function<void()>> tasks;
        {
            std::lock_guard<std::mutex> lock(m_taskQueueMutex);
            tasks.swap(m_taskQueue);
        }

        while (!tasks.empty())
        {
            auto func = std::move(tasks.front());
            tasks.pop();
            if (func) func();
        }
    }

    // 闹钟响时执行的函数。
    void EventLoop::handleTimer()
    {
        uint64_t expirations = 0;
        while (::read(m_timerFd, &expirations, sizeof(expirations)) < 0)
        {
            if (errno == EINTR) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
            return;
        }

        if (m_mainEventLoop)
        {
#ifdef OL_DEBUG
// printf("主事件循环的闹钟时间到了。\n");
#endif
        }
        else
        {
#ifdef OL_DEBUG
            // printf("从事件循环的闹钟时间到了。\n");
            printf("EventLoop::handleTimer(%ld). Fd:", syscall(SYS_gettid));
#endif
            time_t now = time(nullptr); // 获取当前时间

            std::vector<ConnectionPtr> timeoutConns;
            {
                std::lock_guard<std::mutex> lock(m_connsMutex);
                auto it = m_conns.begin();
                while (it != m_conns.end())
                {
#ifdef OL_DEBUG
                    printf("%d ", it->first);
#endif
                    if (it->second->timeout(now, m_timeout))
                    {
                        timeoutConns.push_back(it->second);
                        it = m_conns.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }
            }

            for (const auto& conn : timeoutConns)
            {
                const int fd = conn->getFd();
                conn->disconnect();
                if (m_removeTimeoutConnCb) m_removeTimeoutConnCb(fd);
            }
#ifdef OL_DEBUG
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
#ifdef OL_DEBUG
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
#endif // __unix__

} // namespace ol
