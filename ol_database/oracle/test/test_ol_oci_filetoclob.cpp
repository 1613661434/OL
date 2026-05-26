/*
 *  程序名：test_ol_oci_filetoclob.cpp，演示文本文件存入数据库CLOB字段
 *  作者：ol
 */
#include "ol_oci.h" // 开发框架操作Oracle的头文件。
#include <cstdio>
#include <string>

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

    // ===================== 检查记录是否存在 =====================
    auto stmtCheck = conn.createStmt();
    stmtCheck->prepare("select id from girls where id=1");
    long checkId = 0;
    stmtCheck->bindout(1, checkId);
    stmtCheck->execute();

    // 准备SQL语句（使用memo字段存储CLOB）
    auto stmt = conn.createStmt();
    if (stmtCheck->next() == 100) // OCI_NO_DATA — 记录不存在
    {
        // 先插入一条带空CLOB的记录（注意：不可用null代替empty_clob()）
        stmt->prepare("insert into girls(id,name,memo) values(1,'冰冰',empty_clob())");
        if (!stmt->execute())
        {
            printf("insert failed.\n%s\n%s\n", stmt->sql(), stmt->errorMsg().c_str());
            return -1;
        }
    }
    else
    {
        // 记录存在但memo可能为NULL（被其他测试程序插入的），先设为empty_clob
        stmt->prepare("update girls set memo=empty_clob() where id=1 and memo is null");
        stmt->execute();
    }

    // 使用游标从girls表中提取memo字段并锁定
    stmt->prepare("select memo from girls where id=1 for update");
    stmt->bindclob(1);

    // ===================== 执行查询 =====================
    if (!stmt->execute())
    {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt->sql(), stmt->errorMsg().c_str());
        return -1;
    }

    // 获取一条记录，0-成功，100(OCI_NO_DATA)-无记录
    if (stmt->next() != 0) return 0;

    // 调用filetoclob接口写入文本数据
    const string filename = "/home/mysql/OL/ol_database/oracle/test/data/memo_in.txt";
    if (stmt->filetoclob(1, filename) != 0)
    {
        printf("stmt.filetoclob() failed.\n%s\n", stmt->errorMsg().c_str());
        return -1;
    }

    printf("文本文件已成功存入memo字段(CLOB类型)\n");
    conn.commit();

    return 0;
}