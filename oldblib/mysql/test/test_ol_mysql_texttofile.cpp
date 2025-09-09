/*
 *  程序名：test_ol_mysql_texttofile.cpp，此程序演示开发框架操作MySQL数据库（把数据库的TEXT字段提取到文件）。
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

    sqlstatement stmt(&conn);
    // 准备查询语句，获取memo1字段
    stmt.prepare("select memo1 from girls where id=1");

    // 绑定TEXT字段（提供缓冲区和长度）
    const unsigned long TEXT_BUFFER_SIZE = 1024 * 1024;       // 1MB缓冲区
    string text_buffer;                                       // 使用string作为文本缓冲区
    if (stmt.bindtext(1, text_buffer, TEXT_BUFFER_SIZE) != 0) // 1对应第一个字段memo1
    {
        printf("stmt.bindtext() failed.\n%s\n", stmt.message().c_str());
        return -1;
    }

    // 执行SQL语句
    if (stmt.execute() != 0)
    {
        printf("stmt.execute() failed.\n%s\n%s\n", stmt.sql(), stmt.message().c_str());
        return -1;
    }

    // 获取查询结果
    int ret = stmt.next();
    if (ret == 100)
    {
        printf("没有找到id=1的记录。\n");
        return 0;
    }
    else if (ret != 0)
    {
        printf("stmt.next() failed.\n%s\n", stmt.message().c_str());
        return -1;
    }

    // 把TEXT字段内容写入磁盘文件
    const string filename = R"(D:\Visual Studio Code\VScode\OL\oldblib\mysql\test\data\memo_out.txt)";
    if (stmt.texttofile(1, filename) != 0) // 1对应memo1字段
    {
        printf("stmt.texttofile() failed.\n%s\n", stmt.message().c_str());
        return -1;
    }

    printf("已把数据库的TEXT字段提取到文件。\n");

    return 0;
}