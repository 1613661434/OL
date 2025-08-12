/**************************************************************************************/
/*   程序名：ol_oci.h，此程序是开发框架的C++操作Oracle数据库的声明文件。                  */
/*   作者：ol                                                                          */
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

    struct LOGINENV // OCI登录环境。
    {
        char user[31];    // 数据库的用户名。
        char pass[31];    // 数据库的密码。
        char tnsname[51]; // 数据库的tnsname，在ORACLE_HOME/network/admin/tnsnames.ora中配置。

        OCIEnv* envhp; // 环境变量的句柄。
    };

    struct OCI_CXT // OCI上下文句柄。
    {
        OCISvcCtx* svchp;
        OCIError* errhp;
        OCIEnv* envhp; // 环境变量的句柄。
    };

    struct OCI_HANDLE // OCI的SQL句柄。
    {
        OCISvcCtx* svchp; // 服务器上下文的句柄引用context句柄。
        OCIStmt* smthp;

        OCIBind* bindhp;
        OCIDefine* defhp;

        OCIError* errhp; // 错误句柄引用context句柄。

        OCIEnv* envhp; // 环境变量的句柄。
    };

    struct CDA_DEF // OCI接口函数执行的结果。
    {
        int rc;             // 返回值：0-成功，其它失败。
        unsigned long rpc;  // 如果是insert、update和delete，保存影响记录的行数，如果是select，保存结果集的行数。
        char message[2048]; // 执行SQL语句如果失败，存放错误描述信息。
    };

    int oci_init(LOGINENV* env);
    int oci_close(LOGINENV* env);
    int oci_context_create(LOGINENV* env, OCI_CXT* cxt);
    int oci_context_close(OCI_CXT* cxt);

    int oci_stmt_create(OCI_CXT* cxt, OCI_HANDLE* handle);
    int oci_stmt_close(OCI_HANDLE* handle);

    class connection;
    class sqlstatement;

    // Oracle数据库连接类。
    class connection
    {
        friend class sqlstatement;

    private:
        LOGINENV m_env; // 服务器环境句柄。
        OCI_CXT m_cxt;  // 服务器上下文。

        int m_autocommitopt; // 自动提交标志，0-关闭；1-开启。

        // 设置字符集，如果客户端的字符集与数据库的不一致，就会出现乱码。
        void character(const char* charset);

        void setdbopt(const char* connstr); // 从connstr中解析username、password和tnsname。

        void err_report(); // 获取错误信息。

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
        connection();  // 构造函数。
        ~connection(); // 析构函数。

        // 登录数据库。
        // connstr：数据库的登录参数，格式：username/password@tnsname，username-用户名，password-登录密
        // 码，tnsname-数据库的服务名，在$ORACLE_HOME/network/admin/tnsnames.ora文件中配置。
        // charset：客户端的字符集，必须与数据库保持一致，否则会出现中文乱码的情况。
        // autocommitopt：是否启用自动提交事务，false-不启用，true-启用，缺省是不启用。
        // 返回值：0-成功，其它失败，失败的代码在m_cda.rc中，失败的描述在m_cda.message中。
        int connecttodb(const std::string& connstr, const std::string& charset, bool autocommitopt = false);

        bool isopen(); // 判断数据库是否已连接。

        // 提交事务。
        // 返回值：0-成功，其它失败，程序中一般不必关心返回值。
        int commit();

        // 回滚事务。
        // 返回值：0-成功，其它失败，程序中一般不必关心返回值。
        int rollback();

        // 断开与数据库的连接。
        // 注意，断开与数据库的连接时，全部未提交的事务自动回滚。
        // 返回值：0-成功，其它失败，程序中一般不必关心返回值。
        int disconnect();

        // 执行静态的SQL语句。
        // 如果SQL语句不需要绑定输入和输出变量（无绑定变量、非查询语句），可以直接用此方法执行。
        // 参数说明：这是一个可变参数，用法与printf函数相同。
        // 返回值：0-成功，其它失败，失败的代码在m_cda.rc中，失败的描述在m_cda.message中，
        // 如果成功的执行了非查询语句，在m_cda.rpc中保存了本次执行SQL影响记录的行数。
        // 程序中必须检查execute方法的返回值。
        // 在connection类中提供了execute方法，是为了方便程序员，在该方法中，也是用sqlstatement类来完成功能。
        int execute(const char* fmt, ...);

        // 获取错误代码：0-成功；其它-失败。
        int rc()
        {
            return m_cda.rc;
        }
        // 获取影响数据的行数。
        unsigned long rpc()
        {
            return m_cda.rpc;
        }
        // 获取错误描述。
        const char* message()
        {
            return m_cda.message;
        }
    };

    // 操作SQL语句类。
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
        sqlstatement();                 // 构造函数。
        sqlstatement(connection* conn); // 构造函数，同时指定数据库连接。

        ~sqlstatement();

        // 指定数据库连接。
        // conn：数据库连接connection对象的地址。
        // 返回值：0-成功，其它失败，只要conn参数是有效的，并且数据库的游标资源足够，connect方法不会返回失败。
        // 程序中一般不必关心connect方法的返回值。
        // 注意，每个sqlstatement只需要指定一次，在指定新的connection前，必须先显式的调用disconnect方法。
        int connect(connection* conn);

        bool isopen(); // 判断是否指定数据库连接。

        // 取消与数据库连接的关联。
        // 返回值：0-成功，其它失败，程序中一般不必关心返回值。
        int disconnect();

        // 准备SQL语句。
        // 参数说明：这是一个可变参数，用法与printf函数相同。
        // 返回值：0-成功，其它失败，程序中一般不必关心返回值。
        // 注意：如果SQL语句没有改变，只需要prepare一次就可以了。
        int prepare(const std::string& strsql)
        {
            return prepare(strsql.c_str());
        }
        int prepare(const char* fmt, ...);

        // 绑定输入变量的地址。
        // position：字段的顺序，从1开始，必须与prepare方法中的SQL的序号一一对应。
        // value：输入变量的地址，如果是字符串，内存大小应该是表对应的字段长度加1。
        // 返回值：0-成功，其它失败，程序中一般不必关心返回值。
        // 注意：1）如果SQL语句没有改变，只需要bindin一次就可以了；
        //           2）如果value的类型是std::string，bindin()函数中会resize(len)；
        //           3）如果value的类型是std::string，那么，在用户的程序代码中，不可改变它内部buffer的地址。
        //           4) 如果value的类型是std::string或char，那么传入len的参数不包括最后的NULL。
        int bindin(const unsigned int position, int& value);
        int bindin(const unsigned int position, long& value);
        int bindin(const unsigned int position, unsigned int& value);
        int bindin(const unsigned int position, unsigned long& value);
        int bindin(const unsigned int position, float& value);
        int bindin(const unsigned int position, double& value);
        int bindin(const unsigned int position, char* value, unsigned int len = 2000);
        int bindin(const unsigned int position, std::string& value, unsigned int len = 2000);
        int bindin1(const unsigned int position, std::string& value); // 在这个函数中，不考虑分配内存的问题。

        // 绑定输出变量的地址。
        // position：字段的顺序，从1开始，与SQL的结果集一一对应。
        // value：输出变量的地址，如果是字符串，内存大小应该是表对应的字段长度加1。
        // len：如果输出变量的数据类型是字符串，用len指定它的最大长度，建议采用表对应的字段长度。
        //         如果len的值太小，内容将会被截断。
        // 返回值：0-成功，其它失败，程序中一般不必关心返回值。
        // 注意：1）如果SQL语句没有改变，只需要bindout一次就可以了；
        //           2）如果value的类型是std::string，那么将在内容后面填充0，直到len的大小，value.size()永远是len。
        //           3）如果value的类型是std::string，那么，在用户的程序代码中，不可改变它内部buffer的地址。
        //           4) 如果value的类型是std::string或char，那么传入len的参数不包括最后的NULL。
        int bindout(const unsigned int position, int& value);
        int bindout(const unsigned int position, long& value);
        int bindout(const unsigned int position, unsigned int& value);
        int bindout(const unsigned int position, unsigned long& value);
        int bindout(const unsigned int position, float& value);
        int bindout(const unsigned int position, double& value);
        int bindout(const unsigned int position, char* value, unsigned int len = 2000);
        int bindout(const unsigned int position, std::string& value, unsigned int len = 2000);

        // 执行静态或动态SQL语句。
        // 返回值：0-成功，其它失败，失败的代码在m_cda.rc中，失败的描述在m_cda.message中。
        // 如果成功的执行了非查询语句，在m_cda.rpc中保存了本次执行SQL影响记录的行数。
        // 程序中必须检查execute方法的返回值。
        int execute();

        // 执行静态的SQL语句。
        // 如果SQL语句不需要绑定输入和输出变量（无绑定变量、非查询语句），可以直接用此方法执行。
        // 参数说明：这是一个可变参数，用法与printf函数相同。
        // 返回值：0-成功，其它失败，失败的代码在m_cda.rc中，失败的描述在m_cda.message中，
        // 如果成功的执行了非查询语句，在m_cda.rpc中保存了本次执行SQL影响记录的行数。
        // 程序中必须检查execute方法的返回值。
        int execute(const char* fmt, ...);

        // 从结果集中获取一条记录。
        // 如果执行的SQL语句是查询语句，调用execute方法后，会产生一个结果集（存放在数据库的缓冲区中）。
        // next方法从结果集中获取一条记录，把字段的值放入已绑定的输出变量中。
        // 返回值：0-成功，1403-结果集已无记录，其它-失败，失败的代码在m_cda.rc中，失败的描述在m_cda.message中。
        // 返回失败的原因主要有两个：1）与数据库的连接已断开；2）绑定输出变量的内存太小。
        // 每执行一次next方法，m_cda.rpc的值加1。
        // 程序中必须检查next方法的返回值。
        int next();

        // 绑定clob字段。
        // 返回值：0-成功，其它失败，程序中一般不必关心返回值。
        int bindblob();

        // 绑定blob字段。
        // 返回值：0-成功，其它失败，程序中一般不必关心返回值。
        int bindclob();

        // 把文件的内容导入到clob和blob字段中。
        // filename：待导入的文件名，建议采用绝对路径。
        // 返回值：0-成功，其它失败，程序中一般不必关心返回值。
        int filetolob(const std::string& filename);

        // 从clob和blob字段中导出内容到文件中。
        // filename：导出内容存放的文件名，建议采用绝对路径。
        // 返回值：0-成功，其它失败，程序中一般不必关心返回值。
        int lobtofile(const std::string& filename);

        // 获取SQL语句的文本。
        const char* sql()
        {
            return m_sql.c_str();
        }
        // 获取错误代码Return Code(返回码)：0-成功；其它-失败。
        int rc()
        {
            return m_cda.rc;
        }
        // 获取影响数据的行数Rows Processed Count(处理的行数计数)，如果是insert、update和delete，保存影响记录的行数，如果是select，保存结果集的行数。
        unsigned long rpc()
        {
            return m_cda.rpc;
        }
        // 获取错误描述。
        const char* message()
        {
            return m_cda.message;
        }
    };

} // end namespace ol

#endif // !__OL_OCI_H