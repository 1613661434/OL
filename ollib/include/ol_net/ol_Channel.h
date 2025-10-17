#ifndef OL_CHANNEL_H
#define OL_CHANNEL_H 1

#include "ol_net/ol_EpollChnl.h"
#include "ol_net/ol_EventLoop.h"
#include "ol_net/ol_net_fwd_decls.h"
#include <functional>
#include <memory>

#ifdef __linux__
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif // __linux__

namespace ol
{

#ifdef __linux__
    class Channel
    {
    public:
        using Ptr = std::unique_ptr<Channel>;

    private:
        int m_fd = -1;          ///< Channel拥有的fd，Channel和fd是一对一的关系。
        EventLoop* m_eventLoop; ///< Channel对应的事件循环，Channel与EventLoop是多对一的关系，一个Channel只对应一个EventLoop。
        bool m_inEpoll = false; ///< Channel是否已添加到epoll树上，如果未添加，调用epoll_ctl()的时候用EPOLL_CTL_ADD，否则用EPOLL_CTL_MOD。
        uint32_t m_events = 0;  ///< m_fd需要监视的事件。listenfd和clientfd需要监视EPOLLIN，clientfd还可能需要监视EPOLLOUT。
        uint32_t m_revents = 0; ///< m_fd已发生的事件。

        std::function<void()> m_readCb;  ///< m_fd读事件的回调函数。
        std::function<void()> m_closeCb; ///< 关闭m_fd的回调函数，将回调Connection::closeCb()。
        std::function<void()> m_errorCb; ///< m_fd发生了错误的回调函数，将回调Connection::errorCb()。
        std::function<void()> m_writeCb; ///< m_fd写事件的回调函数，将回调Connection::writeCb()。

    public:
        Channel(EventLoop* eventLoop, int fd); // 构造函数。
        ~Channel();                            // 析构函数。

        int getFd();           // 返回m_fd成员。
        bool getInEpoll();     // 返回m_inepoll成员。
        uint32_t getEvents();  // 返回m_events成员。
        uint32_t getRevents(); // 返回m_revents成员。

        void useET();          // 采用边缘触发。
        void enableReading();  // 让epoll_wait()监视m_fd的读事件。
        void disableReading(); // 取消读事件。
        void enableWriting();  // 注册写事件。
        void disableWriting(); // 取消写事件。
        void disableAll();     // 取消全部的事件。

        void remove(); // 从事件循环中删除Channel。

        void setInEpoll();            // 把m_inepoll成员的值设置为true。
        void setRevents(uint32_t ev); // 设置m_revents成员的值为参数ev。

        void setReadCb(std::function<void()> func);  // 设置m_fd读事件的回调函数。
        void setCloseCb(std::function<void()> func); // 设置关闭m_fd的回调函数。
        void setErrorCb(std::function<void()> func); // 设置m_fd发生了错误的回调函数。
        void setWriteCb(std::function<void()> func); // 设置写事件的回调函数。

        void handleEvent(); // 事件处理函数，epoll_wait()返回的时候，执行它。
    };
#endif // __linux__

} // namespace ol

#endif // !OL_CHANNEL_H