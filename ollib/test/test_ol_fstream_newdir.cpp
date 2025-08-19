/*
 *  程序名：test_ol_fstream_newdir.cpp，此程序演示开发框架中采用newdir函数根据绝对路径的文件名或目录名逐级的创建目录。
 *  作者：ol
 */
#include "ol_fstream.h"

using namespace ol;

int main()
{
#ifdef __linux__
    // 创建"/tmp/test_ol/bbb/ccc/ddd"目录。
    newdir("/tmp/test_ol/bbb/ccc/ddd", false);

    // 创建"/tmp/test_ol/222/333/444"目录。
    newdir("/tmp/test_ol/222/333/444/data.xml", true);
#elif defined(_WIN32)
    // 创建"C:\test_ol\aaa\bbb"目录。
    newdir(R"(C:\test_ol\aaa\bbb)", false);

    // 创建"C:\test_ol\aaa\kkk"目录。
    newdir(R"(C:\test_ol\aaa\kkk\data.txt)", true);
#endif
}