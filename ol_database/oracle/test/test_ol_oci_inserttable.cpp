/*
 *  程序名：test_ol_oci_inserttable.cpp，此程序演示开发框架操作Oracle数据库（向表中插入数据）。
 *  作者：ol
 */
#include "ol_oci.h" // 开发框架操作Oracle的头文件。
#include <cstring>
#include <cstdio>

using namespace std;
using namespace ol::oracle;

int main(int argc, char* argv[])
{
    DBConn conn; // 创建数据库连接类的对象。

    // ===================== 数据库连接 =====================
    conn.setConnectParam("scott/000888@snorcl11g_5", "Simplified Chinese_China.AL32UTF8");
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
        long id;        // 超女编号
        double weight;  // 超女体重
        char name[32];  // 超女姓名
        char btime[20]; // 报名时间
        char memo[301]; // 备注
    } stgirl;

    // 动态SQL语句（Oracle使用:1,:2...,:n作为占位符）
    stmt->prepare("insert into girls(id,name,weight,btime,memo) \
                                         values(:1,:2,:3,to_date(:4,'yyyy-mm-dd hh24:mi:ss'),:5)");
    // 绑定参数（序号从1开始）
    stmt->bindin(1, stgirl.id);        // 第1个占位符 :1
    stmt->bindin(2, stgirl.name, 30);  // 第2个占位符 :2
    stmt->bindin(3, stgirl.weight);    // 第3个占位符 :3
    stmt->bindin(4, stgirl.btime, 19); // 第4个占位符 :4
    stmt->bindin(5, stgirl.memo, 300); // 第5个占位符 :5

    // 循环插入数据
    for (int i = 10; i < 15; ++i)
    {
        memset(&stgirl, 0, sizeof(struct st_girl));

        stgirl.id = i;                                                             // 超女编号
        snprintf(stgirl.name, sizeof(stgirl.name), "微微%05dgirl", i);             // 超女姓名
        stgirl.weight = 45.35 + i;                                                 // 超女体重
        snprintf(stgirl.btime, sizeof(stgirl.btime), "2021-08-25 10:33:%02d", i);  // 报名时间
        snprintf(stgirl.memo, sizeof(stgirl.memo), "这是第%05d个女生的备注。", i); // 备注

        // ===================== execute返回bool，错误信息用errorMsg =====================
        if (!stmt->execute())
        {
            printf("stmt.execute() failed.\n%s\n%s\n", stmt->sql(), stmt->errorMsg().c_str());
            return -1;
        }

        printf("成功插入了%zu条记录。\n", stmt->affectedRows());
    }

    conn.commit(); // 提交事务

    return 0;
}