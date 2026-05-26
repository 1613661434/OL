/*
 *  程序名：test_ol_oci_selecttable.cpp，此程序演示开发框架操作Oracle数据库（查询表中的数据）。
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
        char name[31];  // 姓名
        double weight;  // 体重
        char btime[20]; // 时间
        char memo[301]; // 备注
    } stgirl;

    // 输入参数
    long minid = 11;
    long maxid = 13;

    stmt->prepare("select id, name, weight, to_char(btime, 'yyyy-mm-dd hh24:mi:ss'), memo from girls where id>=:1 and id<=:2");

    // 绑定输入变量
    if (stmt->bindin(1, minid) != 0)
    {
        printf("bindin参数1失败：%s\n", stmt->errorMsg().c_str());
        return -1;
    }
    if (stmt->bindin(2, maxid) != 0)
    {
        printf("bindin参数2失败：%s\n", stmt->errorMsg().c_str());
        return -1;
    }

    // 绑定输出变量
    stmt->bindout(1, stgirl.id);
    stmt->bindout(2, stgirl.name, 30);
    stmt->bindout(3, stgirl.weight);
    stmt->bindout(4, stgirl.btime, 19);
    stmt->bindout(5, stgirl.memo, 300);

    // 调试信息
    printf("实际执行的SQL：%s\n", stmt->sql());
    printf("查询条件：id>=%ld and id<=%ld\n", minid, maxid);

    // ===================== execute返回bool =====================
    if (!stmt->execute())
    {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt->sql(), stmt->errorMsg().c_str());
        return -1;
    }

    // 遍历结果集
    int count = 0;
    while (true)
    {
        memset(&stgirl, 0, sizeof(stgirl));
        int next_ret = stmt->next();

        if (next_ret == 100) // OCI_NO_DATA
        {
            printf("已获取全部记录\n");
            break;
        }
        if (next_ret != 0)
        {
            printf("stmt.next()失败：%s\n", stmt->errorMsg().c_str());
            break;
        }

        printf("记录%d: id=%ld,name=%s,weight=%.2f,btime=%s,memo=%s\n",
               ++count, stgirl.id, stgirl.name, stgirl.weight, stgirl.btime, stgirl.memo);
    }

    printf("本次查询了girls表%zu条记录。\n", stmt->affectedRows());

    return 0;
}