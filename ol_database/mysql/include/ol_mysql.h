/****************************************************************************************/
/*
 * 程序名：ol_mysql.h
 * 功能描述：MySQL数据库实现，适配静态连接池
 * 作者：ol
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
        // 数据库操作结果结构体
        struct DBResult
        {
            int code;              // 0=成功，非0=失败
            size_t affected_rows;  // 影响行数/结果集行数
            std::string error_msg; // 错误信息

            void init();
        };

        class DBStmt;

        // MySQL连接实现类
        class DBConn : public ol::IDBConn
        {
        public:
            DBConn();
            ~DBConn() override;

            // 实现抽象接口
            bool connect() override;
            void disconnect() override;
            bool isConnected() const override;
            void reset() override;
            void* getHandle() override;

            // 连接配置
            void setConnectParam(const std::string& connstr, const std::string& charset, bool autocommit = false);
            int reconnect();

            // 创建语句对象
            std::unique_ptr<DBStmt> createStmt();

            // 事务
            bool beginTransaction();
            bool commit();
            bool rollback();

            // 直接执行SQL
            int execute(const char* fmt, ...);

            // 获取执行结果
            int code() const;
            size_t affectedRows() const;
            std::string errorMsg() const;

        private:
            MYSQL* m_mysql;
            int m_autocommitopt;
            int m_state;
            DBResult m_result;

            std::string m_host;
            std::string m_user;
            std::string m_pass;
            std::string m_dbname;
            unsigned int m_port;
            std::string m_charset;

            void parseConnStr(const char* connstr);
            void errReport();
        };

        // MySQL语句操作类
        class DBStmt : public TypeNonCopyableMovable
        {
        public:
            explicit DBStmt(DBConn& conn);
            ~DBStmt();

            // 预处理SQL
            bool prepare(const char* sql);
            bool prepare(const std::string& sql);
            bool prepare(const char* fmt, ...);

            // 输入绑定
            int bindin(unsigned int pos, int& value);
            int bindin(unsigned int pos, long& value);
            int bindin(unsigned int pos, unsigned int& value);
            int bindin(unsigned int pos, unsigned long& value);
            int bindin(unsigned int pos, float& value);
            int bindin(unsigned int pos, double& value);
            int bindin(unsigned int pos, char* value, unsigned int len = 2000);
            int bindin(unsigned int pos, std::string& value, unsigned int len = 2000);

            // 输出绑定
            int bindout(unsigned int pos, int& value);
            int bindout(unsigned int pos, long& value);
            int bindout(unsigned int pos, unsigned int& value);
            int bindout(unsigned int pos, unsigned long& value);
            int bindout(unsigned int pos, float& value);
            int bindout(unsigned int pos, double& value);
            int bindout(unsigned int pos, char* value, unsigned int len = 2000);
            int bindout(unsigned int pos, std::string& value, unsigned int len = 2000);

            // 执行与获取结果
            bool execute();
            int next();

            // BLOB / TEXT 操作
            int bindblob(unsigned int pos, char* buffer, unsigned long length);
            int filetoblob(unsigned int pos, const std::string& filename);
            int blobtofile(unsigned int pos, const std::string& filename);
            int bindtext(unsigned int pos, char* buffer, unsigned long length);
            int bindtext(unsigned int pos, std::string& buffer, unsigned long length);
            int filetotext(unsigned int pos, const std::string& filename, unsigned int chunk = 4 * 1024 * 1024);
            int texttofile(unsigned int pos, const std::string& filename);

            // 工具方法
            bool isNull(unsigned int pos);
            unsigned long length(unsigned int pos);
            const char* sql() const;
            int code() const;
            size_t affectedRows() const;
            std::string errorMsg() const;

        private:
            DBConn& m_conn;
            MYSQL* m_mysql;
            MYSQL_STMT* m_stmt;
            MYSQL_RES* m_result;
            MYSQL_BIND* m_bindIn;
            MYSQL_BIND* m_bindOut;
            unsigned long* m_outLen;
            bool* m_outNull;
            unsigned long* m_blobLen;

            unsigned int m_paramCount;
            unsigned int m_fieldCount;
            bool m_isQuery;
            std::string m_sql;
            DBResult m_db_result;

            void freeBind();
            void errReport();
            bool isOpen() const;
        };

    } // namespace mysql
} // namespace ol

#endif // OL_MYSQL_H