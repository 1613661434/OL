/*
 *  程序名：test_ol_getxmlbuffer.cpp，此程序演示调用开发框架的getxmlbuffer函数解析xml字符串。
 *  作者：ol
 */
#include "ol_string.h"
#include <iostream>

using namespace ol;
using namespace std;

int main()
{
    // 球员梅西的资料存放在xml中。
    string buffer = "<name>梅西</name><no>10</no><striker>true</striker><age>30</age><weight>68.5</weight><sal>21000000</sal><club>Barcelona</club>";

    // 用于存放足球运动员资料的结构体。
    struct st_player
    {
        string name;   // 姓名
        char no[6];    // 球衣号码
        bool striker;  // 场上位置是否是前锋，true-是；false-不是。
        int age;       // 年龄
        double weight; // 体重，kg。
        long sal;      // 年薪，欧元。
        char club[51]; // 效力的俱乐部
    } stplayer;

    getxmlbuffer(buffer, "name", stplayer.name);
    cout << "name=" << stplayer.name << endl;

    getxmlbuffer(buffer, "no", stplayer.no, 5);
    cout << "no=" << stplayer.no << endl;

    getxmlbuffer(buffer, "striker", stplayer.striker);
    cout << "striker=" << stplayer.striker << endl;

    getxmlbuffer(buffer, "age", stplayer.age);
    cout << "age=" << stplayer.age << endl;

    getxmlbuffer(buffer, "weight", stplayer.weight);
    cout << "weight=" << stplayer.weight << endl;

    getxmlbuffer(buffer, "sal", stplayer.sal);
    cout << "sal=" << stplayer.sal << endl;

    getxmlbuffer(buffer, "club", stplayer.club, 50);
    cout << "club=" << stplayer.club << endl;
}