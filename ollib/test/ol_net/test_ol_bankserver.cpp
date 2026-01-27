/*
 * 程序名：test_ol_bankserver.cpp，网上银行（BankServer）服务器。
 * 作者：ol
 */

#if !defined(__unix__)
#error "仅支持Linux平台，不支持当前系统！"
#endif

#include "ol_BankServer.h"
#include <signal.h>

using namespace ol;

// 1、设置2和15的信号。
// 2、在信号处理函数中停止主从事件循环和工作线程。
// 3、服务程序主动退出。

BankServer* echoServ;

void stop(int sig) // 信号2和15的处理函数，功能是停止服务程序。
{
    printf("sig=%d\n", sig);
    // 调用BankServer::stop()停止服务。
    echoServ->stop();
    printf("BankServer has stopped.\n");
    delete echoServ;
    printf("BankServer has been deleted.\n");
    exit(0);
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("Usage: ./test_ol_Bankserver ip port\n");
        printf("Example: ./test_ol_Bankserver 192.168.195.5 5085\n\n");
        return -1;
    }

    signal(SIGTERM, stop); // 信号15，系统kill或killall命令默认发送的信号。
    signal(SIGINT, stop);  // 信号2，按Ctrl+C发送的信号。

    // Args
    size_t workThreadNum = 0;
    size_t subThreadNum = 1;
    size_t MainMaxEvents = 100;
    size_t SubMaxEvents = 100;
    int epWaitTimeout = 10000;
    int timerTimetvl = 15;
    int timerTimeout = 30;

    /* BankServer(
        const std::string& ip, const uint16_t port,
        size_t workThreadNum = 2, size_t subThreadNum = 3,
        size_t MainMaxEvents = 100, size_t SubMaxEvents = 100,
        int epWaitTimeout = 10000, int timerTimetvl = 5, int timerTimeout = 10
        );
    */
    echoServ = new BankServer(argv[1], atoi(argv[2]), workThreadNum, subThreadNum, MainMaxEvents, SubMaxEvents, epWaitTimeout, timerTimetvl, timerTimeout);
    echoServ->start(); // 运行事件循环。

    return 0;
}