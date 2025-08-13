/**************************************************************************************/
/*
 * 程序名：ol_oci.h
 * 功能描述：开发框架中用于C++操作Oracle数据库的接口声明，提供：
 *          - 数据库连接管理（connection类）：登录、断开、事务提交/回滚等
 *          - SQL语句执行（sqlstatement类）：SQL准备、绑定变量、执行、结果集处理等
 *          - 支持普通数据类型及LOB（CLOB/BLOB）字段操作
 * 作者：ol
 * 依赖：Oracle OCI库（需包含oci.h及链接OCI库）
 * 适用标准：C++11及以上
 */
/**************************************************************************************/

#ifndef __OL_OCI_H
#define __OL_OCI_H

// C/C++库常用头文件
#include <mutex>
#include <oci.h> // OCI的头文件。
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

namespace ol
{

    // OCI登录环境结构体，存储数据库登录信息及环境句柄
    struct LOGINENV
    {
        char user[31];    // 数据库用户名（最大30字节）
        char pass[31];    // 数据库密码（最大30字节）
        char tnsname[51]; // 数据库的tnsname（最大50字节，在ORACLE_HOME/network/admin/tnsnames.ora中配置）
        OCIEnv* envhp;    // 环境变量的句柄。
    };

    // OCI上下文句柄结构体，存储服务器上下文及错误句柄
    struct OCI_CXT
    {
        OCISvcCtx* svchp; // 服务器上下文句柄
        OCIError* errhp;  // 错误句柄
        OCIEnv* envhp;    // 环境句柄
    };

    // OCI SQL句柄结构体，存储SQL执行相关句柄
    struct OCI_HANDLE
    {
        OCISvcCtx* svchp; // 服务器上下文句柄（引用自OCI_CXT）
        OCIStmt* smthp;   // SQL语句句柄
        OCIBind* bindhp;  // 绑定输入变量句柄
        OCIDefine* defhp; // 定义输出变量句柄
        OCIError* errhp;  // 错误句柄（引用自OCI_CXT）
        OCIEnv* envhp;    // 环境句柄
    };

    // OCI操作结果结构体，存储执行状态及错误信息
    struct CDA_DEF
    {
        int rc;             // 返回码：0-成功，其他-失败
        unsigned long rpc;  // 影响行数：DML语句（insert/update/delete）为影响记录数，查询为结果集行数
        char message[2048]; // 错误描述信息（失败时有效）
    };

    // OCI底层初始化与释放函数（内部使用）
    int oci_init(LOGINENV* env);
    int oci_close(LOGINENV* env);
    int oci_context_create(LOGINENV* env, OCI_CXT* cxt);
    int oci_context_close(OCI_CXT* cxt);
    int oci_stmt_create(OCI_CXT* cxt, OCI_HANDLE* handle);
    int oci_stmt_close(OCI_HANDLE* handle);

    class connection;
    class sqlstatement;

    // Oracle数据库连接类，管理数据库连接及事务
    class connection
    {
        friend class sqlstatement;

    private:
        LOGINENV m_env;      // 服务器环境句柄。
        OCI_CXT m_cxt;       // 服务器上下文。
        int m_autocommitopt; // 自动提交标志，0-关闭；1-开启。

        void character(const char* charset); // 设置字符集（防止乱码）
        void setdbopt(const char* connstr);  // 从connstr中解析username、password和tnsname。
        void err_report();                   // 获取错误信息。

        connection(const connection&) = delete;            // 禁用拷贝构造函数。
        connection& operator=(const connection&) = delete; // 禁用赋值函数。

        // 数据库连接状态：connected-已连接；disconnected-未连接。
        enum
        {
            connected,
            disconnected
        };
        int m_state;

        CDA_DEF m_cda; // 数据库操作的结果或最后一次执行SQL语句的结果。

    public:
        // 构造函数，初始化成员变量
        connection();
        // 析构函数，自动断开连接
        ~connection();

        /**
         * 登录数据库
         * @param connstr 连接字符串，格式："username/password@tnsname"
         * @param charset 客户端字符集（需与数据库一致，避免乱码）
         * @param autocommitopt 自动提交开关（默认false-关闭）
         * @return 0-成功，其他-失败（失败的代码在m_cda.rc中，失败的描述在m_cda.message中）
         * @note tnsname-数据库的服务名，在$ORACLE_HOME/network/admin/tnsnames.ora文件中配置。
         */
        int connecttodb(const std::string& connstr, const std::string& charset, bool autocommitopt = false);

        /**
         * 判断数据库是否已连接
         * @return 已连接返回true，否则false
         */
        bool isopen();

        /**
         * 提交事务
         * @return 0-成功，其他-失败（程序中一般不必关心返回值）
         */
        int commit();

        /**
         * 回滚事务
         * @return 0-成功，其他-失败（程序中一般不必关心返回值）
         */
        int rollback();

        /**
         * 断开数据库连接（未提交事务自动回滚）
         * @return 0-成功，其他-失败（程序中一般不必关心返回值）
         */
        int disconnect();

        /**
         * 执行静态SQL语句（无绑定变量的非查询语句）
         * @param fmt 格式化SQL字符串（支持可变参数，同printf）
         * @return 0-成功，其他-失败（失败的代码在m_cda.rc中，失败的描述在m_cda.message中）
         * @note 如果成功的执行了非查询语句，在m_cda.rpc中保存了本次执行SQL影响记录的行数。
         *       程序中必须检查execute方法的返回值。
         *       在connection类中提供了execute方法，是为了方便程序员，在该方法中，也是用sqlstatement类来完成功能。
         */
        int execute(const char* fmt, ...);

        /**
         * 获取错误代码
         * @return 0-成功，其它值-失败
         */
        int rc()
        {
            return m_cda.rc;
        }

        /**
         * 获取影响数据的行数
         * @return 对于insert/update/delete返回影响记录数，对于select返回结果集行数
         */
        unsigned long rpc()
        {
            return m_cda.rpc;
        }

        /**
         * 获取错误描述信息
         * @return 错误描述字符串（失败时有效，成功时为空）
         */
        const char* message()
        {
            return m_cda.message;
        }
    };

    // SQL语句操作类，处理SQL准备、绑定变量、执行及结果集
    class sqlstatement
    {
    private:
        OCI_HANDLE m_handle; // SQL句柄。

        connection* m_conn;   // 数据库连接指针。
        bool m_sqltype;       // SQL语句的类型，false-查询语句；true-非查询语句。
        bool m_autocommitopt; // 自动提交标志，false-关闭；true-开启。
        void err_report();    // 错误报告。

        OCILobLocator* m_lob;    // 指向LOB字段的指针。
        int alloclob();          // 初始化lob指针。
        int filetolob(FILE* fp); // 把文件的内容导入到clob和blob字段中。
        int lobtofile(FILE* fp); // 从clob和blob字段中导出内容到文件中。
        void freelob();          // 释放lob指针。

        sqlstatement(const sqlstatement&) = delete;            // 禁用拷贝构造函数。
        sqlstatement& operator=(const sqlstatement&) = delete; // 禁用赋值函数。

        // 与数据库连接的关联状态，connected-已关联；disconnect-未关联。
        enum
        {
            connected,
            disconnected
        };
        int m_state;

        std::string m_sql; // SQL语句的文本。
        CDA_DEF m_cda;     // 执行SQL语句的结果。

    public:
        // 构造函数，初始化成员变量
        sqlstatement();

        // 构造函数，同时关联数据库连接
        sqlstatement(connection* conn);

        // 析构函数，释放资源
        ~sqlstatement();

        /**
         * 关联数据库连接
         * @param conn 数据库连接对象指针
         * @return 0-成功，其他-失败（只要conn参数是有效的，并且数据库的游标资源足够，connect方法不会返回失败）
         * @note 程序中一般不必关心connect方法的返回值
         *       每个sqlstatement只需要指定一次，在指定新的connection前，必须先显式的调用disconnect方法
         */
        int connect(connection* conn);

        /**
         * 判断是否已关联数据库连接
         * @return 已关联返回true，否则false
         */
        bool isopen();

        /**
         * 解除与数据库连接的关联
         * @return 0-成功，其他-失败（程序中一般不必关心返回值）
         */
        int disconnect();

        /**
         * 准备SQL语句（支持格式化字符串）
         * @param strsql SQL字符串
         * @return 0-成功，其他-失败（程序中一般不必关心返回值）
         * @note 如果SQL语句没有改变，只需要prepare一次就可以了
         */
        int prepare(const std::string& strsql)
        {
            return prepare(strsql.c_str());
        }

        /**
         * 准备SQL语句（支持可变参数，同printf）
         * @param fmt 格式化SQL字符串
         * @return 0-成功，其他-失败（程序中一般不必关心返回值）
         * @note 如果SQL语句没有改变，只需要prepare一次就可以了
         */
        int prepare(const char* fmt, ...);

        /**
         * 绑定输入变量（将SQL中的占位符与变量关联）
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
         * 绑定输出变量（将查询结果字段与变量关联）
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
         * 执行已准备的（静态或动态）SQL语句
         * @return 0-成功，其他-失败（失败的代码在m_cda.rc中，失败的描述在m_cda.message中）
         * @note 如果成功的执行了非查询语句，在m_cda.rpc中保存了本次执行SQL影响记录的行数。
         *       程序中必须检查execute方法的返回值。
         */
        int execute();

        /**
         * 直接执行静态SQL语句（无绑定变量，非查询语句）
         * @param fmt 格式化SQL字符串（支持可变参数，同printf）
         * @return 0-成功，其他-失败（失败的代码在m_cda.rc中，失败的描述在m_cda.message中）
         * @note 如果成功的执行了非查询语句，在m_cda.rpc中保存了本次执行SQL影响记录的行数。
         *       程序中必须检查execute方法的返回值。
         */
        int execute(const char* fmt, ...);

        /**
         * 从结果集中获取下一条记录（仅查询语句有效）
         * @return 0-成功，1403-无更多记录，其他-失败（失败的代码在m_cda.rc中，失败的描述在m_cda.message中）
         * @note 返回失败的原因主要有两个：1）与数据库的连接已断开；2）绑定输出变量的内存太小。
         *       每执行一次next方法，m_cda.rpc的值加1。
         *       程序中必须检查next方法的返回值。
         */
        int next();

        /**
         * 绑定BLOB字段（用于插入/读取二进制大对象）
         * @return 0-成功，其他-失败（程序中一般不必关心返回值）
         */
        int bindblob();

        /**
         * 绑定CLOB字段（用于插入/读取字符大对象）
         * @return 0-成功，其他-失败（程序中一般不必关心返回值）
         */
        int bindclob();

        /**
         * 将文件内容导入到LOB字段
         * @param filename 待导入文件的路径（建议绝对路径）
         * @return 0-成功，其他-失败（程序中一般不必关心返回值）
         */
        int filetolob(const std::string& filename);

        /**
         * 将LOB字段内容导出到文件
         * @param filename 导出文件的路径（建议绝对路径）
         * @return 0-成功，其他-失败（程序中一般不必关心返回值）
         */
        int lobtofile(const std::string& filename);

        /**
         * 获取SQL语句的文本
         * @return SQL语句字符串的常量指针
         */
        const char* sql()
        {
            return m_sql.c_str();
        }

        /**
         * 获取错误代码（Return Code）
         * @return 0-成功，其它值-失败
         */
        int rc()
        {
            return m_cda.rc;
        }

        /**
         * 获取影响数据的行数（Rows Processed Count）
         * @return 对于insert/update/delete返回影响记录数，对于select返回结果集行数
         */
        unsigned long rpc()
        {
            return m_cda.rpc;
        }

        /**
         * 获取错误描述信息
         * @return 错误描述字符串（失败时有效）
         */
        const char* message()
        {
            return m_cda.message;
        }
    };

} // end namespace ol

#endif // !__OL_OCI_H