#include "ol_TimeStamp.h"
#include <cstdio>
#include <time.h>

namespace ol
{

    TimeStamp::TimeStamp()
    {
        m_secSinceEpoch = time(nullptr); // 取系统当前时间。
    }

    TimeStamp::TimeStamp(int64_t secSinceEpoch) : m_secSinceEpoch(static_cast<time_t>(secSinceEpoch))
    {
    }

    // 当前时间。
    TimeStamp TimeStamp::now()
    {
        return TimeStamp(); // 返回当前时间。
    }

    time_t TimeStamp::toInt() const
    {
        return m_secSinceEpoch;
    }

    std::string TimeStamp::toString() const
    {
        char buf[20] = {0}; // 19个字符 + 1个终止符'\0'
        tm tm_time;

        if (!localtime_now(&tm_time, &m_secSinceEpoch))
        {
            return "Invalid time";
        }

        // 19个字符
        snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d",
                 tm_time.tm_year + 1900,
                 tm_time.tm_mon + 1,
                 tm_time.tm_mday,
                 tm_time.tm_hour,
                 tm_time.tm_min,
                 tm_time.tm_sec);

        return buf;
    }

} // namespace ol