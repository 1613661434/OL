/*
 *  程序名：test_ol_fstream_cofile_binary.cpp，此程序演示开发框架中采用cofile类向文件中写入二进制数据。
 *  作者：ol
 */
#include "ol_fstream.h"

using namespace ol;
using namespace std;

int main()
{
    cofile ofile;

    // 创建文件，实际创建的是临时文件，例如/tmp/data/girl.dat.tmp。
    if (ofile.open("/tmp/data/girl.dat", true, ios::binary) == false)
    {
        printf("ofile.open(/tmp/data/girl.dat) failed.\n");
        return -1;
    }

    struct st_girl
    {
        int bh;
        char name[21];
    } girl;

    memset(&girl, 0, sizeof(girl));
    girl.bh = 8;
    strcpy(girl.name, "西施");
    ofile.write(&girl, sizeof(girl));

    // sleep(30);   // 用ls /tmp/data/*.tmp可以看到生成的临时文件。

    // 关闭文件，并把临时文件名改为正式的文件名。
    ofile.closeandrename();
}