/*
 *  程序名：test_ol_chrono_addtime.cpp，此程序演示开发框架中采用addtime函数进行时间的运算。
 *  作者：ol
 */
#include "ol_chrono.h"
#include <iostream>
#include <string.h>

using namespace ol;
using namespace std;

int main()
{
    char strtime[20];

    memset(strtime, 0, sizeof(strtime));
    strcpy(strtime, "2020-01-20 12:35:22");

    char s1[20];                                // C风格的字符串。
    addtime(strtime, s1, 0 - 1 * 24 * 60 * 60); // 减一天。
    printf("s1=%s\n", s1);                      // 输出s1=2020-01-19 12:35:22

    string s2;                              // C++风格的字符串。
    addtime(strtime, s2, 2 * 24 * 60 * 60); // 加两天。  172800
    cout << "s2=" << s2 << endl;            // 输出s2=2020-01-22 12:35:22
}