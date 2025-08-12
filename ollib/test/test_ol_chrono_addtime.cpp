/*
 *  ��������test_ol_chrono_addtime.cpp���˳�����ʾ��������в���addtime��������ʱ������㡣
 *  ���ߣ�ol
 */
#include "ol_chrono.h"
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
    char strtime[20];

    memset(strtime, 0, sizeof(strtime));
    strcpy(strtime, "2020-01-20 12:35:22");

    char s1[20];                                // C�����ַ�����
    addtime(strtime, s1, 0 - 1 * 24 * 60 * 60); // ��һ�졣
    printf("s1=%s\n", s1);                      // ���s1=2020-01-19 12:35:22

    string s2;                              // C++�����ַ�����
    addtime(strtime, s2, 2 * 24 * 60 * 60); // �����졣  172800
    cout << "s2=" << s2 << endl;            // ���s2=2020-01-22 12:35:22
}