#include "ol_chrono.h"
#include "ol_string.h"
#include <memory>
#include <string.h>

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)
#endif

namespace ol
{
    // ��ƽ̨����
    inline void localtime_now(struct tm* tm_out, const time_t* t)
    {
#ifdef _WIN32
        localtime_s(tm_out, t); // Windows
#else
        localtime_r(t, tm_out); // Linux/Unix/macOS
#endif
    }

    // Windows��ʵ��gettimeofday()����
#ifdef _WIN32
#include <stdint.h>
#include <windows.h>

    // 1601-01-01��1970-01-01��100ns�����
#define UNIX_EPOCH_DIFF 116444736000000000ULL

    int gettimeofday(struct timeval* tp, void* tzp)
    {
        (void)tzp; // ����ʱ������

        static int initialized = 0;
        static uint64_t ticks_per_sec = 0;

        // 1. ��ȡ��ǰϵͳʱ�䣨�߾��ȣ�
        FILETIME ft;
        GetSystemTimePreciseAsFileTime(&ft); // Windows 8+

        // 2. ת��Ϊ64λ����
        ULARGE_INTEGER uli;
        uli.LowPart = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;
        uint64_t time_in_100ns = uli.QuadPart;

        // 3. ת��ΪUnix��Ԫʱ�䣨1970-01-01 UTC��
        time_in_100ns -= UNIX_EPOCH_DIFF;

        // 4. ת��Ϊ���΢��
        tp->tv_sec = (long)(time_in_100ns / 10000000);         // 100ns -> ��
        tp->tv_usec = (long)((time_in_100ns % 10000000) / 10); // 100ns -> ΢��

        return 0;
    }
#endif

    // ===========================================================================
    char* ltime(char* strtime, const std::string& fmt, const int timetvl)
    {
        if (strtime == nullptr) return nullptr; // �жϿ�ָ�롣

        time_t timer;
        time(&timer); // ��ȡϵͳ��ǰʱ�䡣

        timer = timer + timetvl; // ����ʱ���ƫ������

        timetostr(timer, strtime, fmt); // ��������ʾ��ʱ��ת��Ϊ�ַ�����ʾ��ʱ�䡣

        return strtime;
    }

    std::string& ltime(std::string& strtime, const std::string& fmt, const int timetvl)
    {
        time_t timer;
        time(&timer); // ��ȡϵͳ��ǰʱ�䡣

        timer = timer + timetvl; // ����ʱ���ƫ������

        timetostr(timer, strtime, fmt); // ��������ʾ��ʱ��ת��Ϊ�ַ�����ʾ��ʱ�䡣

        return strtime;
    }

    std::string ltime1(const std::string& fmt, const int timetvl)
    {
        std::string strtime;

        ltime(strtime, fmt, timetvl); // ֱ�ӵ���std::string& ltime(std::string &strtime,const std::string &fmt="",const int timetvl=0);

        return strtime;
    }
    // ===========================================================================

    // ===========================================================================
    // ��������ʾ��ʱ��ת��Ϊ�ַ�����ʾ��ʱ�䡣
    // ttime��������ʾ��ʱ�䡣
    // strtime���ַ�����ʾ��ʱ�䡣
    // fmt������ַ���ʱ��strtime�ĸ�ʽ����ttime������fmt������ͬ�����fmt�ĸ�ʽ����ȷ��strtime��Ϊ�ա�
    std::string& timetostr(const time_t ttime, std::string& strtime, const std::string& fmt)
    {
        // struct tm sttm = *localtime ( &ttime );        // ���̰߳�ȫ��
        struct tm sttm;
        localtime_now(&sttm, &ttime);       // �̰߳�ȫ��
        sttm.tm_year = sttm.tm_year + 1900; // tm.tm_year��ԱҪ����1900��
        ++sttm.tm_mon;                      // sttm.tm_mon��Ա�Ǵ�0��ʼ�ģ�Ҫ��1��

        // ȱʡ��ʱ���ʽ��
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
        if (strtime == nullptr) return nullptr; // �жϿ�ָ�롣

        std::string str;
        timetostr(ttime, str, fmt); // ֱ�ӵ���std::string& timetostr(const time_t ttime,std::string &strtime,const std::string &fmt="");
        str.copy(strtime, str.length());
        strtime[str.length()] = '\0'; // std::string��copy���������C����ַ����Ľ�β��0��

        return strtime;
    }

    std::string timetostr1(const time_t ttime, const std::string& fmt)
    {
        std::string str;
        timetostr(ttime, str, fmt); // ֱ�ӵ���std::string& timetostr(const time_t ttime,std::string &strtime,const std::string &fmt="");
        return str;
    }

    time_t strtotime(const std::string& strtime)
    {
        std::string strtmp, yyyy, mm, dd, hh, mi, ss;

        picknumber(strtime, strtmp, false, false); // ���ַ����е�����ȫ����ȡ������
        // 2021-12-05 08:30:45
        // 2021/12/05 08:30:45
        // 20211205083045

        if (strtmp.length() != 14) return -1; // ���ʱ���ʽ����yyyymmddhh24miss��˵��ʱ���ʽ����ȷ��

        yyyy = strtmp.substr(0, 4);
        mm = strtmp.substr(4, 2);
        dd = strtmp.substr(6, 2);
        hh = strtmp.substr(8, 2);
        mi = strtmp.substr(10, 2);
        ss = strtmp.substr(12, 2);

        struct tm sttm;

        try
        {
            sttm.tm_year = stoi(yyyy) - 1900; // ��Ҫ��ȥ 1900 ����Ϊ tm_year �洢���� �� 1900 �������������ʷԭ��C ���Ե�ʱ��⣨C++ �̳��� C������� 1970 �������ʱ�� 1900 ��Ϊ��׼����Խ�ʡ�ڴ棨ֻ��һ��������ʾ���ƫ�ƣ���
            sttm.tm_mon = stoi(mm) - 1;       // ��Ҫ��ȥ 1 ����Ϊ�·ݷ�Χ�� 0�C11������ 1�C12��
            sttm.tm_mday = stoi(dd);
            sttm.tm_hour = stoi(hh);
            sttm.tm_min = stoi(mi);
            sttm.tm_sec = stoi(ss);
            sttm.tm_isdst = 0; // ����ʱ (tm_isdst)��0 = ����������ʱ������ = ��������ʱ������ = ��Ϣ�����á�
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

        // ���ַ�����ʾ��ʱ��ת��Ϊ������ʾ��ʱ�䣬�������㡣
        if ((timer = strtotime(in_stime)) == -1)
        {
            out_stime = "";
            return false;
        }

        timer = timer + timetvl; // ʱ�����㡣

        // ��������ʾ��ʱ��ת��Ϊ�ַ�����ʾ��ʱ�䡣
        timetostr(timer, out_stime, fmt);

        return true;
    }

    bool addtime(const std::string& in_stime, char* out_stime, const int timetvl, const std::string& fmt)
    {
        if (out_stime == nullptr) return false; // �жϿ�ָ�롣

        time_t timer;

        // ���ַ�����ʾ��ʱ��ת��Ϊ������ʾ��ʱ�䣬�������㡣
        if ((timer = strtotime(in_stime)) == -1)
        {
            strcpy(out_stime, "");
            return false;
        }

        timer = timer + timetvl; // ʱ�����㡣

        // ��������ʾ��ʱ��ת��Ϊ�ַ�����ʾ��ʱ�䡣
        timetostr(timer, out_stime, fmt);

        return true;
    }
    // ===========================================================================

    // ===========================================================================
    ctimer::ctimer()
    {
        start(); // ��ʱ��ʼ��
    }

    // ��ʱ��ʼ��
    void ctimer::start()
    {
        memset(&m_start, 0, sizeof(struct timeval));
        memset(&m_end, 0, sizeof(struct timeval));

        gettimeofday(&m_start, 0); // ��ȡ��ǰʱ�䣬��ȷ��΢�롣
    }

    // ��������ȥ��ʱ�䣬��λ���룬С���������΢��
    // ÿ����һ�α�����֮���Զ�����Start�������¿�ʼ��ʱ��
    double ctimer::elapsed()
    {
        gettimeofday(&m_end, 0); // ��ȡ��ǰʱ����Ϊ��ʱ������ʱ�䣬��ȷ��΢�롣

        std::string str;
        str = sformat("%ld.%06ld", m_start.tv_sec, m_start.tv_usec);
        double dstart = stod(str); // �Ѽ�ʱ��ʼ��ʱ���ת��Ϊdouble��

        str = sformat("%ld.%06ld", m_end.tv_sec, m_end.tv_usec);
        double dend = stod(str); // �Ѽ�ʱ������ʱ���ת��Ϊdouble��

        start(); // ���¿�ʼ��ʱ��

        return dend - dstart;
    }
    // ===========================================================================
} // namespace ol