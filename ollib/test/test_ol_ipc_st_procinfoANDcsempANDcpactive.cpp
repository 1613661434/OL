/*
 *  程序名：test_ol_ipc_st_procinfoANDcsempANDcpactive.cpp，此程序演示进程的心跳，直接使用类cpactive。
 *  作者：ol
 */

#if !defined(__linux__)
#error "仅支持Linux平台，不支持当前系统！"
#endif

#include "ol_public.h"

using namespace ol;
using namespace std;

cpactive pactive; // 进程心跳，用全局对象（保证析构函数会被调用）

void EXIT(int sig)
{
    printf("sig=%d", sig);

    exit(0);
}

int main()
{
    // 处理程序的退出信号
    signal(SIGINT, EXIT);
    signal(SIGTERM, EXIT);

    if (pactive.addpinfo(30, "server") == false)
    {
        cerr << "失效\n";
        return -1;
    }

    // 执行服务程序
    while (true)
    {
        cout << "服务程序正在运行中..\n";
        sleep(5);

        // 实时更新进程的心跳时间
        pactive.uptatime();
    }

    return 0;
}