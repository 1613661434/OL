/*
 *  程序名：test_ol_mysql_filetoblob.cpp，此程序演示开发框架操作MySQL数据库（把二进制文件存入数据库的BLOB字段中）。
 *  作者：ol
 */
#include "ol_mysql.h" // 开发框架操作MySQL的头文件。
#include <iostream>

using namespace std;
using namespace ol;

int main(int argc, char* argv[])
{
    connection conn; // 创建数据库连接类的对象。

    // 登录数据库（注意：连接字符串格式需符合框架要求："username:password@host:port/dbname"）
    if (conn.connecttodb("root:0088@127.0.0.1:3306/testdb", "utf8mb4") != 0)
    {
        printf("connect database failed.\n%s\n", conn.message().c_str());
        return -1;
    }

    printf("connect database ok.\n");

    sqlstatement stmt(&conn);
    // 插入记录，为BLOB字段预留位置
    stmt.prepare("insert into girls(id,name,pic) values(1,'冰冰冰',NULL)");
    if (stmt.execute() != 0)
    {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message().c_str());
        return -1;
    }

    // 准备更新BLOB字段（使用FOR UPDATE锁定记录）
    stmt.prepare("select pic from girls where id=1 for update");

    // 绑定BLOB字段（需要提供缓冲区和长度，根据实际需求调整大小）
    const unsigned long BLOB_BUFFER_SIZE = 1024 * 1024; // 1MB缓冲区
    char blob_buffer[BLOB_BUFFER_SIZE];
    if (stmt.bindblob(0, blob_buffer, BLOB_BUFFER_SIZE) != 0) // 注意：框架中位置从0开始
    {
        printf("stmt.bindblob() failed.\n%s\n", stmt.message().c_str());
        return -1;
    }

    // 执行查询获取记录
    if (stmt.execute() != 0)
    {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message().c_str());
        return -1;
    }

    // 获取记录（为BLOB字段准备写入）
    if (stmt.next() != 0)
    {
        printf("没有找到id=1的记录。\n");
        return 0;
    }

    // 把文件内容写入BLOB字段（框架的filetoblob只需要文件名参数）
    const string filename = R"(C:\test\data\data\pic_in.jpeg)";
    if (stmt.filetoblob(filename) != 0)
    {
        printf("stmt.filetoblob() failed.\n%s\n", stmt.message().c_str());
        return -1;
    }

    printf("二进制文件已成功存入数据库的BLOB字段中。\n");

    conn.commit(); // 提交事务

    return 0;
}