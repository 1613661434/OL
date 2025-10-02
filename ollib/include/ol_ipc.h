/****************************************************************************************/
/*
 * 程序名：ol_ipc.h
 * 功能描述：Linux进程间通信（IPC）工具类，支持以下特性：
 *          - 信号量操作类（csemp）：提供P/V操作、信号量创建与销毁
 *          - 进程心跳管理类（cpactive）：基于共享内存和信号量实现进程存活监控
 *          - 仅支持Linux平台（依赖sys/ipc.h、sys/sem.h等系统头文件）
 * 作者：ol
 * 适用标准：C++11及以上（需支持Linux系统调用）
 */
/****************************************************************************************/

#ifndef OL_IPC_H
#define OL_IPC_H 1

#include "ol_fstream.h"

#ifdef __linux__
#include <sys/ipc.h>
#include <sys/sem.h> // 定义 SEM_UNDO 常量和信号量相关函数
#include <sys/shm.h>
#endif // __linux__

namespace ol
{

#ifdef __linux__
    // ===========================================================================
    // Linux命令
    // 查看共享内存：  ipcs -m
    // 删除共享内存：  ipcrm -m shmid
    // 查看信号量：    ipcs -s
    // 删除信号量：    ipcrm sem semid

    /**
     * @brief 信号量操作类，用于进程间同步与互斥
     * @note 仅支持Linux平台
     */
    class csemp
    {
    private:
        /** 信号量操作共同体（符合semctl函数要求） */
        union semun
        {
            int val;              // 用于SETVAL命令
            struct semid_ds* buf; // 用于IPC_STAT/IPC_SET命令
            unsigned short* arry; // 用于GETALL/SETALL命令
        };

        // 信号量ID（描述符）
        int m_semid;

        // 如果把sem_flg设置为SEM_UNDO，操作系统将跟踪进程对信号量的修改情况，
        // 在全部修改过信号量的进程（正常或异常）终止后，操作系统将把信号量恢复为初始值。
        // 如果信号量用于互斥锁，设置为SEM_UNDO。
        // 如果信号量用于生产消费者模型，设置为0。
        short m_sem_flg;

        csemp(const csemp&) = delete;            // 禁用拷贝构造函数。
        csemp& operator=(const csemp&) = delete; // 禁用赋值函数。
    public:
        // 构造函数，初始化信号量ID为-1（无效状态）
        csemp() : m_semid(-1)
        {
        }

        /**
         * @brief 初始化信号量（创建或获取已存在的信号量）
         * @param key 信号量键值（通过ftok生成）
         * @param value 初始值（互斥锁填1，生产消费者模型填0）
         * @param sem_flg 操作标志（SEM_UNDO-自动恢复，0-不自动恢复）
         * @return true-成功，false-失败
         * @note 1) 如果用于互斥锁，value填1，sem_flg填SEM_UNDO。
         *       2) 如果用于生产消费者模型，value填0，sem_flg填0。
         */
        bool init(key_t key, unsigned short value = 1, short sem_flg = SEM_UNDO);

        /**
         * @brief 信号量P操作（等待资源，将信号量值减value）
         * @param value 要减去的值（默认-1）
         * @return true-成功，false-失败
         * @note 若信号量值为0，调用进程将阻塞等待
         */
        bool wait(short value = -1);

        /**
         * @brief 信号量V操作（释放资源，将信号量值加value）
         * @param value 要加上的值（默认1）
         * @return true-成功，false-失败
         */
        bool post(short value = 1);

        /**
         * @brief 获取当前信号量的值
         * @param value 用于存储信号量值的引用
         * @return true-成功，false-失败
         */
        bool getvalue(int& value);

        /**
         * @brief 判断信号量是否有效（已初始化）
         * @return true-有效，false-无效
         */
        bool isValid() const;

        /**
         * @brief 销毁信号量（从系统中删除）
         * @return true-成功，false-失败
         */
        bool destroy();

        // 析构函数，不会自动销毁信号量
        ~csemp();
    };
    // ===========================================================================

    // ===========================================================================
    // 进程心跳相关宏定义
#define SHMKEYP 0x5095  // 共享内存的key。
#define SEMPKEYP 0x5095 // 信号量的key。
#define MAXNUMP 1000    // 最大的进程数量。

    // 进程心跳信息结构体，存储在共享内存中
    struct st_procinfo
    {
        int m_pid = 0;          // 进程ID
        char m_pname[51] = {0}; // 进程名称（最多50个字符），可以为空
        int m_timeout = 0;      // 超时时间（秒）
        time_t m_atime = 0;     // 最后一次心跳时间（时间戳）

        st_procinfo() = default; // 默认构造函数

        /**
         * @brief 带参构造函数
         * @param pid 进程ID
         * @param pname 进程名称
         * @param timeout 超时时间（秒）
         * @param atime 初始心跳时间（时间戳）
         */
        st_procinfo(const int pid, const std::string& pname, const int timeout, const time_t atime)
            : m_pid(pid), m_timeout(timeout), m_atime(atime)
        {
            strncpy(m_pname, pname.c_str(), 50);
        }
    };

    /**
     * @brief 进程心跳管理类，基于共享内存和信号量实现进程存活监控
     * @note 仅支持Linux平台
     */
    class cpactive
    {
    private:
        int m_shmid = 0;              // 共享内存ID
        int m_pos = -1;               // 当前进程在共享内存进程组中的位置
        st_procinfo* m_shm = nullptr; // 指向共享内存的地址空间的指针

    public:
        // 初始化成员变量。
        cpactive();

        /**
         * @brief 将当前进程信息加入共享内存进程组
         * @param timeout 超时时间（秒，超过此时长未更新心跳视为进程异常）
         * @param pname 进程名称（可选）
         * @param logfile 日志文件指针（用于输出调试信息，可选）
         * @param SHM_KEY 共享内存键值（默认SHMKEYP）
         * @param SEMP_KEY 信号量键值（默认SEMPKEYP）
         * @param MAX_SIZE_P 最大进程数量（默认MAXNUMP）
         * @return true-成功，false-失败
         */
        bool addpinfo(const int timeout, const std::string& pname = "", clogfile* logfile = nullptr, key_t SHM_KEY = SHMKEYP, key_t SEMP_KEY = SEMPKEYP, size_t MAX_SIZE_P = MAXNUMP);

        /**
         * 更新当前进程的心跳时间（刷新m_atime为当前时间）
         * @return true-成功，false-失败
         */
        bool uptatime();

        // 析构函数，从共享内存进程组中删除当前进程的心跳记录
        ~cpactive();
    };
    // ===========================================================================
#endif // __linux__

} // namespace ol

#endif // !OL_IPC_H