/*
 *  ��������test_ol_fstream_newdir.cpp���˳�����ʾ��������в���newdir�������ݾ���·�����ļ�����Ŀ¼���𼶵Ĵ���Ŀ¼��
 *  ���ߣ�ol
 */
#include "ol_fstream.h"

using namespace ol;

int main()
{
    // /tmp/aaa/bbb/ccc/ddd    /tmp    /tmp/aaa    /tmp/aaa/bbb    /tmp/aaa/bbb/ccc   /tmp/aaa/bbb/ccc/ddd
    newdir("/tmp/aaa/bbb/ccc/ddd", false); // ����"/tmp/aaa/bbb/ccc/ddd"Ŀ¼��

    newdir("/tmp/111/222/333/444/data.xml", true); // ����"/tmp/111/222/333/444"Ŀ¼��
}