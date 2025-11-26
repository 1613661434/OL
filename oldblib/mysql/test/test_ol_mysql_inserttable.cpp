/*
 *  程序名：test_ol_mysql_inserttable.cpp，此程序演示开发框架操作MySQL数据库（向表中插入数据）。
 *  作者：ol
 */
#include "ol_mysql.h" // 开发框架操作MySQL的头文件。
#include <cstdio>
#include <cstring>

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

    // 定义结构体存储数据
    struct st_girl
    {
        int id;         // 超女编号
        double weight;  // 超女体重
        char name[32];  // 超女姓名
        char btime[20]; // 报名时间
        char memo[301]; // 备注
    } stgirl;

    // 动态SQL语句（MySQL使用?作为占位符）
    stmt.prepare("insert into girls(id,name,weight,btime,memo) \
                                         values(?,?,?,?,?)");
    // 绑定参数（位置从0开始，与SQL中的?顺序一致）
    stmt.bindin(1, stgirl.id);        // 第1个?，位置0
    stmt.bindin(2, stgirl.name, 30);  // 第2个?，位置1
    stmt.bindin(3, stgirl.weight);    // 第3个?，位置2
    stmt.bindin(4, stgirl.btime, 19); // 第4个?，位置3
    stmt.bindin(5, stgirl.memo, 300); // 第5个?，位置4

    // 循环插入数据
    for (int i = 10; i < 15; ++i)
    {
        memset(&stgirl, 0, sizeof(struct st_girl));

        stgirl.id = i;                                                                 // 超女编号
        snprintf(stgirl.name, sizeof(stgirl.name), "西施%05dgirl", i);                 // 超女姓名
        stgirl.weight = 45.35 + i;                                                     // 超女体重
        snprintf(stgirl.btime, sizeof(stgirl.btime), "2021-08-25 10:33:%02d", i);      // 报名时间
        snprintf(stgirl.memo, sizeof(stgirl.memo), "这是第%05d个超级女生的备注。", i); // 备注（移除单引号避免SQL注入风险）

        if (stmt.execute() != 0)
        {
            printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message().c_str());
            return -1;
        }

        printf("成功插入了%ld条记录。\n", stmt.rpc());
    }

    conn.commit(); // 提交事务

    return 0;
}