/*
 *  程序名：test_ol_ftp_cftpclient_get.cpp，此程序演示采用开发框架的cftpclient类下载文件。
 *  作者：ol
 */

#if !defined(__linux__)
#error "仅支持Linux平台，不支持当前系统！"
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

    // 把服务器上的/home/mysql/tmp/ol35_server.cpp下载到本地，存为/home/mysql/test/ol35_server.cpp。
    // 如果本地的/home/mysql/test目录不存在，就创建它。
    if (ftp.get("/home/mysql/tmp/ol35_server.cpp", "/home/mysql/test/ol35_server.cpp") == false)
    {
        printf("ftp.get() failed.\n");
        return -1;
    }

    printf("get /home/mysql/tmp/ol35_server.cpp ok.\n");

    /*
    // 删除服务上的/home/mysql/tmp/ol35_server.cpp文件。
    if (ftp.ftpdelete("/home/mysql/tmp/ol35_server.cpp")==false) { printf("ftp.ftpdelete() failed.\n"); return -1; }

    printf("delete /home/mysql/tmp/ol35_server.cpp ok.\n");

    // 删除服务器上的/home/mysql/tmp目录，如果目录非空，删除将失败。
    if (ftp.rmdir("/home/mysql/tmp")==false) { printf("ftp.rmdir() failed.\n"); return -1; }
    */
}
