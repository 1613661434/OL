/*
 *  ��������test_ol_string_replacestr.cpp���˳�����ʾ����������ַ����滻replacestr������ʹ�á�
 *  ���ߣ�ol
 */
#include "ol_string.h"
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
    char str1[301];

    strcpy(str1, "name:messi,no:10,job:striker.");
    replacestr(str1, ":", "="); // ��ð���滻�ɵȺš�
    printf("str1=%s=\n", str1); // ��������str1=name=messi,no=10,job=striker.=

    strcpy(str1, "name:messi,no:10,job:striker.");
    replacestr(str1, "name:", ""); // ��"name:"�滻��""���൱��ɾ������"name:"��
    printf("str1=%s=\n", str1);    // ��������str1=messi,no:10,job:striker.=

    strcpy(str1, "messi----10----striker");
    replacestr(str1, "--", "-", false); // ������"--"�滻��һ��"-"��bloop����Ϊfalse��
    printf("str1=%s=\n", str1);         // ��������str1=messi--10--striker=

    strcpy(str1, "messi----10----striker");
    replacestr(str1, "--", "-", true); // ������"--"�滻��һ��"-"��bloop����Ϊtrue��
    printf("str1=%s=\n", str1);        // ��������str1=messi-10-striker=

    strcpy(str1, "messi-10-striker");
    replacestr(str1, "-", "--", false); // ��һ��"-"�滻������"--"��bLoop����Ϊfalse��
    printf("str1=%s=\n", str1);         // ��������str1=messi--10--striker=

    // ���´����"-"�滻��"--"��bloop����Ϊtrue�������߼�����replacestr����ִ���滻��
    strcpy(str1, "messi-10-striker");
    replacestr(str1, "-", "--", true); // ��һ��"-"�滻������"--"��bloop����Ϊtrue��
    printf("str1=%s=\n", str1);        // ��������str1=messi-10-striker=

    // ////////////////////////////////////
    string str2;
    str2 = "name:messi,no:10,job:striker.";
    replacestr(str2, ":", "=");       // ��ð���滻�ɵȺš�
    cout << "str2=" << str2 << "=\n"; // ��������str2=name=messi,no=10,job=striker.=

    str2 = "name:messi,no:10,job:striker.";
    replacestr(str2, "name:", "");    // ��"name:"�滻��""���൱��ɾ������"name:"��
    cout << "str2=" << str2 << "=\n"; // ��������str2=messi,no:10,job:striker.=

    str2 = "messi----10----striker";
    replacestr(str2, "--", "-", false); // ������"--"�滻��һ��"-"��bLoop����Ϊfalse��
    cout << "str2=" << str2 << "=\n";   // ��������str2=messi--10--striker=

    str2 = "messi----10----striker";
    replacestr(str2, "--", "-", true); // ������"--"�滻��һ��"-"��bLoop����Ϊtrue��
    cout << "str2=" << str2 << "=\n";  // ��������str2=messi-10-striker=

    str2 = "messi-10-striker";
    replacestr(str2, "-", "--", false); // ��һ��"-"�滻������"--"��bLoop����Ϊfalse��
    cout << "str2=" << str2 << "=\n";   // ��������str2=messi--10--striker=

    // ���´����"-"�滻��"--"��bloop����Ϊtrue�������߼�����updatestr����ִ���滻��
    str2 = "messi-10-striker";
    replacestr(str2, "-", "--", true); // ��һ��"-"�滻������"--"��bloop����Ϊtrue��
    cout << "str2=" << str2 << "=\n";  // ��������str2=messi-10-striker=
}