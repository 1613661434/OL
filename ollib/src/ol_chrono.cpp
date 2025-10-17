#include "ol_chrono.h"
#include "ol_string.h"
#include <memory>
#include <string.h>

namespace ol
{

    // Windows下实现gettimeofday()函数
#ifdef _WIN32
#include <stdint.h>
#include <windows.h>

    // 1601-01-01到1970-01-01的100ns间隔数
#define UNIX_EPOCH_DIFF 116444736000000000ULL

    int gettimeofday(struct timeval* tp, void* tzp)
    {
        (void)tzp; // 忽略时区参数

        // 1. 获取当前系统时间（高精度）
        FILETIME ft;
        GetSystemTimePreciseAsFileTime(&ft); // Windows 8+

        // 2. 转换为64位整数
        ULARGE_INTEGER uli;
        uli.LowPart = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;
        uint64_t time_in_100ns = uli.QuadPart;

        // 3. 转换为Unix纪元时间（1970-01-01 UTC）
        time_in_100ns -= UNIX_EPOCH_DIFF;

        // 4. 转换为秒和微秒
        tp->tv_sec = (long)(time_in_100ns / 10000000);         // 100ns -> 秒
        tp->tv_usec = (long)((time_in_100ns % 10000000) / 10); // 100ns -> 微秒

        return 0;
    }
#endif

    // ===========================================================================
    char* ltime(char* strtime, const std::string& fmt, const int timetvl)
    {
        if (strtime == nullptr) return nullptr; // 判断空指针。

        time_t timer;
        time(&timer); // 获取系统当前时间。

        timer = timer + timetvl; // 加上时间的偏移量。

        timetostr(timer, strtime, fmt); // 把整数表示的时间转换为字符串表示的时间。

        return strtime;
    }

    std::string& ltime(std::string& strtime, const std::string& fmt, const int timetvl)
    {
        time_t timer;
        time(&timer); // 获取系统当前时间。

        timer = timer + timetvl; // 加上时间的偏移量。

        timetostr(timer, strtime, fmt); // 把整数表示的时间转换为字符串表示的时间。

        return strtime;
    }

    std::string ltime1(const std::string& fmt, const int timetvl)
    {
        std::string strtime;

        ltime(strtime, fmt, timetvl); // 直接调用std::string& ltime(std::string &strtime,const std::string &fmt="",const int timetvl=0);

        return strtime;
    }
    // ===========================================================================

    // ===========================================================================
    // 把整数表示的时间转换为字符串表示的时间。
    // ttime：整数表示的时间。
    // strtime：字符串表示的时间。
    // fmt：输出字符串时间strtime的格式，与ttime函数的fmt参数相同，如果fmt的格式不正确，strtime将为空。
    std::string& timetostr(const time_t ttime, std::string& strtime, const std::string& fmt)
    {
        // struct tm sttm = *localtime ( &ttime );        // 非线程安全。
        struct tm sttm;
        localtime_now(&sttm, &ttime);       // 线程安全。
        sttm.tm_year = sttm.tm_year + 1900; // tm.tm_year成员要加上1900。
        ++sttm.tm_mon;                      // sttm.tm_mon成员是从0开始的，要加1。

        // 缺省的时间格式。
        if ((fmt == "") || (fmt == "yyyy-mm-dd hh24:mi:ss"))
        {
            strtime = sformat("%04u-%02u-%02u %02u:%02u:%02u", sttm.tm_year, sttm.tm_mon, sttm.tm_mday,
                              sttm.tm_hour, sttm.tm_min, sttm.tm_sec);
            return strtime;
        }

        if (fmt == "yyyy-mm-dd hh24:mi")
        {
            strtime = sformat("%04u-%02u-%02u %02u:%02u", sttm.tm_year, sttm.tm_mon, sttm.tm_mday,
                              sttm.tm_hour, sttm.tm_min);
            return strtime;
        }

        if (fmt == "yyyy-mm-dd hh24")
        {
            strtime = sformat("%04u-%02u-%02u %02u", sttm.tm_year, sttm.tm_mon, sttm.tm_mday, sttm.tm_hour);
            return strtime;
        }

        if (fmt == "yyyy-mm-dd")
        {
            strtime = sformat("%04u-%02u-%02u", sttm.tm_year, sttm.tm_mon, sttm.tm_mday);
            return strtime;
        }

        if (fmt == "yyyy-mm")
        {
            strtime = sformat("%04u-%02u", sttm.tm_year, sttm.tm_mon);
            return strtime;
        }

        if (fmt == "yyyymmddhh24miss")
        {
            strtime = sformat("%04u%02u%02u%02u%02u%02u", sttm.tm_year, sttm.tm_mon, sttm.tm_mday,
                              sttm.tm_hour, sttm.tm_min, sttm.tm_sec);
            return strtime;
        }

        if (fmt == "yyyymmddhh24mi")
        {
            strtime = sformat("%04u%02u%02u%02u%02u", sttm.tm_year, sttm.tm_mon, sttm.tm_mday,
                              sttm.tm_hour, sttm.tm_min);
            return strtime;
        }

        if (fmt == "yyyymmddhh24")
        {
            strtime = sformat("%04u%02u%02u%02u", sttm.tm_year, sttm.tm_mon, sttm.tm_mday, sttm.tm_hour);
            return strtime;
        }

        if (fmt == "yyyymmdd")
        {
            strtime = sformat("%04u%02u%02u", sttm.tm_year, sttm.tm_mon, sttm.tm_mday);
            return strtime;
        }

        if (fmt == "hh24miss")
        {
            strtime = sformat("%02u%02u%02u", sttm.tm_hour, sttm.tm_min, sttm.tm_sec);
            return strtime;
        }

        if (fmt == "hh24mi")
        {
            strtime = sformat("%02u%02u", sttm.tm_hour, sttm.tm_min);
            return strtime;
        }

        if (fmt == "hh24")
        {
            strtime = sformat("%02u", sttm.tm_hour);
            return strtime;
        }

        if (fmt == "mi")
        {
            strtime = sformat("%02u", sttm.tm_min);
            return strtime;
        }

        return strtime;
    }

    char* timetostr(const time_t ttime, char* strtime, const std::string& fmt)
    {
        if (strtime == nullptr) return nullptr; // 判断空指针。

        std::string str;
        timetostr(ttime, str, fmt); // 直接调用std::string& timetostr(const time_t ttime,std::string &strtime,const std::string &fmt="");
        str.copy(strtime, str.length());
        strtime[str.length()] = '\0'; // std::string的copy函数不会给C风格字符串的结尾加0。

        return strtime;
    }

    std::string timetostr1(const time_t ttime, const std::string& fmt)
    {
        std::string str;
        timetostr(ttime, str, fmt); // 直接调用std::string& timetostr(const time_t ttime,std::string &strtime,const std::string &fmt="");
        return str;
    }

    time_t strtotime(const std::string& strtime)
    {
        std::string strtmp, yyyy, mm, dd, hh, mi, ss;

        picknumber(strtime, strtmp, false, false); // 把字符串中的数字全部提取出来。
        // 2021-12-05 08:30:45
        // 2021/12/05 08:30:45
        // 20211205083045

        if (strtmp.length() != 14) return -1; // 如果时间格式不是yyyymmddhh24miss，说明时间格式不正确。

        yyyy = strtmp.substr(0, 4);
        mm = strtmp.substr(4, 2);
        dd = strtmp.substr(6, 2);
        hh = strtmp.substr(8, 2);
        mi = strtmp.substr(10, 2);
        ss = strtmp.substr(12, 2);

        struct tm sttm;

        try
        {
            sttm.tm_year = stoi(yyyy) - 1900; // 需要减去 1900 是因为 tm_year 存储的是 自 1900 年起的年数。历史原因：C 语言的时间库（C++ 继承自 C）设计于 1970 年代，当时用 1900 作为基准点可以节省内存（只需一个整数表示年份偏移）。
            sttm.tm_mon = stoi(mm) - 1;       // 需要减去 1 是因为月份范围是 0–11（而非 1–12）
            sttm.tm_mday = stoi(dd);
            sttm.tm_hour = stoi(hh);
            sttm.tm_min = stoi(mi);
            sttm.tm_sec = stoi(ss);
            sttm.tm_isdst = 0; // 夏令时 (tm_isdst)：0 = 不启用夏令时，正数 = 启用夏令时，负数 = 信息不可用。
        }
        catch (const std::exception&)
        {
            return -1;
        }

        return mktime(&sttm);
    }
    // ===========================================================================

    // ===========================================================================
    bool addtime(const std::string& in_stime, std::string& out_stime, const int timetvl, const std::string& fmt)
    {
        time_t timer;

        // 把字符串表示的时间转换为整数表示的时间，方便运算。
        if ((timer = strtotime(in_stime)) == -1)
        {
            out_stime = "";
            return false;
        }

        timer = timer + timetvl; // 时间运算。

        // 把整数表示的时间转换为字符串表示的时间。
        timetostr(timer, out_stime, fmt);

        return true;
    }

    bool addtime(const std::string& in_stime, char* out_stime, const int timetvl, const std::string& fmt)
    {
        if (out_stime == nullptr) return false; // 判断空指针。

        time_t timer;

        // 把字符串表示的时间转换为整数表示的时间，方便运算。
        if ((timer = strtotime(in_stime)) == -1)
        {
            strcpy(out_stime, "");
            return false;
        }

        timer = timer + timetvl; // 时间运算。

        // 把整数表示的时间转换为字符串表示的时间。
        timetostr(timer, out_stime, fmt);

        return true;
    }
    // ===========================================================================

    // ===========================================================================
    ctimer::ctimer()
    {
        start(); // 计时开始。
    }

    // 计时开始。
    void ctimer::start()
    {
        memset(&m_start, 0, sizeof(struct timeval));
        memset(&m_end, 0, sizeof(struct timeval));

        gettimeofday(&m_start, 0); // 获取当前时间，精确到微秒。
    }

    // 计算已逝去的时间，单位：秒，小数点后面是微秒
    // 每调用一次本方法之后，自动调用Start方法重新开始计时。
    double ctimer::elapsed()
    {
        gettimeofday(&m_end, 0); // 获取当前时间作为计时结束的时间，精确到微秒。

        // 秒差 + 微秒差（转成秒）
        double diff = (m_end.tv_sec - m_start.tv_sec) +
                      (m_end.tv_usec - m_start.tv_usec) / 1e6;

        start(); // 重新开始计时。

        return diff;
    }
    // ===========================================================================
} // namespace ol