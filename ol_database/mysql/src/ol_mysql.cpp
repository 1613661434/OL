/**************************************************************************************/
/*
 * 程序名：ol_mysql.cpp
 * 功能描述：C++ MySQL数据库操作实现（复用ol_core工具函数）
 * 作者：ol
 * 标准：C++11 及以上
 */
/**************************************************************************************/

#include "ol_mysql.h"
#include "ol_fstream.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <errno.h>
#include <direct.h>
#include <cctype>

namespace ol
{
    namespace mysql
    {
        // ===================== DBResult 实现 =====================
        void DBResult::init()
        {
            code = 0;
            affected_rows = 0;
            error_msg.clear();
        }

        // ===================== DBConn 实现 =====================
        DBConn::DBConn()
            : m_mysql(nullptr),
              m_autocommitopt(false),
              m_state(ConnState::Disconnected),
              m_port(3306)
        {
            m_result.init();
            m_result.code = -1;
            m_result.error_msg = "database not open";
        }

        DBConn::~DBConn()
        {
            disconnect();
        }

        void DBConn::setConnectParam(const std::string& connstr, const std::string& charset, bool autocommit)
        {
            parseConnStr(connstr.c_str());
            m_charset = charset.empty() ? "binary" : charset;
            m_autocommitopt = autocommit;
        }

        bool DBConn::connect()
        {
            if (m_state == ConnState::Connected) return true;

            m_result.init();
            m_mysql = mysql_init(nullptr);

            if (!m_mysql)
            {
                m_result.code = -1;
                m_result.error_msg = "mysql_init failed";
                return false;
            }

            if (!mysql_real_connect(m_mysql, m_host.c_str(), m_user.c_str(), m_pass.c_str(),
                                    m_dbname.c_str(), m_port, nullptr, 0))
            {
                errReport();
                mysql_close(m_mysql);
                m_mysql = nullptr;
                return false;
            }

            mysql_set_character_set(m_mysql, m_charset.c_str());
            mysql_autocommit(m_mysql, m_autocommitopt);

            m_state = ConnState::Connected;
            m_result.init();
            return true;
        }

        void DBConn::disconnect()
        {
            if (m_state == ConnState::Disconnected) return;

            if (!m_autocommitopt) rollback();

            mysql_close(m_mysql);
            m_mysql = nullptr;
            m_state = ConnState::Disconnected;
        }

        bool DBConn::isConnected() const
        {
            return m_state == ConnState::Connected;
        }

        void DBConn::reset()
        {
            m_result.init();
        }

        MYSQL* DBConn::getNativeHandle()
        {
            return m_mysql;
        }

        int DBConn::reconnect()
        {
            disconnect();
            return connect() ? 0 : -1;
        }

        bool DBConn::beginTransaction()
        {
            return mysql_autocommit(m_mysql, 0) == 0;
        }

        bool DBConn::commit()
        {
            return mysql_commit(m_mysql) == 0;
        }

        bool DBConn::rollback()
        {
            return mysql_rollback(m_mysql) == 0;
        }

        std::unique_ptr<DBStmt> DBConn::createStmt()
        {
            return std::make_unique<DBStmt>(*this);
        }

        int DBConn::execute(const char* fmt, ...)
        {
            m_result.init(); // 重置结果

            va_list ap;
            va_start(ap, fmt);
            int len = vsnprintf(nullptr, 0, fmt, ap);
            va_end(ap);

            if (len <= 0)
            {
                m_result.code = -1;
                m_result.error_msg = "invalid sql format";
                return -1;
            }

            std::string sql;
            sql.resize(len + 1);

            va_start(ap, fmt);
            vsnprintf(&sql[0], len + 1, fmt, ap);
            va_end(ap);

            DBStmt stmt(*this);
            if (!stmt.prepare(sql))
            {
                // 同步错误信息
                m_result = stmt.m_result;
                return -1;
            }

            if (!stmt.execute())
            {
                // 同步错误信息
                m_result = stmt.m_result;
                return -1;
            }

            // 同步执行结果到连接对象
            m_result = stmt.m_result;
            return 0;
        }

        int DBConn::code() const { return m_result.code; }
        size_t DBConn::affectedRows() const { return m_result.affected_rows; }
        std::string DBConn::errorMsg() const { return m_result.error_msg; }

        void DBConn::parseConnStr(const char* connstr)
        {
            std::string s = connstr;
            size_t p1 = s.find(':');
            size_t p2 = s.find('@');
            size_t p4 = s.find('/');

            if (p1 != std::string::npos && p1 < p2)
                m_user = s.substr(0, p1);
            if (p2 != std::string::npos && p1 < p2)
                m_pass = s.substr(p1 + 1, p2 - p1 - 1);

            std::string hostPart = (p4 != std::string::npos)
                                       ? s.substr(p2 + 1, p4 - p2 - 1)
                                       : s.substr(p2 + 1);

            if (p4 != std::string::npos)
                m_dbname = s.substr(p4 + 1);

            size_t p3 = hostPart.find(':');
            if (p3 != std::string::npos)
            {
                m_host = hostPart.substr(0, p3);
                m_port = static_cast<unsigned int>(atoi(hostPart.substr(p3 + 1).c_str()));
            }
            else
                m_host = hostPart.empty() ? "localhost" : hostPart;
        }

        void DBConn::errReport()
        {
            m_result.code = mysql_errno(m_mysql);
            m_result.error_msg = mysql_error(m_mysql);
        }

        // ===================== DBStmt 实现 =====================
        DBStmt::DBStmt(DBConn& conn)
            : m_conn(conn),
              m_mysql(conn.m_mysql),
              m_stmt(nullptr),
              m_db_result(nullptr),
              m_bindIn(nullptr),
              m_bindOut(nullptr),
              m_outLen(nullptr),
              m_outNull(nullptr),
              m_blobLen(nullptr),
              m_paramCount(0),
              m_fieldCount(0),
              m_isQuery(false)
        {
            m_result.init();
            m_stmt = mysql_stmt_init(m_mysql);
        }

        DBStmt::~DBStmt()
        {
            if (m_db_result) mysql_free_result(m_db_result);
            if (m_stmt) mysql_stmt_close(m_stmt);
            freeBind();
        }

        void DBStmt::freeBind()
        {
            delete[] m_bindIn;
            delete[] m_bindOut;
            delete[] m_outLen;
            delete[] m_outNull;
            delete[] m_blobLen;

            m_bindIn = m_bindOut = nullptr;
            m_outLen = m_blobLen = nullptr;
            m_outNull = nullptr;
        }

        bool DBStmt::isOpen() const
        {
            return m_stmt != nullptr;
        }

        void DBStmt::errReport()
        {
            m_result.code = mysql_stmt_errno(m_stmt);
            m_result.error_msg = mysql_stmt_error(m_stmt);
        }

        bool DBStmt::prepare(const char* sql)
        {
            if (!isOpen()) return false;

            freeBind();
            m_sql = sql;
            m_isQuery = false;

            if (mysql_stmt_prepare(m_stmt, sql, strlen(sql)) != 0)
            {
                errReport();
                return false;
            }

            char head[7] = {0};
            strncpy(head, sql, 6);
            toUpper(head);
            deleteLchr(head, ' ');

            if (strstr(head, "SELECT") || strstr(head, "SHOW") ||
                strstr(head, "DESC") || strstr(head, "EXPLA"))
            {
                m_isQuery = true;
            }

            m_paramCount = mysql_stmt_param_count(m_stmt);
            if (m_paramCount > 0)
            {
                m_bindIn = new MYSQL_BIND[m_paramCount]();
                m_blobLen = new unsigned long[m_paramCount]();
            }

            return true;
        }

        bool DBStmt::prepare(const std::string& sql)
        {
            return prepare(sql.c_str());
        }

        bool DBStmt::prepareFmt(const char* fmt, ...)
        {
            m_result.init();
            if (!isOpen()) return false;

            va_list ap;
            va_start(ap, fmt);
            int len = vsnprintf(nullptr, 0, fmt, ap);
            va_end(ap);
            if (len <= 0) return false;

            m_sql.resize(len + 1);
            va_start(ap, fmt);
            vsnprintf(&m_sql[0], len + 1, fmt, ap);
            va_end(ap);

            return prepare(m_sql.c_str());
        }

        // ===================== 输入绑定 =====================
        int DBStmt::bindin(unsigned int pos, int& value)
        {
            if (pos < 1 || pos > m_paramCount || !m_bindIn)
            {
                m_result.code = -1;
                m_result.error_msg = "bindin failed: position out of range or m_bindIn null";
                return -1;
            }
            MYSQL_BIND& bind = m_bindIn[pos - 1];
            std::memset(&bind, 0, sizeof(MYSQL_BIND));
            bind.buffer_type = MYSQL_TYPE_LONG;
            bind.buffer = &value;
            bind.is_unsigned = false;
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindin(unsigned int pos, long& value)
        {
            if (pos < 1 || pos > m_paramCount || !m_bindIn)
            {
                m_result.code = -1;
                m_result.error_msg = "bindin failed: position out of range or m_bindIn null";
                return -1;
            }
            MYSQL_BIND& bind = m_bindIn[pos - 1];
            std::memset(&bind, 0, sizeof(MYSQL_BIND));
            bind.buffer_type = MYSQL_TYPE_LONGLONG;
            bind.buffer = &value;
            bind.is_unsigned = false;
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindin(unsigned int pos, unsigned int& value)
        {
            if (pos < 1 || pos > m_paramCount || !m_bindIn)
            {
                m_result.code = -1;
                m_result.error_msg = "bindin failed: position out of range or m_bindIn null";
                return -1;
            }
            MYSQL_BIND& bind = m_bindIn[pos - 1];
            std::memset(&bind, 0, sizeof(MYSQL_BIND));
            bind.buffer_type = MYSQL_TYPE_LONG;
            bind.buffer = &value;
            bind.is_unsigned = true;
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindin(unsigned int pos, unsigned long& value)
        {
            if (pos < 1 || pos > m_paramCount || !m_bindIn)
            {
                m_result.code = -1;
                m_result.error_msg = "bindin failed: position out of range or m_bindIn null";
                return -1;
            }
            MYSQL_BIND& bind = m_bindIn[pos - 1];
            std::memset(&bind, 0, sizeof(MYSQL_BIND));
            bind.buffer_type = MYSQL_TYPE_LONGLONG;
            bind.buffer = &value;
            bind.is_unsigned = true;
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindin(unsigned int pos, float& value)
        {
            if (pos < 1 || pos > m_paramCount || !m_bindIn)
            {
                m_result.code = -1;
                m_result.error_msg = "bindin failed: position out of range or m_bindIn null";
                return -1;
            }
            MYSQL_BIND& bind = m_bindIn[pos - 1];
            std::memset(&bind, 0, sizeof(MYSQL_BIND));
            bind.buffer_type = MYSQL_TYPE_FLOAT;
            bind.buffer = &value;
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindin(unsigned int pos, double& value)
        {
            if (pos < 1 || pos > m_paramCount || !m_bindIn)
            {
                m_result.code = -1;
                m_result.error_msg = "bindin failed: position out of range or m_bindIn null";
                return -1;
            }
            MYSQL_BIND& bind = m_bindIn[pos - 1];
            std::memset(&bind, 0, sizeof(MYSQL_BIND));
            bind.buffer_type = MYSQL_TYPE_DOUBLE;
            bind.buffer = &value;
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindin(unsigned int pos, char* value, unsigned int len)
        {
            if (pos < 1 || pos > m_paramCount || !m_bindIn || !value || len == 0)
            {
                m_result.code = -1;
                m_result.error_msg = "bindin failed: invalid params (null/zero length/position)";
                return -1;
            }
            MYSQL_BIND& bind = m_bindIn[pos - 1];
            std::memset(&bind, 0, sizeof(MYSQL_BIND));
            bind.buffer_type = MYSQL_TYPE_STRING;
            bind.buffer = value;
            bind.buffer_length = len;
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindin(unsigned int pos, std::string& value, unsigned int len)
        {
            if (len == 0)
            {
                m_result.code = -1;
                m_result.error_msg = "bindin failed: length cannot be zero";
                return -1;
            }
            value.resize(len);
            return bindin(pos, value.data(), len);
        }

        // ===================== 输出绑定 =====================
        int DBStmt::bindout(unsigned int pos, int& value)
        {
            if (m_fieldCount == 0)
            {
                m_fieldCount = mysql_stmt_field_count(m_stmt);
                m_bindOut = new MYSQL_BIND[m_fieldCount]();
                m_outLen = new unsigned long[m_fieldCount]();
                m_outNull = new bool[m_fieldCount]();
            }
            if (pos < 1 || pos > m_fieldCount)
            {
                m_result.code = -1;
                m_result.error_msg = "bindout failed: position out of range";
                return -1;
            }

            MYSQL_BIND& bind = m_bindOut[pos - 1];
            std::memset(&bind, 0, sizeof(MYSQL_BIND));
            bind.buffer_type = MYSQL_TYPE_LONG;
            bind.buffer = &value;
            bind.length = &m_outLen[pos - 1];
            bind.is_null = &m_outNull[pos - 1];
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindout(unsigned int pos, long& value)
        {
            if (m_fieldCount == 0)
            {
                m_fieldCount = mysql_stmt_field_count(m_stmt);
                m_bindOut = new MYSQL_BIND[m_fieldCount]();
                m_outLen = new unsigned long[m_fieldCount]();
                m_outNull = new bool[m_fieldCount]();
            }
            if (pos < 1 || pos > m_fieldCount)
            {
                m_result.code = -1;
                m_result.error_msg = "bindout failed: position out of range";
                return -1;
            }

            MYSQL_BIND& bind = m_bindOut[pos - 1];
            std::memset(&bind, 0, sizeof(MYSQL_BIND));
            bind.buffer_type = MYSQL_TYPE_LONGLONG;
            bind.buffer = &value;
            bind.length = &m_outLen[pos - 1];
            bind.is_null = &m_outNull[pos - 1];
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindout(unsigned int pos, unsigned int& value)
        {
            if (m_fieldCount == 0)
            {
                m_fieldCount = mysql_stmt_field_count(m_stmt);
                m_bindOut = new MYSQL_BIND[m_fieldCount]();
                m_outLen = new unsigned long[m_fieldCount]();
                m_outNull = new bool[m_fieldCount]();
            }
            if (pos < 1 || pos > m_fieldCount)
            {
                m_result.code = -1;
                m_result.error_msg = "bindout failed: position out of range";
                return -1;
            }

            MYSQL_BIND& bind = m_bindOut[pos - 1];
            std::memset(&bind, 0, sizeof(MYSQL_BIND));
            bind.buffer_type = MYSQL_TYPE_LONG;
            bind.buffer = &value;
            bind.is_unsigned = true;
            bind.length = &m_outLen[pos - 1];
            bind.is_null = &m_outNull[pos - 1];
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindout(unsigned int pos, unsigned long& value)
        {
            if (m_fieldCount == 0)
            {
                m_fieldCount = mysql_stmt_field_count(m_stmt);
                m_bindOut = new MYSQL_BIND[m_fieldCount]();
                m_outLen = new unsigned long[m_fieldCount]();
                m_outNull = new bool[m_fieldCount]();
            }
            if (pos < 1 || pos > m_fieldCount)
            {
                m_result.code = -1;
                m_result.error_msg = "bindout failed: position out of range";
                return -1;
            }

            MYSQL_BIND& bind = m_bindOut[pos - 1];
            std::memset(&bind, 0, sizeof(MYSQL_BIND));
            bind.buffer_type = MYSQL_TYPE_LONGLONG;
            bind.buffer = &value;
            bind.is_unsigned = true;
            bind.length = &m_outLen[pos - 1];
            bind.is_null = &m_outNull[pos - 1];
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindout(unsigned int pos, float& value)
        {
            if (m_fieldCount == 0)
            {
                m_fieldCount = mysql_stmt_field_count(m_stmt);
                m_bindOut = new MYSQL_BIND[m_fieldCount]();
                m_outLen = new unsigned long[m_fieldCount]();
                m_outNull = new bool[m_fieldCount]();
            }
            if (pos < 1 || pos > m_fieldCount)
            {
                m_result.code = -1;
                m_result.error_msg = "bindout failed: position out of range";
                return -1;
            }

            MYSQL_BIND& bind = m_bindOut[pos - 1];
            std::memset(&bind, 0, sizeof(MYSQL_BIND));
            bind.buffer_type = MYSQL_TYPE_FLOAT;
            bind.buffer = &value;
            bind.length = &m_outLen[pos - 1];
            bind.is_null = &m_outNull[pos - 1];
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindout(unsigned int pos, double& value)
        {
            if (m_fieldCount == 0)
            {
                m_fieldCount = mysql_stmt_field_count(m_stmt);
                m_bindOut = new MYSQL_BIND[m_fieldCount]();
                m_outLen = new unsigned long[m_fieldCount]();
                m_outNull = new bool[m_fieldCount]();
            }
            if (pos < 1 || pos > m_fieldCount)
            {
                m_result.code = -1;
                m_result.error_msg = "bindout failed: position out of range";
                return -1;
            }

            MYSQL_BIND& bind = m_bindOut[pos - 1];
            std::memset(&bind, 0, sizeof(MYSQL_BIND));
            bind.buffer_type = MYSQL_TYPE_DOUBLE;
            bind.buffer = &value;
            bind.length = &m_outLen[pos - 1];
            bind.is_null = &m_outNull[pos - 1];
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindout(unsigned int pos, char* value, unsigned int len)
        {
            if (m_fieldCount == 0)
            {
                m_fieldCount = mysql_stmt_field_count(m_stmt);
                m_bindOut = new MYSQL_BIND[m_fieldCount]();
                m_outLen = new unsigned long[m_fieldCount]();
                m_outNull = new bool[m_fieldCount]();
            }
            if (pos < 1 || pos > m_fieldCount || !value || len == 0)
            {
                m_result.code = -1;
                m_result.error_msg = "bindout failed: invalid params (null/zero length/position)";
                return -1;
            }

            MYSQL_BIND& bind = m_bindOut[pos - 1];
            std::memset(&bind, 0, sizeof(MYSQL_BIND));
            bind.buffer_type = MYSQL_TYPE_STRING;
            bind.buffer = value;
            bind.buffer_length = len;
            bind.length = &m_outLen[pos - 1];
            bind.is_null = &m_outNull[pos - 1];
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindout(unsigned int pos, std::string& value, unsigned int len)
        {
            if (len == 0)
            {
                m_result.code = -1;
                m_result.error_msg = "bindout failed: length cannot be zero";
                return -1;
            }
            value.resize(len);
            return bindout(pos, value.data(), len);
        }

        // ===================== 执行与结果 =====================
        bool DBStmt::execute()
        {
            if (!isOpen()) return false;

            if (m_paramCount > 0 && mysql_stmt_bind_param(m_stmt, m_bindIn) != 0)
            {
                errReport();
                return false;
            }

            if (mysql_stmt_execute(m_stmt) != 0)
            {
                errReport();
                return false;
            }

            if (m_isQuery)
            {
                if (m_bindOut)
                    mysql_stmt_bind_result(m_stmt, m_bindOut);
                mysql_stmt_store_result(m_stmt);
                m_result.affected_rows = mysql_stmt_num_rows(m_stmt);
            }
            else
            {
                m_result.affected_rows = mysql_stmt_affected_rows(m_stmt);
            }

            m_result.code = 0;
            return true;
        }

        int DBStmt::next()
        {
            if (!m_isQuery) return -1;

            int ret = mysql_stmt_fetch(m_stmt);
            if (ret == 0) return ret;
            if (ret == MYSQL_NO_DATA) return ret;

            errReport();
            return m_result.code;
        }

        // ===================== BLOB 操作 =====================
        int DBStmt::bindblob(unsigned int pos, char* buffer, unsigned long length)
        {
            if (pos < 1 || !buffer || length == 0)
            {
                m_result.code = -1;
                m_result.error_msg = "bindblob failed: invalid parameters (pos/buffer/length)";
                return -1;
            }

            if (pos <= m_paramCount && m_bindIn)
            {
                m_blobLen[pos - 1] = length;
                MYSQL_BIND& bind = m_bindIn[pos - 1];
                std::memset(&bind, 0, sizeof(MYSQL_BIND));
                bind.buffer_type = MYSQL_TYPE_BLOB;
                bind.buffer = buffer;
                bind.buffer_length = length;
                bind.length = &m_blobLen[pos - 1];
                m_result.code = 0;
                m_result.error_msg.clear();
                return 0;
            }

            if (m_isQuery)
            {
                if (m_fieldCount == 0)
                {
                    m_fieldCount = mysql_stmt_field_count(m_stmt);
                    m_bindOut = new MYSQL_BIND[m_fieldCount]();
                    m_outLen = new unsigned long[m_fieldCount]();
                    m_outNull = new bool[m_fieldCount]();
                }

                if (pos < 1 || pos > m_fieldCount)
                {
                    m_result.code = -1;
                    m_result.error_msg = "bindblob failed: output position out of range";
                    return -1;
                }

                MYSQL_BIND& bind = m_bindOut[pos - 1];
                std::memset(&bind, 0, sizeof(MYSQL_BIND));
                bind.buffer_type = MYSQL_TYPE_BLOB;
                bind.buffer = buffer;
                bind.buffer_length = length;
                bind.length = &m_outLen[pos - 1];
                bind.is_null = &m_outNull[pos - 1];
                m_result.code = 0;
                m_result.error_msg.clear();
                return 0;
            }

            m_result.code = -1;
            m_result.error_msg = "bindblob failed: not input param or result field";
            return -1;
        }

        int DBStmt::filetoblob(unsigned int pos, const std::string& filename, unsigned int chunk)
        {
            if (pos < 1 || pos > m_paramCount || !m_stmt || !m_bindIn)
            {
                m_result.code = -1;
                m_result.error_msg = "filetoblob: invalid position or unbound param";
                return -1;
            }

            // 打开二进制文件
            FILE* fp = fopen(filename.c_str(), "rb");
            if (!fp)
            {
                m_result.code = -1;
                m_result.error_msg = "fopen failed: " + std::string(strerror(errno));
                return -1;
            }

            // 获取文件大小
            fseek(fp, 0, SEEK_END);
            long size = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            if (size <= 0)
            {
                fclose(fp);
                m_result.code = -1;
                m_result.error_msg = "file is empty";
                return -1;
            }

            // 限制分块大小
            if (chunk < 1024) chunk = 1024;
            if (chunk > 8 * 1024 * 1024) chunk = 8 * 1024 * 1024;

            // ===================== 绑定BLOB类型 =====================
            MYSQL_BIND& bind = m_bindIn[pos - 1];
            memset(&bind, 0, sizeof(MYSQL_BIND));
            bind.buffer_type = MYSQL_TYPE_BLOB; // BLOB类型
            bind.buffer = nullptr;
            bind.is_null = nullptr;

            // 绑定参数到语句
            if (mysql_stmt_bind_param(m_stmt, m_bindIn) != 0)
            {
                errReport();
                fclose(fp);
                return -1;
            }

            // ===================== 分块发送数据 =====================
            char* buf = new (std::nothrow) char[chunk];
            if (!buf)
            {
                fclose(fp);
                m_result.code = -1;
                m_result.error_msg = "malloc failed";
                return -1;
            }

            size_t total = 0;
            bool has_error = false;
            while (total < (size_t)size)
            {
                size_t rd = fread(buf, 1, chunk, fp);
                if (rd == 0) break;

                // 发送长二进制数据
                if (mysql_stmt_send_long_data(m_stmt, pos - 1, buf, rd) != 0)
                {
                    errReport();
                    has_error = true;
                    break;
                }

                total += rd;
            }

            // 释放缓冲区
            delete[] buf;
            fclose(fp);

            if (has_error) return -1;

            // 成功
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::blobtofile(unsigned int pos, const std::string& filename)
        {
            if (!m_isQuery || pos < 1 || pos > m_fieldCount || !m_bindOut)
            {
                m_result.code = -1;
                m_result.error_msg = "blobtofile: invalid query or position";
                return -1;
            }

            MYSQL_BIND& bind = m_bindOut[pos - 1];
            unsigned long len = *bind.length;
            char* buf = static_cast<char*>(bind.buffer);

            // 数据校验
            if (len == 0 || !buf)
            {
                m_result.code = -1;
                m_result.error_msg = "blob data is empty or buffer null";
                return -1;
            }

            if (!newdir(filename, true))
            {
                m_result.code = -1;
                m_result.error_msg = "create directory failed";
                return -1;
            }

            // 打开文件
            FILE* fp = fopen(filename.c_str(), "wb");
            if (!fp)
            {
                m_result.code = -1;
                m_result.error_msg = "fopen failed: " + std::string(strerror(errno));
                return -1;
            }

            // 写入数据
            size_t wlen = fwrite(buf, 1, len, fp);
            fclose(fp);

            if (wlen != len)
            {
                m_result.code = -1;
                m_result.error_msg = "fwrite incomplete";
                remove(filename.c_str());
                return -1;
            }

            // 成功
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        // ===================== TEXT 操作 =====================
        int DBStmt::bindtext(unsigned int pos, char* buffer, unsigned long length)
        {
            if (pos < 1 || !buffer || length == 0)
            {
                m_result.code = -1;
                m_result.error_msg = "bindtext failed: invalid parameters";
                return -1;
            }

            if (pos <= m_paramCount && m_bindIn)
            {
                MYSQL_BIND& bind = m_bindIn[pos - 1];
                std::memset(&bind, 0, sizeof(MYSQL_BIND));
                bind.buffer_type = MYSQL_TYPE_STRING;
                bind.buffer = buffer;
                bind.buffer_length = static_cast<unsigned int>(length);
                m_result.code = 0;
                m_result.error_msg.clear();
                return 0;
            }

            if (m_isQuery)
            {
                if (m_fieldCount == 0)
                {
                    m_fieldCount = mysql_stmt_field_count(m_stmt);
                    m_bindOut = new MYSQL_BIND[m_fieldCount]();
                    m_outLen = new unsigned long[m_fieldCount]();
                    m_outNull = new bool[m_fieldCount]();
                }

                if (pos > m_fieldCount)
                {
                    m_result.code = -1;
                    m_result.error_msg = "bindtext failed: output position out of range";
                    return -1;
                }

                MYSQL_BIND& bind = m_bindOut[pos - 1];
                std::memset(&bind, 0, sizeof(MYSQL_BIND));
                bind.buffer_type = MYSQL_TYPE_STRING;
                bind.buffer = buffer;
                bind.buffer_length = static_cast<unsigned int>(length);
                bind.length = &m_outLen[pos - 1];
                bind.is_null = &m_outNull[pos - 1];

                m_result.code = 0;
                m_result.error_msg.clear();
                return 0;
            }

            m_result.code = -1;
            m_result.error_msg = "bindtext failed: not input param or query result";
            return -1;
        }

        int DBStmt::bindtext(unsigned int pos, std::string& buffer, unsigned long length)
        {
            if (length == 0)
            {
                m_result.code = -1;
                m_result.error_msg = "bindtext failed: length cannot be zero";
                return -1;
            }
            buffer.resize(length);
            return bindtext(pos, buffer.data(), length);
        }

        int DBStmt::filetotext(unsigned int pos, const std::string& filename, unsigned int chunk)
        {
            if (pos < 1 || pos > m_paramCount || !m_stmt || !m_bindIn)
            {
                m_result.code = -1;
                m_result.error_msg = "filetotext: invalid position or unbound param";
                return -1;
            }

            // 打开文件
            FILE* fp = fopen(filename.c_str(), "rb");
            if (!fp)
            {
                m_result.code = -1;
                m_result.error_msg = "fopen failed: " + std::string(strerror(errno));
                return -1;
            }

            // 获取文件大小
            fseek(fp, 0, SEEK_END);
            long size = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            if (size <= 0)
            {
                fclose(fp);
                m_result.code = -1;
                m_result.error_msg = "file is empty";
                return -1;
            }

            // 限制分块大小
            if (chunk < 1024) chunk = 1024;
            if (chunk > 8 * 1024 * 1024) chunk = 8 * 1024 * 1024;

            // 绑定TEXT参数类型
            MYSQL_BIND& bind = m_bindIn[pos - 1];
            memset(&bind, 0, sizeof(MYSQL_BIND));
            bind.buffer_type = MYSQL_TYPE_STRING; // TEXT字段必须用STRING类型
            bind.buffer = nullptr;                // 分块传输无需缓冲区
            bind.is_null = nullptr;

            // 绑定参数到语句
            if (mysql_stmt_bind_param(m_stmt, m_bindIn) != 0)
            {
                errReport();
                fclose(fp);
                return -1;
            }

            // 分块传输数据
            char* buf = new (std::nothrow) char[chunk];
            if (!buf)
            {
                fclose(fp);
                m_result.code = -1;
                m_result.error_msg = "malloc failed";
                return -1;
            }

            size_t total = 0;
            bool has_error = false;
            while (total < (size_t)size)
            {
                size_t rd = fread(buf, 1, chunk, fp);
                if (rd == 0) break;

                // 发送长数据
                if (mysql_stmt_send_long_data(m_stmt, pos - 1, buf, rd) != 0)
                {
                    errReport();
                    has_error = true;
                    break;
                }

                total += rd;
            }

            // 清理资源
            delete[] buf;
            fclose(fp);

            if (has_error) return -1;

            // 成功
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::texttofile(unsigned int pos, const std::string& filename)
        {
            return blobtofile(pos, filename);
        }

        // ===================== 工具方法 =====================
        bool DBStmt::isNull(unsigned int pos)
        {
            if (pos < 1 || pos > m_fieldCount || !m_outNull) return true;
            return m_outNull[pos - 1];
        }

        unsigned long DBStmt::length(unsigned int pos)
        {
            if (pos < 1 || pos > m_fieldCount || !m_outLen) return 0;
            return m_outLen[pos - 1];
        }

        const char* DBStmt::sql() const { return m_sql.c_str(); }
        int DBStmt::code() const { return m_result.code; }
        size_t DBStmt::affectedRows() const { return m_result.affected_rows; }
        std::string DBStmt::errorMsg() const { return m_result.error_msg; }

    } // namespace mysql
} // namespace ol