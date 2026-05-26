/*
 *  程序名：test_ol_oci_createtable.cpp，此程序演示开发框架操作Oracle数据库（创建表）。
 *  作者：ol
 */
#include "ol_oci.h" // 开发框架操作Oracle的头文件。
#include <cstdio>

using namespace std;
using namespace ol::oracle;

int main(int argc, char* argv[])
{
    DBConn conn; // 创建数据库连接类的对象。

    // ===================== 数据库连接 =====================
    // 失败代码在conn.code()中，失败描述在conn.errorMsg()中。
    conn.setConnectParam("scott/000888@snorcl11g_5", "Simplified Chinese_China.AL32UTF8");
    if (!conn.connect())
    {
        printf("connect database failed.\n%s\n", conn.errorMsg().c_str());
        return -1;
    }

    printf("connect database ok.\n");

    // ===================== 创建预处理语句 =====================
    auto stmt = conn.createStmt(); // 操作SQL语句的对象。

    // 准备创建表的SQL语句（Oracle数据类型）
    // 超女表girls，超女编号id，超女姓名name，体重weight，报名时间btime，超女说明memo，超女图片pic。
    const char* sql = "\
            create table girls(id    number(10),\
                                        name  varchar2(30),\
                                        weight   number(8,2),\
                                        btime date,\
                                        memo  clob,\
                                        pic   blob,\
                                        primary key (id))";
    stmt->prepare(sql);

    // ===================== 执行SQL =====================
    if (!stmt->execute())
    {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt->sql(), stmt->errorMsg().c_str());
        return -1;
    }

    printf("create table girls ok.\n");

    return 0;
}