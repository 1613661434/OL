// 回显业务的客户端程序。

#if !defined(__linux__)
#error "仅支持Linux平台，不支持当前系统！"
#endif

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define MessageNum 100000
// #define DEBUG

void sendMessage(int fd, char* buf, int bufLen, int i);

void recvMessage(int fd, char* buf, int bufLen, int i);

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("Usage: ./test_ol_echoclient ip port\n");
        printf("Example: ./test_ol_echoclient 192.168.195.5 5085\n\n");
        return -1;
    }

    int sockfd;
    struct sockaddr_in servaddr;
    char buf[1024];

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("socket() failed.\n");
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connect(%s:%s) failed.\n", argv[1], argv[2]);
        close(sockfd);
        return -1;
    }

    printf("connect ok.\n");
    printf("开始时间：%ld\n", time(nullptr));

    // 业务
    for (int i = 1; i <= MessageNum; ++i)
    {
        sendMessage(sockfd, buf, sizeof(buf), i);
        recvMessage(sockfd, buf, sizeof(buf), i);
    }

    // 显式关闭套接字
    close(sockfd);

    // 打印结束时间并强制刷新缓冲区
    printf("结束时间：%ld\n", time(nullptr));
    fflush(stdout); // 强制刷新，确保输出

    // 显式退出
    return 0;
}

void sendMessage(int fd, char* buf, int bufLen, int i)
{
    memset(buf, 0, bufLen);
    char message[1024];
    sprintf(message, "第%d个超女", i);
    int len = strlen(message);

#ifdef DEBUG
    printf("send:%s\n", message);
#endif

    memcpy(buf, &len, 4);
    memcpy(buf + 4, message, len);

    if (send(fd, buf, len + 4, 0) <= 0)
    {
        printf("write() failed.\n");
        close(fd);
        exit(-1);
    }
}

void recvMessage(int fd, char* buf, int bufLen, int i)
{
    memset(buf, 0, bufLen);
    int len;

    // 读取长度（注意：实际应循环读取确保4字节完整接收）
    if (recv(fd, &len, 4, 0) != 4)
    {
        printf("recv len failed (i=%d).\n", i);
        close(fd);
        exit(-1);
    }

    // 读取消息内容（同样需循环读取确保完整）
    int recv_len = 0;
    while (recv_len < len)
    {
        int n = recv(fd, buf + recv_len, len - recv_len, 0);
        if (n <= 0)
        {
            printf("recv data failed (i=%d).\n", i);
            close(fd);
            exit(-1);
        }
        recv_len += n;
    }
#ifdef DEBUG
    printf("recv:%s\n", buf);
#endif
}