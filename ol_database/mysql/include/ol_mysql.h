/****************************************************************************************/
/*
 * 程序名：ol_mysql.h
 * 功能描述：MySQL数据库操作实现，适配连接池，支持以下特性：
 *          - 连接管理：支持数据库连接、重连、断开、状态检测
 *          - 事务控制：支持手动事务提交/回滚、自动提交配置
 *          - 预处理语句：支持参数绑定、结果集绑定、SQL格式化预处理
 *          - 数据类型：支持int/long/float/double/string/BLOB/TEXT全类型绑定
 *          - 大字段操作：支持文件与BLOB/TEXT互转、分块传输大文件
 *          - 错误处理：统一执行结果码、错误信息、受影响行数返回
 * 作者：ol
 * 适用标准：C++11及以上
 */
/****************************************************************************************/

#ifndef OL_MYSQL_H
#define OL_MYSQL_H

#include "ol_database.h"
#include <mysql.h>
#include <string>
#include <memory>

namespace ol
{
    namespace mysql
    {
        /**
         * @brief 数据库操作结果结构体
         */
        struct DBResult
        {
            int code;              ///< 0=成功，非0=失败
            size_t affected_rows;  ///< 影响行数/结果集行数
            std::string error_msg; ///< 错误信息

            /**
             * @brief 初始化结果结构体
             */
            void init();
        };

        class DBStmt;

        /**
         * @brief MySQL连接实现类
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
                Disconnected = 0, ///< 未连接
                Connected = 1     ///< 已连接
            };

        private:
            MYSQL* m_mysql;       ///< MySQL原生操作句柄
            bool m_autocommitopt; ///< 自动提交事务开关
            ConnState m_state;    ///< 当前连接状态
            DBResult m_result;    ///< 数据库操作执行结果

            std::string m_host;    ///< 数据库主机地址
            std::string m_user;    ///< 数据库用户名
            std::string m_pass;    ///< 数据库密码
            std::string m_dbname;  ///< 数据库名
            unsigned int m_port;   ///< 数据库端口
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
             * @brief 获取MySQL原生句柄
             * @return MYSQL* 原生句柄
             */
            MYSQL* getNativeHandle();

            /**
             * @brief 设置数据库连接参数
             * @param connstr 连接字符串（user:pass@host:port/dbname）
             * @param charset 字符集
             * @param autocommit 是否自动提交
             */
            void setConnectParam(const std::string& connstr, const std::string& charset, bool autocommit = false);

            /**
             * @brief 重新连接数据库
             * @return 0成功，-1失败
             */
            int reconnect();

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
            void parseConnStr(const char* connstr);

            /**
             * @brief 记录数据库错误信息
             */
            void errReport();
        };

        /**
         * @brief MySQL预处理语句操作类
         * @note 支持参数绑定、结果集读取、大字段操作
         */
        class DBStmt : public TypeNonCopyableMovable
        {
            friend class DBConn;

        private:
            DBConn& m_conn;           ///< 所属数据库连接
            MYSQL* m_mysql;           ///< MySQL原生句柄
            MYSQL_STMT* m_stmt;       ///< 预处理语句句柄
            MYSQL_RES* m_db_result;   ///< 结果集
            MYSQL_BIND* m_bindIn;     ///< 输入参数绑定
            MYSQL_BIND* m_bindOut;    ///< 输出结果绑定
            unsigned long* m_outLen;  ///< 输出字段长度
            bool* m_outNull;          ///< 输出字段是否为空
            unsigned long* m_blobLen; ///< BLOB字段长度

            unsigned int m_paramCount; ///< 输入参数个数
            unsigned int m_fieldCount; ///< 结果集字段个数
            bool m_isQuery;            ///< 是否为查询语句
            std::string m_sql;         ///< 当前执行的SQL语句
            DBResult m_result;         ///< 语句执行结果

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
             * @return 0成功，MYSQL_NO_DATA无数据，其他错误
             */
            int next();

            // ===================== BLOB / TEXT 操作 =====================
            int bindblob(unsigned int pos, char* buffer, unsigned long length);
            /// 默认分块大小：4194304字节 = 4MB
            int filetoblob(unsigned int pos, const std::string& filename, unsigned int chunk = 4194304u);
            int blobtofile(unsigned int pos, const std::string& filename);
            int bindtext(unsigned int pos, char* buffer, unsigned long length);
            int bindtext(unsigned int pos, std::string& buffer, unsigned long length);
            /// 默认分块大小：4194304字节 = 4MB
            int filetotext(unsigned int pos, const std::string& filename, unsigned int chunk = 4194304u);
            int texttofile(unsigned int pos, const std::string& filename);

            /**
             * @brief 判断字段是否为空
             * @param pos 字段位置
             * @return 为空返回true
             */
            bool isNull(unsigned int pos);

            /**
             * @brief 获取字段长度
             * @param pos 字段位置
             * @return 字段长度
             */
            unsigned long length(unsigned int pos);

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
             * @brief 释放绑定资源
             */
            void freeBind();

            /**
             * @brief 记录语句错误信息
             */
            void errReport();

            /**
             * @brief 检查语句是否初始化
             * @return 已初始化返回true
             */
            bool isOpen() const;
        };

    } // namespace mysql
} // namespace ol

#endif // OL_MYSQL_H