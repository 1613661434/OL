/*
 *  ��������test_ol_string_tolower&toupper.cpp���˳�����ʾ����������ַ�����Сдת��������ʹ�á�
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
    char str1[31]; // C�����ַ�����

    strcpy(str1, "12abz45ABz8��ʩ��");
    toupper(str1);              // ��str1�е�Сд��ĸת��Ϊ��д��
    printf("str1=%s=\n", str1); // ��������str1=12ABZ45ABZ8��ʩ��=

    strcpy(str1, "12abz45ABz8��ʩ��");
    tolower(str1);              // ��str1�еĴ�д��ĸת��ΪСд��
    printf("str1=%s=\n", str1); // ��������str1=12abz45abz8��ʩ��=

    string str2; // C++�����ַ�����

    str2 = "12abz45ABz8��ʩ��";
    toupper(str2);                    // ��str2�е�Сд��ĸת��Ϊ��д��
    cout << "str2=" << str2 << "=\n"; // ��������str2=12ABZ45ABZ8��ʩ��=

    str2 = "12abz45ABz8��ʩ��";
    tolower(str2);                    // ��str2�еĴ�д��ĸת��ΪСд��
    cout << "str2=" << str2 << "=\n"; // ��������str1=12abz45abz8��ʩ��=
}