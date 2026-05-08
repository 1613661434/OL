/*
 *  程序名：test_ol_mysql_updatetable.cpp，此程序演示开发框架操作MySQL数据库（修改表中的数据）。
 *  作者：ol
 */
#include "ol_mysql.h"
#include <cstdio>
#include <cstring>

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

    // 定义结构体
    struct st_girl
    {
        int id;         // 超女编号
        char name[31];  // 超女姓名
        double weight;  // 超女体重
        char btime[20]; // 报名时间
        char memo[301]; // 备注
    } stgirl;

    // 动态SQL语句（MySQL使用?作为占位符）
    stmt->prepare("update girls set name=?,weight=?,btime=? where id=?");

    // 绑定输入参数
    stmt->bindin(1, stgirl.name, 30);
    stmt->bindin(2, stgirl.weight);
    stmt->bindin(3, stgirl.btime, 19);
    stmt->bindin(4, stgirl.id);

    // 为变量赋值
    memset(&stgirl, 0, sizeof(struct st_girl));
    stgirl.id = 14;               // 超女编号
    sprintf(stgirl.name, "幂幂"); // 超女姓名
    stgirl.weight = 43.85;        // 超女体重
    strcpy(stgirl.btime, "2021-08-25 10:33:35");

    // ===================== execute返回bool =====================
    if (!stmt->execute())
    {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt->sql(), stmt->errorMsg().c_str());
        return -1;
    }

    printf("成功修改了%ld条记录。\n", stmt->affectedRows());

    conn.commit(); // 提交事务

    return 0;
}