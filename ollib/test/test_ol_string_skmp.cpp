/*
 *  ��������test_ol_string_skmp.cpp���˳�����ʾskmp������
 *  ���ߣ�ol
 */

#include "ol_string.h"
#include <iostream>
#include <string.h>

using namespace ol;
using namespace std;

int main()
{
    string str = "aabaabaaf";
    string substr = "aabaaf";
    char str2[] = "aabaabaaf";

    size_t pos1 = skmp(str, substr);
    size_t pos2 = skmp(str2, substr);

    cout << "--------------string--------------\n";
    if (pos1 == string::npos)
    {
        cout << "δ�ҵ��Ӵ�\n";
    }
    else
    {
        cout << "�ҵ��Ӵ�,����: " << pos1 << "\n";
    }

    cout << "--------------char*--------------\n";
    if (pos2 == string::npos)
    {
        cout << "δ�ҵ��Ӵ�\n";
    }
    else
    {
        cout << "�ҵ��Ӵ�,����: " << pos2 << "\n";
    }

    return 0;
}