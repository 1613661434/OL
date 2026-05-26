/*
 *  程序名：test_ol_oci_deletetable.cpp，此程序演示开发框架操作Oracle数据库（删除表中的数据）。
 *  作者：ol
 */
#include "ol_oci.h" // 开发框架操作Oracle的头文件。

using namespace std;
using namespace ol::oracle;

int main(int argc, char* argv[])
{
    DBConn conn; // 创建数据库连接类的对象。

    // 登录数据库，返回值：true-成功，false-失败。
    // 失败代码在conn.code()中，失败描述在conn.errorMsg()中。
    conn.setConnectParam("scott/000888@snorcl11g_5", "Simplified Chinese_China.AL32UTF8");
    if (!conn.connect())
    {
        printf("connect database failed.\n%s\n", conn.errorMsg().c_str());
        return -1;
    }

    printf("connect database ok.\n");

    auto stmt = conn.createStmt();

    // 静态SQL语句。
    stmt->prepare("delete from girls where id=10");
    // 执行SQL语句，一定要判断返回值，true-成功，false-失败。
    // 失败代码在stmt->code()中，失败描述在stmt->errorMsg()中。
    if (!stmt->execute())
    {
        printf("stmt->execute() failed.\n%s\n%s\n", stmt->sql(), stmt->errorMsg().c_str());
        return -1;
    }

    printf("成功删除了%zu条记录。\n", stmt->affectedRows()); // stmt->affectedRows()是本次执行SQL影响的记录数。

    int minid = 11, maxid = 13;

    // 动态SQL语句。
    stmt->prepare("delete from girls where id>=:1 and id<=:2"); // :1,:2,...,:n可以理解为输入参数。
    stmt->bindin(1, minid);
    stmt->bindin(2, maxid);

    // 执行SQL语句，一定要判断返回值，true-成功，false-失败。
    // 失败代码在stmt->code()中，失败描述在stmt->errorMsg()中。
    if (!stmt->execute())
    {
        printf("stmt->execute() failed.\n%s\n%s\n", stmt->sql(), stmt->errorMsg().c_str());
        return -1;
    }

    printf("成功删除了%zu条记录。\n", stmt->affectedRows()); // stmt->affectedRows()是本次执行SQL影响的记录数。

    conn.commit(); // 提交事务。

    // 删除整个表
    if (conn.execute("DROP TABLE girls PURGE") != 0)
    {
        printf("drop table girls failed.\n%s\n", conn.errorMsg().c_str());
        return -1;
    }

    printf("drop table girls ok.\n");

    return 0;
}