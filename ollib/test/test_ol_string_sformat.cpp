/*
 *  程序名：test_ol_string_sformat.cpp，此程序演示调用开发框架的格式化输出sformat函数。
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
    int bh = 1;
    char name[31];
    strcpy(name, "LJJ");
    double weight = 48.2;
    string yz = "beautiful";

    char s1[100];
    int len = snprintf(s1, 100, "NO.=%02d,name=%s,weight=%.2f,yz=%s", bh, name, weight, yz.c_str());
    cout << "s1=" << s1 << endl;
    printf("len=%d\n", len);
    printf("strlen(s1)=%zu\n", strlen(s1));

    string s2;
    s2 = "NO.=" + to_string(bh) + ",name=" + name + ",weight=" + to_string(weight) + ",yz=" + yz;
    cout << "s2=" << s2 << endl;

    s2 = sformat("NO.=%02d,name=%s,weight=%.2f,yz=%s", bh, name, weight, yz);
    cout << "s2=" << s2 << endl;

    sformat(s2, "NO.=%02d,name=%s,weight=%.2f,yz=%s", bh, name, weight, yz);
    cout << "s2=" << s2 << endl;

    s2 = sformat("NO.=%02d,name=%s,weight=%.2f,yz=%s", bh, name, weight, yz.c_str());
    cout << "s2=" << s2 << endl;

    sformat(s2, "NO.=%02d,name=%s,weight=%.2f,yz=%s", bh, name, weight, yz.c_str());
    cout << "s2=" << s2 << endl;
}