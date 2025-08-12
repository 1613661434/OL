/*
 *  ��������test_ol_string_delete%chr.cpp���˳�����ʾ���������ɾ���ַ������ҡ�����ָ���ַ���ʹ�÷�����
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
    string str2;   // C++�����ַ�����

    strcpy(str1, "  ��ʩ  ");
    deletelchr(str1, ' ');      // ɾ��str1��ߵĿո�
    printf("str1=%s=\n", str1); // ��������str1=��ʩ  =

    str2 = "  ��ʩ  ";
    deletelchr(str2, ' ');
    cout << "str2=" << str2 << "=\n";

    strcpy(str1, "  ��ʩ  ");
    deleterchr(str1, ' ');      // ɾ��str1�ұߵĿո�
    printf("str1=%s=\n", str1); // ��������str1=  ��ʩ=

    str2 = "  ��ʩ  ";
    deleterchr(str2, ' ');
    cout << "str2=" << str2 << "=\n";

    strcpy(str1, "  ��ʩ  ");
    deletelrchr(str1, ' ');     // ɾ��str1���ߵĿո�
    printf("str1=%s=\n", str1); // ��������str1=��ʩ=

    str2 = "  ��ʩ  ";
    deletelrchr(str2, ' ');
    cout << "str2=" << str2 << "=\n";
}
