#ifndef __OL_IPC_H
#define __OL_IPC_H 1

#include "../include/ol_fstream.h"

#ifdef __linux__
#include <sys/ipc.h>
#include <sys/sem.h> // ���� SEM_UNDO �������ź�����غ���
#include <sys/shm.h>
#endif // __linux__

namespace ol
{

#ifdef __linux__
    ///////////////////////////////////// /////////////////////////////////////
    // �鿴�����ڴ棺  ipcs -m
    // ɾ�������ڴ棺  ipcrm -m shmid
    // �鿴�ź�����      ipcs -s
    // ɾ���ź�����      ipcrm sem semid

    // �ź�����
    class csemp
    {
    private:
        union semun // �����ź��������Ĺ�ͬ�塣
        {
            int val;
            struct semid_ds* buf;
            unsigned short* arry;
        };

        int m_semid; // �ź���id������������

        // �����sem_flg����ΪSEM_UNDO������ϵͳ�����ٽ��̶��ź������޸������
        // ��ȫ���޸Ĺ��ź����Ľ��̣��������쳣����ֹ�󣬲���ϵͳ�����ź����ָ�Ϊ��ʼֵ��
        // ����ź������ڻ�����������ΪSEM_UNDO��
        // ����ź�����������������ģ�ͣ�����Ϊ0��
        short m_sem_flg;

        csemp(const csemp&) = delete;            // ���ÿ������캯����
        csemp& operator=(const csemp&) = delete; // ���ø�ֵ������
    public:
        csemp() : m_semid(-1)
        {
        }

        // ����ź����Ѵ��ڣ���ȡ�ź���������ź��������ڣ��򴴽�������ʼ��Ϊvalue��
        // ������ڻ�������value��1��sem_flg��SEM_UNDO��
        // �����������������ģ�ͣ�value��0��sem_flg��0��
        bool init(key_t key, unsigned short value = 1, short sem_flg = SEM_UNDO);
        bool wait(short value = -1); // �ź�����P����������ź�����ֵ��0���������ȴ���ֱ���ź�����ֵ����0��
        bool post(short value = 1);  // �ź�����V������
        bool getvalue(int& value);   // ��ȡ�ź�����ֵ���ɹ�����true��ʧ�ܷ���false��
        bool isValid() const;        // �ж��ź����Ƿ���Ч��
        bool destroy();              // �����ź�����
        ~csemp();
    };
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // ���¼��������ڽ��̵�������
#define SHMKEYP 0x5095  // �����ڴ��key��
#define SEMPKEYP 0x5095 // �ź�����key��
#define MAXNUMP 1000    // ���Ľ���������

    // ����������Ϣ�Ľṹ�塣
    struct st_procinfo
    {
        int m_pid = 0;           // ����id��
        char m_pname[51] = {0};  // �������ƣ�����Ϊ�ա�
        int m_timeout = 0;       // ��ʱʱ�䣬��λ���롣
        time_t m_atime = 0;      // ���һ��������ʱ�䣬��������ʾ��
        st_procinfo() = default; // �����Զ���Ĺ��캯���������������ṩĬ�Ϲ��캯������������Ĭ�Ϲ��캯����
        st_procinfo(const int pid, const std::string& pname, const int timeout, const time_t atime)
            : m_pid(pid), m_timeout(timeout), m_atime(atime)
        {
            strncpy(m_pname, pname.c_str(), 50);
        }
    };

    // �������������ࡣ
    class cpactive
    {
    private:
        int m_shmid = 0;              // �����ڴ��id��
        int m_pos = -1;               // ��ǰ�����ڹ����ڴ�������е�λ�á�
        st_procinfo* m_shm = nullptr; // ָ�����ڴ�ĵ�ַ�ռ䡣

    public:
        cpactive(); // ��ʼ����Ա������

        // �ѵ�ǰ���̵���Ϣ���빲���ڴ�������С�
        bool addpinfo(const int timeout, const std::string& pname = "", clogfile* logfile = nullptr, key_t SHM_KEY = SHMKEYP, key_t SEMP_KEY = SEMPKEYP, size_t MAX_SIZE_P = MAXNUMP);

        // ���¹����ڴ�������е�ǰ���̵�����ʱ�䡣
        bool uptatime();

        ~cpactive(); // �ӹ����ڴ���ɾ����ǰ���̵�������¼��
    };
    ///////////////////////////////////// /////////////////////////////////////
#endif // __linux__

} // namespace ol

#endif // !__OL_IPC_H