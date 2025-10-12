/****************************************************************************************/
/*
 * 程序名：ol_fstream.h
 * 功能描述：文件系统操作工具类及函数集合，支持跨平台（Linux/Windows）操作，特性包括：
 *          - 目录创建、文件重命名、复制、大小/时间获取等基础文件操作
 *          - 目录遍历类（cdir），支持递归获取文件列表及属性
 *          - 文件读写类（cofile/cifile），支持文本/二进制操作及临时文件机制
 *          - 日志文件类（clogfile），支持自动切换、多线程安全
 *          - 辅助工具：自旋锁、自定义输出操作符等
 * 作者：ol
 * 适用标准：C++11及以上（需支持atomic、fstream、变参模板等特性）
 */
/****************************************************************************************/

#ifndef OL_FSTREAM_H
#define OL_FSTREAM_H 1

// 禁用Windows的min/max宏
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX // 仅在未定义时定义
#endif
#endif

#include "ol_chrono.h"
#include "ol_mutex.h"
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
#elif defined(_WIN32)  // Windows 平台头文件
#include <direct.h>    // 目录操作
#include <io.h>        // 替代 unistd.h
#include <sys/utime.h> // Windows 下的 utime 定义
#include <time.h>      // 时间函数
#endif                 // _WIN32

namespace ol
{

#if defined(__linux__) || defined(_WIN32)
    // ===========================================================================
    /**
     * @brief 根据绝对路径逐级创建目录
     * @param pathorfilename 绝对路径的文件名或目录名
     * @param bisfilename 指定pathorfilename类型（true-文件名，false-目录名，默认true）
     * @return true-成功，false-失败（权限不足、路径非法、磁盘满等）
     */
    bool newdir(const std::string& pathorfilename, bool bisfilename = true);
    // ===========================================================================

    // 文件操作相关的函数
    // ===========================================================================
    /**
     * @brief 重命名文件（类似Linux mv命令）
     * @param srcfilename 原文件名（建议绝对路径）
     * @param dstfilename 目标文件名（建议绝对路径）
     * @return true-成功，false-失败（权限不足、跨分区、磁盘满等）
     * @note 在重命名文件之前，会自动创建dstfilename参数中包含的目录，在应用开发中，可以用renamefile()函数代替rename()库函数
     */
    bool renamefile(const std::string& srcfilename, const std::string& dstfilename);
    // ===========================================================================

    // ===========================================================================
    /**
     * @brief 复制文件（类似Linux cp命令）
     * @param srcfilename 原文件名（建议绝对路径）
     * @param dstfilename 目标文件名（建议绝对路径）
     * @return true-成功，false-失败（权限不足、磁盘满等）
     * @note 1. 自动创建目标目录；2. 采用临时文件机制避免中间状态；3. 保留原文件时间属性
     */
    bool copyfile(const std::string& srcfilename, const std::string& dstfilename);
    // ===========================================================================

    /**
     * @brief 获取文件大小
     * @param filename 文件名（建议绝对路径）
     * @return 文件大小（字节），失败返回-1（文件不存在、无权限等）
     */
    long filesize(const std::string& filename);

    /**
     * @brief 获取文件修改时间（C字符串版本）
     * @param filename 文件名（建议绝对路径）
     * @param mtime 存储时间的字符数组
     * @param fmt 时间格式（默认"yyyymmddhh24miss"，支持ltime兼容格式）
     * @return true-成功，false-失败（文件不存在、无权限等）
     */
    bool filemtime(const std::string& filename, char* mtime, const std::string& fmt = "yyyymmddhh24miss");

    /**
     * @brief 获取文件修改时间（std::string版本）
     * @param filename 文件名（建议绝对路径）
     * @param mtime 存储时间的字符串引用
     * @param fmt 时间格式（默认"yyyymmddhh24miss"）
     * @return true-成功，false-失败
     */
    bool filemtime(const std::string& filename, std::string& mtime, const std::string& fmt = "yyyymmddhh24miss");

    /**
     * @brief 重置文件修改时间属性
     * @param filename 文件名（建议绝对路径）
     * @param mtime 时间字符串（需包含yyyymmddhh24miss，顺序不可变）
     * @return true-成功，false-失败（失败原因见errno）
     */
    bool setmtime(const std::string& filename, const std::string& mtime);
    // ===========================================================================

    // ===========================================================================
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
        std::string m_dirname;   // 目录名，例如：/project/public
        std::string m_filename;  // 文件名，不包括目录名，例如：_public.h
        std::string m_ffilename; // 绝对路径的文件，例如：/project/public/_public.h
        size_t m_filesize;       // 文件的大小，单位：字节。
        std::string m_mtime;     // 文件最后一次被修改的时间，即stat结构体的st_mtime成员。
        std::string m_ctime;     // 文件生成的时间，即stat结构体的st_ctime成员。
        std::string m_atime;     // 文件最后一次被访问的时间，即stat结构体的st_atime成员。

        // 构造函数。
        cdir() : m_pos(0), m_fmt("yyyymmddhh24miss"), m_filesize(0)
        {
        }

        /**
         * @brief 设置文件时间格式
         * @param fmt 支持"yyyy-mm-dd hh24:mi:ss"和"yyyymmddhh24miss"（默认后者）
         */
        void setfmt(const std::string& fmt);

        /**
         * @brief 打开目录并获取文件列表，存放在m_filelist容器中
         * @param dirname 目录名（绝对路径，如/tmp/root）
         * @param rules 文件名匹配规则（不匹配的文件将被忽略）
         * @param maxfiles 最大文件数量（默认10000，如果文件太多，可能消耗太多的内存）
         * @param bandchild 是否递归子目录（默认false）
         * @param bsort 是否按文件名排序（默认false）
         * @param bwithDotFiles 是否包含.开头的特殊目录和文件（默认false）
         * @return true-成功，false-失败
         */
        bool opendir(const std::string& dirname, const std::string& rules, const size_t maxfiles = 10000, const bool bandchild = false, bool bsort = false, const bool bwithDotFiles = false);

    private:
        /**
         * @brief 递归遍历目录的内部实现（被opendir调用）
         * @param dirname 目录名
         * @param rules 文件名匹配规则
         * @param maxfiles 最大文件数量
         * @param bandchild 是否递归子目录
         * @param bwithDotFiles 是否包含.开头的特殊目录和文件
         * @return true-成功，false-失败
         */
        bool _opendir(const std::string& dirname, const std::string& rules, const size_t maxfiles, const bool bandchild, const bool bwithDotFiles);

    public:
        /**
         * @brief 读取下一个文件信息（从m_filelist容器中获取一条记录（文件名），同时获取该文件的大小、修改时间等信息。）
         * @return true-成功（数据存入成员变量），false-已无更多文件
         * @note 调用opendir方法时，m_filelist容器被清空，m_pos归零，每调用一次readdir方法m_pos加1。
         */
        bool readdir();

        /**
         * @brief 获取文件列表总数
         * @return 文件数量
         */
        size_t size()
        {
            return m_filelist.size();
        }

        // 析构函数。
        ~cdir();
    };
    // ===========================================================================

    // ===========================================================================
    // 文件写入类，支持文本/二进制及临时文件机制
    class cofile // class out file
    {
    private:
        std::ofstream fout;        // 写入文件的对象。
        std::string m_filename;    // 文件名，建议采用绝对路径。
        std::string m_filenametmp; // 临时文件名，在m_filename后面加".tmp"。
    public:
        // 构造函数
        cofile()
        {
        }

        /**
         * @brief 判断文件是否已打开
         * @return true-已打开，false-未打开
         */
        bool isopen() const
        {
            return fout.is_open();
        }

        /**
         * @brief 打开文件
         * @param filename 目标文件名
         * @param btmp 是否使用临时文件（默认true，完成后重命名）
         * @param mode 打开模式（默认std::ios::out）
         * @param benbuffer 是否启用缓冲区（默认true）
         * @return true-成功，false-失败
         */
        bool open(const std::string& filename, const bool btmp = true, const std::ios::openmode mode = std::ios::out, const bool benbuffer = true);

        /**
         * @brief 格式化写入文本数据
         * @tparam Types 可变参数类型
         * @param fmt 格式字符串
         * @param args 待格式化的参数
         * @return true-成功，false-失败
         */
        template <typename... Types>
        bool writeline(const char* fmt, Types... args)
        {
            if (fout.is_open() == false) return false;

            fout << sformat(fmt, args...);

            return fout.good();
        }

        /**
         * @brief 重载<<运算符，写入文本数据
         * @tparam T 数据类型
         * @param value 待写入的数据
         * @return 自身引用（支持链式调用）
         * @note 换行需用\n，不可用endl
         */
        template <typename T>
        cofile& operator<<(const T& value)
        {
            fout << value;
            return *this;
        }

        /**
         * @brief 写入二进制数据
         * @param buf 数据缓冲区
         * @param bufsize 数据大小（字节）
         * @return true-成功，false-失败
         */
        bool write(void* buf, size_t bufsize);

        /**
         * @brief 关闭文件并将临时文件重命名为目标文件
         * @return true-成功，false-失败
         */
        bool closeandrename();

        // 关闭文件（若有临时文件则删除）
        void close();

        // 析构函数，自动关闭文件
        ~cofile()
        {
            close();
        };
    };
    // ===========================================================================

    // ===========================================================================
    // 文件读取类，支持文本/二进制读取
    class cifile // class in file
    {
    private:
        std::ifstream fin;      // 读取文件的对象。
        std::string m_filename; // 文件名，建议采用绝对路径。
    public:
        // 构造函数
        cifile()
        {
        }

        /**
         * @brief 判断文件是否已打开
         * @return true-已打开，false-未打开
         */
        bool isopen() const
        {
            return fin.is_open();
        }

        /**
         * @brief 打开文件
         * @param filename 文件名
         * @param mode 打开模式（默认std::ios::in）
         * @return true-成功，false-失败
         */
        bool open(const std::string& filename, const std::ios::openmode mode = std::ios::in);

        /**
         * @brief 按行读取文本文件
         * @param buf 存储读取结果的字符串
         * @param endbz 行结束标志（默认空，即换行）
         * @return true-成功，false-失败（如已到文件尾）
         */
        bool readline(std::string& buf, const std::string& endbz = "");

        /**
         * @brief 读取二进制数据
         * @param buf 接收数据的缓冲区
         * @param bufsize 缓冲区大小（字节）
         * @return 实际读取的字节数
         */
        size_t read(void* buf, const size_t bufsize);

        /**
         * @brief 关闭并删除文件
         * @return true-成功，false-失败
         */
        bool closeandremove();

        // 只关闭文件。
        void close();

        // 析构函数，自动关闭文件
        ~cifile()
        {
            close();
        }
    };
    // ===========================================================================

    // ===========================================================================
    // 日志文件类，支持自动切换和多线程安全
    class clogfile
    {
        std::ofstream fout;        // 日志文件对象。
        std::string m_filename;    // 日志文件名，建议采用绝对路径。
        std::ios::openmode m_mode; // 日志文件的打开模式。
        bool m_backup;             // 是否自动切换日志。
        size_t m_maxsize;          // 当日志文件的大小超过本参数时，自动切换日志。
        bool m_enbuffer;           // 是否启用文件缓冲区。
        spin_mutex m_splock;       // 自旋锁，用于多线程程序中给写日志的操作加锁。

    public:
        /**
         * @brief 构造函数
         * @param maxsize 日志最大大小（MB，默认100）
         */
        clogfile(size_t maxsize = 100) : m_mode(std::ios::app), m_backup(true), m_maxsize(maxsize), m_enbuffer(false)
        {
        }

        /**
         * @brief 打开日志文件
         * @param filename 日志文件名（建议采用绝对路径，目录不存在会自动创建）
         * @param mode 打开模式（默认std::ios::app）
         * @param bbackup 是否自动备份（默认true，多进程需设为false）
         * @param benbuffer 是否启用缓冲区（默认false，立即写入）
         * @return true-成功，false-失败
         * @note 在多进程的程序中，多个进程往同一日志文件写入大量的日志时，可能会出现小混乱，但是，多线程不会。
         * 1）多个进程往同一日志文件写入大量的日志时，可能会出现小混乱，这个问题并不严重，可以容忍；
         * 2）只有同时写大量日志时才会出现混乱，在实际开发中，这种情况不多见。
         * 3）如果业务无法容忍，可以用信号量加锁。
         */
        bool open(const std::string& filename, const std::ios::openmode mode = std::ios::app, const bool bbackup = true, const bool benbuffer = false);

        /**
         * @brief 格式化写入日志（带时间前缀）
         * @tparam Types 可变参数类型
         * @param fmt 格式字符串
         * @param args 待格式化的参数
         * @return true-成功，false-失败
         */
        template <typename... Types>
        bool write(const char* fmt, Types... args)
        {
            if (fout.is_open() == false) return false;

            backup(); // 判断是否需要切换日志文件。

            m_splock.lock();                                  // 加锁。
            fout << ltime1() << " " << sformat(fmt, args...); // 把当前时间和日志内容写入日志文件。
            m_splock.unlock();                                // 解锁。

            return fout.good();
        }

        /**
         * @brief 重载<<运算符，写入日志内容（无时间前缀）
         * @tparam T 数据类型
         * @param value 待写入的内容
         * @return 自身引用（支持链式调用）
         * @note 换行用\n，不可用endl
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
         * @brief 自动备份日志（如果日志文件的大小超过m_maxsize的值，就把当前的日志文件名改为历史日志文件名，再创建新的当前日志文件）
         * @return true-成功，false-失败
         * @note 备份文件名为原文件名+时间戳（如/tmp/log/filetodb.log.20200101123025）
         */
        bool backup();

    public:
        // 关闭日志文件
        void close()
        {
            fout.close();
        }

        // 析构函数，自动关闭文件
        ~clogfile()
        {
            close();
        };
    };
    // ===========================================================================
#endif // defined(__linux__) || defined(_WIN32)

    // 自定义操作符
    // ===========================================================================
    /**
     * @brief 自定义换行操纵符（替代endl，不刷新缓冲区）
     * @param os 输出流
     * @return 输出流引用
     */
    std::ostream& nl(std::ostream& os);

    // 二进制输出辅助结构体
    struct binary_t
    {
        // 待输出的整数值
        unsigned long value;

        /**
         * @brief 构造函数
         * @param v 待转换为二进制输出的整数
         */
        explicit binary_t(unsigned long v) : value(v)
        {
        }
    };

    /**
     * @brief 二进制输出辅助函数
     * @tparam T 整数类型
     * @param value 待输出的整数
     * @return binary_t结构体
     */
    template <typename T>
    binary_t binary(T value)
    {
        return binary_t(static_cast<unsigned long>(value));
    }

    std::ostream& operator<<(std::ostream& os, const binary_t& b);

    /**
     * @brief 清空输入缓冲区（忽略剩余字符直到换行）
     * @param is 输入流
     * @return 输入流引用
     */
    std::istream& clearbuf(std::istream& is);
    // ===========================================================================

} // namespace ol

#endif // !OL_FSTREAM_H