/*
 *  程序名：test_ol_string_delete%chr.cpp，此程序演示开发框架中删除字符串左、右、两边指定字符的使用方法。
 *  作者：ol
 */
#include "ol_string.h"
#include <iostream>
#include <string.h>

using namespace ol;
using namespace std;

int main()
{
    char str1[31]; // C风格的字符串。
    string str2;   // C++风格的字符串。

    strcpy(str1, "  西施  ");
    deleteLchr(str1, ' ');      // 删除str1左边的空格
    printf("str1=%s=\n", str1); // 出输结果是str1=西施  =

    str2 = "  西施  ";
    deleteLchr(str2, ' ');
    cout << "str2=" << str2 << "=\n";

    strcpy(str1, "  西施  ");
    deleteRchr(str1, ' ');      // 删除str1右边的空格
    printf("str1=%s=\n", str1); // 出输结果是str1=  西施=

    str2 = "  西施  ";
    deleteRchr(str2, ' ');
    cout << "str2=" << str2 << "=\n";

    strcpy(str1, "  西施  ");
    deleteLRchr(str1, ' ');     // 删除str1两边的空格
    printf("str1=%s=\n", str1); // 出输结果是str1=西施=

    str2 = "  西施  ";
    deleteLRchr(str2, ' ');
    cout << "str2=" << str2 << "=\n";
}
