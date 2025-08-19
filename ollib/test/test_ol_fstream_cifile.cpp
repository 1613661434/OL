/*
 *  程序名：test_ol_fstream_cifile.cpp，此程序演示开发框架中采用cifile类从文本文件中读取数据。
 *  作者：ol
 */
#include "ol_fstream.h"
#include <cstdio> // 用于remove函数
#include <fstream>
#include <iostream>
#include <string>

using namespace ol;
using namespace std;

// 创建测试文件
void createTestFile(const std::string& filename, const std::string& content)
{
    cofile ofile;

    if (ofile.open(filename, false) == false)
    {
        printf("ofile.open(%s) failed.\n", filename.c_str());
        exit(-1);
    }

    ofile.writeline("%s", content);
}

void Test(const std::string& filename)
{
    cifile ifile;

    // 打开文件。
    if (ifile.open(filename) == false)
    {
        printf("ifile.open(%s) failed.\n", filename.c_str());
        exit(-1);
    }

    string strline; // 存放从文本文件中读取的一行。

    while (true)
    {
        // 从文件中读取一行。
        if (ifile.readline(strline) == false) break;

        cout << "=" << strline << "=\n";
    }

    // ifile.closeandremove();     // 关闭并删除文件。
    ifile.close(); // 关闭文件。
}

int main()
{
    std::string filename;
#ifdef __linux__
    filename = "/tmp/test_ol/ol.txt";
#elif defined(_WIN32)
    filename = R"(C:\test_ol\ol.txt)";
#endif

    // 测试场景1: 文件以换行符结尾
    std::cout << "=== 场景1: 文件以换行符结尾 ===\n";
    std::cout << R"("Line1\nLine2\n")" << "\n";
    createTestFile(filename, "Line1\nLine2\n");
    Test(filename);

    // 测试场景2: 文件不以换行符结尾
    std::cout << "\n\n=== 场景2: 文件不以换行符结尾 ===\n";
    std::cout << R"("Line1\nLine2")" << "\n";
    createTestFile(filename, "Line1\nLine2");
    Test(filename);

    // 测试场景3: 文件首行为换行符
    std::cout << "\n\n=== 场景3: 文件首行为换行符 ===\n";
    std::cout << R"("\nLine2\n")" << "\n";
    createTestFile(filename, "\nLine2\n");
    Test(filename);

    // 测试场景4: 文件为空
    std::cout << "\n\n=== 场景4: 文件为空 ===\n";
    std::cout << R"("")" << "\n";
    createTestFile(filename, "");
    Test(filename);

    // 清理：使用C标准库的remove函数
    if (remove(filename.c_str()) != 0)
    {
        cerr << "删除文件 " << filename << " 失败\n";
    }

    return 0;
}