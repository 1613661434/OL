/****************************************************************************************/
/*
 * 程序名：ol_oci.h
 * 功能描述：Oracle数据库操作实现，适配连接池，支持以下特性：
 *          - 连接管理：支持数据库连接、重连、断开、状态检测
 *          - 事务控制：支持手动事务提交/回滚、自动提交配置
 *          - 预处理语句：支持参数绑定、结果集绑定、SQL格式化预处理
 *          - 数据类型：支持int/long/float/double/string/BLOB/CLOB全类型绑定
 *          - 大字段操作：支持文件与BLOB/CLOB互转、分块传输大文件
 *          - 错误处理：统一执行结果码、错误信息、受影响行数返回
 * 作者：ol
 * 适用标准：C++17及以上
 */
/****************************************************************************************/

/*
// OCI 返回状态码
OCI_SUCCESS                0  // maps to SQL_SUCCESS of SAG CLI  函数执行成功
OCI_SUCCESS_WITH_INFO      1  // maps to SQL_SUCCESS_WITH_INFO   执行成功，但有诊断消息返回，
                              // 可能是警告信息，但是，在测试的时候，我还从未见
                              // 识到OCI_SUCCESS_WITH_INFO是怎么回事
OCI_RESERVED_FOR_INT_USE 200  // reserved
OCI_NO_DATA              100  // maps to SQL_NO_DATA 函数执行完成，但没有其他数据
OCI_ERROR                 -1  // maps to SQL_ERROR 函数执行错误
OCI_INVALID_HANDLE        -2  // maps to SQL_INVALID_HANDLE 传递给函数的参数为无效句柄，
                              // 或传回的句柄无效
OCI_NEED_DATA             99  // maps to SQL_NEED_DATA 需要应用程序提供运行时刻的数据
OCI_STILL_EXECUTING    -3123  // OCI would block error 服务环境建立在非阻塞模式，
                              // OCI函数调用正在执行中

OCI_CONTINUE          -24200  // Continue with the body of the OCI function
                              // 说明回调函数需要OCI库恢复其正常的处理操作
OCI_ROWCBK_DONE       -24201  // done with user row callback
*/

#ifndef OL_OCI_H
#define OL_OCI_H 1

#include "ol_database.h"
#include "ol_string.h"
#include <oci.h>
#include <string>
#include <memory>
#include <cstdarg>

namespace ol
{
    namespace oracle
    {
        /**
         * @brief 数据库操作结果结构体
         */
        struct DBResult
        {
            int code = 0;             ///< 0=成功，非0=失败
            size_t affected_rows = 0; ///< 影响行数/结果集行数
            std::string error_msg;    ///< 错误信息

            /**
             * @brief 初始化结果结构体
             */
            void init();
        };

        class DBStmt;

        /**
         * @brief Oracle连接实现类
         * @note 继承数据库连接抽象接口，适配连接池
         */
        class DBConn : public IDBConn
        {
            friend class DBStmt;

        public:
            /**
             * @brief 连接状态枚举
             */
            enum class ConnState : char
            {
                Disconnected = 0, // 未连接
                Connected = 1     // 已连接
            };

        private:
            OCIEnv* m_envhp = nullptr;                   ///< OCI环境句柄
            OCISvcCtx* m_svchp = nullptr;                ///< OCI服务器上下文句柄
            OCIError* m_errhp = nullptr;                 ///< OCI错误句柄
            bool m_autocommitopt = false;                ///< 自动提交事务开关
            ConnState m_state = ConnState::Disconnected; ///< 当前连接状态
            DBResult m_result;                           ///< 数据库操作执行结果

            std::string m_user;    ///< 数据库用户名
            std::string m_pass;    ///< 数据库密码
            std::string m_tnsname; ///< 数据库TNS服务名
            std::string m_charset; ///< 数据库字符集

        public:
            /**
             * @brief 构造函数
             */
            DBConn();

            /**
             * @brief 析构函数，自动断开连接
             */
            ~DBConn() override;

            /**
             * @brief 建立数据库连接
             * @return 连接成功返回true，失败返回false
             */
            bool connect() override;

            /**
             * @brief 断开数据库连接
             */
            void disconnect() override;

            /**
             * @brief 检查连接是否有效
             * @return 已连接返回true，未连接返回false
             */
            bool isConnected() const override;

            /**
             * @brief 重置连接执行状态
             */
            void reset() override;

            /**
             * @brief 重新连接数据库
             * @return 重连成功返回true，失败返回false
             */
            bool reconnect();

            /**
             * @brief 设置数据库连接参数
             * @param connstr 连接字符串（username/password@tnsname）
             * @param charset 字符集
             * @param autocommit 是否自动提交
             */
            void setConnectParam(const std::string& connstr, const std::string& charset, bool autocommit = false);

            /**
             * @brief 创建预处理语句对象
             * @return 预处理语句智能指针
             */
            std::unique_ptr<DBStmt> createStmt();

            /**
             * @brief 开启事务
             * @return 成功返回true
             */
            bool beginTransaction();

            /**
             * @brief 提交事务
             * @return 成功返回true
             */
            bool commit();

            /**
             * @brief 回滚事务
             * @return 成功返回true
             */
            bool rollback();

            /**
             * @brief 直接执行SQL语句
             * @param fmt SQL格式化字符串
             * @param ... 可变参数
             * @return 0成功，-1失败
             */
            int execute(const char* fmt, ...);

            /**
             * @brief 获取执行结果码
             * @return 结果码
             */
            int code() const;

            /**
             * @brief 获取受影响行数
             * @return 行数
             */
            size_t affectedRows() const;

            /**
             * @brief 获取错误信息
             * @return 错误字符串
             */
            std::string errorMsg() const;

        private:
            /**
             * @brief 解析连接字符串
             * @param connstr 连接字符串
             */
            void parseConnStr(const std::string& connstr);

            /**
             * @brief 设置客户端字符集
             * @param charset 字符集名称
             */
            void setCharset(const char* charset);

            /**
             * @brief 记录数据库错误信息
             */
            void errReport();

            // OCI底层操作（从旧版全局函数迁移为私有方法）
            int ociEnvInit();
            int ociEnvClose();
            int ociContextCreate();
            int ociContextClose();
        };

        /**
         * @brief Oracle预处理语句操作类
         * @note 支持参数绑定、结果集读取、大字段操作
         */
        class DBStmt : public TypeNonCopyableMovable
        {
            friend class DBConn;

        private:
            DBConn& m_conn;               ///< 所属数据库连接
            OCIStmt* m_smthp = nullptr;   ///< OCI语句句柄
            OCIBind* m_bindhp = nullptr;  ///< OCI绑定句柄
            OCIDefine* m_defhp = nullptr; ///< OCI定义句柄
            OCIError* m_errhp = nullptr;  ///< 错误句柄（引用自DBConn）
            OCISvcCtx* m_svchp = nullptr; ///< 服务上下文（引用自DBConn）
            OCIEnv* m_envhp = nullptr;    ///< 环境句柄（引用自DBConn）

            bool m_autocommitopt = false;   ///< 自动提交标志
            bool m_isQuery = false;         ///< 是否为查询语句（SELECT）
            OCILobLocator* m_lob = nullptr; ///< LOB定位器
            std::string m_sql;              ///< 当前执行的SQL语句
            DBResult m_result;              ///< 语句执行结果

        public:
            /**
             * @brief 构造函数
             * @param conn 所属数据库连接
             */
            explicit DBStmt(DBConn& conn);

            /**
             * @brief 析构函数，释放资源
             */
            ~DBStmt();

            /**
             * @brief 预处理SQL语句
             * @param sql SQL字符串
             * @return 预处理成功返回true
             */
            bool prepare(const char* sql);

            /**
             * @brief 预处理SQL语句
             * @param sql SQL字符串
             * @return 预处理成功返回true
             */
            bool prepare(const std::string& sql);

            /**
             * @brief 格式化预处理SQL
             * @param fmt 格式化字符串
             * @param ... 可变参数
             * @return 成功返回true
             */
            bool prepareFmt(const char* fmt, ...);

            /**
             * @brief 绑定输入变量（将SQL中的占位符与变量关联）
             * @param pos 占位符位置（从1开始，必须与prepare方法中的SQL的序号一一对应）
             * @param value 输入变量地址（如果是字符串，内存大小应该是表对应的字段长度加1）
             * @param len 字符串类型的长度（不含终止符，默认512）
             * @return 0-成功，其他-失败（程序中一般不必关心返回值）
             * @note 1）如果SQL语句没有改变，只需要bindin一次就可以了；
             *       2）如果value的类型是std::string，bindin()函数中会resize(len)；
             *       3）如果value的类型是std::string，那么，在用户的程序代码中，不可改变它内部buffer的地址。
             *       4）如果value的类型是std::string或char，那么传入len的参数不包括最后的NULL。
             */
            int bindin(unsigned int pos, int& value);                                  ///< 绑定int类型
            int bindin(unsigned int pos, long& value);                                 ///< 绑定long类型
            int bindin(unsigned int pos, unsigned int& value);                         ///< 绑定unsigned int类型
            int bindin(unsigned int pos, unsigned long& value);                        ///< 绑定unsigned long类型
            int bindin(unsigned int pos, float& value);                                ///< 绑定float类型
            int bindin(unsigned int pos, double& value);                               ///< 绑定double类型
            int bindin(unsigned int pos, char* value, unsigned int len = 512u);        ///< 绑定char*类型
            int bindin(unsigned int pos, std::string& value, unsigned int len = 512u); ///< 绑定std::string类型

            /**
             * @brief 绑定输出变量（将查询结果字段与变量关联）
             * @param pos 结果集字段位置（从1开始，与SQL的结果集一一对应）
             * @param value 输出变量地址（如果是字符串，内存大小应该是表对应的字段长度加1）
             * @param len 字符串类型的最大长度（不含终止符，默认512）
             * @return 0-成功，其他-失败（程序中一般不必关心返回值）
             * @note 1）如果SQL语句没有改变，只需要bindout一次就可以了；
             *       2）如果value的类型是std::string，那么将在内容后面填充0，直到len的大小，value.size()永远是len。
             *       3）如果value的类型是std::string，那么，在用户的程序代码中，不可改变它内部buffer的地址。
             *       4）如果value的类型是std::string或char，那么传入len的参数不包括最后的NULL。
             */
            int bindout(unsigned int pos, int& value);                                  ///< 绑定int类型
            int bindout(unsigned int pos, long& value);                                 ///< 绑定long类型
            int bindout(unsigned int pos, unsigned int& value);                         ///< 绑定unsigned int类型
            int bindout(unsigned int pos, unsigned long& value);                        ///< 绑定unsigned long类型
            int bindout(unsigned int pos, float& value);                                ///< 绑定float类型
            int bindout(unsigned int pos, double& value);                               ///< 绑定double类型
            int bindout(unsigned int pos, char* value, unsigned int len = 512u);        ///< 绑定char*类型
            int bindout(unsigned int pos, std::string& value, unsigned int len = 512u); ///< 绑定std::string类型

            /**
             * @brief 执行预处理语句
             * @return 执行成功返回true
             */
            bool execute();

            /**
             * @brief 获取下一行结果集
             * @return 0成功，OCI_NO_DATA(100)无数据，其他错误
             */
            int next();

            // ===================== BLOB / CLOB 操作 =====================
            /**
             * @brief 绑定BLOB字段（用于读取二进制大对象）
             * @param pos 字段位置（从1开始）
             * @return 0成功，其他失败
             */
            int bindblob(unsigned int pos);

            /**
             * @brief 绑定CLOB字段（用于读取字符大对象）
             * @param pos 字段位置（从1开始）
             * @return 0成功，其他失败
             */
            int bindclob(unsigned int pos);

            /**
             * @brief 将文件内容导入到BLOB字段
             * @param pos 字段位置（从1开始）
             * @param filename 待导入文件的路径
             * @return 0成功，其他失败
             */
            int filetoblob(unsigned int pos, const std::string& filename);

            /**
             * @brief 将BLOB字段内容导出到文件
             * @param pos 字段位置（从1开始）
             * @param filename 导出文件的路径
             * @return 0成功，其他失败
             */
            int blobtofile(unsigned int pos, const std::string& filename);

            /**
             * @brief 将文件内容导入到CLOB字段
             * @param pos 字段位置（从1开始）
             * @param filename 待导入文件的路径
             * @return 0成功，其他失败
             */
            int filetoclob(unsigned int pos, const std::string& filename);

            /**
             * @brief 将CLOB字段内容导出到文件
             * @param pos 字段位置（从1开始）
             * @param filename 导出文件的路径
             * @return 0成功，其他失败
             */
            int clobtofile(unsigned int pos, const std::string& filename);

            /**
             * @brief 获取当前SQL语句
             * @return SQL字符串
             */
            const char* sql() const;

            /**
             * @brief 获取执行结果码
             * @return 结果码
             */
            int code() const;

            /**
             * @brief 获取受影响行数
             * @return 行数
             */
            size_t affectedRows() const;

            /**
             * @brief 获取错误信息
             * @return 错误字符串
             */
            std::string errorMsg() const;

        private:
            /**
             * @brief 检查语句是否初始化
             * @return 已初始化返回true
             */
            bool isOpen() const;

            /**
             * @brief 记录语句错误信息
             */
            void errReport();

            /**
             * @brief 初始化LOB定位器
             * @return 0成功，其他失败
             */
            int alloclob();

            /**
             * @brief 释放LOB定位器
             */
            void freelob();

            /**
             * @brief 将文件内容写入LOB字段（内部实现，分片传输）
             * @param fp 已打开的文件指针
             * @return 0成功，其他失败
             */
            int filetolobInternal(FILE* fp);

            /**
             * @brief 将LOB字段内容写入文件（内部实现，分片读取）
             * @param fp 已打开的文件指针
             * @return 0成功，其他失败
             */
            int lobtofileInternal(FILE* fp);

            // OCI底层操作
            int ociStmtCreate();
            int ociStmtClose();
        };

    } // namespace oracle
} // namespace ol

#endif // OL_OCI_H