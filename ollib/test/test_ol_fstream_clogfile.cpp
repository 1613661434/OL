/*
 *  程序名：test_ol_fstream_clogfile.cpp，此程序演示采用开发框架的clogfile类记录程序的运行日志。
 *  作者：ol
 */
#include "ol_fstream.h"

using namespace ol;
using namespace std;

int main()
{
    clogfile logfile; // 创建日志对象。

    string filePath;

#ifdef __linux__
    filePath = "/tmp/log/test_ol_fstream_clogfile.log";
#elif defined(_WIN32)
    filePath = R"(C:\test\test_ol_fstream_clogfile.log)";
#endif

    // 打开日志文件。
    if (logfile.open(filePath, ios::out, false) == false)
    {
        printf("logfile.open(%s) failed.\n", filePath.c_str());
        return -1;
    }

    logfile.write("程序开始运行。\n");

    for (size_t i = 0; i <= 1000; ++i)
    {
        logfile.write("这是第%d个%s...ok.\n", i, "超级女生");
        logfile.write("第%d个超女开始表演...", i); // 表演前，写一行日志，...表示正在表演中。
        // sleep_ms(2);                                // 超女在表演中。
        logfile << "ok.\n"; // 表演完成后，写入ok。
    }

    logfile.write("程序运行结束。\n");
}