/*
 *  程序名：test_ol_mysql_filetoblob.cpp，演示二进制文件存入数据库BLOB字段
 *  作者：ol
 */
#include "ol_mysql.h"
#include "ol_fstream.h"
#include <cstdio>
#include <string>
#include <sys/stat.h>

using namespace std;
using namespace ol;
using namespace ol::mysql;

int main(int argc, char* argv[])
{
    DBConn conn;

    // ===================== 数据库连接 =====================
    conn.setConnectParam("root:0088@127.0.0.1:3306/testdb", "utf8mb4");
    if (!conn.connect())
    {
        printf("connect database failed.\n%s\n", conn.errorMsg().c_str());
        return -1;
    }

    printf("connect database ok.\n");

    // 检查max_allowed_packet
    // ===================== 创建预处理语句 =====================
    auto stmt_check = conn.createStmt();
    stmt_check->prepare("show variables like 'max_allowed_packet'");
    stmt_check->execute();
    char var[256], val[256];
    stmt_check->bindout(1, var, 255);
    stmt_check->bindout(2, val, 255);
    stmt_check->next();
    printf("当前max_allowed_packet: %s字节\n", val);

    // 检查记录是否存在，不存在则插入，存在则更新
    auto stmt_check_rec = conn.createStmt();
    stmt_check_rec->prepare("select id from girls where id=1");
    stmt_check_rec->execute();

    // 准备SQL语句（使用pic字段存储BLOB）
    auto stmt = conn.createStmt();
    if (stmt_check_rec->next() == 100)
    {
        // 记录不存在，准备插入语句
        stmt->prepare("insert into girls(id,name,pic) values(1,'微微',?)");
    }
    else
    {
        // 记录存在，准备更新语句
        stmt->prepare("update girls set pic=? where id=1");
    }

    // 检查文件
    const string filename = R"(D:\Visual Studio Code\Projects\OL\ol_database\mysql\test\data\pic_in.jpeg)";
    long file_size = filesize(filename);
    if (file_size <= 0)
    {
        printf("文件不存在或为空: %s\n", filename.c_str());
        return -1;
    }
    printf("待写入BLOB文件大小: %ld字节\n", file_size);

    // 调用filetoblob接口写入二进制数据
    if (stmt->filetoblob(1, filename) != 0)
    {
        printf("filetoblob failed: %s\n", stmt->errorMsg().c_str());
        return -1;
    }

    // 执行预处理语句
    if (!stmt->execute())
    {
        printf("execute failed: %s\nSQL: %s\n", stmt->errorMsg().c_str(), stmt->sql());
        return -1;
    }

    printf("二进制文件已成功存入pic字段(BLOB类型)\n");
    conn.commit();

    return 0;
}