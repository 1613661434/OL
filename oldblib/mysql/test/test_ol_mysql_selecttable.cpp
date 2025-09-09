/*
 *  程序名：test_ol_mysql_selecttable.cpp，此程序演示开发框架操作MySQL数据库（查询表中的数据）。
 *  作者：ol
 */
#include "ol_mysql.h" // 开发框架操作MySQL的头文件。
#include <cstring>
#include <iostream>

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

    // 定义结构体存储数据
    struct st_girl
    {
        int id;         // 超女编号
        char name[31];  // 姓名
        double weight;  // 体重
        char btime[20]; // 时间
        char memo[301]; // 备注
    } stgirl;

    // 输入参数
    int minid = 11;
    int maxid = 13;

    // 准备查询SQL
    stmt.prepare("select id,name,weight,btime,memo from girls where id>=? and id<=?");

    // 绑定输入变量
    if (stmt.bindin(1, minid) != 0)
    {
        printf("bindin参数1失败：%s\n", stmt.message().c_str());
        return -1;
    }
    if (stmt.bindin(2, maxid) != 0)
    {
        printf("bindin参数2失败：%s\n", stmt.message().c_str());
        return -1;
    }

    // 绑定输出变量
    stmt.bindout(1, stgirl.id);
    stmt.bindout(2, stgirl.name, 30);
    stmt.bindout(3, stgirl.weight);
    stmt.bindout(4, stgirl.btime, 19);
    stmt.bindout(5, stgirl.memo, 300);

    // 调试信息
    printf("实际执行的SQL：%s\n", stmt.sql());
    printf("查询条件：id>=%d and id<=%d\n", minid, maxid);

    // 执行SQL
    if (stmt.execute() != 0)
    {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message().c_str());
        return -1;
    }

    // 遍历结果集
    int count = 0;
    while (true)
    {
        memset(&stgirl, 0, sizeof(stgirl));
        int next_ret = stmt.next();

        if (next_ret == 100)
        {
            printf("已获取全部记录\n");
            break;
        }
        if (next_ret != 0)
        {
            printf("stmt.next()失败：%s\n", stmt.message().c_str());
            break;
        }

        printf("记录%d: id=%ld,name=%s,weight=%.2f,btime=%s,memo=%s\n",
               ++count, stgirl.id, stgirl.name, stgirl.weight, stgirl.btime, stgirl.memo);
    }

    printf("本次查询了girls表%lu条记录。\n", stmt.rpc());

    return 0;
}