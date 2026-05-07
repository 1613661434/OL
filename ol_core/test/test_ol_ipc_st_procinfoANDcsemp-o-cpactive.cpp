/*
 *  程序名：test_ol_ipc_st_procinfoANDcsemp-o-cpactive.cpp，此程序演示进程的心跳，之后会把里面的部分代码封装成类cpactive。
 *  作者：ol
 */

#if !defined(__unix__)
#error "仅支持Linux平台，不支持当前系统！"
#endif

#include "ol_public.h"

using namespace ol;
using namespace std;

int m_shmid = -1;             // 共享内存id
st_procinfo* m_shm = nullptr; // 共享内存指针
int m_pos = -1;               // 共享内存中当前进程心跳的位置

void EXIT(int sig);

int main()
{
    // 设置信号退出函数
    signal(SIGINT, EXIT);
    signal(SIGTERM, EXIT);

    // 获取共享内存id
    if ((m_shmid = shmget((key_t)0x5001, sizeof(st_procinfo) * 1000, 0640 | IPC_CREAT)) == -1)
    {
        cerr << "shmget(0x5001) failed.\n";
        return -1;
    }

    // 将共享内存连接到当前进程地址空间
    if ((m_shm = (st_procinfo*)shmat(m_shmid, 0, 0)) == (void*)-1)
    {
        cerr << "shmat() failed\n";
        return -1;
    }

    // 把共享内存中全部进程信息显示，用于调试
    for (int i = 0; i < 1000; ++i)
    {
        if (m_shm[i].m_pid != 0)
        {
            printf("i=%d,m_pid=%d,m_pname=%s,m_timeout=%d,m_atime=%ld\n",
                   i, m_shm[i].m_pid, m_shm[i].m_pname, m_shm[i].m_timeout, m_shm[i].m_atime);
        }
    }

    // 初始化进程心跳
    st_procinfo procinfo(getpid(), "server", 30, time(0));

    // 创建信号量给共享内存加锁
    // 只需要给进程找共享内存空位置的时候加锁，其它情况不需要，因为都只会操作自己的位置
    csemp shmlock;

    if (shmlock.init(0x5001) == false)
    {
        cerr << "创建/获取信号量0x5001失败\n";
        EXIT(-1);
    }

    // 查找共享内存的空位置
    // 注意：进程id是循环使用的，如果曾经有一个进程异常退出，没有清理自己的心跳信息，
    // 它的进程信息将残留在共享内存中，不巧的是，如果当前进程重用了它的id，
    // 所以，如果是共享内存中已存在当前进程编号，一定是其它进程残留的信息，当前进程应该重用这个位置
    shmlock.wait(); // 加锁

    for (int i = 0; i < 1000; ++i)
    {
        if (m_shm[i].m_pid == procinfo.m_pid) // 如果有重，直接重用
        {
            m_pos = i;
            cout << "找到新位置" << i << "，且位置有重\n";
            break;
        }

        if (m_shm[i].m_pid != 0) continue;
        if (m_pos != -1) continue;
        m_pos = i;
        cout << "找到新位置" << i << "\n";
    }

    // 没有找到
    if (m_pos == -1)
    {
        shmlock.post(); // 失败也要解锁
        cerr << "共享内存空间已用完\n";
        EXIT(-1);
    }

    // 将进程心跳复制到共享内存中
    memcpy(&m_shm[m_pos], &procinfo, sizeof(st_procinfo));

    shmlock.post(); // 解锁

    // 执行服务程序
    while (true)
    {
        cout << "服务程序正在运行中..\n";
        sleep(5);

        // 实时更新进程的心跳时间
        m_shm[m_pos].m_atime = time(0);
    }

    return 0;
}

// 程序退出和信号2,15的处理函数
void EXIT(int sig)
{
    cerr << "sig=" << sig << "\n";

    // 从共享内存中删除当前进程的心跳信息
    if (m_pos != -1) memset(&m_shm[m_pos], 0, sizeof(st_procinfo));

    // 从进程中分离共享内存
    if (m_shm != nullptr) shmdt(m_shm);

    // 退出
    exit(0);
}