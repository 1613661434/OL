/*
 *  程序名：test_ol_mysql_deletetable.cpp，此程序演示开发框架操作MySQL数据库（删除表中的数据）。
 *  作者：ol
 */
#include "ol_mysql.h"
#include <cstdio>

using namespace std;
using namespace ol::mysql;

int main(int argc, char* argv[])
{
    DBConn conn;

    // ===================== 数据库连接 =====================
    conn.setConnectParam("root:0088@127.0.0.1:3306/testdb", "utf8mb4");
    if (!conn.connect())
    {
        printf("connect database failed.\n%s\n", conn.errorMsg().c_str());
        return -1;
    }

    printf("connect database ok.\n");

    // ===================== 创建预处理语句 =====================
    auto stmt = conn.createStmt();

    // 静态SQL语句：删除id=10的数据
    stmt->prepare("delete from girls where id=10");
    if (!stmt->execute())
    {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt->sql(), stmt->errorMsg().c_str());
        return -1;
    }

    // ===================== 影响行数 =====================
    printf("成功删除了%ld条记录。\n", stmt->affectedRows());

    int minid = 11, maxid = 13;

    // 动态SQL语句：删除id区间数据
    stmt->prepare("delete from girls where id>=? and id<=?");
    stmt->bindin(1, minid);
    stmt->bindin(2, maxid);

    if (!stmt->execute())
    {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt->sql(), stmt->errorMsg().c_str());
        return -1;
    }

    printf("成功删除了%ld条记录。\n", stmt->affectedRows());

    // 提交事务
    conn.commit();

    // ===================== 删除整个 girls 表 =====================
    if (conn.execute("DROP TABLE IF EXISTS girls;") != 0)
    {
        printf("删除表 girls 失败：%s\n", conn.errorMsg().c_str());
        return -1;
    }
    printf("已成功删除整个 girls 数据表！\n");

    return 0;
}