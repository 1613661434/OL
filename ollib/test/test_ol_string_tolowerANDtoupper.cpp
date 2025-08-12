/*
 *  程序名：test_ol_string_tolower&toupper.cpp，此程序演示开发框架中字符串大小写转换函数的使用。
 *  作者：ol
 */
#include "ol_string.h"
#include <iostream>
#include <string.h>

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)
#endif

using namespace ol;
using namespace std;

int main()
{
    char str1[31]; // C风格的字符串。

    strcpy(str1, "12abz45ABz8西施。");
    toupper(str1);              // 把str1中的小写字母转换为大写。
    printf("str1=%s=\n", str1); // 出输结果是str1=12ABZ45ABZ8西施。=

    strcpy(str1, "12abz45ABz8西施。");
    tolower(str1);              // 把str1中的大写字母转换为小写。
    printf("str1=%s=\n", str1); // 出输结果是str1=12abz45abz8西施。=

    string str2; // C++风格的字符串。

    str2 = "12abz45ABz8西施。";
    toupper(str2);                    // 把str2中的小写字母转换为大写。
    cout << "str2=" << str2 << "=\n"; // 出输结果是str2=12ABZ45ABZ8西施。=

    str2 = "12abz45ABz8西施。";
    tolower(str2);                    // 把str2中的大写字母转换为小写。
    cout << "str2=" << str2 << "=\n"; // 出输结果是str1=12abz45abz8西施。=
}