/*
 *  程序名：test_ol_string_matchstr.cpp，此程序演示开发框架正则表达示matchstr函数的使用。
 *  作者：ol
 */
#include "ol_string.h"
#include <iostream>

using namespace ol;
using namespace std;

int main()
{
    // 以下代码将输出yes。
    if (matchstr("_public.h", "*.h,*.cpp") == true)
        printf("yes\n");
    else
        printf("no\n");

    // 以下代码将输出yes。
    if (matchstr("_public.h", "*.H") == true)
        printf("yes\n");
    else
        printf("no\n");

    // 以下代码将输出no。
    if (matchstr("_public.h", "*p*k*.h") == true)
        printf("yes\n");
    else
        printf("no\n");

    // 以下代码将输出yes。
    if (matchstr("_public.h", "*") == true)
        printf("yes\n");
    else
        printf("no\n");
}