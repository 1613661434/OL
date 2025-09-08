/*
 *  程序名：test_ol_mysql_updatetable.cpp，此程序演示开发框架操作MySQL数据库（修改表中的数据）。
 *  作者：ol
 */
#include "ol_mysql.h" // 开发框架操作MySQL的头文件。

using namespace std;
using namespace ol;

int main(int argc, char* argv[])
{
    connection conn; // 创建数据库连接类的对象。

    // 登录数据库
    if (conn.connecttodb("root:0088@127.0.0.1:3306/testdb", "utf8mb4") != 0)
    {
        printf("connect database failed.\n%s\n", conn.message().c_str());
        return -1;
    }

    printf("connect database ok.\n");

    sqlstatement stmt(&conn);

    // 定义结构体
    struct st_girl
    {
        long id;        // 超女编号
        char name[31];  // 超女姓名
        double weight;  // 超女体重
        char btime[20]; // 报名时间
        char memo[301]; // 备注
    } stgirl;

    // 动态SQL语句（MySQL使用?作为占位符）
    stmt.prepare("update girls set name=?,weight=?,btime=? where id=?");
    stmt.bindin(1, stgirl.name, 30);
    stmt.bindin(2, stgirl.weight);
    stmt.bindin(3, stgirl.btime, 19);
    stmt.bindin(4, stgirl.id);

    // 为变量赋值
    memset(&stgirl, 0, sizeof(struct st_girl));
    stgirl.id = 11;               // 超女编号
    sprintf(stgirl.name, "幂幂"); // 超女姓名
    stgirl.weight = 43.85;        // 超女体重
    strcpy(stgirl.btime, "2021-08-25 10:33:35");

    // 执行SQL语句
    if (stmt.execute() != 0)
    {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message().c_str());
        return -1;
    }

    printf("成功修改了%ld条记录。\n", stmt.rpc());

    conn.commit(); // 提交事务

    return 0;
}