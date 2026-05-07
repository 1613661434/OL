/*
 *  程序名：test_ol_fstream_cofile.cpp，此程序演示开发框架中采用cofile类向文件中写入文本数据。
 *  作者：ol
 */
#include "ol_fstream.h"

using namespace ol;

int main()
{
    cofile ofile;

    // 创建文件，实际创建的是临时文件，例如/tmp/data/girl.xml.tmp。
    if (ofile.open("/tmp/data/girl.xml") == false)
    {
        printf("ofile.open(/tmp/data/girl.xml) failed.\n");
        return -1;
    }

    // 用<<输出到文件，与cout的用法相同。
    ofile << "<data>" << "\n"; // 换行只能用\n，不能用endl。

    // 格式化输出到文件。
    ofile.writeline("<name>%s</name><age>%d</age><sc>%s</sc><yz>%s</yz><memo>%s</memo><endl/>\n",
                    "妲已", 28, "火辣", "漂亮", "商要亡，关我什么事。");
    ofile.writeline("<name>%s</name><age>25</age><sc>火辣</sc><yz>漂亮</yz><memo>1、中国排名第一的美女；\n"
                    "2、男朋友是范蠡；\n"
                    "3、老公是夫差，被勾践弄死了。</memo><endl/>\n",
                    "西施");

    ofile << "</data>\n"; // 换行只能用\n，不能用endl。

    // sleep(10);   // 用ls /tmp/data/*.tmp可以看到生成的临时文件。

    // 关闭文件，并把临时文件名改为正式的文件名。
    ofile.closeandrename();
}