#ifndef __OL_FSTREAM_H
#define __OL_FSTREAM_H 1

// 禁用Windows的min/max宏
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX  // 仅在未定义时定义
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
#elif defined(_WIN32)  // Windows 平台头文件
#include <direct.h>    // 目录操作
#include <io.h>        // 替代 unistd.h
#include <sys/utime.h> // Windows 下的 utime 定义
#include <time.h>      // 时间函数
#endif                 // _WIN32

namespace ol
{

#if defined(__linux__) || defined(_WIN32)
    ///////////////////////////////////// /////////////////////////////////////
    // 根据绝对路径的文件名或目录名逐级的创建目录。
    // pathorfilename：绝对路径的文件名或目录名。
    // bisfilename：指定pathorfilename的类型，true-pathorfilename是文件名，否则是目录名，缺省值为true。
    // 返回值：true-成功，false-失败，如果返回失败，原因有大概有三种情况：
    // 1）权限不足；2）pathorfilename参数不是合法的文件名或目录名；3）磁盘空间不足。
    bool newdir(const std::string& pathorfilename, bool bisfilename = true);
    ///////////////////////////////////// /////////////////////////////////////

    // 文件操作相关的函数
    ///////////////////////////////////// /////////////////////////////////////
    // 重命名文件，类似Linux系统的mv命令。
    // srcfilename：原文件名，建议采用绝对路径的文件名。
    // dstfilename：目标文件名，建议采用绝对路径的文件名。
    // 返回值：true-成功；false-失败，失败的主要原因是权限不足或磁盘空间不够，如果原文件和目标文件不在同一个磁盘分区，重命名也可能失败。
    // 注意，在重命名文件之前，会自动创建dstfilename参数中包含的目录。
    // 在应用开发中，可以用renamefile()函数代替rename()库函数。
    bool renamefile(const std::string& srcfilename, const std::string& dstfilename);
    ///////////////////////////////////// /////////////////////////////////////

    // 复制文件，类似Linux系统的cp命令。
    // srcfilename：原文件名，建议采用绝对路径的文件名。
    // dstfilename：目标文件名，建议采用绝对路径的文件名。
    // 返回值：true-成功；false-失败，失败的主要原因是权限不足或磁盘空间不够。
    // 注意：
    // 1）在复制文件之前，会自动创建dstfilename参数中的目录名。
    // 2）复制文件的过程中，采用临时文件命名的方法，复制完成后再改名为dstfilename，避免中间状态的文件被读取。
    // 3）复制后的文件的时间与原文件相同，这一点与Linux系统cp命令不同。
    bool copyfile(const std::string& srcfilename, const std::string& dstfilename);

    // 获取文件的大小。
    // filename：待获取的文件名，建议采用绝对路径的文件名。
    // 返回值：如果文件不存在或没有访问权限，返回-1，成功返回文件的大小，单位是字节。
    long filesize(const std::string& filename);

    // 获取文件的时间。
    // filename：待获取的文件名，建议采用绝对路径的文件名。
    // mtime：用于存放文件的时间，即stat结构体的st_mtime。
    // fmt：设置时间的输出格式，与ltime()函数相同，但缺省是"yyyymmddhh24miss"。
    // 返回值：如果文件不存在或没有访问权限，返回false，成功返回true。
    bool filemtime(const std::string& filename, char* mtime, const std::string& fmt = "yyyymmddhh24miss");
    bool filemtime(const std::string& filename, std::string& mtime, const std::string& fmt = "yyyymmddhh24miss");

    // 重置文件的修改时间属性。
    // filename：待重置的文件名，建议采用绝对路径的文件名。
    // mtime：字符串表示的时间，格式不限，但一定要包括yyyymmddhh24miss，一个都不能少，顺序也不能变。
    // 返回值：true-成功；false-失败，失败的原因保存在errno中。
    bool setmtime(const std::string& filename, const std::string& mtime);
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // 获取某目录及其子目录中的文件列表的类。
    class cdir
    {
    private:
        std::vector<std::string> m_filelist; // 存放文件列表的容器（绝对路径的文件名）。
        size_t m_pos;                        // 从文件列表m_filelist中已读取文件的位置。
        std::string m_fmt;                   // 文件时间格式，缺省"yyyymmddhh24miss"。

        cdir(const cdir&) = delete;            // 禁用拷贝构造函数。
        cdir& operator=(const cdir&) = delete; // 禁用赋值函数。
    public:
        // /project/public/_public.h
        std::string m_dirname;   // 目录名，例如：/project/public
        std::string m_filename;  // 文件名，不包括目录名，例如：_public.h
        std::string m_ffilename; // 绝对路径的文件，例如：/project/public/_public.h
        size_t m_filesize;       // 文件的大小，单位：字节。
        std::string m_mtime;     // 文件最后一次被修改的时间，即stat结构体的st_mtime成员。
        std::string m_ctime;     // 文件生成的时间，即stat结构体的st_ctime成员。
        std::string m_atime;     // 文件最后一次被访问的时间，即stat结构体的st_atime成员。

        cdir() : m_pos(0), m_fmt("yyyymmddhh24miss"), m_filesize(0)
        {
        } // 构造函数。

        // 设置文件时间的格式，支持"yyyy-mm-dd hh24:mi:ss"和"yyyymmddhh24miss"两种，缺省是后者。
        void setfmt(const std::string& fmt);

        // 打开目录，获取目录中文件的列表，存放在m_filelist容器中。
        // dirname，目录名，采用绝对路径，如/tmp/root。
        // rules，文件名的匹配规则，不匹配的文件将被忽略。
        // maxfiles，本次获取文件的最大数量，缺省值为10000个，如果文件太多，可能消耗太多的内存。
        // bandchild，是否打开各级子目录，缺省值为false-不打开子目录。
        // bsort，是否按文件名排序，缺省值为false-不排序。
        // 返回值：true-成功，false-失败。
        bool opendir(const std::string& dirname, const std::string& rules, const size_t maxfiles = 10000, const bool bandchild = false, bool bsort = false);

    private:
        // 这是一个递归函数，被opendir()的调用，在cdir类的外部不需要调用它。
        bool _opendir(const std::string& dirname, const std::string& rules, const size_t maxfiles, const bool bandchild);

    public:
        // 从m_filelist容器中获取一条记录（文件名），同时获取该文件的大小、修改时间等信息。
        // 调用opendir方法时，m_filelist容器被清空，m_pos归零，每调用一次readdir方法m_pos加1。
        // 当m_pos小于m_filelist.size()，返回true，否则返回false。
        bool readdir();

        size_t size()
        {
            return m_filelist.size();
        }

        ~cdir(); // 析构函数。
    };
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // 写文件的类。
    class cofile // class out file
    {
    private:
        std::ofstream fout;        // 写入文件的对象。
        std::string m_filename;    // 文件名，建议采用绝对路径。
        std::string m_filenametmp; // 临时文件名，在m_filename后面加".tmp"。
    public:
        cofile()
        {
        }

        // 文件是否已打开。
        bool isopen() const
        {
            return fout.is_open();
        }

        // 打开文件。
        // filename，待打开的文件名。
        // btmp，是否采用临时文件的方案。
        // mode，打开文件的模式。
        // benbuffer，是否启用文件缓冲区。
        bool open(const std::string& filename, const bool btmp = true, const std::ios::openmode mode = std::ios::out, const bool benbuffer = true);

        // 把数据以文本的方式格式化输出到文件。
        template <typename... Args>
        bool writeline(const char* fmt, Args... args)
        {
            if (fout.is_open() == false) return false;

            fout << sformat(fmt, args...);

            return fout.good();
        }

        // 重载<<运算符，把数据以文本的方式输出到文件。
        // 注意：换行只能用\n，不能用endl。
        template <typename T>
        cofile& operator<<(const T& value)
        {
            fout << value;
            return *this;
        }

        // 把二进制数据写入文件。
        bool write(void* buf, int bufsize);

        // 关闭文件，并且把临时文件名改为正式文件名。
        bool closeandrename();

        // 关闭文件，如果有临时文件，则删除它。
        void close();

        ~cofile()
        {
            close();
        };
    };
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // 读取文件的类。
    class cifile // class in file
    {
    private:
        std::ifstream fin;      // 读取文件的对象。
        std::string m_filename; // 文件名，建议采用绝对路径。
    public:
        cifile()
        {
        }

        // 判断文件是否已打开。
        bool isopen() const
        {
            return fin.is_open();
        }

        // 打开文件。
        // filename，待打开的文件名。
        // mode，打开文件的模式。
        bool open(const std::string& filename, const std::ios::openmode mode = std::ios::in);

        // 以行的方式读取文本文件，endbz指定行的结尾标志，缺省为空，没有结尾标志。
        bool readline(std::string& buf, const std::string& endbz = "");

        // 读取二进制文件，返回实际读取到的字节数。
        size_t read(void* buf, const size_t bufsize);

        // 关闭并删除文件。
        bool closeandremove();

        // 只关闭文件。
        void close();

        ~cifile()
        {
            close();
        }
    };
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // 自旋锁。
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
        void lock() // 加锁。
        {
            while (flag.test_and_set());
        }
        void unlock() // 解锁。
        {
            flag.clear();
        }
    };
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // 日志文件。
    class clogfile
    {
        std::ofstream fout;        // 日志文件对象。
        std::string m_filename;    // 日志文件名，建议采用绝对路径。
        std::ios::openmode m_mode; // 日志文件的打开模式。
        bool m_backup;             // 是否自动切换日志。
        size_t m_maxsize;          // 当日志文件的大小超过本参数时，自动切换日志。
        bool m_enbuffer;           // 是否启用文件缓冲区。
        spinlock_mutex m_splock;   // 自旋锁，用于多线程程序中给写日志的操作加锁。

    public:
        // 构造函数，日志文件的大小缺省100M。
        clogfile(size_t maxsize = 100) : m_mode(std::ios::app), m_backup(true), m_maxsize(maxsize), m_enbuffer(false)
        {
        }

        // 打开日志文件。
        // filename：日志文件名，建议采用绝对路径，如果文件名中的目录不存在，就先创建目录。
        // openmode：日志文件的打开模式，缺省值是ios::app。
        // bbackup：是否自动切换（备份），true-切换，false-不切换，在多进程的服务程序中，如果多个进程共用一个日志文件，bbackup必须为false。
        // benbuffer：是否启用文件缓冲机制，true-启用，false-不启用，如果启用缓冲区，那么写进日志文件中的内容不会立即写入文件，缺省是不启用。
        // 注意，在多进程的程序中，多个进程往同一日志文件写入大量的日志时，可能会出现小混乱，但是，多线程不会。
        // 1）多个进程往同一日志文件写入大量的日志时，可能会出现小混乱，这个问题并不严重，可以容忍；
        // 2）只有同时写大量日志时才会出现混乱，在实际开发中，这种情况不多见。
        // 3）如果业务无法容忍，可以用信号量加锁。
        bool open(const std::string& filename, const std::ios::openmode mode = std::ios::app, const bool bbackup = true, const bool benbuffer = false);

        // 把日志内容以文本的方式格式化输出到日志文件，并且，在日志内容前面写入时间。
        template <typename... Args>
        bool write(const char* fmt, Args... args)
        {
            if (fout.is_open() == false) return false;

            backup(); // 判断是否需要切换日志文件。

            m_splock.lock();                                  // 加锁。
            fout << ltime1() << " " << sformat(fmt, args...); // 把当前时间和日志内容写入日志文件。
            m_splock.unlock();                                // 解锁。

            return fout.good();
        }

        // 重载<<运算符，把日志内容以文本的方式输出到日志文件，不会在日志内容前面写时间。
        // 注意：内容换行用\n，不能用endl。
        template <typename T>
        clogfile& operator<<(const T& value)
        {
            m_splock.lock();
            fout << value;
            m_splock.unlock();

            return *this;
        }

    private:
        // 如果日志文件的大小超过m_maxsize的值，就把当前的日志文件名改为历史日志文件名，再创建新的当前日志文件。
        // 备份后的文件会在日志文件名后加上日期时间，如/tmp/log/filetodb.log.20200101123025。
        // 注意，在多进程的程序中，日志文件不可切换，多线的程序中，日志文件可以切换。
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
    // 自定义操作符
    // ======================

    // 换行操纵符
    std::ostream& nl(std::ostream& os);

    // 输出整数的二进制表示
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

    // 清空输入缓冲区（忽略所有剩余字符直到换行符）
    std::istream& clearbuf(std::istream& is);
    ///////////////////////////////////// /////////////////////////////////////

} // namespace ol

#endif // !__OL_FSTREAM_H