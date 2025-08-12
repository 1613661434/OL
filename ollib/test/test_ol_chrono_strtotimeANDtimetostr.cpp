/*
 *  ��������test_ol_chrono_strtotime&timetostr.cpp���˳�����ʾ���������������ʾ��ʱ����ַ�����ʾ��ʱ��֮���ת����
 *  ���ߣ�ol
 */
#include "ol_chrono.h"
#include <iostream>

using namespace ol;
using namespace std;

int main()
{
    string strtime;
    strtime = "2020-01-01 12:35:22";

    time_t ttime;
    ttime = strtotime(strtime);    // ת��Ϊ������ʱ��
    printf("ttime=%lld\n", ttime); // ���ttime=1577853322

    char s1[20];                                   // C�����ַ�����
    timetostr(ttime, s1, "yyyy-mm-dd hh24:mi:ss"); // ת��Ϊ�ַ�����ʱ��
    cout << "s1=" << s1 << endl;

    string s2;                                     // C++�����ַ�����
    timetostr(ttime, s2, "yyyy-mm-dd hh24:mi:ss"); // ת��Ϊ�ַ�����ʱ��
    cout << "s2=" << s2 << endl;
}