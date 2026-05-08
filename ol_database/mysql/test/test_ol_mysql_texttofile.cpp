/*
 *  程序名：test_ol_mysql_texttofile.cpp，此程序演示开发框架操作MySQL数据库（把数据库的TEXT字段提取到文件）。
 *  作者：ol
 */
#include "ol_mysql.h"
#include "ol_fstream.h"
#include <cstdio>
#include <direct.h>
#include <errno.h>
#include <string>
#include <sys/stat.h>
#include <cstring>

using namespace std;
using namespace ol;
using namespace ol::mysql;

int main(int argc, char* argv[])
{
    // 创建数据库连接类的对象
    DBConn conn;

    // ===================== 数据库连接 =====================
    conn.setConnectParam("root:0088@127.0.0.1:3306/testdb", "utf8mb4");
    if (!conn.connect())
    {
        printf("数据库连接失败: %s\n", conn.errorMsg().c_str());
        return -1;
    }
    printf("数据库连接成功\n");

    // ===================== 创建预处理语句 =====================
    auto stmt = conn.createStmt();

    // 准备查询语句，获取memo字段
    if (!stmt->prepare("select memo from girls where id=1"))
    {
        printf("SQL语句准备失败: %s\nSQL: %s\n", stmt->errorMsg().c_str(), stmt->sql());
        return -1;
    }

    // 绑定TEXT字段缓冲区
    const unsigned long TEXT_BUFFER_SIZE = 1024 * 1024; // 1MB缓冲区
    char text_buffer[TEXT_BUFFER_SIZE];
    if (stmt->bindtext(1, text_buffer, TEXT_BUFFER_SIZE) != 0)
    {
        printf("绑定TEXT缓冲区失败: %s\n", stmt->errorMsg().c_str());
        return -1;
    }

    // ===================== execute返回bool =====================
    if (!stmt->execute())
    {
        printf("执行SQL失败: %s\nSQL: %s\n", stmt->errorMsg().c_str(), stmt->sql());
        return -1;
    }

    // 获取查询结果
    int ret = stmt->next();
    if (ret == 100)
    {
        printf("没有找到id=1的记录\n");
        return 0;
    }
    else if (ret != 0)
    {
        printf("获取查询结果失败: %s\n", stmt->errorMsg().c_str());
        return -1;
    }

    const string filename = R"(D:\Visual Studio Code\Projects\OL\ol_database\mysql\test\data\memo_out.txt)";

    // 把TEXT字段内容写入磁盘文件
    if (stmt->texttofile(1, filename) != 0)
    {
        printf("导出TEXT到文件失败: %s\n", stmt->errorMsg().c_str());
        return -1;
    }

    // 验证文件是否生成成功
    long file_size = filesize(filename);
    if (file_size <= 0)
    {
        printf("警告: 导出的文件为空或未创建\n");
        return -1;
    }

    printf("成功将数据库TEXT字段(memo)提取到文件: %s\n", filename.c_str());
    printf("文件大小: %ld字节\n", file_size);

    return 0;
}