/*
 *  程序名：test_ol_chrono_ltime.cpp，此程序演示开发框架中ltime时间函数的使用（获取操作系统时间）。
 *  作者：ol
 */
#include "ol_chrono.h"
#include <iostream>
#include <string.h>

using namespace ol;
using namespace std;

int main()
{
    // C风格的字符串。
    char strtime1[20]; // 存放系统时间。
    memset(strtime1, 0, sizeof(strtime1));

    ltime(strtime1, "yyyy-mm-dd hh24:mi:ss"); // 获取当前时间。
    printf("strtime1=%s\n", strtime1);

    ltime(strtime1, "yyyy-mm-dd hh24:mi:ss", -30); // 获取30秒前的时间。
    printf("strtime1=%s\n", strtime1);

    ltime(strtime1, "yyyy-mm-dd hh24:mi:ss", 30); // 获取30秒后的时间。
    printf("strtime1=%s\n", strtime1);

    // C++风格的字符串。
    string strtime2;

    ltime(strtime2, "yyyy-mm-dd hh24:mi:ss"); // 获取当前时间。
    cout << "strtime2=" << strtime2 << "\n";

    ltime(strtime2, "yyyy-mm-dd hh24:mi:ss", -30); // 获取30秒前的时间。
    cout << "strtime2=" << strtime2 << "\n";

    ltime(strtime2, "yyyy-mm-dd hh24:mi:ss", 30); // 获取30秒后的时间。
    cout << "strtime2=" << strtime2 << "\n";
}