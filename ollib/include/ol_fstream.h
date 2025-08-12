#ifndef __OL_FSTREAM_H
#define __OL_FSTREAM_H 1

// ����Windows��min/max��
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX  // ����δ����ʱ����
#endif
#endif

#include "../include/ol_chrono.h"
#include "../include/ol_string.h"
#include <algorithm>
#include <atomic>
#include <bitset>
#include <fstream>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <vector>

#ifdef __linux__
#include <dirent.h>
#include <unistd.h>
#include <utime.h>
#elif defined(_WIN32)  // Windows ƽ̨ͷ�ļ�
#include <direct.h>    // Ŀ¼����
#include <io.h>        // ��� unistd.h
#include <sys/utime.h> // Windows �µ� utime ����
#include <time.h>      // ʱ�亯��
#endif                 // _WIN32

namespace ol
{

#if defined(__linux__) || defined(_WIN32)
    ///////////////////////////////////// /////////////////////////////////////
    // ���ݾ���·�����ļ�����Ŀ¼���𼶵Ĵ���Ŀ¼��
    // pathorfilename������·�����ļ�����Ŀ¼����
    // bisfilename��ָ��pathorfilename�����ͣ�true-pathorfilename���ļ�����������Ŀ¼����ȱʡֵΪtrue��
    // ����ֵ��true-�ɹ���false-ʧ�ܣ��������ʧ�ܣ�ԭ���д�������������
    // 1��Ȩ�޲��㣻2��pathorfilename�������ǺϷ����ļ�����Ŀ¼����3�����̿ռ䲻�㡣
    bool newdir(const std::string& pathorfilename, bool bisfilename = true);
    ///////////////////////////////////// /////////////////////////////////////

    // �ļ�������صĺ���
    ///////////////////////////////////// /////////////////////////////////////
    // �������ļ�������Linuxϵͳ��mv���
    // srcfilename��ԭ�ļ�����������þ���·�����ļ�����
    // dstfilename��Ŀ���ļ�����������þ���·�����ļ�����
    // ����ֵ��true-�ɹ���false-ʧ�ܣ�ʧ�ܵ���Ҫԭ����Ȩ�޲������̿ռ䲻�������ԭ�ļ���Ŀ���ļ�����ͬһ�����̷�����������Ҳ����ʧ�ܡ�
    // ע�⣬���������ļ�֮ǰ�����Զ�����dstfilename�����а�����Ŀ¼��
    // ��Ӧ�ÿ����У�������renamefile()��������rename()�⺯����
    bool renamefile(const std::string& srcfilename, const std::string& dstfilename);
    ///////////////////////////////////// /////////////////////////////////////

    // �����ļ�������Linuxϵͳ��cp���
    // srcfilename��ԭ�ļ�����������þ���·�����ļ�����
    // dstfilename��Ŀ���ļ�����������þ���·�����ļ�����
    // ����ֵ��true-�ɹ���false-ʧ�ܣ�ʧ�ܵ���Ҫԭ����Ȩ�޲������̿ռ䲻����
    // ע�⣺
    // 1���ڸ����ļ�֮ǰ�����Զ�����dstfilename�����е�Ŀ¼����
    // 2�������ļ��Ĺ����У�������ʱ�ļ������ķ�����������ɺ��ٸ���Ϊdstfilename�������м�״̬���ļ�����ȡ��
    // 3�����ƺ���ļ���ʱ����ԭ�ļ���ͬ����һ����Linuxϵͳcp���ͬ��
    bool copyfile(const std::string& srcfilename, const std::string& dstfilename);

    // ��ȡ�ļ��Ĵ�С��
    // filename������ȡ���ļ�����������þ���·�����ļ�����
    // ����ֵ������ļ������ڻ�û�з���Ȩ�ޣ�����-1���ɹ������ļ��Ĵ�С����λ���ֽڡ�
    long filesize(const std::string& filename);

    // ��ȡ�ļ���ʱ�䡣
    // filename������ȡ���ļ�����������þ���·�����ļ�����
    // mtime�����ڴ���ļ���ʱ�䣬��stat�ṹ���st_mtime��
    // fmt������ʱ��������ʽ����ltime()������ͬ����ȱʡ��"yyyymmddhh24miss"��
    // ����ֵ������ļ������ڻ�û�з���Ȩ�ޣ�����false���ɹ�����true��
    bool filemtime(const std::string& filename, char* mtime, const std::string& fmt = "yyyymmddhh24miss");
    bool filemtime(const std::string& filename, std::string& mtime, const std::string& fmt = "yyyymmddhh24miss");

    // �����ļ����޸�ʱ�����ԡ�
    // filename�������õ��ļ�����������þ���·�����ļ�����
    // mtime���ַ�����ʾ��ʱ�䣬��ʽ���ޣ���һ��Ҫ����yyyymmddhh24miss��һ���������٣�˳��Ҳ���ܱ䡣
    // ����ֵ��true-�ɹ���false-ʧ�ܣ�ʧ�ܵ�ԭ�򱣴���errno�С�
    bool setmtime(const std::string& filename, const std::string& mtime);
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // ��ȡĳĿ¼������Ŀ¼�е��ļ��б���ࡣ
    class cdir
    {
    private:
        std::vector<std::string> m_filelist; // ����ļ��б������������·�����ļ�������
        size_t m_pos;                        // ���ļ��б�m_filelist���Ѷ�ȡ�ļ���λ�á�
        std::string m_fmt;                   // �ļ�ʱ���ʽ��ȱʡ"yyyymmddhh24miss"��

        cdir(const cdir&) = delete;            // ���ÿ������캯����
        cdir& operator=(const cdir&) = delete; // ���ø�ֵ������
    public:
        // /project/public/_public.h
        std::string m_dirname;   // Ŀ¼�������磺/project/public
        std::string m_filename;  // �ļ�����������Ŀ¼�������磺_public.h
        std::string m_ffilename; // ����·�����ļ������磺/project/public/_public.h
        size_t m_filesize;       // �ļ��Ĵ�С����λ���ֽڡ�
        std::string m_mtime;     // �ļ����һ�α��޸ĵ�ʱ�䣬��stat�ṹ���st_mtime��Ա��
        std::string m_ctime;     // �ļ����ɵ�ʱ�䣬��stat�ṹ���st_ctime��Ա��
        std::string m_atime;     // �ļ����һ�α����ʵ�ʱ�䣬��stat�ṹ���st_atime��Ա��

        cdir() : m_pos(0), m_fmt("yyyymmddhh24miss"), m_filesize(0)
        {
        } // ���캯����

        // �����ļ�ʱ��ĸ�ʽ��֧��"yyyy-mm-dd hh24:mi:ss"��"yyyymmddhh24miss"���֣�ȱʡ�Ǻ��ߡ�
        void setfmt(const std::string& fmt);

        // ��Ŀ¼����ȡĿ¼���ļ����б������m_filelist�����С�
        // dirname��Ŀ¼�������þ���·������/tmp/root��
        // rules���ļ�����ƥ����򣬲�ƥ����ļ��������ԡ�
        // maxfiles�����λ�ȡ�ļ������������ȱʡֵΪ10000��������ļ�̫�࣬��������̫����ڴ档
        // bandchild���Ƿ�򿪸�����Ŀ¼��ȱʡֵΪfalse-������Ŀ¼��
        // bsort���Ƿ��ļ�������ȱʡֵΪfalse-������
        // ����ֵ��true-�ɹ���false-ʧ�ܡ�
        bool opendir(const std::string& dirname, const std::string& rules, const size_t maxfiles = 10000, const bool bandchild = false, bool bsort = false);

    private:
        // ����һ���ݹ麯������opendir()�ĵ��ã���cdir����ⲿ����Ҫ��������
        bool _opendir(const std::string& dirname, const std::string& rules, const size_t maxfiles, const bool bandchild);

    public:
        // ��m_filelist�����л�ȡһ����¼���ļ�������ͬʱ��ȡ���ļ��Ĵ�С���޸�ʱ�����Ϣ��
        // ����opendir����ʱ��m_filelist��������գ�m_pos���㣬ÿ����һ��readdir����m_pos��1��
        // ��m_posС��m_filelist.size()������true�����򷵻�false��
        bool readdir();

        size_t size()
        {
            return m_filelist.size();
        }

        ~cdir(); // ����������
    };
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // д�ļ����ࡣ
    class cofile // class out file
    {
    private:
        std::ofstream fout;        // д���ļ��Ķ���
        std::string m_filename;    // �ļ�����������þ���·����
        std::string m_filenametmp; // ��ʱ�ļ�������m_filename�����".tmp"��
    public:
        cofile()
        {
        }

        // �ļ��Ƿ��Ѵ򿪡�
        bool isopen() const
        {
            return fout.is_open();
        }

        // ���ļ���
        // filename�����򿪵��ļ�����
        // btmp���Ƿ������ʱ�ļ��ķ�����
        // mode�����ļ���ģʽ��
        // benbuffer���Ƿ������ļ���������
        bool open(const std::string& filename, const bool btmp = true, const std::ios::openmode mode = std::ios::out, const bool benbuffer = true);

        // ���������ı��ķ�ʽ��ʽ��������ļ���
        template <typename... Args>
        bool writeline(const char* fmt, Args... args)
        {
            if (fout.is_open() == false) return false;

            fout << sformat(fmt, args...);

            return fout.good();
        }

        // ����<<����������������ı��ķ�ʽ������ļ���
        // ע�⣺����ֻ����\n��������endl��
        template <typename T>
        cofile& operator<<(const T& value)
        {
            fout << value;
            return *this;
        }

        // �Ѷ���������д���ļ���
        bool write(void* buf, int bufsize);

        // �ر��ļ������Ұ���ʱ�ļ�����Ϊ��ʽ�ļ�����
        bool closeandrename();

        // �ر��ļ����������ʱ�ļ�����ɾ������
        void close();

        ~cofile()
        {
            close();
        };
    };
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // ��ȡ�ļ����ࡣ
    class cifile // class in file
    {
    private:
        std::ifstream fin;      // ��ȡ�ļ��Ķ���
        std::string m_filename; // �ļ�����������þ���·����
    public:
        cifile()
        {
        }

        // �ж��ļ��Ƿ��Ѵ򿪡�
        bool isopen() const
        {
            return fin.is_open();
        }

        // ���ļ���
        // filename�����򿪵��ļ�����
        // mode�����ļ���ģʽ��
        bool open(const std::string& filename, const std::ios::openmode mode = std::ios::in);

        // ���еķ�ʽ��ȡ�ı��ļ���endbzָ���еĽ�β��־��ȱʡΪ�գ�û�н�β��־��
        bool readline(std::string& buf, const std::string& endbz = "");

        // ��ȡ�������ļ�������ʵ�ʶ�ȡ�����ֽ�����
        size_t read(void* buf, const size_t bufsize);

        // �رղ�ɾ���ļ���
        bool closeandremove();

        // ֻ�ر��ļ���
        void close();

        ~cifile()
        {
            close();
        }
    };
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // ��������
    class spinlock_mutex
    {
    private:
        std::atomic_flag flag;

        spinlock_mutex(const spinlock_mutex&) = delete;
        spinlock_mutex& operator=(const spinlock_mutex) = delete;

    public:
        spinlock_mutex()
        {
            flag.clear();
        }
        void lock() // ������
        {
            while (flag.test_and_set());
        }
        void unlock() // ������
        {
            flag.clear();
        }
    };
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // ��־�ļ���
    class clogfile
    {
        std::ofstream fout;        // ��־�ļ�����
        std::string m_filename;    // ��־�ļ�����������þ���·����
        std::ios::openmode m_mode; // ��־�ļ��Ĵ�ģʽ��
        bool m_backup;             // �Ƿ��Զ��л���־��
        size_t m_maxsize;          // ����־�ļ��Ĵ�С����������ʱ���Զ��л���־��
        bool m_enbuffer;           // �Ƿ������ļ���������
        spinlock_mutex m_splock;   // �����������ڶ��̳߳����и�д��־�Ĳ���������

    public:
        // ���캯������־�ļ��Ĵ�Сȱʡ100M��
        clogfile(size_t maxsize = 100) : m_mode(std::ios::app), m_backup(true), m_maxsize(maxsize), m_enbuffer(false)
        {
        }

        // ����־�ļ���
        // filename����־�ļ�����������þ���·��������ļ����е�Ŀ¼�����ڣ����ȴ���Ŀ¼��
        // openmode����־�ļ��Ĵ�ģʽ��ȱʡֵ��ios::app��
        // bbackup���Ƿ��Զ��л������ݣ���true-�л���false-���л����ڶ���̵ķ�������У����������̹���һ����־�ļ���bbackup����Ϊfalse��
        // benbuffer���Ƿ������ļ�������ƣ�true-���ã�false-�����ã�������û���������ôд����־�ļ��е����ݲ�������д���ļ���ȱʡ�ǲ����á�
        // ע�⣬�ڶ���̵ĳ����У����������ͬһ��־�ļ�д���������־ʱ�����ܻ����С���ң����ǣ����̲߳��ᡣ
        // 1�����������ͬһ��־�ļ�д���������־ʱ�����ܻ����С���ң�������Ⲣ�����أ��������̣�
        // 2��ֻ��ͬʱд������־ʱ�Ż���ֻ��ң���ʵ�ʿ����У���������������
        // 3�����ҵ���޷����̣��������ź���������
        bool open(const std::string& filename, const std::ios::openmode mode = std::ios::app, const bool bbackup = true, const bool benbuffer = false);

        // ����־�������ı��ķ�ʽ��ʽ���������־�ļ������ң�����־����ǰ��д��ʱ�䡣
        template <typename... Args>
        bool write(const char* fmt, Args... args)
        {
            if (fout.is_open() == false) return false;

            backup(); // �ж��Ƿ���Ҫ�л���־�ļ���

            m_splock.lock();                                  // ������
            fout << ltime1() << " " << sformat(fmt, args...); // �ѵ�ǰʱ�����־����д����־�ļ���
            m_splock.unlock();                                // ������

            return fout.good();
        }

        // ����<<�����������־�������ı��ķ�ʽ�������־�ļ�����������־����ǰ��дʱ�䡣
        // ע�⣺���ݻ�����\n��������endl��
        template <typename T>
        clogfile& operator<<(const T& value)
        {
            m_splock.lock();
            fout << value;
            m_splock.unlock();

            return *this;
        }

    private:
        // �����־�ļ��Ĵ�С����m_maxsize��ֵ���Ͱѵ�ǰ����־�ļ�����Ϊ��ʷ��־�ļ������ٴ����µĵ�ǰ��־�ļ���
        // ���ݺ���ļ�������־�ļ������������ʱ�䣬��/tmp/log/filetodb.log.20200101123025��
        // ע�⣬�ڶ���̵ĳ����У���־�ļ������л������ߵĳ����У���־�ļ������л���
        bool backup();

    public:
        void close()
        {
            fout.close();
        }

        ~clogfile()
        {
            close();
        };
    };
    ///////////////////////////////////// /////////////////////////////////////
#endif // defined(__linux__) || defined(_WIN32)

    ///////////////////////////////////// /////////////////////////////////////
    // ======================
    // �Զ��������
    // ======================

    // ���в��ݷ�
    std::ostream& nl(std::ostream& os);

    // ��������Ķ����Ʊ�ʾ
    struct binary_t
    {
        unsigned long value;
        explicit binary_t(unsigned long v) : value(v)
        {
        }
    };

    template <typename T>
    binary_t binary(T value)
    {
        return binary_t(static_cast<unsigned long>(value));
    }

    std::ostream& operator<<(std::ostream& os, const binary_t& b);

    // ������뻺��������������ʣ���ַ�ֱ�����з���
    std::istream& clearbuf(std::istream& is);
    ///////////////////////////////////// /////////////////////////////////////

} // namespace ol

#endif // !__OL_FSTREAM_H