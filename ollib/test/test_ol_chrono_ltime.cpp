/*
 *  ��������test_ol_chrono_ltime.cpp���˳�����ʾ���������ltimeʱ�亯����ʹ�ã���ȡ����ϵͳʱ�䣩��
 *  ���ߣ�ol
 */
#include "ol_chrono.h"
#include <iostream>
#include <string.h>

using namespace ol;
using namespace std;

int main()
{
    // C�����ַ�����
    char strtime1[20]; // ���ϵͳʱ�䡣
    memset(strtime1, 0, sizeof(strtime1));

    ltime(strtime1, "yyyy-mm-dd hh24:mi:ss"); // ��ȡ��ǰʱ�䡣
    printf("strtime1=%s\n", strtime1);

    ltime(strtime1, "yyyy-mm-dd hh24:mi:ss", -30); // ��ȡ30��ǰ��ʱ�䡣
    printf("strtime1=%s\n", strtime1);

    ltime(strtime1, "yyyy-mm-dd hh24:mi:ss", 30); // ��ȡ30����ʱ�䡣
    printf("strtime1=%s\n", strtime1);

    // C++�����ַ�����
    string strtime2;

    ltime(strtime2, "yyyy-mm-dd hh24:mi:ss"); // ��ȡ��ǰʱ�䡣
    cout << "strtime2=" << strtime2 << "\n";

    ltime(strtime2, "yyyy-mm-dd hh24:mi:ss", -30); // ��ȡ30��ǰ��ʱ�䡣
    cout << "strtime2=" << strtime2 << "\n";

    ltime(strtime2, "yyyy-mm-dd hh24:mi:ss", 30); // ��ȡ30����ʱ�䡣
    cout << "strtime2=" << strtime2 << "\n";
}