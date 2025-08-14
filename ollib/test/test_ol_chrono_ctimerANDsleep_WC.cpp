/*
 *  程序名：test_ol_chrono_ctimer&sleep_%.cpp，此程序演示开发框架中的ctimer类（计时器）的用法。
 *  作者：ol
 */
#include "ol_chrono.h"
#include <iostream>

using namespace ol;
using namespace std;

int main()
{
    ctimer timer;

    printf("elapsed=%lf\n", timer.elapsed());
    sleep_sec(1);
    printf("elapsed=%lf\n", timer.elapsed());
    sleep_sec(1);
    printf("elapsed=%lf\n", timer.elapsed());
    sleep_us(1000);
    printf("elapsed=%lf\n", timer.elapsed());
    sleep_us(100);
    printf("elapsed=%lf\n", timer.elapsed());
    sleep_sec(10);
    printf("elapsed=%lf\n", timer.elapsed());

    // 使用通用模板（需要显式指定时间单位）
    sleep(std::chrono::seconds(2));        // 休眠2秒
    sleep(std::chrono::milliseconds(100)); // 休眠100毫秒

    // 使用重载函数（直接传数值）
    sleep_sec(1);      // 休眠1秒
    sleep_ms(250);     // 休眠250毫秒
    sleep_us(500000);  // 休眠500000微秒（0.5秒）
    sleep_ns(1000000); // 休眠1000000纳秒（1毫秒）

    printf("elapsed=%lf\n", timer.elapsed());
}