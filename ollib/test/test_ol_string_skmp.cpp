/*
 *  程序名：test_ol_string_skmp.cpp，此程序演示skmp函数。
 *  作者：ol
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
        cout << "未找到子串\n";
    }
    else
    {
        cout << "找到子串,索引: " << pos1 << "\n";
    }

    cout << "--------------char*--------------\n";
    if (pos2 == string::npos)
    {
        cout << "未找到子串\n";
    }
    else
    {
        cout << "找到子串,索引: " << pos2 << "\n";
    }

    return 0;
}