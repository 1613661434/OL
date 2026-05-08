/****************************************************************************************/
/*
 * 程序名：ol_mysql.h
 * 功能描述：MySQL数据库实现，命名空间 ol::mysql
 * 结构：DBConn(继承IDBConn) + DBStmt，DBConn可直接创建DBStmt
 * 作者：ol
 */
/****************************************************************************************/
#ifndef OL_MYSQL_H
#define OL_MYSQL_H

#include "ol_database.h"
#include <memory>

namespace ol
{
    namespace mysql
    {
        class DBStmt;

        // ====================== MySQL连接类 ======================
        class DBConn : public ol::IDBConn
        {
        public:
            DBConn() = default;
            ~DBConn() override { disconnect(); }

            // 实现基类接口
            bool connect() override;
            void disconnect() override;
            bool isConnected() const override;
            void reset() override;
            void* getHandle() override;

            // 创建对应DBStmt
            std::unique_ptr<DBStmt> createStmt();

            // 事务
            bool beginTransaction();
            bool commit();
            bool rollback();
        };

        // ====================== MySQL语句类 ======================
        class DBStmt
        {
            friend class DBConn;

        public:
            explicit DBStmt(DBConn& conn);
            ~DBStmt();

            // 语句操作接口
            bool prepare(const char* sql);
            bool bind(int idx, const char* val);
            bool execute();
            bool next();
            // 取值方法...
        };

    } // namespace mysql
} // namespace ol

#endif // OL_MYSQL_H