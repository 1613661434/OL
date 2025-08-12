#include "../include/ol_tcp.h"

namespace ol
{

#ifdef __linux__
    bool ctcpclient::connect(const std::string& ip, const int port)
    {
        // ��������ӵ�����ˣ���Ͽ������ִ�����û���ر��ԭ�򣬲�Ҫ���ᡣ
        if (m_connfd != -1)
        {
            ::close(m_connfd);
            m_connfd = -1;
        }

        // ����SIGPIPE�źţ���ֹ�����쳣�˳���
        // ���send��һ��disconnected socket�ϣ��ں˾ͻᷢ��SIGPIPE�źš�����ź�
        // ��ȱʡ����������ֹ���̣������ʱ���ⶼ�������������ġ��������¶�����
        // ���źŵĴ�����������������ֱ����������
        signal(SIGPIPE, SIG_IGN);

        m_ip = ip;
        m_port = port;

        struct hostent* h;
        struct sockaddr_in servaddr;

        if ((m_connfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) return false;

        if (!(h = gethostbyname(m_ip.c_str())))
        {
            ::close(m_connfd);
            m_connfd = -1;
            return false;
        }

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(m_port); // ָ������˵�ͨѶ�˿�
        memcpy(&servaddr.sin_addr, h->h_addr, h->h_length);

        if (::connect(m_connfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0)
        {
            ::close(m_connfd);
            m_connfd = -1;
            return false;
        }

        return true;
    }

    void ctcpclient::close()
    {
        if (m_connfd >= 0) ::close(m_connfd);

        m_connfd = -1;
        m_port = 0;
    }

    ctcpclient::~ctcpclient()
    {
        close();
    }

    bool ctcpserver::initserver(const unsigned int port, const int backlog)
    {
        // �������˵�socket>0���ص��������ִ�����û���ر��ԭ�򣬲�Ҫ���ᡣ
        if (m_listenfd > 0)
        {
            ::close(m_listenfd);
            m_listenfd = -1;
        }

        if ((m_listenfd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) return false;

        // ����SIGPIPE�źţ���ֹ�����쳣�˳���
        // ������ѹرյ�socket����д���ݣ������SIGPIPE�źţ�����ȱʡ��Ϊ����ֹ��������Ҫ��������
        signal(SIGPIPE, SIG_IGN);

        // ��SO_REUSEADDRѡ�����������Ӵ���TIME_WAIT״̬ʱ�����ٴ�������������
        // ����bind()���ܻ᲻�ɹ�������Address already in use��
        int opt = 1;
        setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        memset(&m_servaddr, 0, sizeof(m_servaddr));
        m_servaddr.sin_family = AF_INET;
        m_servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // ����ip��ַ��
        m_servaddr.sin_port = htons(port);
        if (bind(m_listenfd, (struct sockaddr*)&m_servaddr, sizeof(m_servaddr)) != 0)
        {
            closelisten();
            return false;
        }

        if (listen(m_listenfd, backlog) != 0)
        {
            closelisten();
            return false;
        }

        return true;
    }

    bool ctcpserver::accept()
    {
        if (m_listenfd == -1) return false;

        int m_socklen = sizeof(struct sockaddr_in);
        if ((m_connfd = ::accept(m_listenfd, (struct sockaddr*)&m_clientaddr, (socklen_t*)&m_socklen)) < 0)
            return false;

        return true;
    }

    char* ctcpserver::getip()
    {
        return (inet_ntoa(m_clientaddr.sin_addr));
    }

    bool ctcpserver::read(void* buffer, const int ibuflen, const int itimeout) // ���ն��������ݡ�
    {
        if (m_connfd == -1) return false;

        return (tcpread(m_connfd, buffer, ibuflen, itimeout));
    }

    bool ctcpserver::read(std::string& buffer, const int itimeout) // �����ı����ݡ�
    {
        if (m_connfd == -1) return false;

        return (tcpread(m_connfd, buffer, itimeout));
    }

    bool ctcpclient::read(void* buffer, const int ibuflen, const int itimeout) // ���ն��������ݡ�
    {
        if (m_connfd == -1) return false;

        return (tcpread(m_connfd, buffer, ibuflen, itimeout));
    }

    bool ctcpclient::read(std::string& buffer, const int itimeout) // �����ı����ݡ�
    {
        if (m_connfd == -1) return false;

        return (tcpread(m_connfd, buffer, itimeout));
    }

    bool ctcpserver::write(const void* buffer, const int ibuflen) // ���Ͷ��������ݡ�
    {
        if (m_connfd == -1) return false;

        return (tcpwrite(m_connfd, (char*)buffer, ibuflen));
    }

    bool ctcpserver::write(const std::string& buffer)
    {
        if (m_connfd == -1) return false;

        return (tcpwrite(m_connfd, buffer));
    }

    bool ctcpclient::write(const void* buffer, const int ibuflen)
    {
        if (m_connfd == -1) return false;

        return (tcpwrite(m_connfd, (char*)buffer, ibuflen));
    }

    bool ctcpclient::write(const std::string& buffer)
    {
        if (m_connfd == -1) return false;

        return (tcpwrite(m_connfd, buffer));
    }

    void ctcpserver::closelisten()
    {
        if (m_listenfd >= 0)
        {
            ::close(m_listenfd);
            m_listenfd = -1;
        }
    }

    void ctcpserver::closeclient()
    {
        if (m_connfd >= 0)
        {
            ::close(m_connfd);
            m_connfd = -1;
        }
    }

    ctcpserver::~ctcpserver()
    {
        closelisten();
        closeclient();
    }

    bool tcpread(const int sockfd, void* buffer, const int ibuflen, const int itimeout) // ���ն��������ݡ�
    {
        if (sockfd == -1) return false;

        // ���itimeout>0����ʾ��Ҫ�ȴ�itimeout�룬���itimeout���û�����ݵ������false��
        if (itimeout > 0)
        {
            struct pollfd fds;
            fds.fd = sockfd;
            fds.events = POLLIN;
            if (poll(&fds, 1, itimeout * 1000) <= 0) return false;
        }

        // ���itimeout==-1����ʾ���ȴ��������ж�socket�Ļ��������Ƿ������ݣ����û�У�����false��
        if (itimeout == -1)
        {
            struct pollfd fds;
            fds.fd = sockfd;
            fds.events = POLLIN;
            if (poll(&fds, 1, 0) <= 0) return false;
        }

        // ��ȡ�������ݡ�
        if (readn(sockfd, (char*)buffer, ibuflen) == false) return false;

        return true;
    }

    bool tcpread(const int sockfd, std::string& buffer, const int itimeout) // �����ı����ݡ�
    {
        if (sockfd == -1) return false;

        // ���itimeout>0����ʾ�ȴ�itimeout�룬���itimeout�����ջ������л�û�����ݣ�����false��
        if (itimeout > 0)
        {
            struct pollfd fds;
            fds.fd = sockfd;
            fds.events = POLLIN;
            if (poll(&fds, 1, itimeout * 1000) <= 0) return false;
        }

        // ���itimeout==-1����ʾ���ȴ��������ж�socket�Ľ��ջ��������Ƿ������ݣ����û�У�����false��
        if (itimeout == -1)
        {
            struct pollfd fds;
            fds.fd = sockfd;
            fds.events = POLLIN;
            if (poll(&fds, 1, 0) <= 0) return false;
        }

        int buflen = 0;

        // �ȶ�ȡ���ĳ��ȣ�4���ֽڡ�
        if (readn(sockfd, (char*)&buflen, 4) == false) return false;

        buffer.resize(buflen); // ����buffer�Ĵ�С��

        // �ٶ�ȡ�������ݡ�
        if (readn(sockfd, &buffer[0], buflen) == false) return false;

        return true;
    }

    bool tcpwrite(const int sockfd, const void* buffer, const int ibuflen) // ���Ͷ��������ݡ�
    {
        if (sockfd == -1) return false;

        if (writen(sockfd, (char*)buffer, ibuflen) == false) return false;

        return true;
    }

    bool tcpwrite(const int sockfd, const std::string& buffer) // �����ı����ݡ�
    {
        if (sockfd == -1) return false;

        int buflen = buffer.size();

        // �ȷ��ͱ�ͷ��
        if (writen(sockfd, (char*)&buflen, 4) == false) return false;

        // �ٷ��ͱ����塣
        if (writen(sockfd, buffer.c_str(), buflen) == false) return false;

        return true;
    }

    // ���Ѿ�׼���õ�socket�ж�ȡ���ݡ�
    // sockfd���Ѿ�׼���õ�socket���ӡ�
    // buffer���������ݻ������ĵ�ַ��
    // n�����ν������ݵ��ֽ�����
    // ����ֵ���ɹ����յ�n�ֽڵ����ݺ󷵻�true��socket���Ӳ����÷���false��
    bool readn(const int sockfd, char* buffer, const size_t n)
    {
        int nleft = n; // ʣ����Ҫ��ȡ���ֽ�����
        int idx = 0;   // �ѳɹ���ȡ���ֽ�����
        int nread;     // ÿ�ε���recv()�����������ֽ�����

        while (nleft > 0)
        {
            if ((nread = recv(sockfd, buffer + idx, nleft, 0)) <= 0) return false;

            idx = idx + nread;
            nleft = nleft - nread;
        }

        return true;
    }

    // ���Ѿ�׼���õ�socket��д�����ݡ�
    // sockfd���Ѿ�׼���õ�socket���ӡ�
    // buffer�����������ݻ������ĵ�ַ��
    // n�����������ݵ��ֽ�����
    // ����ֵ���ɹ�������n�ֽڵ����ݺ󷵻�true��socket���Ӳ����÷���false��
    bool writen(const int sockfd, const char* buffer, const size_t n)
    {
        int nleft = n; // ʣ����Ҫд����ֽ�����
        int idx = 0;   // �ѳɹ�д����ֽ�����
        int nwritten;  // ÿ�ε���send()����д����ֽ�����

        while (nleft > 0)
        {
            if ((nwritten = send(sockfd, buffer + idx, nleft, 0)) <= 0) return false;

            nleft = nleft - nwritten;
            idx = idx + nwritten;
        }

        return true;
    }
#endif // __linux__

} // namespace ol