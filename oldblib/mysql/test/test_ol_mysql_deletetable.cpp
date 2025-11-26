/*
 *  程序名：test_ol_mysql_deletetable.cpp，此程序演示开发框架操作MySQL数据库（删除表中的数据）。
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

    DBStmt stmt(&conn);

    // 静态SQL语句
    stmt.prepare("delete from girls where id=10");
    if (stmt.execute() != 0)
    {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message().c_str());
        return -1;
    }

    printf("成功删除了%ld条记录。\n", stmt.rpc()); // MySQL使用rpc()获取影响行数

    int minid = 11, maxid = 13;

    // 动态SQL语句（MySQL使用?作为占位符）
    stmt.prepare("delete from girls where id>=? and id<=?");
    stmt.bindin(1, minid);
    stmt.bindin(2, maxid);

    if (stmt.execute() != 0)
    {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message().c_str());
        return -1;
    }

    printf("成功删除了%ld条记录。\n", stmt.rpc());

    conn.commit(); // 提交事务

    return 0;
}