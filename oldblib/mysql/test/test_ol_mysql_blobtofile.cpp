/*
 *  程序名：test_ol_mysql_blobtofile.cpp
 *  功能：演示使用开发框架从MySQL数据库读取BLOB类型字段，并将其内容保存到文件
 *  作者：ol
 */
#include "ol_mysql.h" // 开发框架操作MySQL的头文件
#include <string>     // 包含string类支持

using namespace std;
using namespace ol;

int main(int argc, char* argv[])
{
    // 创建数据库连接对象
    connection conn;

    // 登录数据库，返回值：0-成功，其它-失败
    if (conn.connecttodb("root:0088@127.0.0.1:3306/testdb", "utf8mb4") != 0)
    {
        printf("数据库连接失败：%s\n", conn.message().c_str());
        return -1;
    }

    printf("数据库连接成功。\n");

    // 创建SQL语句对象，关联到数据库连接
    sqlstatement stmt(&conn);

    // 准备SQL语句：查询id=1的记录的pic字段（BLOB类型）
    if (stmt.prepare("select pic from girls where id=1") != 0)
    {
        printf("SQL语句准备失败：%s\n", stmt.message().c_str());
        return -1;
    }

    // 为BLOB字段分配缓冲区（根据实际需求调整大小）
    char blob_buffer[1024 * 1024];                   // 1MB缓冲区
    unsigned long blob_length = sizeof(blob_buffer); // 缓冲区长度

    // 绑定BLOB字段（按头文件声明传递3个参数：位置、缓冲区、长度）
    if (stmt.bindblob(1, blob_buffer, blob_length) != 0)
    {
        printf("绑定BLOB字段失败：%s\n", stmt.message().c_str());
        return -1;
    }

    // 执行SQL查询
    if (stmt.execute() != 0)
    {
        printf("SQL执行失败：%s\n", stmt.message().c_str());
        return -1;
    }

    // 获取查询结果集
    int ret = stmt.next();
    if (ret == 100)
    {
        printf("没有找到id=1的记录。\n");
        return 0;
    }
    else if (ret != 0)
    {
        printf("获取记录失败：%s\n", stmt.message().c_str());
        return -1;
    }

    // 将BLOB字段内容写入文件（添加字段位置参数1）
    string output_file = R"(D:\Visual Studio Code\VScode\OL\oldblib\mysql\test\data\pic_out.jpeg)";
    if (stmt.blobtofile(1, output_file) != 0)
    {
        printf("BLOB字段写入文件失败：%s\n", stmt.message().c_str());
        return -1;
    }

    printf("成功将数据库BLOB字段提取到文件：%s，大小：%lu字节\n",
           output_file.c_str(), blob_length);

    return 0;
}