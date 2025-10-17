#ifndef OL_EPOLLFD_H
#define OL_EPOLLFD_H 1

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <vector>

#ifdef __linux__
#include <sys/epoll.h>
#include <unistd.h>
#endif // __linux__

namespace ol
{

#ifdef __linux__
    // EpollFd类。
    class EpollFd
    {
    private:
        int m_epollFd = -1;    ///< epoll句柄，在构造函数中创建。
        epoll_event* m_events; ///< 存放poll_wait()返回事件的数组，在构造函数中分配内存。
        size_t m_MaxEvents;    ///< 最大事件数

    public:
        explicit EpollFd(size_t MaxEvents = 100);
        ~EpollFd(); // 在析构函数中关闭epollfd_。

        void addFd(int fd, uint32_t op);                 // 把fd和它需要监视的事件添加到红黑树上。
        std::vector<epoll_event> loop(int timeout = -1); // 运行epoll_wait()，等待事件的发生，已发生的事件用vector容器返回。
        size_t getMaxEvents();
    };
#endif // __linux__

} // namespace ol

#endif // !OL_EPOLLFD_H