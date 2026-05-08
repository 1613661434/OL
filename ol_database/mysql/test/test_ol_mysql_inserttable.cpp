/*
 *  程序名：test_ol_mysql_inserttable.cpp，此程序演示开发框架操作MySQL数据库（向表中插入数据）。
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
    stmt->prepare("insert into girls(id,name,weight,btime,memo) \
                                         values(?,?,?,?,?)");
    // 绑定参数（新版序号从1开始，与原代码一致，无需修改）
    stmt->bindin(1, stgirl.id);        // 第1个?
    stmt->bindin(2, stgirl.name, 30);  // 第2个?
    stmt->bindin(3, stgirl.weight);    // 第3个?
    stmt->bindin(4, stgirl.btime, 19); // 第4个?
    stmt->bindin(5, stgirl.memo, 300); // 第5个?

    // 循环插入数据
    for (int i = 10; i < 15; ++i)
    {
        memset(&stgirl, 0, sizeof(struct st_girl));

        stgirl.id = i;                                                                 // 超女编号
        snprintf(stgirl.name, sizeof(stgirl.name), "西施%05dgirl", i);                 // 超女姓名
        stgirl.weight = 45.35 + i;                                                     // 超女体重
        snprintf(stgirl.btime, sizeof(stgirl.btime), "2021-08-25 10:33:%02d", i);      // 报名时间
        snprintf(stgirl.memo, sizeof(stgirl.memo), "这是第%05d个超级女生的备注。", i); // 备注

        // ===================== execute返回bool，错误信息用errorMsg =====================
        if (!stmt->execute())
        {
            printf("stmt.execute() failed.\n%s\n%s\n", stmt->sql(), stmt->errorMsg().c_str());
            return -1;
        }

        printf("成功插入了%ld条记录。\n", stmt->affectedRows());
    }

    conn.commit(); // 提交事务

    return 0;
}