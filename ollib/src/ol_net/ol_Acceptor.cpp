#include "ol_net/ol_Acceptor.h"

namespace ol
{

#ifdef __unix__
    Acceptor::Acceptor(EventLoop* eventLoop, const std::string& ip, const uint16_t port)
        : m_eventLoop(eventLoop), m_servFd(createFdNonblocking()), m_acceptChnl(m_eventLoop, m_servFd.getFd())
    {
        InetAddr servAddr(ip, port); // 服务端的地址和协议。
        m_servFd.setKeepalive(true);
        m_servFd.setReuseaddr(true);
        m_servFd.setReuseport(true);
        m_servFd.setTcpnodelay(true);
        m_servFd.bind(servAddr);
        m_servFd.listen();

        m_acceptChnl.setReadCb(std::bind(&Acceptor::newConn, this));
        m_acceptChnl.enableReading(); // 让epoll_wait()监视m_acceptChnl的读事件。
    }

    Acceptor::~Acceptor()
    {
    }

    // 设置处理新客户端连接请求的回调函数，将在创建Acceptor对象的时候（TcpServer类的构造函数中）设置。
    void Acceptor::setNewConnCb(std::function<void(SocketFd::Ptr)> func)
    {
        m_newConnCb = func;
    }

    // 处理新客户端连接请求。
    void Acceptor::newConn()
    {
        InetAddr cliAddr; // 客户端的地址和协议。
        SocketFd::Ptr cliFd = std::make_unique<SocketFd>(m_servFd.accept(cliAddr));

        cliFd->setAddr(cliAddr);

        m_newConnCb(std::move(cliFd)); // 回调TcpServer::newConn()
    }
#endif // __unix__

} // namespace ol