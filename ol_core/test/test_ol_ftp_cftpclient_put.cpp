/*
 *  程序名：test_ol_ftp_cftpclient_put.cpp，此程序演示采用开发框架的cftpclient类上传文件。
 *  作者：ol
 */

#if !defined(__unix__)
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

    // 在ftp服务器上创建/home/mysql/tmp，注意，如果目录已存在，会返回失败。
    if (ftp.mkdir("/home/mysql/tmp") == false)
    {
        printf("ftp.mkdir() failed.\n");
        return -1;
    }

    // 把ftp服务器上的工作目录切换到/home/mysql/tmp
    if (ftp.chdir("/home/mysql/tmp") == false)
    {
        printf("ftp.chdir() failed.\n");
        return -1;
    }

    // 把本地的demo51.cpp上传到ftp服务器的当前工作目录。
    if (ftp.put("/home/mysql/src_mysql/Test/ol35_server.cpp", "ol35_server.cpp") == true)
        printf("put ol35_server.cpp ok.\n");
    else
        printf("put ol35_server.cpp failed.\n");

    // 如果不调用chdir切换工作目录，以下代码采用全路径上传文件。
    // ftp.put("/project/public/demo/demo51.cpp","/home/wucz/tmp/demo51.cpp");
}