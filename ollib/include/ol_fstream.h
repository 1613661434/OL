/****************************************************************************************/
/*
 * ��������ol_fstream.h
 * �����������ļ�ϵͳ���������༰�������ϣ�֧�ֿ�ƽ̨��Linux/Windows�����������԰�����
 *          - Ŀ¼�������ļ������������ơ���С/ʱ���ȡ�Ȼ����ļ�����
 *          - Ŀ¼�����ࣨcdir����֧�ֵݹ��ȡ�ļ��б�����
 *          - �ļ���д�ࣨcofile/cifile����֧���ı�/�����Ʋ�������ʱ�ļ�����
 *          - ��־�ļ��ࣨclogfile����֧���Զ��л������̰߳�ȫ
 *          - �������ߣ����������Զ��������������
 * ���ߣ�ol
 * ���ñ�׼��C++11�����ϣ���֧��atomic��fstream�����ģ������ԣ�
 */
/****************************************************************************************/

#ifndef __OL_FSTREAM_H
#define __OL_FSTREAM_H 1

// ����Windows��min/max��
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX // ����δ����ʱ����
#endif
#endif

#include "ol_chrono.h"
#include "ol_string.h"
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
    // ===========================================================================
    /**
     * ���ݾ���·���𼶴���Ŀ¼
     * @param pathorfilename ����·�����ļ�����Ŀ¼��
     * @param bisfilename ָ��pathorfilename���ͣ�true-�ļ�����false-Ŀ¼����Ĭ��true��
     * @return true-�ɹ���false-ʧ�ܣ�Ȩ�޲��㡢·���Ƿ����������ȣ�
     */
    bool newdir(const std::string& pathorfilename, bool bisfilename = true);
    // ===========================================================================

    // �ļ�������صĺ���
    // ===========================================================================
    /**
     * �������ļ�������Linux mv���
     * @param srcfilename ԭ�ļ������������·����
     * @param dstfilename Ŀ���ļ������������·����
     * @return true-�ɹ���false-ʧ�ܣ�Ȩ�޲��㡢��������������ȣ�
     * @note ���������ļ�֮ǰ�����Զ�����dstfilename�����а�����Ŀ¼����Ӧ�ÿ����У�������renamefile()��������rename()�⺯��
     */
    bool renamefile(const std::string& srcfilename, const std::string& dstfilename);
    // ===========================================================================

    // ===========================================================================
    /**
     * �����ļ�������Linux cp���
     * @param srcfilename ԭ�ļ������������·����
     * @param dstfilename Ŀ���ļ������������·����
     * @return true-�ɹ���false-ʧ�ܣ�Ȩ�޲��㡢�������ȣ�
     * @note 1. �Զ�����Ŀ��Ŀ¼��2. ������ʱ�ļ����Ʊ����м�״̬��3. ����ԭ�ļ�ʱ������
     */
    bool copyfile(const std::string& srcfilename, const std::string& dstfilename);
    // ===========================================================================

    /**
     * ��ȡ�ļ���С
     * @param filename �ļ������������·����
     * @return �ļ���С���ֽڣ���ʧ�ܷ���-1���ļ������ڡ���Ȩ�޵ȣ�
     */
    long filesize(const std::string& filename);

    /**
     * ��ȡ�ļ��޸�ʱ�䣨C�ַ����汾��
     * @param filename �ļ������������·����
     * @param mtime �洢ʱ����ַ�����
     * @param fmt ʱ���ʽ��Ĭ��"yyyymmddhh24miss"��֧��ltime���ݸ�ʽ��
     * @return true-�ɹ���false-ʧ�ܣ��ļ������ڡ���Ȩ�޵ȣ�
     */
    bool filemtime(const std::string& filename, char* mtime, const std::string& fmt = "yyyymmddhh24miss");

    /**
     * ��ȡ�ļ��޸�ʱ�䣨std::string�汾��
     * @param filename �ļ������������·����
     * @param mtime �洢ʱ����ַ�������
     * @param fmt ʱ���ʽ��Ĭ��"yyyymmddhh24miss"��
     * @return true-�ɹ���false-ʧ��
     */
    bool filemtime(const std::string& filename, std::string& mtime, const std::string& fmt = "yyyymmddhh24miss");

    /**
     * �����ļ��޸�ʱ������
     * @param filename �ļ������������·����
     * @param mtime ʱ���ַ����������yyyymmddhh24miss��˳�򲻿ɱ䣩
     * @return true-�ɹ���false-ʧ�ܣ�ʧ��ԭ���errno��
     */
    bool setmtime(const std::string& filename, const std::string& mtime);
    // ===========================================================================

    // ===========================================================================
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

        // ���캯����
        cdir() : m_pos(0), m_fmt("yyyymmddhh24miss"), m_filesize(0)
        {
        }

        /**
         * �����ļ�ʱ���ʽ
         * @param fmt ֧��"yyyy-mm-dd hh24:mi:ss"��"yyyymmddhh24miss"��Ĭ�Ϻ��ߣ�
         */
        void setfmt(const std::string& fmt);

        /**
         * ��Ŀ¼����ȡ�ļ��б������m_filelist������
         * @param dirname Ŀ¼��������·������/tmp/root��
         * @param rules �ļ���ƥ����򣨲�ƥ����ļ��������ԣ�
         * @param maxfiles ����ļ�������Ĭ��10000������ļ�̫�࣬��������̫����ڴ棩
         * @param bandchild �Ƿ�ݹ���Ŀ¼��Ĭ��false��
         * @param bsort �Ƿ��ļ�������Ĭ��false��
         * @return true-�ɹ���false-ʧ��
         */
        bool opendir(const std::string& dirname, const std::string& rules, const size_t maxfiles = 10000, const bool bandchild = false, bool bsort = false);

    private:
        /**
         * �ݹ����Ŀ¼���ڲ�ʵ�֣���opendir���ã�
         * @param dirname Ŀ¼��
         * @param rules �ļ���ƥ�����
         * @param maxfiles ����ļ�����
         * @param bandchild �Ƿ�ݹ���Ŀ¼
         * @return true-�ɹ���false-ʧ��
         */
        bool _opendir(const std::string& dirname, const std::string& rules, const size_t maxfiles, const bool bandchild);

    public:
        /**
         * ��ȡ��һ���ļ���Ϣ����m_filelist�����л�ȡһ����¼���ļ�������ͬʱ��ȡ���ļ��Ĵ�С���޸�ʱ�����Ϣ����
         * @return true-�ɹ������ݴ����Ա��������false-���޸����ļ�
         * @note ����opendir����ʱ��m_filelist��������գ�m_pos���㣬ÿ����һ��readdir����m_pos��1��
         */
        bool readdir();

        /**
         * ��ȡ�ļ��б�����
         * @return �ļ�����
         */
        size_t size()
        {
            return m_filelist.size();
        }

        // ����������
        ~cdir();
    };
    // ===========================================================================

    // ===========================================================================
    // �ļ�д���֧࣬���ı�/�����Ƽ���ʱ�ļ�����
    class cofile // class out file
    {
    private:
        std::ofstream fout;        // д���ļ��Ķ���
        std::string m_filename;    // �ļ�����������þ���·����
        std::string m_filenametmp; // ��ʱ�ļ�������m_filename�����".tmp"��
    public:
        // ���캯��
        cofile()
        {
        }

        /**
         * �ж��ļ��Ƿ��Ѵ�
         * @return true-�Ѵ򿪣�false-δ��
         */
        bool isopen() const
        {
            return fout.is_open();
        }

        /**
         * ���ļ�
         * @param filename Ŀ���ļ���
         * @param btmp �Ƿ�ʹ����ʱ�ļ���Ĭ��true����ɺ���������
         * @param mode ��ģʽ��Ĭ��std::ios::out��
         * @param benbuffer �Ƿ����û�������Ĭ��true��
         * @return true-�ɹ���false-ʧ��
         */
        bool open(const std::string& filename, const bool btmp = true, const std::ios::openmode mode = std::ios::out, const bool benbuffer = true);

        /**
         * ��ʽ��д���ı�����
         * @tparam Types �ɱ��������
         * @param fmt ��ʽ�ַ���
         * @param args ����ʽ���Ĳ���
         * @return true-�ɹ���false-ʧ��
         */
        template <typename... Types>
        bool writeline(const char* fmt, Types... args)
        {
            if (fout.is_open() == false) return false;

            fout << sformat(fmt, args...);

            return fout.good();
        }

        /**
         * ����<<�������д���ı�����
         * @tparam T ��������
         * @param value ��д�������
         * @return �������ã�֧����ʽ���ã�
         * @note ��������\n��������endl
         */
        template <typename T>
        cofile& operator<<(const T& value)
        {
            fout << value;
            return *this;
        }

        /**
         * д�����������
         * @param buf ���ݻ�����
         * @param bufsize ���ݴ�С���ֽڣ�
         * @return true-�ɹ���false-ʧ��
         */
        bool write(void* buf, int bufsize);

        /**
         * �ر��ļ�������ʱ�ļ�������ΪĿ���ļ�
         * @return true-�ɹ���false-ʧ��
         */
        bool closeandrename();

        // �ر��ļ���������ʱ�ļ���ɾ����
        void close();

        // �����������Զ��ر��ļ�
        ~cofile()
        {
            close();
        };
    };
    // ===========================================================================

    // ===========================================================================
    // �ļ���ȡ�֧࣬���ı�/�����ƶ�ȡ
    class cifile // class in file
    {
    private:
        std::ifstream fin;      // ��ȡ�ļ��Ķ���
        std::string m_filename; // �ļ�����������þ���·����
    public:
        // ���캯��
        cifile()
        {
        }

        /**
         * �ж��ļ��Ƿ��Ѵ�
         * @return true-�Ѵ򿪣�false-δ��
         */
        bool isopen() const
        {
            return fin.is_open();
        }

        /**
         * ���ļ�
         * @param filename �ļ���
         * @param mode ��ģʽ��Ĭ��std::ios::in��
         * @return true-�ɹ���false-ʧ��
         */
        bool open(const std::string& filename, const std::ios::openmode mode = std::ios::in);

        /**
         * ���ж�ȡ�ı��ļ�
         * @param buf �洢��ȡ������ַ���
         * @param endbz �н�����־��Ĭ�Ͽգ������У�
         * @return true-�ɹ���false-ʧ�ܣ����ѵ��ļ�β��
         */
        bool readline(std::string& buf, const std::string& endbz = "");

        /**
         * ��ȡ����������
         * @param buf �������ݵĻ�����
         * @param bufsize ��������С���ֽڣ�
         * @return ʵ�ʶ�ȡ���ֽ���
         */
        size_t read(void* buf, const size_t bufsize);

        /**
         * �رղ�ɾ���ļ�
         * @return true-�ɹ���false-ʧ��
         */
        bool closeandremove();

        // ֻ�ر��ļ���
        void close();

        // �����������Զ��ر��ļ�
        ~cifile()
        {
            close();
        }
    };
    // ===========================================================================

    // ===========================================================================
    // �������࣬���ڶ��߳�ͬ��
    class spinlock_mutex
    {
    private:
        std::atomic_flag flag;

        spinlock_mutex(const spinlock_mutex&) = delete;
        spinlock_mutex& operator=(const spinlock_mutex) = delete;

    public:
        // ���캯������ʼ��ԭ�ӱ�־
        spinlock_mutex()
        {
            flag.clear();
        }

        // �����������ȴ�ֱ����ȡ����
        void lock()
        {
            while (flag.test_and_set());
        }

        // ����
        void unlock()
        {
            flag.clear();
        }
    };
    // ===========================================================================

    // ===========================================================================
    // ��־�ļ��֧࣬���Զ��л��Ͷ��̰߳�ȫ
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
        /**
         * ���캯��
         * @param maxsize ��־����С��MB��Ĭ��100��
         */
        clogfile(size_t maxsize = 100) : m_mode(std::ios::app), m_backup(true), m_maxsize(maxsize), m_enbuffer(false)
        {
        }

        /**
         * ����־�ļ�
         * @param filename ��־�ļ�����������þ���·����Ŀ¼�����ڻ��Զ�������
         * @param mode ��ģʽ��Ĭ��std::ios::app��
         * @param bbackup �Ƿ��Զ����ݣ�Ĭ��true�����������Ϊfalse��
         * @param benbuffer �Ƿ����û�������Ĭ��false������д�룩
         * @return true-�ɹ���false-ʧ��
         * @note �ڶ���̵ĳ����У����������ͬһ��־�ļ�д���������־ʱ�����ܻ����С���ң����ǣ����̲߳��ᡣ
         * 1�����������ͬһ��־�ļ�д���������־ʱ�����ܻ����С���ң�������Ⲣ�����أ��������̣�
         * 2��ֻ��ͬʱд������־ʱ�Ż���ֻ��ң���ʵ�ʿ����У���������������
         * 3�����ҵ���޷����̣��������ź���������
         */
        bool open(const std::string& filename, const std::ios::openmode mode = std::ios::app, const bool bbackup = true, const bool benbuffer = false);

        /**
         * ��ʽ��д����־����ʱ��ǰ׺��
         * @tparam Types �ɱ��������
         * @param fmt ��ʽ�ַ���
         * @param args ����ʽ���Ĳ���
         * @return true-�ɹ���false-ʧ��
         */
        template <typename... Types>
        bool write(const char* fmt, Types... args)
        {
            if (fout.is_open() == false) return false;

            backup(); // �ж��Ƿ���Ҫ�л���־�ļ���

            m_splock.lock();                                  // ������
            fout << ltime1() << " " << sformat(fmt, args...); // �ѵ�ǰʱ�����־����д����־�ļ���
            m_splock.unlock();                                // ������

            return fout.good();
        }

        /**
         * ����<<�������д����־���ݣ���ʱ��ǰ׺��
         * @tparam T ��������
         * @param value ��д�������
         * @return �������ã�֧����ʽ���ã�
         * @note ������\n��������endl
         */
        template <typename T>
        clogfile& operator<<(const T& value)
        {
            m_splock.lock();
            fout << value;
            m_splock.unlock();

            return *this;
        }

    private:
        /**
         * �Զ�������־�������־�ļ��Ĵ�С����m_maxsize��ֵ���Ͱѵ�ǰ����־�ļ�����Ϊ��ʷ��־�ļ������ٴ����µĵ�ǰ��־�ļ���
         * @return true-�ɹ���false-ʧ��
         * @note �����ļ���Ϊԭ�ļ���+ʱ�������/tmp/log/filetodb.log.20200101123025��
         */
        bool backup();

    public:
        // �ر���־�ļ�
        void close()
        {
            fout.close();
        }

        // �����������Զ��ر��ļ�
        ~clogfile()
        {
            close();
        };
    };
    // ===========================================================================
#endif // defined(__linux__) || defined(_WIN32)

    // �Զ��������
    // ===========================================================================
    /**
     * �Զ��廻�в��ݷ������endl����ˢ�»�������
     * @param os �����
     * @return ���������
     */
    std::ostream& nl(std::ostream& os);

    /**
     * ��������������ṹ��
     */
    struct binary_t
    {
        // �����������ֵ
        unsigned long value;

        /**
         * ���캯��
         * @param v ��ת��Ϊ���������������
         */
        explicit binary_t(unsigned long v) : value(v)
        {
        }
    };

    /**
     * �����������������
     * @tparam T ��������
     * @param value �����������
     * @return binary_t�ṹ��
     */
    template <typename T>
    binary_t binary(T value)
    {
        return binary_t(static_cast<unsigned long>(value));
    }

    std::ostream& operator<<(std::ostream& os, const binary_t& b);

    /**
     * ������뻺����������ʣ���ַ�ֱ�����У�
     * @param is ������
     * @return ����������
     */
    std::istream& clearbuf(std::istream& is);
    // ===========================================================================

} // namespace ol

#endif // !__OL_FSTREAM_H