/*
 *  ��������test_ol_getxmlbuffer.cpp���˳�����ʾ���ÿ�����ܵ�getxmlbuffer��������xml�ַ�����
 *  ���ߣ�ol
 */
#include "ol_string.h"
#include <iostream>

using namespace ol;
using namespace std;

int main()
{
    // ��Ա÷�������ϴ����xml�С�
    string buffer = "<name>÷��</name><no>10</no><striker>true</striker><age>30</age><weight>68.5</weight><sal>21000000</sal><club>Barcelona</club>";

    // ���ڴ�������˶�Ա���ϵĽṹ�塣
    struct st_player
    {
        string name;   // ����
        char no[6];    // ���º���
        bool striker;  // ����λ���Ƿ���ǰ�棬true-�ǣ�false-���ǡ�
        int age;       // ����
        double weight; // ���أ�kg��
        long sal;      // ��н��ŷԪ��
        char club[51]; // Ч���ľ��ֲ�
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