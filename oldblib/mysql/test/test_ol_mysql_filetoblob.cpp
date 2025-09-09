/*
 *  程序名：test_ol_mysql_filetoblob.cpp，此程序演示开发框架操作MySQL数据库（把二进制文件存入数据库的BLOB字段中）。
 *  作者：ol
 */
#include "ol_mysql.h" // 开发框架操作MySQL的头文件。
#include <cstdio>
#include <cstring>
#include <iostream>

using namespace std;
using namespace ol;

int main(int argc, char* argv[])
{
    connection conn; // 创建数据库连接类的对象。

    // 登录数据库
    if (conn.connecttodb("root:0088@127.0.0.1:3306/testdb", "utf8mb4") != 0)
    {
        printf("connect database failed.\n%s\n", conn.message().c_str());
        return -1;
    }

    printf("connect database ok.\n");

    // 步骤1：插入一条带BLOB字段的记录（初始为NULL）
    sqlstatement stmt_insert(&conn);
    stmt_insert.prepare("insert into girls(id,name,pic) values(1,'冰冰冰',NULL)");
    if (stmt_insert.execute() != 0)
    {
        printf("stmt_insert.execute() failed.\n%s\n%s\n", stmt_insert.sql(), stmt_insert.message().c_str());
        return -1;
    }
    printf("插入初始记录成功，影响行数：%ld\n", stmt_insert.rpc());

    // 步骤2：准备更新BLOB字段
    sqlstatement stmt_blob(&conn);
    // 使用UPDATE语句直接更新BLOB字段，这是更简洁的方式
    stmt_blob.prepare("update girls set pic=? where id=1");

    // 步骤3：将文件内容写入BLOB字段
    const string filename = R"(D:\Visual Studio Code\VScode\OL\oldblib\mysql\test\data\pic_in.jpeg)";
    // 检查文件是否存在
    FILE* fp = fopen(filename.c_str(), "rb");
    if (fp == nullptr)
    {
        printf("无法打开文件：%s\n", filename.c_str());
        return -1;
    }
    fclose(fp);

    // 调用框架方法将文件写入BLOB，注意添加了position参数（1表示第一个参数）
    if (stmt_blob.filetoblob(1, filename) != 0)
    {
        printf("stmt_blob.filetoblob() failed.\n%s\n", stmt_blob.message().c_str());
        return -1;
    }

    printf("二进制文件已成功存入数据库的BLOB字段中。\n");

    conn.commit(); // 提交事务

    return 0;
}