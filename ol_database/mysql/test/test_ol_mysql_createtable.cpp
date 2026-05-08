/*
 *  程序名：test_ol_mysql_createtable.cpp，此程序演示开发框架操作MySQL数据库（创建表）。
 *  作者：ol
 */
#include "ol_mysql.h"
#include <cstdio>

using namespace std;
using namespace ol::mysql;

int main(int argc, char* argv[])
{
    DBConn conn;

    // ===================== 登录数据库 =====================
    conn.setConnectParam("root:0088@127.0.0.1:3306/testdb", "utf8mb4");
    if (!conn.connect())
    {
        printf("connect database failed.\n%s\n", conn.errorMsg().c_str());
        return -1;
    }

    printf("connect database ok.\n");

    // ===================== 创建预处理语句 =====================
    auto stmt = conn.createStmt();

    // 准备创建表的SQL语句（MySQL数据类型）
    // 超女表girls，超女编号id，超女姓名name，体重weight，报名时间btime，超女说明memo，超女图片pic。
    const char* sql = "\
            create table if not exists girls(id    int,\
                                        name  varchar(30),\
                                        weight   decimal(8,2),\
                                        btime datetime,\
                                        memo  text,\
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