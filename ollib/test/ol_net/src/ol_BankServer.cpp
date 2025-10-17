#include "ol_BankServer.h"

#ifdef __linux__
#include <sys/syscall.h>
#include <unistd.h>
#endif // __linux__

// #define DEBUG
// #define DEBUG2

namespace ol
{

#ifdef __linux__
    BankServer::BankServer(const std::string& ip, const uint16_t port, size_t workThreadNum, size_t subThreadNum, size_t MainMaxEvents, size_t SubMaxEvents, int epWaitTimeout, int timerTimetvl, int timerTimeout)
        : m_tcpServ(ip, port, subThreadNum, MainMaxEvents, SubMaxEvents, epWaitTimeout, timerTimetvl, timerTimeout), m_threadPool(workThreadNum, 0)
    {
        // 以下代码不是必须的，业务关心什么事件，就指定相应的回调函数。
        m_tcpServ.setNewConnCb(std::bind(&BankServer::handleNewConn, this, std::placeholders::_1));
        m_tcpServ.setCloseCb(std::bind(&BankServer::handleClose, this, std::placeholders::_1));
        m_tcpServ.setErrorCb(std::bind(&BankServer::handleError, this, std::placeholders::_1));
        m_tcpServ.setOnMessageCb(std::bind(&BankServer::handleMessage, this, std::placeholders::_1, std::placeholders::_2));
        // m_tcpServ.setSendCompleteCb(std::bind(&BankServer::handleSendComplete, this, std::placeholders::_1));
        // m_tcpServ.setTimeoutCb(std::bind(&BankServer::handleTimeOut, this, std::placeholders::_1));
        m_tcpServ.setTimerTimeoutCb(std::bind(&BankServer::handleTimerTimeOut, this, std::placeholders::_1));
    }

    BankServer::~BankServer()
    {
    }

    // 启动服务。
    void BankServer::start(int newConnTimeout)
    {
        m_tcpServ.start(newConnTimeout);
    }

    // 停止服务。
    void BankServer::stop()
    {
        // 停止工作线程。
        m_threadPool.stop();
#ifdef DEBUG
        printf("工作线程已停止。\n");
#endif

        // 停止TcpServer。
        m_tcpServ.stop();
    }

    // 处理新客户端连接请求，在TcpServer类中回调此函数。
    void BankServer::handleNewConn(Connection::Ptr conn)
    {
        // 新客户端连上来时，把用户连接的信息保存到状态机中。
        UserInfo::Ptr userinfo(new UserInfo(conn->getFd(), conn->getIp()));
        {
            std::lock_guard<std::mutex> lock(m_userInfoMutex);
            m_userInfo_umap[conn->getFd()] = userinfo; // 把用户添加到状态机中。
        }
        printf("%s 新建连接（ip=%s）.\n", TimeStamp::now().toString().c_str(), conn->getIp());
    }

    // 关闭客户端的连接，在TcpServer类中回调此函数。
    void BankServer::handleClose(Connection::Ptr conn)
    {
        // 关闭客户端连接的时候，从状态机中删除客户端连接的信息。
        printf("%s 连接已断开（ip=%s）.\n", TimeStamp::now().toString().c_str(), conn->getIp());
        {
            std::lock_guard<std::mutex> gd(m_userInfoMutex);
            m_userInfo_umap.erase(conn->getFd()); // 从状态机中删除用户信息。
        }
    }

    // 客户端的连接错误，在TcpServer类中回调此函数。
    void BankServer::handleError(Connection::Ptr conn)
    {
        handleClose(conn);
    }

    // 处理客户端的请求报文，在TcpServer类中回调此函数。
    void BankServer::handleMessage(Connection::Ptr conn, std::string& message)
    {
#ifdef DEBUG
        printf("BankServer::handleMessage(%ld).\n", syscall(SYS_gettid));
#endif // DEBUG

        if (m_threadPool.getWorkerNum() == 0)
        {
            // 如果没有工作线程，表示在IO线程中计算
            onMessage(conn, message);
        }
        else
        {
            // 把业务添加到线程池的任务队列中。
            m_threadPool.addTask(std::bind(&BankServer::onMessage, this, conn, message));
        }
    }

    // 数据发送完成后，在TcpServer类中回调此函数。
    void BankServer::handleSendComplete(Connection::Ptr conn)
    {
        std::cout << "Message send complete." << std::endl;

        // 根据业务的需求，在这里可以增加其它的代码。
    }

    // epoll_wait()超时，在TcpServer类中回调此函数。
    void BankServer::handleTimeOut(EventLoop* eventLoop)
    {
        std::cout << "BankServer timeout." << std::endl;

        // 根据业务的需求，在这里可以增加其它的代码。
    }

    // 客户端的连接超时，在TcpServer类中回调此函数。
    void BankServer::handleTimerTimeOut(int fd)
    {
        printf("fd(%d) 已超时。\n", fd);

        std::lock_guard<std::mutex> lock(m_userInfoMutex);
        m_userInfo_umap.erase(fd); // 从状态机中删除用户信息。
    }

    // 处理客户端的请求报文，用于添加给线程池。
    void BankServer::onMessage(Connection::Ptr conn, std::string& message)
    {
        UserInfo::Ptr userinfo = m_userInfo_umap[conn->getFd()]; // 从状态机中获取客户端的信息。

        // 解析客户端的请求报文，处理各种业务。
        // <bizcode>00101</bizcode><username>ol</username><password>123456</password>
        std::string bizcode;                   // 业务代码。
        std::string replaymessage;             // 回应报文。
        getByXml(message, "bizcode", bizcode); // 从请求报文中解析出业务代码。

        if (bizcode == "00101") // 登录业务。
        {
            std::string username, password;
            getByXml(message, "username", username);          // 解析用户名。
            getByXml(message, "password", password);          // 解析密码。
            if ((username == "ol") && (password == "123456")) // 假设从数据库或Redis中查询用户名和密码。
            {
                // 用户名和密码正确。
                replaymessage = "<bizcode>00102</bizcode><retcode>0</retcode><message>ok</message>";
                userinfo->setLogin(true); // 设置用户的登录状态为true。
            }
            else
            {
                // 用户名和密码不正确。
                replaymessage = "<bizcode>00102</bizcode><retcode>-1</retcode><message>用户名或密码不正确。</message>";
            }
        }
        else if (bizcode == "00201") // 查询余额业务。
        {
            if (userinfo->login() == true)
            {
                // 把用户的余额从数据库或Redis中查询出来。
                replaymessage = "<bizcode>00202</bizcode><retcode>0</retcode><message>5088.80</message>";
            }
            else
            {
                replaymessage = "<bizcode>00202</bizcode><retcode>-1</retcode><message>用户未登录。</message>";
            }
        }
        else if (bizcode == "00901") // 注销业务。
        {
            if (userinfo->login() == true)
            {
                replaymessage = "<bizcode>00902</bizcode><retcode>0</retcode><message>ok</message>";
                userinfo->setLogin(false); // 设置用户的登录状态为false。
            }
            else
            {
                replaymessage = "<bizcode>00902</bizcode><retcode>-1</retcode><message>用户未登录。</message>";
            }
        }
        else if (bizcode == "00001") // 心跳。
        {
            if (userinfo->login() == true)
            {
                replaymessage = "<bizcode>00002</bizcode><retcode>0</retcode><message>ok</message>";
            }
            else
            {
                replaymessage = "<bizcode>00002</bizcode><retcode>-1</retcode><message>用户未登录。</message>";
            }
        }

        conn->send(replaymessage.data(), replaymessage.size()); // 把数据发送出去。
    }
#endif // __linux__

} // namespace ol