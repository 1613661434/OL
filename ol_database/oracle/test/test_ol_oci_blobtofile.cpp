/*
 *  程序名：test_ol_oci_blobtofile.cpp，演示从数据库读取BLOB字段并保存到文件
 *  作者：ol
 */
#include "ol_oci.h" // 开发框架操作Oracle的头文件。
#include <cstdio>
#include <string>

using namespace std;
using namespace ol::oracle;

int main(int argc, char* argv[])
{
    DBConn conn; // 创建数据库连接类的对象。

    // ===================== 数据库连接 =====================
    conn.setConnectParam("scott/000888@snorcl11g_5", "Simplified Chinese_China.AL32UTF8");
    if (!conn.connect())
    {
        printf("connect database failed.\n%s\n", conn.errorMsg().c_str());
        return -1;
    }

    printf("connect database ok.\n");

    // ===================== 创建预处理语句 =====================
    auto stmt = conn.createStmt();

    stmt->prepare("select pic from girls where id=1");
    stmt->bindblob(1);

    // ===================== execute返回bool =====================
    if (!stmt->execute())
    {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt->sql(), stmt->errorMsg().c_str());
        return -1;
    }

    // 获取查询结果
    int ret = stmt->next();
    if (ret == 100) // OCI_NO_DATA
    {
        printf("没有找到id=1的记录。\n");
        return 0;
    }
    else if (ret != 0)
    {
        printf("获取记录失败：%s\n", stmt->errorMsg().c_str());
        return -1;
    }

    // 将BLOB字段内容写入文件
    const string filename = "/home/mysql/OL/ol_database/oracle/test/data/pic_out.jpeg";
    if (stmt->blobtofile(1, filename) != 0)
    {
        printf("BLOB字段写入文件失败：%s\n", stmt->errorMsg().c_str());
        return -1;
    }

    printf("已把数据库的BLOB字段提取到文件。\n");
    printf("路径：%s\n", filename.c_str());

    return 0;
}