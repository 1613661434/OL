#ifndef OL_EPOLLCHNL_H
#define OL_EPOLLCHNL_H 1

#include "ol_net/ol_Channel.h"
#include "ol_net/ol_net_fwd_decls.h"
#include <cstddef>
#include <errno.h>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <vector>

#ifdef __linux__
#include <sys/epoll.h>
#endif // __linux__

namespace ol
{

#ifdef __linux__
    // EpollChnl类。
    class EpollChnl
    {
    public:
        using Ptr = std::unique_ptr<EpollChnl>;

    private:
        int m_epollFd = -1;    ///< epoll句柄，在构造函数中创建。
        epoll_event* m_events; ///< 存放poll_wait()返回事件的数组，在构造函数中分配内存。
        size_t m_MaxEvents;    ///< 最大事件数

    public:
        explicit EpollChnl(size_t MaxEvents = 100);
        ~EpollChnl(); // 在析构函数中关闭m_epollFd。

        void updateChnl(Channel* chnl);               // 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
        void removeChnl(Channel* chnl);               // 从红黑树上删除channel。
        std::vector<Channel*> loop(int timeout = -1); // 运行epoll_wait()，等待事件的发生，已发生的事件用vector容器返回。
        size_t getMaxEvents();
    };
#endif // __linux__

} // namespace ol

#endif // !OL_EPOLLCHNL_H