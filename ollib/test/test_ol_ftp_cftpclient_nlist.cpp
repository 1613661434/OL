/*
 *  程序名：test_ol_ftp_cftpclient_nlist.cpp，此程序演示采用开发框架的cftpclient类获取ftp服务器上的文件列表、时间和大小。
 *  作者：ol
 */

#if !defined(__linux__)
#error "test_fifo_receiver.cpp 仅支持Linux平台，不支持当前系统！"
#endif

#include "ol_ftp.h"
#include <iostream>

using namespace ol;
using namespace std;

int main(int argc, char* argv[])
{
    cftpclient ftp;

    // 登录远程ftp服务器，请改为你自己服务器的ip地址。
    if (ftp.login("192.168.195.5:21", "mysql", "000888") == false)
    {
        printf("ftp.login(192.168.195.5:21,mysql/000888) failed.\n");
        return -1;
    }

    // 获取服务器上/PROJECT/OL/include/*.h文件列表，保存在本地的/tmp/list/tmp.list文件中。
    // 如果/tmp/list目录不存在，会自动创建它。
    if (ftp.nlist("/PROJECT/OL/include/*.h", "/tmp/list/tmp.list") == false)
    {
        printf("ftp.nlist() failed.\n");
        return -1;
    }
    cout << "ret=" << ftp.response() << endl;

    cifile ifile; // 采用开发框架的cifile类来操作list文件。
    string strFileName;

    ifile.open("/tmp/list/tmp.list"); // 打开list文件。

    while (true) // 获取每个文件的时间和大小。
    {
        if (ifile.readline(strFileName) == false) break;

        ftp.mtime(strFileName); // 获取文件时间。
        ftp.size(strFileName);  // 获取文件大小。

        printf("filename=%s,mtime=%s,size=%d\n", strFileName.c_str(), ftp.m_mtime.c_str(), ftp.m_size);
    }
}