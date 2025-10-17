/****************************************************************************************/
/*
 * 程序名：ol_chrono.h
 * 功能描述：时间操作工具类及函数集合，支持以下特性：
 *          - 时间字符串格式化（多种格式转换）
 *          - 时间戳与字符串互转
 *          - 时间偏移计算（增减秒数）
 *          - 高精度计时器（微秒级）
 *          - 跨平台休眠函数（支持纳秒、微秒、毫秒、秒级）
 * 作者：ol
 * 适用标准：C++11及以上（需支持chrono、thread等特性）
 */
/****************************************************************************************/

#ifndef OL_CHRONO_H
#define OL_CHRONO_H 1

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

    // 跨平台兼容：将localtime_s/localtime_r的结果统一为bool返回
    inline bool localtime_now(struct tm* tm_out, const time_t* t)
    {
        // 基本参数校验
        if (tm_out == nullptr || t == nullptr)
        {
            return false; // 无效指针直接返回失败
        }

#ifdef _WIN32
        // Windows：localtime_s返回errno_t，0表示成功
        errno_t err = localtime_s(tm_out, t);
        return (err == 0); // 成功返回true，失败返回false
#else
        // Linux/Unix/macOS：localtime_r返回struct tm*，非NULL表示成功
        struct tm* ret = localtime_r(t, tm_out);
        return (ret != nullptr); // 成功返回true，失败返回false
#endif
    }

    // 时间操作的若干函数。
    // ===========================================================================
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

    /**
     * @brief 获取操作系统时间并格式化为字符串
     * @param strtime 用于存放结果的字符串引用
     * @param fmt 输出格式（默认"yyyy-mm-dd hh24:mi:ss"），支持格式见备注
     * @param timetvl 时间偏移量（秒），正数为未来，负数为过去，0为当前时间
     * @return 格式化后的时间字符串引用
     * @note 支持的格式包括："yyyy-mm-dd hh24:mi:ss"、"yyyymmddhh24miss"、"yyyy-mm-dd"等
     */
    std::string& ltime(std::string& strtime, const std::string& fmt = "", const int timetvl = 0);

    /**
     * @brief 获取操作系统时间并格式化为C字符串
     * @param strtime 用于存放结果的字符数组指针（需确保足够空间）
     * @param fmt 输出格式（默认"yyyy-mm-dd hh24:mi:ss"）
     * @param timetvl 时间偏移量（秒）
     * @return 格式化后的C字符串指针
     */
    char* ltime(char* strtime, const std::string& fmt = "", const int timetvl = 0);

    /**
     * @brief 获取操作系统时间并格式化为字符串（无参数重载，避免歧义）
     * @param fmt 输出格式
     * @param timetvl 时间偏移量（秒）
     * @return 格式化后的时间字符串
     */
    std::string ltime1(const std::string& fmt = "", const int timetvl = 0);
    // ===========================================================================

    // ===========================================================================
    /**
     * @brief 将时间戳转换为指定格式的字符串
     * @param ttime 时间戳（time_t类型）
     * @param strtime 用于存放结果的字符串引用
     * @param fmt 输出格式（默认"yyyy-mm-dd hh24:mi:ss"）
     * @return 格式化后的时间字符串引用
     * @note 如果fmt的格式不正确，strtime将为空
     */
    std::string& timetostr(const time_t ttime, std::string& strtime, const std::string& fmt = "");

    /**
     * @brief 将时间戳转换为指定格式的C字符串
     * @param ttime 时间戳（time_t类型）
     * @param strtime 用于存放结果的字符数组指针
     * @param fmt 输出格式
     * @return 格式化后的C字符串指针
     * @note 如果fmt的格式不正确，strtime将为空
     */
    char* timetostr(const time_t ttime, char* strtime, const std::string& fmt = "");

    /**
     * @brief 将时间戳转换为指定格式的字符串（无参数重载，避免歧义）
     * @param ttime 时间戳（time_t类型）
     * @param fmt 输出格式
     * @return 格式化后的时间字符串
     */
    std::string timetostr1(const time_t ttime, const std::string& fmt = "");

    /**
     * @brief 将字符串转换为时间戳
     * @param strtime 包含完整时间信息的字符串（需包含yyyymmddhh24miss）
     * @return 对应的时间戳，格式错误时返回-1
     */
    time_t strtotime(const std::string& strtime);
    // ===========================================================================

    // ===========================================================================
    /**
     * @brief 对时间字符串进行偏移计算（C字符串版本）
     * @param in_stime 输入时间字符串（格式不限，但一定要包括yyyymmddhh24miss，一个都不能少，顺序也不能变）
     * @param out_stime 输出偏移后的时间字符串
     * @param timetvl 偏移秒数（正数为未来，负数为过去）
     * @param fmt 输出格式
     * @return 成功返回true，失败返回false（输入格式错误）
     * @note in_stime和out_stime参数可以是同一个变量的地址，如果调用失败，out_stime的内容会清空。
     */
    bool addtime(const std::string& in_stime, char* out_stime, const int timetvl, const std::string& fmt = "");

    /**
     * @brief 对时间字符串进行偏移计算（std::string版本）
     * @param in_stime 输入时间字符串（格式不限，但一定要包括yyyymmddhh24miss，一个都不能少，顺序也不能变）
     * @param out_stime 输出偏移后的时间字符串引用
     * @param timetvl 偏移秒数
     * @param fmt 输出格式
     * @return 成功返回true，失败返回false
     * @note in_stime和out_stime参数可以是同一个变量的地址，如果调用失败，out_stime的内容会清空。
     */
    bool addtime(const std::string& in_stime, std::string& out_stime, const int timetvl, const std::string& fmt = "");
    // ===========================================================================

    // ===========================================================================
    // 高精度计时器类（微秒级）
    class ctimer
    {
    private:
        struct timeval m_start; // 计时开始的时间点。
        struct timeval m_end;   // 计时结束的时间点。
    public:
        // 构造函数中会调用start方法。
        ctimer();

        // 开始计时。
        void start();

        /**
         * @brief 计算从上次start()到当前的逝去时间
         * @return 逝去时间（秒），小数点后为微秒
         * @note 调用后会自动重新开始计时
         */
        double elapsed();
    };
    // ===========================================================================

    // ===========================================================================
    /**
     * @brief 通用休眠函数（支持任意时间单位）
     * @param duration 休眠时长（std::chrono::duration类型）
     */
    template <typename Rep, typename Period>
    inline void sleep(std::chrono::duration<Rep, Period> duration)
    {
        std::this_thread::sleep_for(duration);
    }

    /**
     * @brief 纳秒级休眠
     * @param nanoseconds 休眠时长（纳秒）
     */
    inline void sleep_ns(long long nanoseconds)
    {
        sleep(std::chrono::nanoseconds(nanoseconds));
    }

    /**
     * @brief 微秒级休眠
     * @param microseconds 休眠时长（微秒）
     */
    inline void sleep_us(long long microseconds)
    {
        sleep(std::chrono::microseconds(microseconds));
    }

    /**
     * @brief 毫秒级休眠
     * @param milliseconds 休眠时长（毫秒）
     */
    inline void sleep_ms(long long milliseconds)
    {
        sleep(std::chrono::milliseconds(milliseconds));
    }

    /**
     * @brief 秒级休眠
     * @param seconds 休眠时长（秒）
     */
    inline void sleep_sec(long long seconds)
    {
        sleep(std::chrono::seconds(seconds));
    }
    // ===========================================================================

} // namespace ol

#endif // !OL_CHRONO_H