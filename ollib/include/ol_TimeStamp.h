#ifndef OL_TIMESTAMP_H
#define OL_TIMESTAMP_H 1

#include <cstdint>
#include <ol_chrono.h>
#include <string>

namespace ol
{

    // 时间戳
    class TimeStamp
    {
    private:
        time_t m_secSinceEpoch; // 整数表示的时间（从1970到现在已逝去的秒数）。

    public:
        TimeStamp();                               // 用当前时间初始化对象。
        explicit TimeStamp(int64_t secSinceEpoch); // 用一个整数表示的时间初始化对象。

        static TimeStamp now(); // 返回当前时间的TimeStamp对象。

        time_t toInt() const;         // 返回整数表示的时间。
        std::string toString() const; // 返回字符串表示的时间，格式：yyyy-mm-dd hh24:mi:ss
    };

} // namespace ol

#endif // !OL_TIMESTAMP_H