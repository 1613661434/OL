/*
 *  ��������test_ol_string_matchstr.cpp���˳�����ʾ�������������ʾmatchstr������ʹ�á�
 *  ���ߣ�ol
 */
#include "ol_string.h"
#include <iostream>

using namespace ol;
using namespace std;

int main()
{
    // ���´��뽫���yes��
    if (matchstr("_public.h", "*.h,*.cpp") == true)
        printf("yes\n");
    else
        printf("no\n");

    // ���´��뽫���yes��
    if (matchstr("_public.h", "*.H") == true)
        printf("yes\n");
    else
        printf("no\n");

    // ���´��뽫���no��
    if (matchstr("_public.h", "*p*k*.h") == true)
        printf("yes\n");
    else
        printf("no\n");

    // ���´��뽫���yes��
    if (matchstr("_public.h", "*") == true)
        printf("yes\n");
    else
        printf("no\n");
}