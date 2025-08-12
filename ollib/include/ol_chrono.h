#ifndef __OL_CHRONO_H
#define __OL_CHRONO_H 1

#include <chrono>
#include <ctime>
#include <string>
#include <thread>

#ifdef _WIN32
#include <windows.h> // Windows 平台使用 windows.h 替代 sys/time.h
#elif defined(__linux__)
#include <sys/time.h> // Linus 平台使用 sys/time.h
#endif

namespace ol
{

    ///////////////////////////////////// /////////////////////////////////////
    // 时间操作的若干函数。
    /*
      取操作系统的时间（用字符串表示）。
      strtime：用于存放获取到的时间。
      timetvl：时间的偏移量，单位：秒，0是缺省值，表示当前时间，30表示当前时间30秒之后的时间点，-30表示当前时间30秒之前的时间点。
      fmt：输出时间的格式，fmt每部分的含义：yyyy-年份；mm-月份；dd-日期；hh24-小时；mi-分钟；ss-秒，
      缺省是"yyyy-mm-dd hh24:mi:ss"，目前支持以下格式：
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
      注意：
        1）小时的表示方法是hh24，不是hh，这么做的目的是为了保持与数据库的时间表示方法一致；
        2）以上列出了常用的时间格式，如果不能满足你应用开发的需求，请修改源代码timetostr()函数增加更多的格式支持；
        3）调用函数的时候，如果fmt与上述格式都匹配，strtime的内容将为空。
        4）时间的年份是四位，其它的可能是一位和两位，如果不足两位，在前面补0。
    */
    std::string& ltime(std::string& strtime, const std::string& fmt = "", const int timetvl = 0);
    char* ltime(char* strtime, const std::string& fmt = "", const int timetvl = 0);
    // 为了避免重载的岐义，增加ltime1()函数。
    std::string ltime1(const std::string& fmt = "", const int timetvl = 0);
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // 把整数表示的时间转换为字符串表示的时间。
    // ttime：整数表示的时间。
    // strtime：字符串表示的时间。
    // fmt：输出字符串时间strtime的格式，与ltime()函数的fmt参数相同，如果fmt的格式不正确，strtime将为空。
    std::string& timetostr(const time_t ttime, std::string& strtime, const std::string& fmt = "");
    char* timetostr(const time_t ttime, char* strtime, const std::string& fmt = "");
    // 为了避免重载的岐义，增加timetostr1()函数。
    std::string timetostr1(const time_t ttime, const std::string& fmt = "");

    // 把字符串表示的时间转换为整数表示的时间。
    // strtime：字符串表示的时间，格式不限，但一定要包括yyyymmddhh24miss，一个都不能少，顺序也不能变。
    // 返回值：整数表示的时间，如果strtime的格式不正确，返回-1。
    time_t strtotime(const std::string& strtime);
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // 把字符串表示的时间加上一个偏移的秒数后得到一个新的字符串表示的时间。
    // in_stime：输入的字符串格式的时间，格式不限，但一定要包括yyyymmddhh24miss，一个都不能少，顺序也不能变。
    // out_stime：输出的字符串格式的时间。
    // timetvl：需要偏移的秒数，正数往后偏移，负数往前偏移。
    // fmt：输出字符串时间out_stime的格式，与ltime()函数的fmt参数相同。
    // 注意：in_stime和out_stime参数可以是同一个变量的地址，如果调用失败，out_stime的内容会清空。
    // 返回值：true-成功，false-失败，如果返回失败，可以认为是in_stime的格式不正确。
    bool addtime(const std::string& in_stime, char* out_stime, const int timetvl, const std::string& fmt = "");
    bool addtime(const std::string& in_stime, std::string& out_stime, const int timetvl, const std::string& fmt = "");
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // 这是一个精确到微秒的计时器。
    class ctimer
    {
    private:
        struct timeval m_start; // 计时开始的时间点。
        struct timeval m_end;   // 计时结束的时间点。
    public:
        ctimer(); // 构造函数中会调用start方法。

        void start(); // 开始计时。

        // 计算已逝去的时间，单位：秒，小数点后面是微秒。
        // 每调用一次本方法之后，自动调用start方法重新开始计时。
        double elapsed();
    };
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // 通用休眠模板（支持任意时间单位）
    template <typename Rep, typename Period>
    inline void sleep(std::chrono::duration<Rep, Period> duration)
    {
        std::this_thread::sleep_for(duration);
    }

    // 重载函数：纳秒休眠
    inline void sleep_ns(long long nanoseconds)
    {
        sleep(std::chrono::nanoseconds(nanoseconds));
    }

    // 重载函数：微秒休眠
    inline void sleep_us(long long microseconds)
    {
        sleep(std::chrono::microseconds(microseconds));
    }

    // 重载函数：毫秒休眠
    inline void sleep_ms(long long milliseconds)
    {
        sleep(std::chrono::milliseconds(milliseconds));
    }

    // 重载函数：秒休眠
    inline void sleep_sec(long long seconds)
    {
        sleep(std::chrono::seconds(seconds));
    }
    ///////////////////////////////////// /////////////////////////////////////

} // namespace ol

#endif // !__OL_CHRONO_H