#ifndef OL_EVENTLOOP_H
#define OL_EVENTLOOP_H 1

#include "ol_net/ol_Channel.h"
#include "ol_net/ol_Connection.h"
#include "ol_net/ol_EpollChnl.h"
#include "ol_net/ol_net_fwd_decls.h"
#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <unistd.h>
#include <unordered_map>

#ifdef __linux__
#include <sys/eventfd.h>
#include <sys/syscall.h>
#include <sys/timerfd.h>
#endif // __linux__

namespace ol
{

#ifdef __linux__
    // 事件循环类。
    class EventLoop
    {
    public:
        using Ptr = std::unique_ptr<EventLoop>;

    private:
        bool m_mainEventLoop;                             ///< true-是主事件循环，false-是从事件循环。
        std::atomic_bool m_stop;                          ///< 初始值为false，如果设置为true，表示停止事件循环。
        int m_timetvl;                                    ///< time interval闹钟时间间隔，单位：秒。
        int m_timeout;                                    ///< Connection对象超时的时间，单位：秒。
        EpollChnlPtr m_epChnl;                            ///< 每个事件循环只有一个EpollChnl。
        std::function<void(EventLoop*)> m_epollTimeoutCb; ///< epoll_wait()超时的回调函数。
        pid_t m_threadId;                                 ///< 事件循环所在线程的id。
        std::mutex m_taskQueueMutex;                      ///< 任务队列同步的互斥锁。
        std::queue<std::function<void()>> m_taskQueue;    ///< 事件循环线程被eventfd唤醒后执行的任务队列。
        int m_wakeUpFd;                                   ///< 用于唤醒事件循环线程的eventfd。
        ChannelPtr m_wakeUpChnl;                          ///< eventfd的Channel。
        int m_timerFd;                                    ///< 定时器的fd。
        ChannelPtr m_timerChnl;                           ///< 定时器的Channel。
        std::mutex m_connsMutex;                          ///< 保护m_conns的互斥锁。
        std::unordered_map<int, ConnectionPtr> m_conns;   ///< 存放运行在该事件循环上全部的Connection对象。
        std::function<void(int)> m_removeTimeoutConnCb;   ///< 删除TcpServer中超时的Connection对象，将被设置为TcpServer::removeConnection()
    public:
        explicit EventLoop(bool mainEventLoop, size_t MaxEvents = 100, int timetvl = 30, int timeout = 80); // 在构造函数中创建EpollChnl对象m_epChnl。
        ~EventLoop();                                                                                       // 在析构函数中销毁m_epChnl。

        void setEpollTimeoutCb(std::function<void(EventLoop*)> func); // 设置epoll_wait()超时的回调函数。

        void run(int timeout = 10000); // 运行事件循环。
        void stop();                   // 停止事件循环。

        void updateChnl(Channel* ch); // 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
        void removeChnl(Channel* ch); // 从红黑树上删除channel。

        void pushToQueue(std::function<void()> func); // 把任务添加到队列中。
        void wakeUp();                                // 用eventfd唤醒事件循环线程。
        void handleWakeUp();                          // 事件循环线程被eventfd唤醒后执行的函数。

        void handleTimer(); // 闹钟响时执行的函数。

        void newConn(ConnectionPtr conn);   // 把Connection对象保存在m_conns中。
        void closeConn(ConnectionPtr conn); // 把Connection对象从m_conns中删除。

        // 判断当前线程是否为事件循环线程。
        inline bool isInLoopThread() const
        {
            return m_threadId == syscall(SYS_gettid);
        }

        void setRemoveTimeoutConnCb(std::function<void(int)> func); // 将被设置为TcpServer::removeConn(int fd)
    };
#endif // __linux__

} // namespace ol

#endif // !OL_EVENTLOOP_H