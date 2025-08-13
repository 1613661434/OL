/****************************************************************************************/
/*
 * ��������ol_ipc.h
 * ����������Linux���̼�ͨ�ţ�IPC�������֧࣬���������ԣ�
 *          - �ź��������ࣨcsemp�����ṩP/V�������ź�������������
 *          - �������������ࣨcpactive�������ڹ����ڴ���ź���ʵ�ֽ��̴����
 *          - ��֧��Linuxƽ̨������sys/ipc.h��sys/sem.h��ϵͳͷ�ļ���
 * ���ߣ�ol
 * ���ñ�׼��C++11�����ϣ���֧��Linuxϵͳ���ã�
 */
/****************************************************************************************/

#ifndef __OL_IPC_H
#define __OL_IPC_H 1

#include "ol_fstream.h"

#ifdef __linux__
#include <sys/ipc.h>
#include <sys/sem.h> // ���� SEM_UNDO �������ź�����غ���
#include <sys/shm.h>
#endif // __linux__

namespace ol
{

#ifdef __linux__
    // ===========================================================================
    // Linux����
    // �鿴�����ڴ棺  ipcs -m
    // ɾ�������ڴ棺  ipcrm -m shmid
    // �鿴�ź�����    ipcs -s
    // ɾ���ź�����    ipcrm sem semid

    /**
     * �ź��������࣬���ڽ��̼�ͬ���뻥��
     * @note ��֧��Linuxƽ̨
     */
    class csemp
    {
    private:
        /** �ź���������ͬ�壨����semctl����Ҫ�� */
        union semun
        {
            int val;              // ����SETVAL����
            struct semid_ds* buf; // ����IPC_STAT/IPC_SET����
            unsigned short* arry; // ����GETALL/SETALL����
        };

        // �ź���ID����������
        int m_semid;

        // �����sem_flg����ΪSEM_UNDO������ϵͳ�����ٽ��̶��ź������޸������
        // ��ȫ���޸Ĺ��ź����Ľ��̣��������쳣����ֹ�󣬲���ϵͳ�����ź����ָ�Ϊ��ʼֵ��
        // ����ź������ڻ�����������ΪSEM_UNDO��
        // ����ź�����������������ģ�ͣ�����Ϊ0��
        short m_sem_flg;

        csemp(const csemp&) = delete;            // ���ÿ������캯����
        csemp& operator=(const csemp&) = delete; // ���ø�ֵ������
    public:
        // ���캯������ʼ���ź���IDΪ-1����Ч״̬��
        csemp() : m_semid(-1)
        {
        }

        /**
         * ��ʼ���ź������������ȡ�Ѵ��ڵ��ź�����
         * @param key �ź�����ֵ��ͨ��ftok���ɣ�
         * @param value ��ʼֵ����������1������������ģ����0��
         * @param sem_flg ������־��SEM_UNDO-�Զ��ָ���0-���Զ��ָ���
         * @return true-�ɹ���false-ʧ��
         * @note 1) ������ڻ�������value��1��sem_flg��SEM_UNDO��
         *       2) �����������������ģ�ͣ�value��0��sem_flg��0��
         */
        bool init(key_t key, unsigned short value = 1, short sem_flg = SEM_UNDO);

        /**
         * �ź���P�������ȴ���Դ�����ź���ֵ��value��
         * @param value Ҫ��ȥ��ֵ��Ĭ��-1��
         * @return true-�ɹ���false-ʧ��
         * @note ���ź���ֵΪ0�����ý��̽������ȴ�
         */
        bool wait(short value = -1);

        /**
         * �ź���V�������ͷ���Դ�����ź���ֵ��value��
         * @param value Ҫ���ϵ�ֵ��Ĭ��1��
         * @return true-�ɹ���false-ʧ��
         */
        bool post(short value = 1);

        /**
         * ��ȡ��ǰ�ź�����ֵ
         * @param value ���ڴ洢�ź���ֵ������
         * @return true-�ɹ���false-ʧ��
         */
        bool getvalue(int& value);

        /**
         * �ж��ź����Ƿ���Ч���ѳ�ʼ����
         * @return true-��Ч��false-��Ч
         */
        bool isValid() const;

        /**
         * �����ź�������ϵͳ��ɾ����
         * @return true-�ɹ���false-ʧ��
         */
        bool destroy();

        // ���������������Զ������ź���
        ~csemp();
    };
    // ===========================================================================

    // ===========================================================================
    // ����������غ궨��
#define SHMKEYP 0x5095  // �����ڴ��key��
#define SEMPKEYP 0x5095 // �ź�����key��
#define MAXNUMP 1000    // ���Ľ���������

    // ����������Ϣ�ṹ�壬�洢�ڹ����ڴ���
    struct st_procinfo
    {
        int m_pid = 0;          // ����ID
        char m_pname[51] = {0}; // �������ƣ����50���ַ���������Ϊ��
        int m_timeout = 0;      // ��ʱʱ�䣨�룩
        time_t m_atime = 0;     // ���һ������ʱ�䣨ʱ�����

        st_procinfo() = default; // Ĭ�Ϲ��캯��

        /**
         * ���ι��캯��
         * @param pid ����ID
         * @param pname ��������
         * @param timeout ��ʱʱ�䣨�룩
         * @param atime ��ʼ����ʱ�䣨ʱ�����
         */
        st_procinfo(const int pid, const std::string& pname, const int timeout, const time_t atime)
            : m_pid(pid), m_timeout(timeout), m_atime(atime)
        {
            strncpy(m_pname, pname.c_str(), 50);
        }
    };

    /**
     * �������������࣬���ڹ����ڴ���ź���ʵ�ֽ��̴����
     * @note ��֧��Linuxƽ̨
     */
    class cpactive
    {
    private:
        int m_shmid = 0;              // �����ڴ�ID
        int m_pos = -1;               // ��ǰ�����ڹ����ڴ�������е�λ��
        st_procinfo* m_shm = nullptr; // ָ�����ڴ�ĵ�ַ�ռ��ָ��

    public:
        // ��ʼ����Ա������
        cpactive();

        /**
         * ����ǰ������Ϣ���빲���ڴ������
         * @param timeout ��ʱʱ�䣨�룬������ʱ��δ����������Ϊ�����쳣��
         * @param pname �������ƣ���ѡ��
         * @param logfile ��־�ļ�ָ�루�������������Ϣ����ѡ��
         * @param SHM_KEY �����ڴ��ֵ��Ĭ��SHMKEYP��
         * @param SEMP_KEY �ź�����ֵ��Ĭ��SEMPKEYP��
         * @param MAX_SIZE_P ������������Ĭ��MAXNUMP��
         * @return true-�ɹ���false-ʧ��
         */
        bool addpinfo(const int timeout, const std::string& pname = "", clogfile* logfile = nullptr, key_t SHM_KEY = SHMKEYP, key_t SEMP_KEY = SEMPKEYP, size_t MAX_SIZE_P = MAXNUMP);

        /**
         * ���µ�ǰ���̵�����ʱ�䣨ˢ��m_atimeΪ��ǰʱ�䣩
         * @return true-�ɹ���false-ʧ��
         */
        bool uptatime();

        // �����������ӹ����ڴ��������ɾ����ǰ���̵�������¼
        ~cpactive();
    };
    // ===========================================================================
#endif // __linux__

} // namespace ol

#endif // !__OL_IPC_H