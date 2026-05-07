/*
 *  程序名：test_ol_fstream_clogfile.cpp，此程序演示采用开发框架的clogfile类记录程序的运行日志。
 *  作者：ol
 */
#include "ol_fstream.h"

using namespace ol;
using namespace std;

int main()
{
    clogfile<> logfile; // 创建日志对象。

    string filePath;

#ifdef __unix__
    filePath = "/tmp/test_ol/test_ol_fstream_clogfile.log";
#elif defined(_WIN32)
    filePath = R"(C:\test_ol\test_ol_fstream_clogfile.log)";
#endif

    // 打开日志文件。
    if (logfile.open(filePath, ios::out, true, 2, false) == false)
    {
        printf("logfile.open(%s) failed.\n", filePath.c_str());
        return -1;
    }

    logfile.write("程序开始运行。\n");

    for (size_t i = 0; i <= 100000; ++i)
    {
        logfile.write("这是第%d个%s...ok.\n", i, "OL");
        logfile.write("第%d个OL开始编程...", i);
        // sleep_ms(2);
        logfile << "ok.\n";
    }

    for (size_t i = 0; i <= 1000; ++i)
    {
        logfile.debug("DEBUG MODE");
        logfile.info("INFO MODE");
        logfile.warn("WARN MODE");
        logfile.error("ERROR MODE");
    }

    logfile.write("程序运行结束。\n");
}