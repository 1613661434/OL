/*
 *  程序名：test_ol_mysql_createtable.cpp，此程序演示开发框架操作MySQL数据库（创建表）。
 *  作者：ol
 */
#include "ol_mysql.h" // 开发框架操作MySQL的头文件。

using namespace std;
using namespace ol::mysql;

int main(int argc, char* argv[])
{
    DBConn conn; // 创建数据库连接类的对象。

    // 登录数据库
    if (conn.connecttodb("root:0088@127.0.0.1:3306/testdb", "utf8mb4") != 0)
    {
        printf("connect database failed.\n%s\n", conn.message().c_str());
        return -1;
    }

    printf("connect database ok.\n");

    DBStmt stmt;         // 操作SQL语句的对象。
    stmt.connect(&conn); // 指定stmt对象使用的数据库连接。
    // 准备创建表的SQL语句（MySQL数据类型）
    // 超女表girls，超女编号id，超女姓名name，体重weight，报名时间btime，超女说明memo，超女图片pic。
    stmt.prepare("\
            create table if not exists girls(id    int,\
                                        name  varchar(30),\
                                        weight   decimal(8,2),\
                                        btime datetime,\
                                        memo  text,\
                                        pic   blob,\
                                        primary key (id))");

    // 执行SQL语句
    if (stmt.execute() != 0)
    {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message().c_str());
        return -1;
    }

    printf("create table girls ok.\n");

    return 0;
}