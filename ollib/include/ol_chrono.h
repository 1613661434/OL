#ifndef __OL_CHRONO_H
#define __OL_CHRONO_H 1

#include <chrono>
#include <ctime>
#include <string>
#include <thread>

#ifdef _WIN32
#include <windows.h> // Windows ƽ̨ʹ�� windows.h ��� sys/time.h
#elif defined(__linux__)
#include <sys/time.h> // Linus ƽ̨ʹ�� sys/time.h
#endif

namespace ol
{

    ///////////////////////////////////// /////////////////////////////////////
    // ʱ����������ɺ�����
    /*
      ȡ����ϵͳ��ʱ�䣨���ַ�����ʾ����
      strtime�����ڴ�Ż�ȡ����ʱ�䡣
      timetvl��ʱ���ƫ��������λ���룬0��ȱʡֵ����ʾ��ǰʱ�䣬30��ʾ��ǰʱ��30��֮���ʱ��㣬-30��ʾ��ǰʱ��30��֮ǰ��ʱ��㡣
      fmt�����ʱ��ĸ�ʽ��fmtÿ���ֵĺ��壺yyyy-��ݣ�mm-�·ݣ�dd-���ڣ�hh24-Сʱ��mi-���ӣ�ss-�룬
      ȱʡ��"yyyy-mm-dd hh24:mi:ss"��Ŀǰ֧�����¸�ʽ��
      "yyyy-mm-dd hh24:mi:ss"
      "yyyymmddhh24miss"
      "yyyy-mm-dd"
      "yyyymmdd"
      "hh24:mi:ss"
      "hh24miss"
      "hh24:mi"
      "hh24mi"
      "hh24"
      "mi"
      ע�⣺
        1��Сʱ�ı�ʾ������hh24������hh����ô����Ŀ����Ϊ�˱��������ݿ��ʱ���ʾ����һ�£�
        2�������г��˳��õ�ʱ���ʽ���������������Ӧ�ÿ������������޸�Դ����timetostr()�������Ӹ���ĸ�ʽ֧�֣�
        3�����ú�����ʱ�����fmt��������ʽ��ƥ�䣬strtime�����ݽ�Ϊ�ա�
        4��ʱ����������λ�������Ŀ�����һλ����λ�����������λ����ǰ�油0��
    */
    std::string& ltime(std::string& strtime, const std::string& fmt = "", const int timetvl = 0);
    char* ltime(char* strtime, const std::string& fmt = "", const int timetvl = 0);
    // Ϊ�˱������ص���壬����ltime1()������
    std::string ltime1(const std::string& fmt = "", const int timetvl = 0);
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // ��������ʾ��ʱ��ת��Ϊ�ַ�����ʾ��ʱ�䡣
    // ttime��������ʾ��ʱ�䡣
    // strtime���ַ�����ʾ��ʱ�䡣
    // fmt������ַ���ʱ��strtime�ĸ�ʽ����ltime()������fmt������ͬ�����fmt�ĸ�ʽ����ȷ��strtime��Ϊ�ա�
    std::string& timetostr(const time_t ttime, std::string& strtime, const std::string& fmt = "");
    char* timetostr(const time_t ttime, char* strtime, const std::string& fmt = "");
    // Ϊ�˱������ص���壬����timetostr1()������
    std::string timetostr1(const time_t ttime, const std::string& fmt = "");

    // ���ַ�����ʾ��ʱ��ת��Ϊ������ʾ��ʱ�䡣
    // strtime���ַ�����ʾ��ʱ�䣬��ʽ���ޣ���һ��Ҫ����yyyymmddhh24miss��һ���������٣�˳��Ҳ���ܱ䡣
    // ����ֵ��������ʾ��ʱ�䣬���strtime�ĸ�ʽ����ȷ������-1��
    time_t strtotime(const std::string& strtime);
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // ���ַ�����ʾ��ʱ�����һ��ƫ�Ƶ�������õ�һ���µ��ַ�����ʾ��ʱ�䡣
    // in_stime��������ַ�����ʽ��ʱ�䣬��ʽ���ޣ���һ��Ҫ����yyyymmddhh24miss��һ���������٣�˳��Ҳ���ܱ䡣
    // out_stime��������ַ�����ʽ��ʱ�䡣
    // timetvl����Ҫƫ�Ƶ���������������ƫ�ƣ�������ǰƫ�ơ�
    // fmt������ַ���ʱ��out_stime�ĸ�ʽ����ltime()������fmt������ͬ��
    // ע�⣺in_stime��out_stime����������ͬһ�������ĵ�ַ���������ʧ�ܣ�out_stime�����ݻ���ա�
    // ����ֵ��true-�ɹ���false-ʧ�ܣ��������ʧ�ܣ�������Ϊ��in_stime�ĸ�ʽ����ȷ��
    bool addtime(const std::string& in_stime, char* out_stime, const int timetvl, const std::string& fmt = "");
    bool addtime(const std::string& in_stime, std::string& out_stime, const int timetvl, const std::string& fmt = "");
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // ����һ����ȷ��΢��ļ�ʱ����
    class ctimer
    {
    private:
        struct timeval m_start; // ��ʱ��ʼ��ʱ��㡣
        struct timeval m_end;   // ��ʱ������ʱ��㡣
    public:
        ctimer(); // ���캯���л����start������

        void start(); // ��ʼ��ʱ��

        // ��������ȥ��ʱ�䣬��λ���룬С���������΢�롣
        // ÿ����һ�α�����֮���Զ�����start�������¿�ʼ��ʱ��
        double elapsed();
    };
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // ͨ������ģ�壨֧������ʱ�䵥λ��
    template <typename Rep, typename Period>
    inline void sleep(std::chrono::duration<Rep, Period> duration)
    {
        std::this_thread::sleep_for(duration);
    }

    // ���غ�������������
    inline void sleep_ns(long long nanoseconds)
    {
        sleep(std::chrono::nanoseconds(nanoseconds));
    }

    // ���غ�����΢������
    inline void sleep_us(long long microseconds)
    {
        sleep(std::chrono::microseconds(microseconds));
    }

    // ���غ�������������
    inline void sleep_ms(long long milliseconds)
    {
        sleep(std::chrono::milliseconds(milliseconds));
    }

    // ���غ�����������
    inline void sleep_sec(long long seconds)
    {
        sleep(std::chrono::seconds(seconds));
    }
    ///////////////////////////////////// /////////////////////////////////////

} // namespace ol

#endif // !__OL_CHRONO_H