/*
 *  程序名：test_ol_fstream_nlANDclearbufANDbinary.cpp，此程序演示自定义操作符。
 *  作者：ol
 */
#include "ol_fstream.h"
#include <iostream>

using namespace ol;
using namespace std;

int main()
{
    printf("请输入密码:");
    string password;
    cin >> password >> clearbuf; // 清理缓冲区

    printf("请确认密码(Y/N):");
    int ch = getchar();
    if (ch == 'Y' || ch == 'y')
    {
        cout << "确认成功" << nl; // 换行
    }
    else
    {
        printf("确认失败\n");
    }

    cout << binary(atoi(password.c_str())) << nl;

    return 0;
}