#include "ol_ipc.h"
#include <iostream>

namespace ol
{

#ifdef __unix__
    // ===========================================================================
    // 如果信号量已存在，获取信号量；如果信号量不存在，则创建它并初始化为value。
    // 如果用于互斥锁，value填1，sem_flg填SEM_UNDO。
    // 如果用于生产消费者模型，value填0，sem_flg填0。
    bool csemp::init(key_t key, unsigned short value, short sem_flg)
    {
        // 信号量的初始化分三个步骤：
        // 1）获取信号量，如果成功，函数返回。
        // 2）如果失败，则创建信号量。
        // 3) 设置信号量的初始值。

        if (m_semid != -1) return false; // 已初始化

        m_sem_flg = sem_flg;

        // 1. 尝试获取已存在的信号量
        if ((m_semid = semget(key, 1, 0666)) != -1)
        {
            return true; // 获取成功
        }

        // 2. 处理获取失败的情况
        if (errno != ENOENT)
        {
            perror("init: semget() failed (not ENOENT)");
            return false; // 非"信号量不存在"错误
        }

        // 3. 信号量不存在，创建它
        if ((m_semid = semget(key, 1, 0666 | IPC_CREAT | IPC_EXCL)) == -1)
        {
            if (errno == EEXIST)
            {
                // 被其他进程抢先创建，重新获取
                if ((m_semid = semget(key, 1, 0666)) == -1)
                {
                    perror("init: semget() failed after EEXIST");
                    return false;
                }
                return true;
            }
            else
            {
                perror("init: semget() failed (create)");
                return false;
            }
        }

        // 4. 初始化信号量值
        union semun sem_union;
        sem_union.val = value;
        if (semctl(m_semid, 0, SETVAL, sem_union) < 0)
        {
            perror("init: semctl(SETVAL) failed");
            m_semid = -1; // 重置状态
            return false;
        }

        return true;
    }

    // 信号量的P操作（把信号量的值加value，注意：value应为负数），如果信号量的值是0，将阻塞等待，直到信号量的值大于0。
    bool csemp::wait(short value)
    {
        if (m_semid == -1) return false;

        struct sembuf sem_b;
        sem_b.sem_num = 0;    // 信号量编号，0代表第一个信号量。
        sem_b.sem_op = value; // P操作的value必须小于0。
        sem_b.sem_flg = m_sem_flg;
        if (semop(m_semid, &sem_b, 1) == -1)
        {
            perror("P semop()");
            return false;
        }

        return true;
    }

    // 信号量的V操作（把信号量的值减value）。
    bool csemp::post(short value)
    {
        if (m_semid == -1) return false;

        struct sembuf sem_b;
        sem_b.sem_num = 0;    // 信号量编号，0代表第一个信号量。
        sem_b.sem_op = value; // V操作的value必须大于0。
        sem_b.sem_flg = m_sem_flg;
        if (semop(m_semid, &sem_b, 1) == -1)
        {
            perror("V semop()");
            return false;
        }

        return true;
    }

    // 获取信号量的值，成功返回信号量的值，失败返回-1。
    bool csemp::getvalue(int& value)
    {
        int result = semctl(m_semid, 0, GETVAL);
        if (result == -1)
        {
            perror("getvalue semctl()");
            return false;
        }
        value = result;
        return true;
    };

    // 判断信号量是否有效。
    bool csemp::isValid() const
    {
        return m_semid != -1;
    }

    // 销毁信号量。
    bool csemp::destroy()
    {
        if (m_semid == -1) return false;

        if (semctl(m_semid, 0, IPC_RMID) == -1)
        {
            perror("destroy semctl()");
            return false;
        }

        m_semid = -1; // 重置状态

        return true;
    }

    csemp::~csemp()
    {
    }
    // ===========================================================================

    // ===========================================================================
    cpactive::cpactive()
    {
        m_shmid = 0;
        m_pos = -1;
        m_shm = nullptr;
    }

    // 把当前进程的信息加入共享内存进程组中。
    bool cpactive::addpinfo(const int timeout, const std::string& pname, clogfile* logfile, key_t SHM_KEY, key_t SEMP_KEY, size_t MAX_SIZE_P)
    {
        if (m_pos != -1) return true;

        // 创建/获取共享内存，键值为SHM_KEY，大小为MAX_SIZE_P个st_procinfo结构体的大小。
        if ((m_shmid = shmget((key_t)SHM_KEY, MAX_SIZE_P * sizeof(struct st_procinfo), 0666 | IPC_CREAT)) == -1)
        {
            if (logfile != nullptr)
                logfile->write("创建/获取共享内存(%x)失败。\n", SHM_KEY);
            else
                std::cerr << "创建/获取共享内存(" << SHM_KEY << ")失败。\n";

            return false;
        }

        // 将共享内存连接到当前进程地址空间
        if ((m_shm = (struct st_procinfo*)shmat(m_shmid, 0, 0)) == (void*)-1)
        {
            if (logfile != nullptr)
                logfile->write("连接共享内存(%x)失败。\n", SHM_KEY);
            else
                std::cerr << "连接共享内存(" << SHM_KEY << ")失败。\n";

            return false;
        }

        // 初始化当前进程心跳信息的结构体。
        st_procinfo procinfo(getpid(), pname.c_str(), timeout, time(0));

        // 创建信号量给共享内存加锁
        // 只需要给进程找共享内存空位置的时候加锁，其它情况不需要，因为都只会操作自己的位置
        csemp shmlock;

        if (shmlock.init((key_t)SEMP_KEY) == false) // 初始化信号量。
        {
            if (logfile != nullptr)
                logfile->write("创建/获取信号量(%x)失败。\n", SEMP_KEY);
            else
                std::cerr << "创建/获取信号量(" << SEMP_KEY << ")失败。\n";

            return false;
        }

        // 查找共享内存的空位置
        // 注意：进程id是循环使用的，如果曾经有一个进程异常退出，没有清理自己的心跳信息，
        // 它的进程信息将残留在共享内存中，不巧的是，如果当前进程重用了它的id，
        // 所以，如果是共享内存中已存在当前进程编号，一定是其它进程残留的信息，当前进程应该重用这个位置
        shmlock.wait(); // 加锁

        // 根据MAX_SIZE_P选择策略
        if (MAX_SIZE_P <= 1000)
        {
            // 小内存场景：使用策略1（完整遍历，优先重用残留PID）
            for (size_t i = 0; i < MAX_SIZE_P; ++i)
            {
                if (m_shm[i].m_pid == procinfo.m_pid)
                {
                    m_pos = i;
                    break;
                }
                if (m_shm[i].m_pid == 0 && m_pos == -1)
                {
                    m_pos = i;
                }
            }
        }
        else
        {
            // 大内存场景：使用策略2（快速查找，遇到残留PID或空位立即使用）
            for (size_t i = 0; i < MAX_SIZE_P; ++i)
            {
                if (m_shm[i].m_pid == procinfo.m_pid || m_shm[i].m_pid == 0)
                {
                    m_pos = i;
                    break;
                }
            }
        }

        // 如果m_pos==-1，表示没找到空位置，说明共享内存的空间已用完。
        if (m_pos == -1)
        {
            if (logfile != nullptr)
                logfile->write("共享内存空间已用完。\n");
            else
                std::cerr << "共享内存空间已用完。\n";

            shmlock.post(); // 解锁。

            return false;
        }

        // 把当前进程的心跳信息存入共享内存的进程组中。
        memcpy(&m_shm[m_pos], &procinfo, sizeof(struct st_procinfo));

        shmlock.post(); // 解锁。

        return true;
    }

    // 更新共享内存进程组中当前进程的心跳时间。
    bool cpactive::uptatime()
    {
        if (m_pos == -1) return false;

        m_shm[m_pos].m_atime = time(0);

        return true;
    }

    cpactive::~cpactive()
    {
        // 把当前进程从共享内存的进程组中移去。
        if (m_pos != -1) *reinterpret_cast<struct st_procinfo*>(m_shm + m_pos) = st_procinfo();

        // 把共享内存从当前进程中分离。
        if (m_shm != nullptr) shmdt(m_shm);
    }
    // ===========================================================================
#endif // __unix__

} // namespace ol