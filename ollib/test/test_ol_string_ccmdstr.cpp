/*
 *  ��������test_ol_string_ccmdstr.cpp���˳�����ʾ������ܲ���ַ���ccmdstr���ʹ�á�
 *  ���ߣ�ol
 */
#include "ol_string.h"
#include <iostream>
#include <string.h>

using namespace ol;
using namespace std;

// ���ڴ�������˶�Ա���ϵĽṹ�塣
struct st_player
{
    char name[51]; // ����
    char no[6];    // ���º���
    bool striker;  // ����λ���Ƿ���ǰ�棬true-�ǣ�false-���ǡ�
    int age;       // ����
    double weight; // ���أ�kg��
    long sal;      // ��н��ŷԪ��
    char club[51]; // Ч���ľ��ֲ�
} stplayer;

int main()
{
    memset(&stplayer, 0, sizeof(struct st_player));

    string buffer = "messi~!~10~!~true~!~a30~!~68.5~!~2100000~!~Barc,elona"; // ÷�������ϡ�

    // ccmdstr cmdstr;                               // �������ַ����Ķ���
    // cmdstr.splittocmd(buffer,"~!~");           // ���buffer��
    ccmdstr cmdstr(buffer, "~!~"); // �������ַ����Ķ��󲢲���ַ�����

    // ���������һ�����ʲ�ֺ��Ԫ�ء�
    for (size_t ii = 0; ii < cmdstr.size(); ++ii)
    {
        cout << "cmdstr[" << ii << "]=" << cmdstr[ii] << endl;
    }

    // �����ֺ��Ԫ�أ�һ�����ڵ��ԡ�
    cout << cmdstr;

    // ��ȡ��ֺ�Ԫ�ص����ݡ�
    cmdstr.getvalue(0, stplayer.name, 50); // ��ȡ����
    cmdstr.getvalue(1, stplayer.no, 5);    // ��ȡ���º���
    cmdstr.getvalue(2, stplayer.striker);  // ����λ��
    cmdstr.getvalue(3, stplayer.age);      // ��ȡ����
    cmdstr.getvalue(4, stplayer.weight);   // ��ȡ����
    cmdstr.getvalue(5, stplayer.sal);      // ��ȡ��н��ŷԪ��
    cmdstr.getvalue(6, stplayer.club, 50); // ��ȡЧ���ľ��ֲ�

    printf("name=%s,no=%s,striker=%d,age=%d,weight=%.1f,sal=%ld,club=%s\n",
           stplayer.name, stplayer.no, stplayer.striker, stplayer.age,
           stplayer.weight, stplayer.sal, stplayer.club);
    // ������:name=messi,no=10,striker=1,age=30,weight=68.5,sal=21000000,club=Barcelona
}
