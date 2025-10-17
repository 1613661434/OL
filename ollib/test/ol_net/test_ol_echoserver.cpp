/*
 * 程序名：test_ol_echoserver.cpp，回显（EchoServer）服务器。
 * 作者：ol
 */

#if !defined(__linux__)
#error "仅支持Linux平台，不支持当前系统！"
#endif

#include "ol_EchoServer.h"
#include <signal.h>

using namespace ol;

// 1、设置2和15的信号。
// 2、在信号处理函数中停止主从事件循环和工作线程。
// 3、服务程序主动退出。

EchoServer* echoServ;

void stop(int sig) // 信号2和15的处理函数，功能是停止服务程序。
{
    printf("sig=%d\n", sig);
    // 调用EchoServer::stop()停止服务。
    echoServ->stop();
    printf("EchoServer has stopped.\n");
    delete echoServ;
    printf("EchoServer has been deleted.\n");
    exit(0);
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("Usage: ./ol_test_echoserver ip port\n");
        printf("Example: ./ol_test_echoserver 192.168.195.5 5085\n\n");
        return -1;
    }

    signal(SIGTERM, stop); // 信号15，系统kill或killall命令默认发送的信号。
    signal(SIGINT, stop);  // 信号2，按Ctrl+C发送的信号。

    // Args
    size_t workThreadNum = 0;
    size_t subThreadNum = 3;
    size_t MainMaxEvents = 100;
    size_t SubMaxEvents = 100;
    int epWaitTimeout = 10000;
    int timerTimetvl = 100;
    int timerTimeout = 100;

    /* EchoServer(
        const std::string& ip, const uint16_t port,
        size_t workThreadNum = 2, size_t subThreadNum = 3,
        size_t MainMaxEvents = 100, size_t SubMaxEvents = 100,
        int epWaitTimeout = 10000, int timerTimetvl = 5, int timerTimeout = 10
        );
    */
    echoServ = new EchoServer(argv[1], atoi(argv[2]), workThreadNum, subThreadNum, MainMaxEvents, SubMaxEvents, epWaitTimeout, timerTimetvl, timerTimeout);
    echoServ->start(); // 运行事件循环。

    return 0;
}