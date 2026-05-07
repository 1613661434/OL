/*
 * 程序名：test_ol_fstream_cdir.cpp，此程序演示开发框架中采用cdir类获取某目录及其子目录中的文件列表信息
 * 作者：ol
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
#elif defined(__unix__)
    const string test_dir = R"(/home/mysql/test_ol)";
#endif

    const string match_rule = "*";
    const size_t max_files = 1000;
    const bool recursive = true;
    const bool sort_files = true;
    const bool withDotFiles = false;

    cout << "===== 开始测试目录遍历 =====" << "\n";
    cout << "测试目录: " << test_dir << "\n";
    cout << "匹配规则: " << match_rule << "\n";
    cout << "是否递归子目录: " << (recursive ? "是" : "否") << "\n";
    cout << "最大文件数: " << max_files << "\n";
    cout << "===========================" << "\n";

    // 打开目录
    if (dir.opendir(test_dir, match_rule, max_files, recursive, sort_files, withDotFiles) == false)
    {
#ifdef _WIN32
        // 使用VS推荐的strerror_s替代strerror，避免安全警告
        char err_msg[256];                           // 存储错误信息的缓冲区
        strerror_s(err_msg, sizeof(err_msg), errno); // 安全版本的错误信息函数
#elif defined(__unix__)
        perror("错误：opendir() 失败！原因：\n");
#endif
        return -1;
    }

    if (dir.size() == 0)
    {
        cout << "提示：未找到符合条件的文件。" << "\n";
        return 0;
    }

    cout << "\n===== 找到 " << dir.size() << " 个文件 =====" << "\n";
    size_t count = 0;
    while (dir.readdir())
    {
        ++count;
        cout << "[" << count << "] "
             << "全路径: " << dir.m_ffilename << "\n"
             << "文件名: " << dir.m_filename << "\n"
             << "大小: " << dir.m_filesize << "字节" << "\n";
    }

    cout << "\n===== 遍历完成 =====" << "\n";
    return 0;
}