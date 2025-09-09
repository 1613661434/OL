/**************************************************************************************/
/*
 * 程序名：ol_mysql.h
 * 功能描述：开发框架中用于C++操作MySQL数据库的接口声明，提供：
 *          - 数据库连接管理（connection类）：登录、断开、事务提交/回滚等
 *          - SQL语句执行（sqlstatement类）：SQL准备、绑定变量、执行、结果集处理等
 *          - 支持普通数据类型及BLOB字段操作
 * 作者：ol
 * 依赖：MySQL C API库（需包含mysql.h及链接mysqlclient库）
 * 适用标准：C++11及以上
 */
/**************************************************************************************/

#ifndef __OL_MYSQL_H
#define __OL_MYSQL_H 1

// C/C++库常用头文件
#include <direct.h> // 目录操作
#include <mutex>
#include <mysql.h> // MySQL C API的头文件
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

namespace ol
{

    // 数据库操作结果结构体，存储执行状态及错误信息
    struct CDA_DEF
    {
        int rc;              // 返回码：0-成功，其他-失败
        unsigned long rpc;   // 影响行数：DML语句（insert/update/delete）为影响记录数，查询为结果集行数
        std::string message; // 错误描述信息（失败时有效）

        void init();
    };

    class connection;
    class sqlstatement;

    // MySQL数据库连接类，管理数据库连接及事务
    class connection
    {
        friend class sqlstatement;

    private:
        MYSQL* m_mysql;      // MySQL连接句柄
        int m_autocommitopt; // 自动提交标志，0-关闭；1-开启

        void character(const char* charset); // 设置字符集（防止乱码）
        void setdbopt(const char* connstr);  // 从connstr中解析username、password和dbname等信息
        void err_report();                   // 获取错误信息

        connection(const connection&) = delete;            // 禁用拷贝构造函数
        connection& operator=(const connection&) = delete; // 禁用赋值函数

        // 数据库连接状态：connected-已连接；disconnected-未连接
        enum
        {
            connected,
            disconnected
        };
        int m_state;

        CDA_DEF m_cda; // 数据库操作的结果或最后一次执行SQL语句的结果

        // 连接参数
        std::string m_host;
        std::string m_user;
        std::string m_pass;
        std::string m_dbname;
        unsigned int m_port;
        std::string m_unix_socket;

    public:
        // 构造函数，初始化成员变量
        connection();
        // 析构函数，自动断开连接
        ~connection();

        /**
         * @brief 登录数据库
         * @param connstr 连接字符串，格式："username:password@host:port/dbname"
         * @param charset 客户端字符集（需与数据库一致，避免乱码）
         * @param autocommitopt 自动提交开关（默认false-关闭）
         * @return 0-成功，其他-失败（失败的代码在m_cda.rc中，失败的描述在m_cda.message中）
         */
        int connecttodb(const std::string& connstr, const std::string& charset, bool autocommitopt = false);

        /**
         * @brief 判断数据库是否已连接
         * @return 已连接返回true，否则false
         */
        bool isopen();

        /**
         * @brief 提交事务
         * @return 0-成功，其他-失败（程序中一般不必关心返回值）
         */
        int commit();

        /**
         * @brief 回滚事务
         * @return 0-成功，其他-失败（程序中一般不必关心返回值）
         */
        int rollback();

        /**
         * @brief 断开数据库连接（未提交事务自动回滚）
         * @return 0-成功，其他-失败（程序中一般不必关心返回值）
         */
        int disconnect();

        /**
         * @brief 执行静态SQL语句（无绑定变量的非查询语句）
         * @param fmt 格式化SQL字符串（支持可变参数，同printf）
         * @return 0-成功，其他-失败（失败的代码在m_cda.rc中，失败的描述在m_cda.message中）
         * @note 如果成功的执行了非查询语句，在m_cda.rpc中保存了本次执行SQL影响记录的行数。
         *       程序中必须检查execute方法的返回值。
         *       在connection类中提供了execute方法，是为了方便程序员，在该方法中，也是用sqlstatement类来完成功能。
         */
        int execute(const char* fmt, ...);

        /**
         * @brief 获取错误代码
         * @return 0-成功，其它值-失败
         */
        int rc();

        /**
         * @brief 获取影响数据的行数
         * @return 对于insert/update/delete返回影响记录数，对于select返回结果集行数
         */
        unsigned long rpc();

        /**
         * @brief 获取错误描述信息
         * @return 错误描述字符串（失败时有效，成功时为空）
         */
        std::string message();
    };

    // SQL语句操作类，处理SQL准备、绑定变量、执行及结果集
    class sqlstatement
    {
    private:
        MYSQL* m_mysql;                // MySQL连接句柄（引用自connection）
        MYSQL_STMT* m_stmt;            // MySQL语句句柄
        MYSQL_RES* m_result;           // 结果集句柄
        MYSQL_BIND* m_bindin;          // 绑定输入变量数组
        MYSQL_BIND* m_bindout;         // 绑定输出变量数组
        unsigned int m_param_count;    // 输入参数数量
        unsigned int m_field_count;    // 输出字段数量
        unsigned long* m_out_lengths;  // 输出字段的实际长度
        bool* m_out_is_null;           // 输出字段是否为NULL
        unsigned long* m_blob_lengths; // BLOB字段长度存储

        connection* m_conn;   // 数据库连接指针
        bool m_sqltype;       // SQL语句的类型，false-查询语句；true-非查询语句
        bool m_autocommitopt; // 自动提交标志，false-关闭；true-开启
        void err_report();    // 错误报告

        sqlstatement(const sqlstatement&) = delete;            // 禁用拷贝构造函数
        sqlstatement& operator=(const sqlstatement&) = delete; // 禁用赋值函数

        // 与数据库连接的关联状态，connected-已关联；disconnect-未关联
        enum
        {
            connected,
            disconnected
        };
        int m_state;

        std::string m_sql; // SQL语句的文本
        CDA_DEF m_cda;     // 执行SQL语句的结果

        // 释放绑定资源
        void free_bind();

    public:
        // 构造函数，初始化成员变量
        sqlstatement();

        // 构造函数，同时关联数据库连接
        sqlstatement(connection* conn);

        // 析构函数，释放资源
        ~sqlstatement();

        /**
         * @brief 关联数据库连接
         * @param conn 数据库连接对象指针
         * @return 0-成功，其他-失败（只要conn参数是有效的，并且数据库的游标资源足够，connect方法不会返回失败）
         * @note 程序中一般不必关心connect方法的返回值
         *       每个sqlstatement只需要指定一次，在指定新的connection前，必须先显式的调用disconnect方法
         */
        int connect(connection* conn);

        /**
         * @brief 判断是否已关联数据库连接
         * @return 已关联返回true，否则false
         */
        bool isopen();

        /**
         * @brief 解除与数据库连接的关联
         * @return 0-成功，其他-失败（程序中一般不必关心返回值）
         */
        int disconnect();

        /**
         * @brief 准备SQL语句（支持格式化字符串）
         * @param strsql SQL字符串
         * @return 0-成功，其他-失败（程序中一般不必关心返回值）
         * @note 如果SQL语句没有改变，只需要prepare一次就可以了
         */
        int prepare(const std::string& strsql);

        /**
         * @brief 准备SQL语句（支持可变参数，同printf）
         * @param fmt 格式化SQL字符串
         * @return 0-成功，其他-失败（程序中一般不必关心返回值）
         * @note 如果SQL语句没有改变，只需要prepare一次就可以了
         */
        int prepare(const char* fmt, ...);

        /**
         * @brief 绑定输入变量（将SQL中的占位符与变量关联）
         * @param position 占位符位置（从1开始，必须与prepare方法中的SQL的序号一一对应）
         * @param value 输入变量地址（如果是字符串，内存大小应该是表对应的字段长度加1）
         * @param len 字符串类型的长度（不含终止符，默认2000）
         * @return 0-成功，其他-失败（程序中一般不必关心返回值）
         * @note 1）如果SQL语句没有改变，只需要bindin一次就可以了；
         *       2）如果value的类型是std::string，bindin()函数中会resize(len)；
         *       3）如果value的类型是std::string，那么，在用户的程序代码中，不可改变它内部buffer的地址。
         *       4）如果value的类型是std::string或char，那么传入len的参数不包括最后的NULL。
         */
        int bindin(const unsigned int position, int& value);                                  // 绑定int类型
        int bindin(const unsigned int position, long& value);                                 // 绑定long类型
        int bindin(const unsigned int position, unsigned int& value);                         // 绑定unsigned int类型
        int bindin(const unsigned int position, unsigned long& value);                        // 绑定unsigned long类型
        int bindin(const unsigned int position, float& value);                                // 绑定float类型
        int bindin(const unsigned int position, double& value);                               // 绑定double类型
        int bindin(const unsigned int position, char* value, unsigned int len = 2000);        // 绑定char*类型
        int bindin(const unsigned int position, std::string& value, unsigned int len = 2000); // 绑定std::string类型
        int bindin1(const unsigned int position, std::string& value);                         // 绑定std::string（不分配内存）

        /**
         * @brief 绑定输出变量（将查询结果字段与变量关联）
         * @param position 结果集字段位置（从1开始，与SQL的结果集一一对应）
         * @param value 输出变量地址（如果是字符串，内存大小应该是表对应的字段长度加1）
         * @param len 字符串类型的最大长度（不含终止符，默认2000）
         * @return 0-成功，其他-失败（程序中一般不必关心返回值）
         * @note 1）如果SQL语句没有改变，只需要bindout一次就可以了；
         *       2）如果value的类型是std::string，那么将在内容后面填充0，直到len的大小，value.size()永远是len。
         *       3）如果value的类型是std::string，那么，在用户的程序代码中，不可改变它内部buffer的地址。
         *       4）如果value的类型是std::string或char，那么传入len的参数不包括最后的NULL。
         */
        int bindout(const unsigned int position, int& value);                                  // 绑定int类型
        int bindout(const unsigned int position, long& value);                                 // 绑定long类型
        int bindout(const unsigned int position, unsigned int& value);                         // 绑定unsigned int类型
        int bindout(const unsigned int position, unsigned long& value);                        // 绑定unsigned long类型
        int bindout(const unsigned int position, float& value);                                // 绑定float类型
        int bindout(const unsigned int position, double& value);                               // 绑定double类型
        int bindout(const unsigned int position, char* value, unsigned int len = 2000);        // 绑定char*类型
        int bindout(const unsigned int position, std::string& value, unsigned int len = 2000); // 绑定std::string类型

        /**
         * @brief 执行已准备的（静态或动态）SQL语句
         * @return 0-成功，其他-失败（失败的代码在m_cda.rc中，失败的描述在m_cda.message中）
         * @note 如果成功的执行了非查询语句，在m_cda.rpc中保存了本次执行SQL影响记录的行数。
         *       程序中必须检查execute方法的返回值。
         */
        int execute();

        /**
         * @brief 直接执行静态SQL语句（无绑定变量，非查询语句）
         * @param fmt 格式化SQL字符串（支持可变参数，同printf）
         * @return 0-成功，其他-失败（失败的代码在m_cda.rc中，失败的描述在m_cda.message中）
         * @note 如果成功的执行了非查询语句，在m_cda.rpc中保存了本次执行SQL影响记录的行数。
         *       程序中必须检查execute方法的返回值。
         */
        int execute(const char* fmt, ...);

        /**
         * @brief 从结果集中获取下一条记录（仅查询语句有效）
         * @return 0-成功，100-无更多记录（也可以使用宏MYSQL_NO_DATA），其他-失败（失败的代码在m_cda.rc中，失败的描述在m_cda.message中）
         * @note 返回失败的原因主要有两个：1）与数据库的连接已断开；2）绑定输出变量的内存太小。
         *       每执行一次next方法，m_cda.rpc的值加1。
         *       程序中必须检查next方法的返回值。
         */
        int next();

        /**
         * @brief 绑定BLOB字段（用于插入/读取二进制大对象）
         * @param position 占位符位置（从1开始）
         * @param buffer 数据缓冲区
         * @param length 数据长度
         * @return 0-成功，其他-失败（程序中一般不必关心返回值）
         */
        int bindblob(const unsigned int position, char* buffer, unsigned long length);

        /**
         * @brief 将文件内容导入到BLOB字段
         * @param position 占位符位置
         * @param filename 待导入文件的路径（建议绝对路径）
         * @return 0-成功，其他-失败（程序中一般不必关心返回值）
         */
        int filetoblob(const unsigned int position, const std::string& filename);

        /**
         * @brief 将BLOB字段内容导出到文件
         * @param position 结果集字段位置
         * @param filename 导出文件的路径（建议绝对路径）
         * @return 0-成功，其他-失败（程序中一般不必关心返回值）
         */
        int blobtofile(const unsigned int position, const std::string& filename);

        /**
         * @brief 绑定TEXT字段（用于插入/读取大文本）
         * @param position 占位符位置（从1开始）
         * @param buffer 文本缓冲区
         * @param length 文本长度
         * @return 0-成功，其他-失败
         */
        int bindtext(const unsigned int position, char* buffer, unsigned long length);
        int bindtext(const unsigned int position, std::string& buffer, unsigned long length);

        /**
         * @brief 将文件内容导入到TEXT字段（分块传输版本）
         * @param position 占位符位置
         * @param filename 待导入文件的路径
         * @param chunk_size 分块大小（默认4MB，建议小于max_allowed_packet）
         * @return 0-成功，其他-失败
         */
        int filetotext(const unsigned int position, const std::string& filename, unsigned int chunk_size = 4 * 1024 * 1024);

        /**
         * @brief 将TEXT字段内容导出到文件
         * @param position 结果集字段位置
         * @param filename 导出文件的路径
         * @return 0-成功，其他-失败
         */
        int texttofile(const unsigned int position, const std::string& filename);

        /**
         * @brief 获取SQL语句的文本
         * @return SQL语句字符串的常量指针
         */
        const char* sql();

        /**
         * @brief 获取错误代码（Return Code）
         * @return 0-成功，其它值-失败
         */
        int rc();

        /**
         * @brief 获取影响数据的行数（Rows Processed Count）
         * @return 对于insert/update/delete返回影响记录数，对于select返回结果集行数
         */
        unsigned long rpc();

        /**
         * @brief 获取错误描述信息
         * @return 错误描述字符串（失败时有效）
         */
        std::string message();

        /**
         * @brief 判断绑定的输出字段是否为NULL
         * @param position 结果集字段位置（从1开始）
         * @return 字段为NULL返回true，否则返回false
         * @note 必须在调用next()之后使用，且该字段必须已通过bindout绑定
         */
        bool is_null(const unsigned int position);

        /**
         * @brief 获取绑定的输出字段的实际长度
         * @param position 结果集字段位置（从1开始）
         * @return 字段的实际字节长度
         * @note 1. 对于字符串类型，返回实际字符数（不含终止符）
         *       2. 对于数值类型，返回其在数据库中的存储长度
         *       3. 必须在调用next()之后使用，且该字段必须已通过bindout绑定
         */
        unsigned long length(const unsigned int position);
    };

} // namespace ol

#endif // !__OL_MYSQL_H