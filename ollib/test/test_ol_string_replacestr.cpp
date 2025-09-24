/*
 *  程序名：test_ol_string_replacestr.cpp，此程序演示开发框架中字符串替换replacestr函数的使用。
 *  作者：ol
 */
#include "ol_string.h"
#include <iostream>
#include <string.h>

using namespace ol;
using namespace std;

int main()
{
    char str1[301];

    strcpy(str1, "name:messi,no:10,job:striker.");
    replacestr(str1, ":", "="); // 把冒号替换成等号。
    printf("str1=%s=\n", str1); // 出输结果是str1=name=messi,no=10,job=striker.=

    strcpy(str1, "name:messi,no:10,job:striker.");
    replacestr(str1, "name:", ""); // 把"name:"替换成""，相当于删除内容"name:"。
    printf("str1=%s=\n", str1);    // 出输结果是str1=messi,no:10,job:striker.=

    strcpy(str1, "messi----10----striker");
    replacestr(str1, "--", "-", false); // 把两个"--"替换成一个"-"，bloop参数为false。
    printf("str1=%s=\n", str1);         // 出输结果是str1=messi--10--striker=

    strcpy(str1, "messi----10----striker");
    replacestr(str1, "--", "-", true); // 把两个"--"替换成一个"-"，bloop参数为true。
    printf("str1=%s=\n", str1);        // 出输结果是str1=messi-10-striker=

    strcpy(str1, "messi-10-striker");
    replacestr(str1, "-", "--", false); // 把一个"-"替换成两个"--"，bLoop参数为false。
    printf("str1=%s=\n", str1);         // 出输结果是str1=messi--10--striker=

    // 以下代码把"-"替换成"--"，bloop参数为true，存在逻辑错误，replacestr将不执行替换。
    strcpy(str1, "messi-10-striker");
    replacestr(str1, "-", "--", true); // 把一个"-"替换成两个"--"，bloop参数为true。
    printf("str1=%s=\n", str1);        // 出输结果是str1=messi-10-striker=

    // ////////////////////////////////////
    string str2;
    str2 = "name:messi,no:10,job:striker.";
    replacestr(str2, ":", "=");       // 把冒号替换成等号。
    cout << "str2=" << str2 << "=\n"; // 出输结果是str2=name=messi,no=10,job=striker.=

    str2 = "name:messi,no:10,job:striker.";
    replacestr(str2, "name:", "");    // 把"name:"替换成""，相当于删除内容"name:"。
    cout << "str2=" << str2 << "=\n"; // 出输结果是str2=messi,no:10,job:striker.=

    str2 = "messi----10----striker";
    replacestr(str2, "--", "-", false); // 把两个"--"替换成一个"-"，bLoop参数为false。
    cout << "str2=" << str2 << "=\n";   // 出输结果是str2=messi--10--striker=

    str2 = "messi----10----striker";
    replacestr(str2, "--", "-", true); // 把两个"--"替换成一个"-"，bLoop参数为true。
    cout << "str2=" << str2 << "=\n";  // 出输结果是str2=messi-10-striker=

    str2 = "messi-10-striker";
    replacestr(str2, "-", "--", false); // 把一个"-"替换成两个"--"，bLoop参数为false。
    cout << "str2=" << str2 << "=\n";   // 出输结果是str2=messi--10--striker=

    // 以下代码把"-"替换成"--"，bloop参数为true，存在逻辑错误，updatestr将不执行替换。
    str2 = "messi-10-striker";
    replacestr(str2, "-", "--", true); // 把一个"-"替换成两个"--"，bloop参数为true。
    cout << "str2=" << str2 << "=\n";  // 出输结果是str2=messi-10-striker=
}