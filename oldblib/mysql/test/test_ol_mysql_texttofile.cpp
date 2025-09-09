/*
 *  程序名：test_ol_mysql_texttofile.cpp，此程序演示开发框架操作MySQL数据库（把数据库的TEXT字段提取到文件）。
 *  作者：ol
 */
#include "ol_mysql.h" // 开发框架操作MySQL的头文件。
#include <cstdio>
#include <direct.h> // 用于创建目录
#include <errno.h>  // 用于错误处理
#include <string>
#include <sys/stat.h> // 用于获取文件大小

using namespace std;
using namespace ol;

// 获取文件大小的辅助函数
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
    // 创建数据库连接类的对象
    connection conn;

    // 登录数据库（连接字符串格式："username:password@host:port/dbname"）
    if (conn.connecttodb("root:0088@127.0.0.1:3306/testdb", "utf8mb4") != 0)
    {
        printf("数据库连接失败: %s\n", conn.message().c_str());
        return -1;
    }
    printf("数据库连接成功\n");

    // 创建SQL语句对象
    sqlstatement stmt(&conn);

    // 准备查询语句，获取memo字段（已修改字段名）
    if (stmt.prepare("select memo from girls where id=1") != 0)
    {
        printf("SQL语句准备失败: %s\nSQL: %s\n", stmt.message().c_str(), stmt.sql());
        return -1;
    }

    // 绑定TEXT字段缓冲区（关键步骤：必须在执行前绑定）
    const unsigned long TEXT_BUFFER_SIZE = 1024 * 1024;       // 1MB缓冲区，可根据需要调整
    char text_buffer[TEXT_BUFFER_SIZE];                       // 使用字符数组作为缓冲区更可靠
    if (stmt.bindtext(1, text_buffer, TEXT_BUFFER_SIZE) != 0) // 1对应第一个字段memo
    {
        printf("绑定TEXT缓冲区失败: %s\n", stmt.message().c_str());
        return -1;
    }

    // 执行SQL查询
    if (stmt.execute() != 0)
    {
        printf("执行SQL失败: %s\nSQL: %s\n", stmt.message().c_str(), stmt.sql());
        return -1;
    }

    // 获取查询结果
    int ret = stmt.next();
    if (ret == 100)
    {
        printf("没有找到id=1的记录\n");
        return 0;
    }
    else if (ret != 0)
    {
        printf("获取查询结果失败: %s\n", stmt.message().c_str());
        return -1;
    }

    // 验证目录是否存在，不存在则创建
    const string filename = R"(D:\Visual Studio Code\VScode\OL\oldblib\mysql\test\data\memo_out.txt)";
    size_t last_slash = filename.find_last_of("/\\");
    if (last_slash != string::npos)
    {
        string dir = filename.substr(0, last_slash);
#ifdef _WIN32
        if (_mkdir(dir.c_str()) == -1 && errno != EEXIST)
#else
        if (mkdir(dir.c_str(), 0755) == -1 && errno != EEXIST)
#endif
        {
            printf("创建目录失败: %s，错误信息: %s\n", dir.c_str(), strerror(errno));
            return -1;
        }
    }

    // 把TEXT字段内容写入磁盘文件
    if (stmt.texttofile(1, filename) != 0) // 1对应memo字段
    {
        printf("导出TEXT到文件失败: %s\n", stmt.message().c_str());
        return -1;
    }

    // 验证文件是否生成成功
    long file_size = get_file_size(filename);
    if (file_size <= 0)
    {
        printf("警告: 导出的文件为空或未创建\n");
        return -1;
    }

    printf("成功将数据库TEXT字段(memo)提取到文件: %s\n", filename.c_str());
    printf("文件大小: %ld字节\n", file_size);

    return 0;
}