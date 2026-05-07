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

    // 首次调用elapsed()：获取从计时器初始化到现在的时间
    // 预期：接近0秒（因为刚创建计时器，几乎无耗时）
    printf("elapsed=%lf\n", timer.elapsed());

    // 休眠1秒（1000毫秒）
    sleep_sec(1);
    // 调用elapsed()：获取上一次elapsed()到现在的间隔（即sleep_sec(1)的实际耗时）
    // 预期：略大于1秒（受系统调度影响，通常在1.000-1.010秒之间）
    printf("elapsed=%lf\n", timer.elapsed());

    // 再休眠1秒
    sleep_sec(1);
    // 调用elapsed()：获取上一次elapsed()到现在的间隔（第二次sleep_sec(1)的实际耗时）
    // 预期：略大于1秒
    printf("elapsed=%lf\n", timer.elapsed());

    // 休眠1000微秒（即1毫秒）
    sleep_us(1000);
    // 调用elapsed()：获取上一次elapsed()到现在的间隔（sleep_us(1000)的实际耗时）
    // 预期：略大于0.001秒（1毫秒），可能在0.001-0.010秒之间（微秒级休眠受系统精度限制）
    printf("elapsed=%lf\n", timer.elapsed());

    // 休眠100微秒
    sleep_us(100);
    // 调用elapsed()：获取上一次elapsed()到现在的间隔（sleep_us(100)的实际耗时）
    // 预期：略大于0.0001秒（100微秒），但可能因系统调度有较大误差（可能达到0.001秒级别）
    printf("elapsed=%lf\n", timer.elapsed());

    // 休眠10秒
    sleep_sec(10);
    // 调用elapsed()：获取上一次elapsed()到现在的间隔（sleep_sec(10)的实际耗时）
    // 预期：略大于10秒，通常在10.000-10.010秒之间
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