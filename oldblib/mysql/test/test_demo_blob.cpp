#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DB_HOST "127.0.0.1"
#define DB_USER "root"
#define DB_PASS "0088"
#define DB_NAME "testdb"
#define DB_PORT 3306

void error_exit(MYSQL* conn, const char* msg)
{
    fprintf(stderr, "%s: %s\n", msg, mysql_error(conn));
    mysql_close(conn);
    exit(1);
}

// 将二进制数据转换为十六进制字符串（用于SQL插入）
char* bin_to_hex(const unsigned char* bin, int len)
{
    char* hex = (char*)malloc(len * 2 + 1);
    for (int i = 0; i < len; i++)
    {
        sprintf(hex + i * 2, "%02X", bin[i]);
    }
    return hex;
}

int main()
{
    MYSQL* conn = mysql_init(NULL);
    if (!conn)
    {
        fprintf(stderr, "mysql_init failed\n");
        return 1;
    }

    // 强制二进制字符集
    mysql_options(conn, MYSQL_SET_CHARSET_NAME, "binary");

    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS,
                            DB_NAME, DB_PORT, NULL, 0))
    {
        error_exit(conn, "连接失败");
    }
    printf("连接字符集: %s\n", mysql_character_set_name(conn));

    // 创建表
    if (mysql_query(conn, "CREATE TABLE IF NOT EXISTS test_hex ("
                          "id INT PRIMARY KEY AUTO_INCREMENT, "
                          "data LONGBLOB) ENGINE=InnoDB DEFAULT CHARSET=binary"))
    {
        error_exit(conn, "创建表失败");
    }

    // 测试数据（16字节）
    unsigned char test_data[16] = {
        0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46,
        0x49, 0x46, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01};
    printf("写入数据前16字节: ");
    for (int i = 0; i < 16; i++) printf("%02X ", test_data[i]);
    printf("\n");

    // 转换为十六进制字符串，用INSERT ... VALUES (0x...)语法
    char* hex_str = bin_to_hex(test_data, 16);
    char sql[1024];
    sprintf(sql, "INSERT INTO test_hex (data) VALUES (0x%s)", hex_str);
    free(hex_str);

    if (mysql_query(conn, sql) != 0)
    {
        error_exit(conn, "插入失败");
    }
    unsigned long long id = mysql_insert_id(conn);
    printf("写入成功，ID: %llu\n", id);

    // 读取数据
    sprintf(sql, "SELECT data FROM test_hex WHERE id = %llu", id);
    if (mysql_query(conn, sql) != 0)
    {
        error_exit(conn, "查询失败");
    }

    MYSQL_RES* result = mysql_store_result(conn);
    MYSQL_ROW row = mysql_fetch_row(result);
    unsigned long* lengths = mysql_fetch_lengths(result);

    if (row && lengths[0] == 16)
    {
        printf("读取数据前16字节: ");
        for (int i = 0; i < 16; i++)
        {
            printf("%02X ", (unsigned char)row[0][i]);
        }
        printf("\n");

        if (memcmp(test_data, row[0], 16) == 0)
        {
            printf("数据验证成功！\n");
        }
        else
        {
            printf("数据验证失败！\n");
        }
    }
    else
    {
        printf("读取数据失败！\n");
    }

    mysql_free_result(result);
    mysql_close(conn);
    return 0;
}