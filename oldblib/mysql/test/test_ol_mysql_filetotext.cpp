/*
 *  程序名：test_ol_mysql_filetotext.cpp，演示文本文件存入数据库TEXT字段（分块传输）
 *  作者：ol
 */
#include "ol_mysql.h"
#include <cstdio>
#include <string>
#include <sys/stat.h>

using namespace std;
using namespace ol;

// 获取文件大小
long get_file_size(const string& filename)
{
#ifdef _WIN32
    struct _stat file_info;
    if (_stat(filename.c_str(), &file_info) != 0) return -1;
#else
    struct stat file_info;
    if (stat(filename.c_str(), &file_info) != 0) return -1;
#endif
    return file_info.st_size;
}

int main(int argc, char* argv[])
{
    DBConn conn;

    // 登录数据库
    if (conn.connecttodb("root:0088@127.0.0.1:3306/testdb", "utf8mb4") != 0)
    {
        printf("connect database failed.\n%s\n", conn.message().c_str());
        return -1;
    }

    printf("connect database ok.\n");

    // 检查max_allowed_packet
    DBStmt stmt_check(&conn);
    stmt_check.prepare("show variables like 'max_allowed_packet'");
    stmt_check.execute();
    char var[256], val[256];
    stmt_check.bindout(1, var, 255);
    stmt_check.bindout(2, val, 255);
    stmt_check.next();
    printf("当前max_allowed_packet: %s字节\n", val);

    // 准备SQL语句（使用memo字段）
    DBStmt stmt(&conn);

    // 检查记录是否存在，不存在则插入
    DBStmt stmt_check_rec(&conn);
    stmt_check_rec.prepare("select id from girls where id=1");
    stmt_check_rec.execute();
    if (stmt_check_rec.next() == 100)
    {
        // 记录不存在，准备插入语句
        stmt.prepare("insert into girls(id,name,memo) values(1,'冰冰',?)");
    }
    else
    {
        // 记录存在，准备更新语句
        stmt.prepare("update girls set memo=? where id=1");
    }

    // 检查文件
    const string filename = R"(D:\Visual Studio Code\VScode\OL\oldblib\mysql\test\data\memo_in.txt)";
    long file_size = get_file_size(filename);
    if (file_size <= 0)
    {
        printf("文件不存在或为空: %s\n", filename.c_str());
        return -1;
    }
    printf("待写入文件大小: %ld字节\n", file_size);

    // 关键：调用修复后的分块传输接口，指定分块大小（建议小于max_allowed_packet）
    unsigned int chunk_size = 4 * 1024 * 1024; // 4MB分块
    if (stmt.filetotext(1, filename, chunk_size) != 0)
    {
        printf("filetotext failed: %s\n", stmt.message().c_str());
        return -1;
    }

    // 执行SQL
    if (stmt.execute() != 0)
    {
        printf("execute failed: %s\nSQL: %s\n", stmt.message().c_str(), stmt.sql());
        return -1;
    }

    printf("文本文件已通过分块传输成功存入memo字段\n");
    conn.commit();

    return 0;
}