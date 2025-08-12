/*
 *  程序名：test_ol_fstream_cifile_binary.cpp，此程序演示开发框架中采用cifile类从文件中读取二进制数据。
 *  作者：ol
 */
#include "ol_fstream.h"

using namespace ol;
using namespace std;

int main()
{
    cifile ifile;

    // 打开文件。
    if (ifile.open("/tmp/data/girl.dat", ios::binary) == false)
    {
        printf("ifile.open(/tmp/data/girl.dat) failed.\n");
        return -1;
    }

    struct st_girl
    {
        int bh;
        char name[21];
    } girl;

    memset(&girl, 0, sizeof(girl));

    ifile.read(&girl, sizeof(girl));

    printf("bh=%d,name=%s\n", girl.bh, girl.name);

    // sleep(30);   // 用ls /tmp/data/*.tmp可以看到生成的临时文件。

    // ifile.closeandremove();     // 关闭并删除文件。
    ifile.close(); // 关闭文件。
}