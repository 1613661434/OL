/*
 *  程序名：test_ol_string_ccmdstr.cpp，此程序演示开发框架拆分字符串ccmdstr类的使用。
 *  作者：ol
 */
#include "ol_string.h"
#include <iostream>
#include <string.h>

using namespace ol;
using namespace std;

// 用于存放足球运动员资料的结构体。
struct st_player
{
    char name[51]; // 姓名
    char no[6];    // 球衣号码
    bool striker;  // 场上位置是否是前锋，true-是；false-不是。
    int age;       // 年龄
    double weight; // 体重，kg。
    long sal;      // 年薪，欧元。
    char club[51]; // 效力的俱乐部
} stplayer;

int main()
{
    memset(&stplayer, 0, sizeof(struct st_player));

    string buffer = "messi~!~10~!~true~!~a30~!~68.5~!~2100000~!~Barc,elona"; // 梅西的资料。

    // ccmdstr cmdstr;                               // 定义拆分字符串的对象。
    // cmdstr.splittocmd(buffer,"~!~");           // 拆分buffer。
    ccmdstr cmdstr(buffer, "~!~"); // 定义拆分字符串的对象并拆分字符串。

    // 像访问数组一样访问拆分后的元素。
    for (size_t ii = 0; ii < cmdstr.size(); ++ii)
    {
        cout << "cmdstr[" << ii << "]=" << cmdstr[ii] << endl;
    }

    // 输出拆分后的元素，一般用于调试。
    cout << cmdstr;

    // 获取拆分后元素的内容。
    cmdstr.getvalue(0, stplayer.name, 50); // 获取姓名
    cmdstr.getvalue(1, stplayer.no, 5);    // 获取球衣号码
    cmdstr.getvalue(2, stplayer.striker);  // 场上位置
    cmdstr.getvalue(3, stplayer.age);      // 获取年龄
    cmdstr.getvalue(4, stplayer.weight);   // 获取体重
    cmdstr.getvalue(5, stplayer.sal);      // 获取年薪，欧元。
    cmdstr.getvalue(6, stplayer.club, 50); // 获取效力的俱乐部

    printf("name=%s,no=%s,striker=%d,age=%d,weight=%.1f,sal=%ld,club=%s\n",
           stplayer.name, stplayer.no, stplayer.striker, stplayer.age,
           stplayer.weight, stplayer.sal, stplayer.club);
    // 输出结果:name=messi,no=10,striker=1,age=30,weight=68.5,sal=21000000,club=Barcelona
}
