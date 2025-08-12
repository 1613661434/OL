#ifndef __OL_IPC_H
#define __OL_IPC_H 1

#include "../include/ol_fstream.h"

#ifdef __linux__
#include <sys/ipc.h>
#include <sys/sem.h> // 定义 SEM_UNDO 常量和信号量相关函数
#include <sys/shm.h>
#endif // __linux__

namespace ol
{

#ifdef __linux__
    ///////////////////////////////////// /////////////////////////////////////
    // 查看共享内存：  ipcs -m
    // 删除共享内存：  ipcrm -m shmid
    // 查看信号量：      ipcs -s
    // 删除信号量：      ipcrm sem semid

    // 信号量。
    class csemp
    {
    private:
        union semun // 用于信号量操作的共同体。
        {
            int val;
            struct semid_ds* buf;
            unsigned short* arry;
        };

        int m_semid; // 信号量id（描述符）。

        // 如果把sem_flg设置为SEM_UNDO，操作系统将跟踪进程对信号量的修改情况，
        // 在全部修改过信号量的进程（正常或异常）终止后，操作系统将把信号量恢复为初始值。
        // 如果信号量用于互斥锁，设置为SEM_UNDO。
        // 如果信号量用于生产消费者模型，设置为0。
        short m_sem_flg;

        csemp(const csemp&) = delete;            // 禁用拷贝构造函数。
        csemp& operator=(const csemp&) = delete; // 禁用赋值函数。
    public:
        csemp() : m_semid(-1)
        {
        }

        // 如果信号量已存在，获取信号量；如果信号量不存在，则创建它并初始化为value。
        // 如果用于互斥锁，value填1，sem_flg填SEM_UNDO。
        // 如果用于生产消费者模型，value填0，sem_flg填0。
        bool init(key_t key, unsigned short value = 1, short sem_flg = SEM_UNDO);
        bool wait(short value = -1); // 信号量的P操作，如果信号量的值是0，将阻塞等待，直到信号量的值大于0。
        bool post(short value = 1);  // 信号量的V操作。
        bool getvalue(int& value);   // 获取信号量的值，成功返回true，失败返回false。
        bool isValid() const;        // 判断信号量是否有效。
        bool destroy();              // 销毁信号量。
        ~csemp();
    };
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // 以下几个宏用于进程的心跳。
#define SHMKEYP 0x5095  // 共享内存的key。
#define SEMPKEYP 0x5095 // 信号量的key。
#define MAXNUMP 1000    // 最大的进程数量。

    // 进程心跳信息的结构体。
    struct st_procinfo
    {
        int m_pid = 0;           // 进程id。
        char m_pname[51] = {0};  // 进程名称，可以为空。
        int m_timeout = 0;       // 超时时间，单位：秒。
        time_t m_atime = 0;      // 最后一次心跳的时间，用整数表示。
        st_procinfo() = default; // 有了自定义的构造函数，编译器将不提供默认构造函数，所以启用默认构造函数。
        st_procinfo(const int pid, const std::string& pname, const int timeout, const time_t atime)
            : m_pid(pid), m_timeout(timeout), m_atime(atime)
        {
            strncpy(m_pname, pname.c_str(), 50);
        }
    };

    // 进程心跳操作类。
    class cpactive
    {
    private:
        int m_shmid = 0;              // 共享内存的id。
        int m_pos = -1;               // 当前进程在共享内存进程组中的位置。
        st_procinfo* m_shm = nullptr; // 指向共享内存的地址空间。

    public:
        cpactive(); // 初始化成员变量。

        // 把当前进程的信息加入共享内存进程组中。
        bool addpinfo(const int timeout, const std::string& pname = "", clogfile* logfile = nullptr, key_t SHM_KEY = SHMKEYP, key_t SEMP_KEY = SEMPKEYP, size_t MAX_SIZE_P = MAXNUMP);

        // 更新共享内存进程组中当前进程的心跳时间。
        bool uptatime();

        ~cpactive(); // 从共享内存中删除当前进程的心跳记录。
    };
    ///////////////////////////////////// /////////////////////////////////////
#endif // __linux__

} // namespace ol

#endif // !__OL_IPC_H