#ifndef __OL_TCP_H
#define __OL_TCP_H 1

#include "../include/ol_fstream.h"
#include <iostream>
#include <signal.h>
#include <string>

#ifdef __linux__
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/sem.h> // ���� SEM_UNDO �������ź�����غ���
#include <sys/shm.h>
#include <sys/socket.h>
#endif // __linux__

namespace ol
{

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // ======================
    // socketͨѶ�ĺ�������
    // ======================
#ifdef __linux__
    // socketͨѶ�Ŀͻ�����
    class ctcpclient
    {
    private:
        int m_connfd;     // �ͻ��˵�socket.
        std::string m_ip; // ����˵�ip��ַ��
        int m_port;       // �����ͨѶ�Ķ˿ڡ�
    public:
        ctcpclient() : m_connfd(-1), m_port(0)
        {
        } // ���캯����

        // �����˷�����������
        // ip������˵�ip��ַ��
        // port�������ͨѶ�Ķ˿ڡ�
        // ����ֵ��true-�ɹ���false-ʧ�ܡ�
        bool connect(const std::string& ip, const int port);

        // ���նԶ˷��͹��������ݡ�
        // buffer����Ž������ݻ�������
        // ibuflen: ����������ݵĴ�С��
        // itimeout���ȴ����ݵĳ�ʱʱ�䣨�룩��-1-���ȴ���0-���޵ȴ���>0-�ȴ���������
        // ����ֵ��true-�ɹ���false-ʧ�ܣ�ʧ�������������1���ȴ���ʱ��2��socket�����Ѳ����á�
        bool read(std::string& buffer, const int itimeout = 0);             // �����ı����ݡ�
        bool read(void* buffer, const int ibuflen, const int itimeout = 0); // ���ն��������ݡ�

        // ��Զ˷������ݡ�
        // buffer�����������ݻ�������
        // ibuflen�����������ݵĴ�С��
        // ����ֵ��true-�ɹ���false-ʧ�ܣ����ʧ�ܣ���ʾsocket�����Ѳ����á�
        bool write(const std::string& buffer);             // �����ı����ݡ�
        bool write(const void* buffer, const int ibuflen); // ���Ͷ��������ݡ�

        // �Ͽ������˵�����
        void close();

        ~ctcpclient(); // ���������Զ��ر�socket���ͷ���Դ��
    };

    // socketͨѶ�ķ������
    class ctcpserver
    {
    private:
        int m_socklen;                   // �ṹ��struct sockaddr_in�Ĵ�С��
        struct sockaddr_in m_clientaddr; // �ͻ��˵ĵ�ַ��Ϣ��
        struct sockaddr_in m_servaddr;   // ����˵ĵ�ַ��Ϣ��
        int m_listenfd;                  // ��������ڼ�����socket��
        int m_connfd;                    // �ͻ�������������socket��
    public:
        ctcpserver() : m_listenfd(-1), m_connfd(-1)
        {
        } // ���캯����

        // ����˳�ʼ����
        // port��ָ����������ڼ����Ķ˿ڡ�
        // backlog��ָ��δ������Ӷ��е���󳤶ȣ�Ĭ��Ϊ5��
        // ����ֵ��true-�ɹ���false-ʧ�ܣ�һ������£�ֻҪport������ȷ��û�б�ռ�ã���ʼ������ɹ���
        bool initserver(const unsigned int port, const int backlog = 5);

        // �������Ӷ����л�ȡһ���ͻ������ӣ���������Ӷ���Ϊ�գ��������ȴ���
        // ����ֵ��true-�ɹ��Ļ�ȡ��һ���ͻ������ӣ�false-ʧ�ܣ����acceptʧ�ܣ���������accept��
        bool accept();

        // ��ȡ�ͻ��˵�ip��ַ��
        // ����ֵ���ͻ��˵�ip��ַ����"192.168.1.100"��
        char* getip();

        // ���նԶ˷��͹��������ݡ�
        // buffer����Ž������ݵĻ�������
        // ibuflen: ����������ݵĴ�С��
        // itimeout���ȴ����ݵĳ�ʱʱ�䣨�룩��-1-���ȴ���0-���޵ȴ���>0-�ȴ���������
        // ����ֵ��true-�ɹ���false-ʧ�ܣ�ʧ�������������1���ȴ���ʱ��2��socket�����Ѳ����á�
        bool read(std::string& buffer, const int itimeout = 0);             // �����ı����ݡ�
        bool read(void* buffer, const int ibuflen, const int itimeout = 0); // ���ն��������ݡ�

        // ��Զ˷������ݡ�
        // buffer�����������ݻ�������
        // ibuflen�����������ݵĴ�С��
        // ����ֵ��true-�ɹ���false-ʧ�ܣ����ʧ�ܣ���ʾsocket�����Ѳ����á�
        bool write(const std::string& buffer);             // �����ı����ݡ�
        bool write(const void* buffer, const int ibuflen); // ���Ͷ��������ݡ�

        // �رռ�����socket����m_listenfd�������ڶ���̷��������ӽ��̴����С�
        void closelisten();

        // �رտͻ��˵�socket����m_connfd�������ڶ���̷������ĸ����̴����С�
        void closeclient();

        ~ctcpserver(); // ���������Զ��ر�socket���ͷ���Դ��
    };

    // ����socket�ĶԶ˷��͹��������ݡ�
    // sockfd�����õ�socket���ӡ�
    // buffer���������ݻ������ĵ�ַ��
    // ibuflen�����γɹ��������ݵ��ֽ�����
    // itimeout����ȡ���ݳ�ʱ��ʱ�䣬��λ���룬-1-���ȴ���0-���޵ȴ���>0-�ȴ���������
    // ����ֵ��true-�ɹ���false-ʧ�ܣ�ʧ�������������1���ȴ���ʱ��2��socket�����Ѳ����á�
    bool tcpread(const int sockfd, std::string& buffer, const int itimeout = 0);             // ��ȡ�ı����ݡ�
    bool tcpread(const int sockfd, void* buffer, const int ibuflen, const int itimeout = 0); // ��ȡ���������ݡ�

    // ��socket�ĶԶ˷������ݡ�
    // sockfd�����õ�socket���ӡ�
    // buffer�����������ݻ������ĵ�ַ��
    // ibuflen�����������ݵ��ֽ�����
    // ����ֵ��true-�ɹ���false-ʧ�ܣ����ʧ�ܣ���ʾsocket�����Ѳ����á�
    bool tcpwrite(const int sockfd, const std::string& buffer);             // д���ı����ݡ�
    bool tcpwrite(const int sockfd, const void* buffer, const int ibuflen); // д����������ݡ�

    // ���Ѿ�׼���õ�socket�ж�ȡ���ݡ�
    // sockfd���Ѿ�׼���õ�socket���ӡ�
    // buffer��������ݵĵ�ַ��
    // n�����δ����ȡ���ݵ��ֽ�����
    // ����ֵ���ɹ����յ�n�ֽڵ����ݺ󷵻�true��socket���Ӳ����÷���false��
    bool readn(const int sockfd, char* buffer, const size_t n);

    // ���Ѿ�׼���õ�socket��д�����ݡ�
    // sockfd���Ѿ�׼���õ�socket���ӡ�
    // buffer����д�����ݵĵ�ַ��
    // n����д�����ݵ��ֽ�����
    // ����ֵ���ɹ�д����n�ֽڵ����ݺ󷵻�true��socket���Ӳ����÷���false��
    bool writen(const int sockfd, const char* buffer, const size_t n);
    ///////////////////////////////////// /////////////////////////////////////
#endif // __linux__

} // namespace ol

#endif // !__OL_TCP_H