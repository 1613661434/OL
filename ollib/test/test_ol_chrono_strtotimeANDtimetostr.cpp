/*
 *  程序名：test_ol_chrono_strtotime&timetostr.cpp，此程序演示开发框架中整数表示的时间和字符串表示的时间之间的转换。
 *  作者：ol
 */
#include "ol_chrono.h"
#include <iostream>

using namespace ol;
using namespace std;

int main()
{
    string strtime;
    strtime = "2020-01-01 12:35:22";

    time_t ttime;
    ttime = strtotime(strtime);    // 转换为整数的时间
    printf("ttime=%lld\n", ttime); // 输出ttime=1577853322

    char s1[20];                                   // C风格的字符串。
    timetostr(ttime, s1, "yyyy-mm-dd hh24:mi:ss"); // 转换为字符串的时间
    cout << "s1=" << s1 << endl;

    string s2;                                     // C++风格的字符串。
    timetostr(ttime, s2, "yyyy-mm-dd hh24:mi:ss"); // 转换为字符串的时间
    cout << "s2=" << s2 << endl;
}