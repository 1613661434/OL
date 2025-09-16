/*
 *  程序名：test_ol_mysql_blobtofile.cpp
 *  功能：演示使用开发框架从MySQL数据库读取BLOB类型字段，并将其内容保存到文件
 *  作者：ol
 */
#include "ol_mysql.h" // 开发框架操作MySQL的头文件
#include <fstream>    // 用于检查文件是否存在
#include <string>
#include <sys/stat.h> // 用于获取文件大小

using namespace std;
using namespace ol;

// 辅助函数：检查文件是否存在
bool file_exists(const string& path)
{
#ifdef _WIN32
    struct _stat info;
    return _stat(path.c_str(), &info) == 0;
#else
    struct stat info;
    return stat(path.c_str(), &info) == 0;
#endif
}

// 辅助函数：获取文件大小
unsigned long file_size(const string& path)
{
#ifdef _WIN32
    struct _stat info;
    if (_stat(path.c_str(), &info) != 0) return 0;
#else
    struct stat info;
    if (stat(path.c_str(), &info) != 0) return 0;
#endif
    return (unsigned long)info.st_size;
}

int main(int argc, char* argv[])
{
    // 创建数据库连接对象
    DBConn conn;

    // 登录数据库
    if (conn.connecttodb("root:0088@127.0.0.1:3306/testdb", "utf8mb4") != 0)
    {
        printf("数据库连接失败：%s\n", conn.message().c_str());
        return -1;
    }

    printf("数据库连接成功。\n");

    // 创建SQL语句对象
    DBStmt stmt(&conn);

    // 准备查询语句
    if (stmt.prepare("select pic from girls where id=1") != 0)
    {
        printf("SQL语句准备失败：%s\n", stmt.message().c_str());
        return -1;
    }

    // 为BLOB字段分配缓冲区
    char blob_buffer[1024 * 1024];                        // 1MB缓冲区
    unsigned long blob_buffer_size = sizeof(blob_buffer); // 缓冲区大小

    // 绑定BLOB字段（输出参数）
    if (stmt.bindblob(1, blob_buffer, blob_buffer_size) != 0)
    {
        printf("绑定BLOB字段失败：%s\n", stmt.message().c_str());
        return -1;
    }

    // 执行查询
    if (stmt.execute() != 0)
    {
        printf("SQL执行失败：%s\n", stmt.message().c_str());
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
        printf("获取记录失败：%s\n", stmt.message().c_str());
        return -1;
    }

    // 将BLOB字段内容写入文件
    string output_file = R"(D:\Visual Studio Code\VScode\OL\oldblib\mysql\test\data\pic_out.jpeg)";
    if (stmt.blobtofile(1, output_file) != 0)
    {
        printf("BLOB字段写入文件失败：%s\n", stmt.message().c_str());
        return -1;
    }

    // 验证文件是否存在并输出实际信息（关键：确认文件状态）
    if (file_exists(output_file))
    {
        unsigned long file_size_val = file_size(output_file);
        printf("文件生成成功！\n");
        printf("路径：%s\n", output_file.c_str());
        printf("实际大小：%lu字节\n", file_size_val);
    }
    else
    {
        printf("警告：程序提示成功，但文件不存在！\n");
        printf("预期路径：%s\n", output_file.c_str());
        return -1;
    }

    return 0;
}