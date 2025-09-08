/*
 *  程序名：test_ol_mysql_filetotext.cpp，此程序演示开发框架操作MySQL数据库（把文本文件存入数据库表的TEXT字段中）。
 *  作者：ol
 */
#include "ol_mysql.h" // 开发框架操作MySQL的头文件。
#include <string>

using namespace std;
using namespace ol;

int main(int argc, char* argv[])
{
    connection conn; // 创建数据库连接类的对象。

    // 登录数据库（注意连接字符串格式："username:password@host:port/dbname"）
    if (conn.connecttodb("root:0088@127.0.0.1:3306/testdb", "utf8mb4") != 0)
    {
        printf("connect database failed.\n%s\n", conn.message().c_str());
        return -1;
    }

    printf("connect database ok.\n");

    // 确保表有text类型字段：alter table girls add memo1 text;
    sqlstatement stmt(&conn);
    // 插入记录，为TEXT字段预留位置
    stmt.prepare("insert into girls(id,name,memo1) values(1,'冰冰',NULL)");
    if (stmt.execute() != 0)
    {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message().c_str());
        return -1;
    }

    // 准备更新TEXT字段（使用FOR UPDATE锁定记录）
    stmt.prepare("select memo1 from girls where id=1 for update");

    // 绑定TEXT字段（位置从0开始，提供缓冲区和长度）
    const unsigned long TEXT_BUFFER_SIZE = 1024 * 1024;       // 1MB缓冲区
    string text_buffer;                                       // 使用string作为缓冲区
    if (stmt.bindtext(0, text_buffer, TEXT_BUFFER_SIZE) != 0) // 位置0对应第一个字段
    {
        printf("stmt.bindtext() failed.\n%s\n", stmt.message().c_str());
        return -1;
    }

    // 执行查询获取记录
    if (stmt.execute() != 0)
    {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message().c_str());
        return -1;
    }

    // 获取记录（为TEXT字段准备写入）
    if (stmt.next() != 0)
    {
        printf("没有找到id=1的记录。\n");
        return 0;
    }

    // 把文件内容写入TEXT字段（参数：位置和文件名）
    const string filename = R"(C:\test\data\memo_in.txt)";
    if (stmt.filetotext(0, filename) != 0) // 位置0对应memo1字段
    {
        printf("stmt.filetotext() failed.\n%s\n", stmt.message().c_str());
        return -1;
    }

    printf("文本文件已成功存入数据库的TEXT字段中。\n");

    conn.commit(); // 提交事务

    return 0;
}