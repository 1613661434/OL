/*
 * ��������test_ol_fstream_cdir.cpp���˳�����ʾ��������в���cdir���ȡĳĿ¼������Ŀ¼�е��ļ��б���Ϣ
 * ���ߣ�ol
 */

#include "ol_fstream.h"
#include <cstring>
#include <iostream>

using namespace ol;
using namespace std;

int main()
{
    cdir dir;
#ifdef _WIN32
    const string test_dir = R"(D:\Visual Studio)";
#elif defined(__linux__)
    const string test_dir = R"(/PROJECT/)";
#endif

    const string match_rule = "*";
    const size_t max_files = 1000;
    const bool recursive = true;
    const bool sort_files = true;

    cout << "===== ��ʼ����Ŀ¼���� =====" << "\n";
    cout << "����Ŀ¼: " << test_dir << "\n";
    cout << "ƥ�����: " << match_rule << "\n";
    cout << "�Ƿ�ݹ���Ŀ¼: " << (recursive ? "��" : "��") << "\n";
    cout << "����ļ���: " << max_files << "\n";
    cout << "===========================" << "\n";

    // ��Ŀ¼
    if (dir.opendir(test_dir, match_rule, max_files, recursive, sort_files) == false)
    {
#ifdef _WIN32
        // ʹ��VS�Ƽ���strerror_s���strerror�����ⰲȫ����
        char err_msg[256];                           // �洢������Ϣ�Ļ�����
        strerror_s(err_msg, sizeof(err_msg), errno); // ��ȫ�汾�Ĵ�����Ϣ����
#elif defined(__linux__)
        perror("����opendir() ʧ�ܣ�ԭ��\n");
#endif
        return -1;
    }

    if (dir.size() == 0)
    {
        cout << "��ʾ��δ�ҵ������������ļ���" << "\n";
        return 0;
    }

    cout << "\n===== �ҵ� " << dir.size() << " ���ļ� =====" << "\n";
    size_t count = 0;
    while (dir.readdir())
    {
        ++count;
        cout << "[" << count << "] "
             << "ȫ·��: " << dir.m_ffilename << "\n"
             << "�ļ���: " << dir.m_filename << "\n"
             << "��С: " << dir.m_filesize << "�ֽ�" << "\n";
    }

    cout << "\n===== ������� =====" << "\n";
    return 0;
}