/*
 *  ��������test_ol_string_picknumber.cpp���˳�����ʾ���������picknumber������ʹ�á�
 *  ���ߣ�ol
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
    char str1[30];
    string str2;

    strcpy(str1, "iab+12.3xy");
    picknumber(str1, str1, false, false);
    printf("str1=%s=\n", str1); // ��������str1=123=

    str2 = "iab+12.3xy";
    picknumber(str2, str2, false, false);
    cout << "str2=" << str2 << "=\n"; // ��������str2=123=

    strcpy(str1, "iab+12.3xy");
    picknumber(str1, str1, true, false);
    printf("str1=%s=\n", str1); // ��������str1=+123=

    str2 = "iab+12.3xy";
    picknumber(str2, str2, true, false);
    cout << "str2=" << str2 << "=\n"; // ��������str2=+123=

    strcpy(str1, "iab+12.3xy");
    picknumber(str1, str1, true, true);
    printf("str1=%s=\n", str1); // ��������str1=+12.3=

    str2 = "iab+12.3xy";
    picknumber(str2, str2, true, true);
    cout << "str2=" << str2 << "=\n"; // ��������str2=+12.3=
}