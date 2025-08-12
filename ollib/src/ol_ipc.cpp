#include "../include/ol_ipc.h"
#include <iostream>

namespace ol
{

#ifdef __linux__
    ///////////////////////////////////// /////////////////////////////////////
    // ����ź����Ѵ��ڣ���ȡ�ź���������ź��������ڣ��򴴽�������ʼ��Ϊvalue��
    // ������ڻ�������value��1��sem_flg��SEM_UNDO��
    // �����������������ģ�ͣ�value��0��sem_flg��0��
    bool csemp::init(key_t key, unsigned short value, short sem_flg)
    {
        // �ź����ĳ�ʼ�����������裺
        // 1����ȡ�ź���������ɹ����������ء�
        // 2�����ʧ�ܣ��򴴽��ź�����
        // 3) �����ź����ĳ�ʼֵ��

        if (m_semid != -1) return false; // �ѳ�ʼ��

        m_sem_flg = sem_flg;

        // 1. ���Ի�ȡ�Ѵ��ڵ��ź���
        if ((m_semid = semget(key, 1, 0666)) != -1)
        {
            return true; // ��ȡ�ɹ�
        }

        // 2. �����ȡʧ�ܵ����
        if (errno != ENOENT)
        {
            perror("init: semget() failed (not ENOENT)");
            return false; // ��"�ź���������"����
        }

        // 3. �ź��������ڣ�������
        if ((m_semid = semget(key, 1, 0666 | IPC_CREAT | IPC_EXCL)) == -1)
        {
            if (errno == EEXIST)
            {
                // �������������ȴ��������»�ȡ
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

        // 4. ��ʼ���ź���ֵ
        union semun sem_union;
        sem_union.val = value;
        if (semctl(m_semid, 0, SETVAL, sem_union) < 0)
        {
            perror("init: semctl(SETVAL) failed");
            m_semid = -1; // ����״̬
            return false;
        }

        return true;
    }

    // �ź�����P���������ź�����ֵ��value��ע�⣺valueӦΪ������������ź�����ֵ��0���������ȴ���ֱ���ź�����ֵ����0��
    bool csemp::wait(short value)
    {
        if (m_semid == -1) return false;

        struct sembuf sem_b;
        sem_b.sem_num = 0;    // �ź�����ţ�0�����һ���ź�����
        sem_b.sem_op = value; // P������value����С��0��
        sem_b.sem_flg = m_sem_flg;
        if (semop(m_semid, &sem_b, 1) == -1)
        {
            perror("p semop()");
            return false;
        }

        return true;
    }

    // �ź�����V���������ź�����ֵ��value����
    bool csemp::post(short value)
    {
        if (m_semid == -1) return false;

        struct sembuf sem_b;
        sem_b.sem_num = 0;    // �ź�����ţ�0�����һ���ź�����
        sem_b.sem_op = value; // V������value�������0��
        sem_b.sem_flg = m_sem_flg;
        if (semop(m_semid, &sem_b, 1) == -1)
        {
            perror("V semop()");
            return false;
        }

        return true;
    }

    // ��ȡ�ź�����ֵ���ɹ������ź�����ֵ��ʧ�ܷ���-1��
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

    // �ж��ź����Ƿ���Ч��
    bool csemp::isValid() const
    {
        return m_semid != -1;
    }

    // �����ź�����
    bool csemp::destroy()
    {
        if (m_semid == -1) return false;

        if (semctl(m_semid, 0, IPC_RMID) == -1)
        {
            perror("destroy semctl()");
            return false;
        }

        m_semid = -1; // ����״̬

        return true;
    }

    csemp::~csemp()
    {
    }
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    cpactive::cpactive()
    {
        m_shmid = 0;
        m_pos = -1;
        m_shm = nullptr;
    }

    // �ѵ�ǰ���̵���Ϣ���빲���ڴ�������С�
    bool cpactive::addpinfo(const int timeout, const std::string& pname, clogfile* logfile, key_t SHM_KEY, key_t SEMP_KEY, size_t MAX_SIZE_P)
    {
        if (m_pos != -1) return true;

        // ����/��ȡ�����ڴ棬��ֵΪSHM_KEY����СΪMAX_SIZE_P��st_procinfo�ṹ��Ĵ�С��
        if ((m_shmid = shmget((key_t)SHM_KEY, MAX_SIZE_P * sizeof(struct st_procinfo), 0666 | IPC_CREAT)) == -1)
        {
            if (logfile != nullptr)
                logfile->write("����/��ȡ�����ڴ�(%x)ʧ�ܡ�\n", SHM_KEY);
            else
                std::cerr << "����/��ȡ�����ڴ�(" << SHM_KEY << ")ʧ�ܡ�\n";

            return false;
        }

        // �������ڴ����ӵ���ǰ���̵�ַ�ռ�
        if ((m_shm = (struct st_procinfo*)shmat(m_shmid, 0, 0)) == (void*)-1)
        {
            if (logfile != nullptr)
                logfile->write("���ӹ����ڴ�(%x)ʧ�ܡ�\n", SHM_KEY);
            else
                std::cerr << "���ӹ����ڴ�(" << SHM_KEY << ")ʧ�ܡ�\n";

            return false;
        }

        // ��ʼ����ǰ����������Ϣ�Ľṹ�塣
        st_procinfo procinfo(getpid(), pname.c_str(), timeout, time(0));

        // �����ź����������ڴ����
        // ֻ��Ҫ�������ҹ����ڴ��λ�õ�ʱ������������������Ҫ����Ϊ��ֻ������Լ���λ��
        csemp shmlock;

        if (shmlock.init((key_t)SEMP_KEY) == false) // ��ʼ���ź�����
        {
            if (logfile != nullptr)
                logfile->write("����/��ȡ�ź���(%x)ʧ�ܡ�\n", SEMP_KEY);
            else
                std::cerr << "����/��ȡ�ź���(" << SEMP_KEY << ")ʧ�ܡ�\n";

            return false;
        }

        // ���ҹ����ڴ�Ŀ�λ��
        // ע�⣺����id��ѭ��ʹ�õģ����������һ�������쳣�˳���û�������Լ���������Ϣ��
        // ���Ľ�����Ϣ�������ڹ����ڴ��У����ɵ��ǣ������ǰ��������������id��
        // ���ԣ�����ǹ����ڴ����Ѵ��ڵ�ǰ���̱�ţ�һ�����������̲�������Ϣ����ǰ����Ӧ���������λ��
        shmlock.wait(); // ����

        // ����MAX_SIZE_Pѡ�����
        if (MAX_SIZE_P <= 1000)
        {
            // С�ڴ泡����ʹ�ò���1�������������������ò���PID��
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
            // ���ڴ泡����ʹ�ò���2�����ٲ��ң���������PID���λ����ʹ�ã�
            for (size_t i = 0; i < MAX_SIZE_P; ++i)
            {
                if (m_shm[i].m_pid == procinfo.m_pid || m_shm[i].m_pid == 0)
                {
                    m_pos = i;
                    break;
                }
            }
        }

        // ���m_pos==-1����ʾû�ҵ���λ�ã�˵�������ڴ�Ŀռ������ꡣ
        if (m_pos == -1)
        {
            if (logfile != nullptr)
                logfile->write("�����ڴ�ռ������ꡣ\n");
            else
                std::cerr << "�����ڴ�ռ������ꡣ\n";

            shmlock.post(); // ������

            return false;
        }

        // �ѵ�ǰ���̵�������Ϣ���빲���ڴ�Ľ������С�
        memcpy(&m_shm[m_pos], &procinfo, sizeof(struct st_procinfo));

        shmlock.post(); // ������

        return true;
    }

    // ���¹����ڴ�������е�ǰ���̵�����ʱ�䡣
    bool cpactive::uptatime()
    {
        if (m_pos == -1) return false;

        m_shm[m_pos].m_atime = time(0);

        return true;
    }

    cpactive::~cpactive()
    {
        // �ѵ�ǰ���̴ӹ����ڴ�Ľ���������ȥ��
        if (m_pos != -1) memset(m_shm + m_pos, 0, sizeof(struct st_procinfo));

        // �ѹ����ڴ�ӵ�ǰ�����з��롣
        if (m_shm != nullptr) shmdt(m_shm);
    }
    ///////////////////////////////////// /////////////////////////////////////
#endif // __linux__

} // namespace ol