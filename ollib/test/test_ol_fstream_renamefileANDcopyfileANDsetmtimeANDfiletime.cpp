/*
 *  程序名：test_ol_fstream_renamefileANDcopyfileANDsetmtimeANDfiletime.cpp，此程序演示开发框架的文件操作函数的用法
 *  作者：ol
 */
#include "ol_fstream.h"
#include <iostream>

using namespace ol;
using namespace std;

int main()
{
    // 重命名文件。
    if (renamefile("/project/public/lib_public.so", "/tmp/aaa/bbb/ccc/lib_public.so") == false)
    {
        printf("renamefile(/project/public/lib_public.so) %d:%s\n", errno, strerror(errno));
    }

    // 复制文件。
    if (copyfile("/project/public/libftp.a", "/tmp/aaa/ddd/ccc/libftp.a") == false)
    {
        printf("copyfile(/project/public/libftp.a) %d:%s\n", errno, strerror(errno));
    }

    // 获取文件的大小。
    printf("size=%ld\n", filesize("/project/public/_public.h"));

    // 重置文件的时间。
    setmtime("/project/public/_public.h", "2020-01-05 13:37:29");

    // 获取文件的时间。
    string mtime;
    filemtime("/project/public/_public.h", mtime, "yyyy-mm-dd hh24:mi:ss");
    cout << "mtime=" << mtime << endl; // 输出mtime=2020-01-05 13:37:29
    filemtime("/project/public/_public.h", mtime);
    cout << "mtime=" << mtime << endl; // 输出mtime=20200105133729
}