/*
 *  程序名：test_ol_oci_createtable.cpp，此程序演示开发框架操作Oracle数据库（创建表）。
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

    sqlstatement stmt;   // 操作SQL语句的对象。
    stmt.connect(&conn); // 指定stmt对象使用的数据库连接。
    // 准备创建表的SQL语句。
    // 如果SQL语句有错误，prepare()不会返回失败，所以，prepare()不需要判断返回值。
    // 超女表girls，超女编号id，超女姓名name，体重weight，报名时间btime，超女说明memo，超女图片pic。
    stmt.prepare("\
            create table girls(id    number(10),\
                                        name  varchar2(30),\
                                        weight   number(8,2),\
                                        btime date,\
                                        memo  varchar2(300),\
                                        pic   blob,\
                                        primary key (id))");

    // 执行SQL语句，一定要判断返回值，0-成功，其它-失败。
    // 失败代码在stmt.m_cda.rc中，失败描述在stmt.m_cda.message中。
    if (stmt.execute() != 0)
    {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message());
        return -1;
    }

    printf("create table girls ok.\n");

    // conn.disconnect();              // 在connection类的析构函数中会自动调用disconnect()方法。

    return 0;
}