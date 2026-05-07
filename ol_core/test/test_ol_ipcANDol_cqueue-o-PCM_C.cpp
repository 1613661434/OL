/*
 *  程序名：test_ol_ipcANDol_cqueue-o-PCM_C.cpp，此程序演示生产者-消费者模型的消费者。
 *  作者：ol
 */

#if !defined(__unix__)
#error "仅支持Linux平台，不支持当前系统！"
#endif

#include "ol_cqueue.h"
#include "ol_ipc.h"
#include <iostream>
#include <signal.h>

using namespace ol;
using namespace std;

struct Sgirl
{
    int id;
    char name[51];

    Sgirl(int i, char* n)
    {
        id = i;
        strcpy(name, n);
    }
};

// 全局变量：供信号处理函数访问
using shmtype = cqueue<Sgirl, 15>;
int g_shmid = -1;
shmtype* g_CQptr = nullptr;
csemp g_mutex;
csemp g_hav_ele;
csemp g_hav_emp;

// 信号处理函数
void Exit(int signo)
{
    // 脱离共享内存
    if (g_CQptr != nullptr)
        shmdt(g_CQptr);

    // 删除共享内存
    if (g_shmid != -1 && shmctl(g_shmid, IPC_RMID, 0) == -1)
    {
        cerr << "shmctl(IPC_RMID) fail\n";
        exit(-1);
    }

    // 销毁信号量
    g_mutex.destroy();
    g_hav_ele.destroy();
    g_hav_emp.destroy();

    exit(0);
}

int main()
{
    // 注册信号处理函数：捕获Ctrl+C(SIGINT)和终止信号(SIGTERM)
    signal(SIGINT, Exit);
    signal(SIGTERM, Exit);

    // 获取共享内存id
    g_shmid = shmget(0x5005, sizeof(shmtype), 0640 | IPC_CREAT);
    if (g_shmid == -1)
    {
        cerr << "shmget() fail\n";
        return -1;
    }

    // 将共享内存连接到当前进程的地址空间
    g_CQptr = (shmtype*)shmat(g_shmid, 0, 0);
    if (g_CQptr == (void*)-1)
    {
        cerr << "shmat() fail\n";
        return -1;
    }

    // 循环队列初始化
    g_CQptr->init();

    // 设置信号量
    // 互斥锁
    g_mutex.init(0x5001);

    // 资源数
    g_hav_ele.init(0x5002, 0, 0);

    // 空位数
    g_hav_emp.init(0x5003, 5, 0);

    while (true)
    {
        // 资源数-1
        g_hav_ele.wait();

        // 上锁
        g_mutex.wait();

        // 消费数据1
        // 双重检查队列是否为空（防止信号量与队列状态不一致）
        if (g_CQptr->empty())
        {
            g_mutex.post();   // 释放互斥锁
            g_hav_emp.post(); // 释放空槽信号量（抵消之前的hav_ele.wait()）
            continue;
        }

        Sgirl sg = g_CQptr->front();
        g_CQptr->pop();
        cout << "id=" << sg.id << ",name=" << sg.name << "\n";

        // 开锁
        g_mutex.post();

        // 空位数+1
        g_hav_emp.post();

        usleep(100); // 控制消费频率
    }

    return 0;
}