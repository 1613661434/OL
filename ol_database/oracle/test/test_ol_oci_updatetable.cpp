/*
 *  程序名：test_ol_oci_updatetable.cpp，此程序演示开发框架操作Oracle数据库（修改表中的数据）。
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

    // 定义结构体
    struct st_girl
    {
        long id;        // 超女编号
        char name[31];  // 超女姓名
        double weight;  // 超女体重
        char btime[20]; // 报名时间
        char memo[301]; // 备注
    } stgirl;

    // 动态SQL语句（Oracle使用:1,:2...,:n作为占位符）
    stmt->prepare("update girls set name=:1,weight=:2,btime=to_date(:3,'yyyy-mm-dd hh24:mi:ss'),memo=:4 where id=:5");

    // 绑定输入参数
    stmt->bindin(1, stgirl.name, 30);
    stmt->bindin(2, stgirl.weight);
    stmt->bindin(3, stgirl.btime, 19);
    stmt->bindin(4, stgirl.memo, 300);
    stmt->bindin(5, stgirl.id);

    // 为变量赋值
    memset(&stgirl, 0, sizeof(struct st_girl));
    stgirl.id = 14;             // 超女编号
    sprintf(stgirl.name, "VV"); // 超女姓名
    stgirl.weight = 43.85;      // 超女体重
    strcpy(stgirl.btime, "2021-08-25 10:33:35");
    sprintf(stgirl.memo, "这是第%05d个女生的备注。", 14);

    // ===================== execute返回bool =====================
    if (!stmt->execute())
    {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt->sql(), stmt->errorMsg().c_str());
        return -1;
    }

    printf("成功修改了%zu条记录。\n", stmt->affectedRows());

    conn.commit(); // 提交事务

    return 0;
}