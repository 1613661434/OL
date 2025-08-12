/*
 *  程序名：test_ol_ooci_filetoclob.cpp，此程序演示开发框架操作Oracle数据库（把文本文件存入数据库表的CLOB字段中）。
 *  作者：ol
 */
#include "ol_oci.h" // 开发框架操作Oracle的头文件。

using namespace std;
using namespace ol;

int main(int argc, char* argv[])
{
    connection conn; // 创建数据库连接类的对象。

    // 登录数据库，返回值：0-成功，其它-失败。
    // 失败代码在conn.m_cda.rc中，失败描述在conn.m_cda.message中。
    if (conn.connecttodb("scott/000888@snorcl11g_5", "Simplified Chinese_China.AL32UTF8") != 0)
    {
        printf("connect database failed.\n%s\n", conn.message());
        return -1;
    }

    printf("connect database ok.\n");

    // 修改girls表结构，增加memo1字段，用于测试。 alter table girls add memo1 clob;
    sqlstatement stmt(&conn);
    stmt.prepare("insert into girls(id,name,memo1) values(1,'冰冰',empty_clob())"); // 注意：不可用null代替empty_clob()。
    if (stmt.execute() != 0)
    {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
        return -1;
    }

    // 使用游标从girls表中提取记录的memo1字段
    stmt.prepare("select memo1 from girls where id=1 for update");
    stmt.bindclob();

    // 执行SQL语句，一定要判断返回值，0-成功，其它-失败。
    if (stmt.execute() != 0)
    {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
        return -1;
    }

    // 获取一条记录，一定要判断返回值，0-成功，1403-无记录，其它-失败。
    if (stmt.next() != 0) return 0;

    // 把磁盘文件memo_in.txt的内容写入CLOB字段，一定要判断返回值，0-成功，其它-失败。
    if (stmt.filetolob("/PROJECT/OL/db/oracle/Test/data/memo_in.txt") != 0)
    {
        printf("stmt.filetolob() failed.\n%s\n", stmt.message());
        return -1;
    }

    printf("文本文件已存入数据库的CLOB字段中。\n");

    conn.commit();

    return 0;
}