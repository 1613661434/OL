/*
 *  程序名：test_ol_mysql_selecttable.cpp，此程序演示开发框架操作MySQL数据库（查询表中的数据）。
 *  作者：ol
 */
#include "ol_mysql.h" // 开发框架操作MySQL的头文件。
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
        long id;        // 超女编号，对应MySQL的int（8字节，确保后续double对齐）
        double weight;  // 超女体重，对应MySQL的decimal(8,2)（8字节，紧跟long后自然对齐）
        char name[32];  // 超女姓名，对应MySQL的varchar(30)（32字节，8的倍数，避免对齐问题）
        char btime[20]; // 报名时间，对应MySQL的datetime
        char memo[301]; // 备注，对应MySQL的varchar(300)
    } stgirl;

    int minid = 11, maxid = 13;
    // 准备查询表的SQL语句（MySQL使用date_format处理日期）
    stmt.prepare("select id,name,weight,btime,memo from girls where id>= 11 and id<= 13");
    // 绑定输入变量
    // stmt.bindin(1, minid);
    // stmt.bindin(2, maxid);

    // // 准备查询表的SQL语句（MySQL使用date_format处理日期）
    // stmt.prepare("select id,name,weight,date_format(btime,'%Y-%m-%d %H:%i:%s'),memo from girls");
    // 绑定输出变量
    stmt.bindout(1, stgirl.id);
    stmt.bindout(2, stgirl.name, 30);
    stmt.bindout(3, stgirl.weight);
    stmt.bindout(4, stgirl.btime, 19);
    stmt.bindout(5, stgirl.memo, 300);

    // 执行SQL语句
    if (stmt.execute() != 0)
    {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message().c_str());
        return -1;
    }

    // 循环获取结果集
    while (true)
    {
        memset(&stgirl, 0, sizeof(stgirl));

        // 从结果集中获取一条记录，100-无记录
        if (stmt.next() != 0) break;

        // 打印获取到的记录
        printf("id=%ld,name=%s,weight=%.02f,btime=%s,memo=%s\n",
               stgirl.id, stgirl.name, stgirl.weight, stgirl.btime, stgirl.memo);
    }
    printf("本次查询了girls表%lu条记录。\n", stmt.rpc());

    return 0;
}