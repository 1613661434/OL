/**************************************************************************************/
/*
 * 程序名：ol_mysql.cpp
 * 功能描述：开发框架中C++操作MySQL数据库的实现文件，对应ol_mysql.h的接口实现，包括：
 *          - connection类：数据库连接、事务管理等方法的具体实现
 *          - sqlstatement类：SQL语句准备、变量绑定、执行及结果集处理的实现
 *          - 底层MySQL C API调用封装，处理错误信息及内存管理
 * 作者：ol
 * 依赖：MySQL C API库（需正确配置MySQL环境及链接库）
 * 适用标准：C++11及以上
 */
/**************************************************************************************/

#include "ol_mysql.h"

namespace ol
{

    // 辅助函数实现
    void MY__ToUpper(char* str)
    {
        if (str == nullptr) return;
        if (strlen(str) == 0) return;

        int istrlen = strlen(str);
        for (int i = 0; i < istrlen; ++i)
        {
            if ((str[i] >= 'a') && (str[i] <= 'z'))
                str[i] = str[i] - 32;
        }
    }

    void MY__DeleteLChar(char* str, const char chr)
    {
        if (str == nullptr) return;
        if (strlen(str) == 0) return;

        char strTemp[strlen(str) + 1];
        int iTemp = 0;

        memset(strTemp, 0, sizeof(strTemp));
        strcpy(strTemp, str);

        while (strTemp[iTemp] == chr)
            ++iTemp;

        memset(str, 0, strlen(str) + 1);
        strcpy(str, strTemp + iTemp);
    }

    // 数据库操作结果结构体的相关实现（无额外实现，结构体仅作数据存储）
    // ===========================================================================

    // CDA_DEF结构体实现
    // ===========================================================================
    void CDA_DEF::init()
    {
        rc = 0;
        rpc = 0;
        message.clear();
    }
    // ===========================================================================

    // connection类实现
    // ===========================================================================
    connection::connection() : m_mysql(nullptr), m_autocommitopt(0), m_state(disconnected)
    {
        m_cda.init();
        m_cda.rc = -1;
        m_cda.message = "database not open.";
        m_port = 3306; // MySQL默认端口
    }

    connection::~connection()
    {
        disconnect();
    }

    int connection::connecttodb(const std::string& connstr, const std::string& charset, bool autocommitopt)
    {
        // 如果已连接上数据库，就不再连接
        if (m_state == connected) return 0;

        m_cda.init();

        // 解析连接字符串
        setdbopt(connstr.c_str());

        // 初始化MySQL连接
        m_mysql = mysql_init(nullptr);
        if (m_mysql == nullptr)
        {
            m_cda.rc = -1;
            m_cda.message = "mysql_init failed.";
            return -1;
        }

        // 连接数据库
        if (!mysql_real_connect(m_mysql, m_host.c_str(), m_user.c_str(), m_pass.c_str(),
                                m_dbname.c_str(), m_port, m_unix_socket.empty() ? nullptr : m_unix_socket.c_str(), 0))
        {
            m_cda.rc = mysql_errno(m_mysql);
            m_cda.message = mysql_error(m_mysql);
            mysql_close(m_mysql);
            m_mysql = nullptr;
            return -1;
        }

        // 设置字符集
        if (!charset.empty())
        {
            if (mysql_set_character_set(m_mysql, charset.c_str()) != 0)
            {
                m_cda.rc = mysql_errno(m_mysql);
                m_cda.message = mysql_error(m_mysql);
                mysql_close(m_mysql);
                m_mysql = nullptr;
                return -1;
            }
        }

        // 设置自动提交
        m_autocommitopt = autocommitopt ? 1 : 0;
        mysql_autocommit(m_mysql, m_autocommitopt);

        m_state = connected;
        m_cda.rc = 0;
        m_cda.message[0] = '\0';

        return 0;
    }

    bool connection::isopen()
    {
        return (m_state == connected);
    }

    int connection::commit()
    {
        m_cda.init();

        if (m_state == disconnected)
        {
            m_cda.rc = -1;
            m_cda.message = "database not open.";
            return -1;
        }

        if (mysql_commit(m_mysql) != 0)
        {
            err_report();
            return m_cda.rc;
        }

        return 0;
    }

    int connection::rollback()
    {
        m_cda.init();

        if (m_state == disconnected)
        {
            m_cda.rc = -1;
            m_cda.message = "database not open.";
            return -1;
        }

        if (mysql_rollback(m_mysql) != 0)
        {
            err_report();
            return m_cda.rc;
        }

        return 0;
    }

    int connection::disconnect()
    {
        m_cda.init();

        if (m_state == disconnected)
        {
            m_cda.rc = -1;
            m_cda.message = "database not open.";
            return -1;
        }

        // 未提交事务自动回滚
        if (m_autocommitopt == 0)
        {
            rollback();
        }

        mysql_close(m_mysql);
        m_mysql = nullptr;
        m_state = disconnected;

        return 0;
    }

    int connection::execute(const char* fmt, ...)
    {
        va_list ap;
        va_start(ap, fmt);
        int len = vsnprintf(nullptr, 0, fmt, ap);
        va_end(ap);
        if (len <= 0) return -1;

        va_start(ap, fmt);
        std::string strsql;
        strsql.resize(len);
        vsnprintf(&strsql[0], len + 1, fmt, ap);
        va_end(ap);

        sqlstatement stmt(this);
        return stmt.execute(strsql.c_str());
    }

    int connection::rc()
    {
        return m_cda.rc;
    }

    unsigned long connection::rpc()
    {
        return m_cda.rpc;
    }

    std::string connection::message()
    {
        return m_cda.message;
    }

    // connection类私有方法
    void connection::character(const char* charset)
    {
        if (charset == nullptr || m_mysql == nullptr) return;
        mysql_set_character_set(m_mysql, charset);
    }

    void connection::setdbopt(const char* connstr)
    {
        // 格式："username:password@host:port/dbname"
        std::string str = connstr;
        size_t pos1 = str.find(':');
        size_t pos2 = str.find('@');
        size_t pos3 = str.find(':');
        size_t pos4 = str.find('/');

        if (pos1 != std::string::npos && pos1 < pos2)
        {
            m_user = str.substr(0, pos1);
        }

        if (pos2 != std::string::npos && pos1 < pos2)
        {
            m_pass = str.substr(pos1 + 1, pos2 - pos1 - 1);
        }

        std::string host_part;
        if (pos4 != std::string::npos && pos2 < pos4)
        {
            host_part = str.substr(pos2 + 1, pos4 - pos2 - 1);
            m_dbname = str.substr(pos4 + 1);
        }
        else if (pos2 != std::string::npos)
        {
            host_part = str.substr(pos2 + 1);
        }

        if (!host_part.empty())
        {
            pos3 = host_part.find(':');
            if (pos3 != std::string::npos)
            {
                m_host = host_part.substr(0, pos3);
                std::string port_str = host_part.substr(pos3 + 1);
                m_port = atoi(port_str.c_str());
            }
            else
            {
                m_host = host_part;
            }
        }

        // 设置默认值
        if (m_host.empty()) m_host = "localhost";
    }

    void connection::err_report()
    {
        if (m_state == disconnected)
        {
            m_cda.rc = -1;
            m_cda.message = "database not open.";
            return;
        }

        m_cda.rc = mysql_errno(m_mysql);
        m_cda.message = mysql_error(m_mysql);
    }
    // ===========================================================================

    // sqlstatement类实现
    // ===========================================================================
    sqlstatement::sqlstatement() : m_mysql(nullptr), m_stmt(nullptr), m_result(nullptr),
                                   m_bind(nullptr), m_param_count(0), m_field_count(0), m_conn(nullptr),
                                   m_sqltype(true), m_autocommitopt(false), m_blob_buffer(nullptr),
                                   m_blob_length(0), m_state(disconnected)
    {
        m_cda.init();
        m_cda.rc = -1;
        m_cda.message = "sqlstatement not connect to connection.";
    }

    sqlstatement::sqlstatement(connection* conn) : m_mysql(nullptr), m_stmt(nullptr), m_result(nullptr),
                                                   m_bind(nullptr), m_param_count(0), m_field_count(0), m_conn(nullptr),
                                                   m_sqltype(true), m_autocommitopt(false), m_blob_buffer(nullptr),
                                                   m_blob_length(0), m_state(disconnected)
    {
        m_cda.init();
        m_cda.rc = -1;
        m_cda.message = "sqlstatement not connect to connection.";
        connect(conn);
    }

    sqlstatement::~sqlstatement()
    {
        disconnect();
        free_bind();
        if (m_blob_buffer)
        {
            delete[] m_blob_buffer;
            m_blob_buffer = nullptr;
        }
    }

    int sqlstatement::connect(connection* conn)
    {
        // 注意，一个sqlstatement在程序中只能指定一个connection，不允许指定多个connection
        // 所以，只要这个sqlstatement已指定connection，直接返回成功
        if (m_state == connected) return 0;

        m_cda.init();

        m_conn = conn;

        // 如果数据库连接对象的指针为空，直接返回失败
        if (m_conn == nullptr)
        {
            m_cda.rc = -1;
            m_cda.message = "database not open.";
            return -1;
        }

        // 如果数据库连接不可用，直接返回失败
        if (m_conn->m_state == connection::disconnected)
        {
            m_cda.rc = -1;
            m_cda.message = "database not open.";
            return -1;
        }

        m_mysql = m_conn->m_mysql;
        m_autocommitopt = m_conn->m_autocommitopt;

        // 创建语句句柄
        m_stmt = mysql_stmt_init(m_mysql);
        if (m_stmt == nullptr)
        {
            err_report();
            return m_cda.rc;
        }

        m_state = connected;
        m_cda.rc = 0;
        m_cda.message[0] = '\0';

        return 0;
    }

    bool sqlstatement::isopen()
    {
        return (m_state == connected);
    }

    int sqlstatement::disconnect()
    {
        if (m_state == disconnected) return 0;

        m_cda.init();

        // 释放结果集
        if (m_result)
        {
            mysql_free_result(m_result);
            m_result = nullptr;
        }

        // 释放语句句柄
        if (m_stmt)
        {
            mysql_stmt_close(m_stmt);
            m_stmt = nullptr;
        }

        free_bind();

        m_state = disconnected;
        m_mysql = nullptr;
        m_conn = nullptr;

        m_cda.rc = -1;
        m_cda.message = "cursor not open.";
        return 0;
    }

    int sqlstatement::prepare(const std::string& strsql)
    {
        return prepare(strsql.c_str());
    }

    int sqlstatement::prepare(const char* fmt, ...)
    {
        m_cda.init();

        if (m_state == disconnected)
        {
            m_cda.rc = -1;
            m_cda.message = "cursor not open.";
            return -1;
        }

        // 释放之前的绑定和结果集
        free_bind();
        if (m_result)
        {
            mysql_free_result(m_result);
            m_result = nullptr;
        }

        // 构建SQL语句
        m_sql.clear();
        va_list ap;
        va_start(ap, fmt);
        int len = vsnprintf(nullptr, 0, fmt, ap);
        va_end(ap);
        if (len <= 0)
        {
            m_cda.rc = -1;
            m_cda.message = "Invalid SQL format.";
            return -1;
        }

        va_start(ap, fmt);
        m_sql.resize(len);
        vsnprintf(&m_sql[0], len + 1, fmt, ap);
        va_end(ap);

        // 准备SQL语句
        if (mysql_stmt_prepare(m_stmt, m_sql.c_str(), m_sql.length()) != 0)
        {
            err_report();
            return m_cda.rc;
        }

        // 获取参数数量
        m_param_count = mysql_stmt_param_count(m_stmt);
        if (m_param_count > 0)
        {
            m_bind = new MYSQL_BIND[m_param_count];
            memset(m_bind, 0, sizeof(MYSQL_BIND) * m_param_count);
        }

        // 判断是否是查询语句
        m_sqltype = true;
        char strtemp[31];
        memset(strtemp, 0, sizeof(strtemp));
        strncpy(strtemp, m_sql.c_str(), 30);
        MY__ToUpper(strtemp);
        MY__DeleteLChar(strtemp, ' ');
        if (strncmp(strtemp, "SELECT", 6) == 0 ||
            strncmp(strtemp, "SHOW", 4) == 0 ||
            strncmp(strtemp, "DESCRIBE", 8) == 0 ||
            strncmp(strtemp, "EXPLAIN", 7) == 0)
        {
            m_sqltype = false;
        }

        m_cda.rc = 0;
        return 0;
    }

    // 绑定输入变量系列方法
    int sqlstatement::bindin(const unsigned int position, int& value)
    {
        if (position == 0 || position > m_param_count || m_bind == nullptr)
            return -1;

        MYSQL_BIND* bind = &m_bind[position - 1];
        bind->buffer_type = MYSQL_TYPE_LONG;
        bind->buffer = &value;
        bind->is_unsigned = false;
        bind->length = nullptr;
        bind->is_null = nullptr;

        return 0;
    }

    int sqlstatement::bindin(const unsigned int position, long& value)
    {
        if (position == 0 || position > m_param_count || m_bind == nullptr)
            return -1;

        MYSQL_BIND* bind = &m_bind[position - 1];
        bind->buffer_type = MYSQL_TYPE_LONGLONG;
        bind->buffer = &value;
        bind->is_unsigned = false;
        bind->length = nullptr;
        bind->is_null = nullptr;

        return 0;
    }

    int sqlstatement::bindin(const unsigned int position, unsigned int& value)
    {
        if (position == 0 || position > m_param_count || m_bind == nullptr)
            return -1;

        MYSQL_BIND* bind = &m_bind[position - 1];
        bind->buffer_type = MYSQL_TYPE_LONG;
        bind->buffer = &value;
        bind->is_unsigned = true;
        bind->length = nullptr;
        bind->is_null = nullptr;

        return 0;
    }

    int sqlstatement::bindin(const unsigned int position, unsigned long& value)
    {
        if (position == 0 || position > m_param_count || m_bind == nullptr)
            return -1;

        MYSQL_BIND* bind = &m_bind[position - 1];
        bind->buffer_type = MYSQL_TYPE_LONGLONG;
        bind->buffer = &value;
        bind->is_unsigned = true;
        bind->length = nullptr;
        bind->is_null = nullptr;

        return 0;
    }

    int sqlstatement::bindin(const unsigned int position, float& value)
    {
        if (position == 0 || position > m_param_count || m_bind == nullptr)
            return -1;

        MYSQL_BIND* bind = &m_bind[position - 1];
        bind->buffer_type = MYSQL_TYPE_FLOAT;
        bind->buffer = &value;
        bind->length = nullptr;
        bind->is_null = nullptr;

        return 0;
    }

    int sqlstatement::bindin(const unsigned int position, double& value)
    {
        if (position == 0 || position > m_param_count || m_bind == nullptr)
            return -1;

        MYSQL_BIND* bind = &m_bind[position - 1];
        bind->buffer_type = MYSQL_TYPE_DOUBLE;
        bind->buffer = &value;
        bind->length = nullptr;
        bind->is_null = nullptr;

        return 0;
    }

    int sqlstatement::bindin(const unsigned int position, char* value, unsigned int len)
    {
        if (position == 0 || position > m_param_count || m_bind == nullptr || value == nullptr)
            return -1;

        MYSQL_BIND* bind = &m_bind[position - 1];
        bind->buffer_type = MYSQL_TYPE_STRING;
        bind->buffer = value;
        bind->buffer_length = len;
        bind->length = nullptr;
        bind->is_null = nullptr;

        return 0;
    }

    int sqlstatement::bindin(const unsigned int position, std::string& value, unsigned int len)
    {
        value.resize(len);
        return bindin(position, &value[0], len);
    }

    int sqlstatement::bindin1(const unsigned int position, std::string& value)
    {
        return bindin(position, &value[0], value.size());
    }

    // 绑定输出变量系列方法
    int sqlstatement::bindout(const unsigned int position, int& value)
    {
        if (m_field_count == 0)
        {
            m_field_count = mysql_stmt_field_count(m_stmt);
            if (m_field_count > 0 && m_bind == nullptr)
            {
                m_bind = new MYSQL_BIND[m_field_count];
                memset(m_bind, 0, sizeof(MYSQL_BIND) * m_field_count);
            }
        }

        if (position == 0 || position > m_field_count || m_bind == nullptr)
            return -1;

        MYSQL_BIND* bind = &m_bind[position - 1];
        bind->buffer_type = MYSQL_TYPE_LONG;
        bind->buffer = &value;
        bind->is_unsigned = false;
        bind->length = nullptr;
        bind->is_null = nullptr;

        return mysql_stmt_bind_result(m_stmt, m_bind);
    }

    int sqlstatement::bindout(const unsigned int position, long& value)
    {
        if (m_field_count == 0)
        {
            m_field_count = mysql_stmt_field_count(m_stmt);
            if (m_field_count > 0 && m_bind == nullptr)
            {
                m_bind = new MYSQL_BIND[m_field_count];
                memset(m_bind, 0, sizeof(MYSQL_BIND) * m_field_count);
            }
        }

        if (position == 0 || position > m_field_count || m_bind == nullptr)
            return -1;

        MYSQL_BIND* bind = &m_bind[position - 1];
        bind->buffer_type = MYSQL_TYPE_LONGLONG;
        bind->buffer = &value;
        bind->is_unsigned = false;
        bind->length = nullptr;
        bind->is_null = nullptr;

        return mysql_stmt_bind_result(m_stmt, m_bind);
    }

    int sqlstatement::bindout(const unsigned int position, unsigned int& value)
    {
        if (m_field_count == 0)
        {
            m_field_count = mysql_stmt_field_count(m_stmt);
            if (m_field_count > 0 && m_bind == nullptr)
            {
                m_bind = new MYSQL_BIND[m_field_count];
                memset(m_bind, 0, sizeof(MYSQL_BIND) * m_field_count);
            }
        }

        if (position == 0 || position > m_field_count || m_bind == nullptr)
            return -1;

        MYSQL_BIND* bind = &m_bind[position - 1];
        bind->buffer_type = MYSQL_TYPE_LONG;
        bind->buffer = &value;
        bind->is_unsigned = true;
        bind->length = nullptr;
        bind->is_null = nullptr;

        return mysql_stmt_bind_result(m_stmt, m_bind);
    }

    int sqlstatement::bindout(const unsigned int position, unsigned long& value)
    {
        if (m_field_count == 0)
        {
            m_field_count = mysql_stmt_field_count(m_stmt);
            if (m_field_count > 0 && m_bind == nullptr)
            {
                m_bind = new MYSQL_BIND[m_field_count];
                memset(m_bind, 0, sizeof(MYSQL_BIND) * m_field_count);
            }
        }

        if (position == 0 || position > m_field_count || m_bind == nullptr)
            return -1;

        MYSQL_BIND* bind = &m_bind[position - 1];
        bind->buffer_type = MYSQL_TYPE_LONGLONG;
        bind->buffer = &value;
        bind->is_unsigned = true;
        bind->length = nullptr;
        bind->is_null = nullptr;

        return mysql_stmt_bind_result(m_stmt, m_bind);
    }

    int sqlstatement::bindout(const unsigned int position, float& value)
    {
        if (m_field_count == 0)
        {
            m_field_count = mysql_stmt_field_count(m_stmt);
            if (m_field_count > 0 && m_bind == nullptr)
            {
                m_bind = new MYSQL_BIND[m_field_count];
                memset(m_bind, 0, sizeof(MYSQL_BIND) * m_field_count);
            }
        }

        if (position == 0 || position > m_field_count || m_bind == nullptr)
            return -1;

        MYSQL_BIND* bind = &m_bind[position - 1];
        bind->buffer_type = MYSQL_TYPE_FLOAT;
        bind->buffer = &value;
        bind->length = nullptr;
        bind->is_null = nullptr;

        return mysql_stmt_bind_result(m_stmt, m_bind);
    }

    int sqlstatement::bindout(const unsigned int position, double& value)
    {
        if (m_field_count == 0)
        {
            m_field_count = mysql_stmt_field_count(m_stmt);
            if (m_field_count > 0 && m_bind == nullptr)
            {
                m_bind = new MYSQL_BIND[m_field_count];
                memset(m_bind, 0, sizeof(MYSQL_BIND) * m_field_count);
            }
        }

        if (position == 0 || position > m_field_count || m_bind == nullptr)
            return -1;

        MYSQL_BIND* bind = &m_bind[position - 1];
        bind->buffer_type = MYSQL_TYPE_DOUBLE;
        bind->buffer = &value;
        bind->length = nullptr;
        bind->is_null = nullptr;

        return mysql_stmt_bind_result(m_stmt, m_bind);
    }

    int sqlstatement::bindout(const unsigned int position, char* value, unsigned int len)
    {
        if (m_field_count == 0)
        {
            m_field_count = mysql_stmt_field_count(m_stmt);
            if (m_field_count > 0 && m_bind == nullptr)
            {
                m_bind = new MYSQL_BIND[m_field_count];
                memset(m_bind, 0, sizeof(MYSQL_BIND) * m_field_count);
            }
        }

        if (position == 0 || position > m_field_count || m_bind == nullptr || value == nullptr)
            return -1;

        MYSQL_BIND* bind = &m_bind[position - 1];
        bind->buffer_type = MYSQL_TYPE_STRING;
        bind->buffer = value;
        bind->buffer_length = len;
        bind->length = nullptr;
        bind->is_null = nullptr;

        return mysql_stmt_bind_result(m_stmt, m_bind);
    }

    int sqlstatement::bindout(const unsigned int position, std::string& value, unsigned int len)
    {
        value.resize(len);
        return bindout(position, &value[0], len);
    }

    int sqlstatement::execute()
    {
        m_cda.init();

        if (m_state == disconnected)
        {
            m_cda.rc = -1;
            m_cda.message = "cursor not open.";
            return -1;
        }

        // 绑定参数
        if (m_param_count > 0 && m_bind != nullptr)
        {
            if (mysql_stmt_bind_param(m_stmt, m_bind) != 0)
            {
                err_report();
                return m_cda.rc;
            }
        }

        // 执行语句
        if (mysql_stmt_execute(m_stmt) != 0)
        {
            err_report();
            return m_cda.rc;
        }

        // 如果是查询语句，准备结果集并统计行数
        if (!m_sqltype)
        {
            if (mysql_stmt_store_result(m_stmt) != 0)
            {
                err_report();
                return m_cda.rc;
            }
            // 查询语句用mysql_stmt_num_rows获取结果集行数
            m_cda.rpc = mysql_stmt_num_rows(m_stmt);
            m_conn->m_cda.rpc = m_cda.rpc;
        }
        // 如果是非查询语句，获取影响记录的行数，且自动提交
        else if (m_autocommitopt)
        {
            m_cda.rpc = mysql_stmt_affected_rows(m_stmt);
            m_conn->m_cda.rpc = m_cda.rpc;
            m_conn->commit();
        }

        return 0;
    }

    int sqlstatement::execute(const char* fmt, ...)
    {
        std::string strtmp;

        va_list ap;
        va_start(ap, fmt);
        int len = vsnprintf(nullptr, 0, fmt, ap);
        va_end(ap);
        if (len <= 0)
        {
            m_cda.rc = -1;
            m_cda.message = "Invalid SQL format.";
            return -1;
        }

        va_start(ap, fmt);
        strtmp.resize(len);
        vsnprintf(&strtmp[0], len + 1, fmt, ap);
        va_end(ap);

        if (prepare(strtmp.c_str()) != 0)
            return m_cda.rc;

        return execute();
    }

    int sqlstatement::next()
    {
        if (m_state == disconnected)
        {
            m_cda.rc = -1;
            m_cda.message = "cursor not open.";
            return -1;
        }

        // 如果语句未执行成功，直接返回失败
        if (m_cda.rc != 0)
            return m_cda.rc;

        // 判断是否是查询语句，如果不是，直接返回错误
        if (m_sqltype)
        {
            m_cda.rc = -1;
            m_cda.message = "no recordset found.";
            return -1;
        }

        // 获取下一行
        int ret = mysql_stmt_fetch(m_stmt);
        if (ret == 0)
        {
            m_cda.rc = 0;
            return 0;
        }
        else if (ret == MYSQL_NO_DATA)
        {
            m_cda.rc = 100; // 100表示无更多记录
            return 100;
        }
        else
        {
            err_report();
            return m_cda.rc;
        }
    }

    int sqlstatement::bindblob(const unsigned int position, char* buffer, unsigned long length)
    {
        if (position == 0 || position > m_param_count || m_bind == nullptr || buffer == nullptr)
            return -1;

        // 保存BLOB数据指针和长度
        m_blob_buffer = buffer;
        m_blob_length = length;

        MYSQL_BIND* bind = &m_bind[position - 1];
        bind->buffer_type = MYSQL_TYPE_BLOB;
        bind->buffer = buffer;
        bind->buffer_length = length;
        bind->length = &m_blob_length;
        bind->is_null = nullptr;

        return 0;
    }

    int sqlstatement::filetoblob(const std::string& filename)
    {
        FILE* fp = fopen(filename.c_str(), "rb");
        if (fp == nullptr)
        {
            m_cda.rc = -1;
            m_cda.message = "fopen failed.";
            return -1;
        }

        // 获取文件大小
        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        if (file_size <= 0)
        {
            fclose(fp);
            return 0;
        }

        // 分配缓冲区
        char* buffer = new char[file_size];
        if (buffer == nullptr)
        {
            fclose(fp);
            m_cda.rc = -1;
            m_cda.message = "memory allocation failed.";
            return -1;
        }

        // 读取文件内容
        size_t bytes_read = fread(buffer, 1, file_size, fp);
        fclose(fp);

        if (bytes_read != (size_t)file_size)
        {
            delete[] buffer;
            m_cda.rc = -1;
            m_cda.message = "fread failed.";
            return -1;
        }

        // 绑定BLOB数据（假设绑定到第一个参数）
        int ret = bindblob(1, buffer, file_size);
        if (ret != 0)
        {
            delete[] buffer;
            return ret;
        }

        // 执行语句
        ret = execute();

        delete[] buffer;
        return ret;
    }

    int sqlstatement::blobtofile(const std::string& filename)
    {
        if (m_sqltype || m_result == nullptr)
        {
            m_cda.rc = -1;
            m_cda.message = "no blob data available.";
            return -1;
        }

        // 假设BLOB字段是结果集中的第一个字段
        MYSQL_FIELD* fields = mysql_fetch_fields(m_result);
        if (fields == nullptr || fields[0].type != MYSQL_TYPE_BLOB)
        {
            m_cda.rc = -1;
            m_cda.message = "no blob field found.";
            return -1;
        }

        MYSQL_ROW row = mysql_fetch_row(m_result);
        if (row == nullptr)
        {
            m_cda.rc = -1;
            m_cda.message = "no data found.";
            return -1;
        }

        unsigned long* lengths = mysql_fetch_lengths(m_result);
        if (lengths == nullptr || lengths[0] == 0)
            return 0;

        FILE* fp = fopen(filename.c_str(), "wb");
        if (fp == nullptr)
        {
            m_cda.rc = -1;
            m_cda.message = "fopen failed.";
            return -1;
        }

        size_t bytes_written = fwrite(row[0], 1, lengths[0], fp);
        fclose(fp);

        if (bytes_written != lengths[0])
        {
            m_cda.rc = -1;
            m_cda.message = "fwrite failed.";
            remove(filename.c_str()); // 删除不完整文件
            return -1;
        }

        return 0;
    }

    int sqlstatement::bindtext(const unsigned int position, char* buffer, unsigned long length)
    {
        if (position == 0 || position > m_param_count || m_bind == nullptr || buffer == nullptr)
            return -1;

        MYSQL_BIND* bind = &m_bind[position - 1];
        bind->buffer_type = MYSQL_TYPE_STRING; // TEXT使用字符串类型
        bind->buffer = buffer;
        bind->buffer_length = length;
        bind->length = &length;
        bind->is_null = nullptr;

        return 0;
    }

    int sqlstatement::bindtext(const unsigned int position, std::string& buffer, unsigned long length)
    {
        buffer.resize(length);
        return bindtext(position, &buffer[0], length);
    }

    int sqlstatement::filetotext(const unsigned int position, const std::string& filename)
    {
        FILE* fp = fopen(filename.c_str(), "r");
        if (fp == nullptr)
        {
            m_cda.rc = -1;
            m_cda.message = "fopen failed for " + filename + ": " + strerror(errno);
            return -1;
        }

        // 获取文件大小
        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        if (file_size <= 0)
        {
            fclose(fp);
            return 0; // 空文件直接返回成功
        }

        // 分配缓冲区（+1用于存放字符串结束符）
        char* buffer = new char[file_size + 1];
        if (buffer == nullptr)
        {
            fclose(fp);
            m_cda.rc = -1;
            m_cda.message = "memory allocation failed.";
            return -1;
        }

        // 读取文件内容
        size_t bytes_read = fread(buffer, 1, file_size, fp);
        fclose(fp);

        if (bytes_read != (size_t)file_size)
        {
            delete[] buffer;
            m_cda.rc = -1;
            m_cda.message = "fread failed.";
            return -1;
        }
        buffer[file_size] = '\0'; // 确保字符串结束

        // 绑定TEXT数据
        int ret = bindtext(position, buffer, file_size);
        if (ret != 0)
        {
            delete[] buffer;
            return ret;
        }

        // 执行语句
        ret = execute();

        delete[] buffer;
        return ret;
    }

    int sqlstatement::texttofile(const unsigned int position, const std::string& filename)
    {
        if (m_sqltype) // 检查是否为查询语句
        {
            m_cda.rc = -1;
            m_cda.message = "not a query statement.";
            return -1;
        }

        // 获取结果集元数据
        MYSQL_RES* meta_result = mysql_stmt_result_metadata(m_stmt);
        if (meta_result == nullptr)
        {
            err_report();
            return m_cda.rc;
        }

        // 检查字段类型是否为TEXT
        MYSQL_FIELD* fields = mysql_fetch_fields(meta_result);
        if (position >= mysql_num_fields(meta_result) ||
            (fields[position].type != MYSQL_TYPE_STRING &&
             fields[position].type != MYSQL_TYPE_VAR_STRING &&
             fields[position].type != MYSQL_TYPE_BLOB))
        {
            mysql_free_result(meta_result);
            m_cda.rc = -1;
            m_cda.message = "invalid TEXT field position.";
            return -1;
        }
        mysql_free_result(meta_result);

        // 绑定输出变量
        char* buffer = nullptr;
        unsigned long text_length = 0;
        MYSQL_BIND bind;
        memset(&bind, 0, sizeof(bind));
        bind.buffer_type = MYSQL_TYPE_STRING;
        bind.length = &text_length;

        // 第一次获取长度
        if (mysql_stmt_fetch(m_stmt) != 0)
        {
            err_report();
            return m_cda.rc;
        }

        // 分配缓冲区
        if (text_length > 0)
        {
            buffer = new char[text_length + 1];
            bind.buffer = buffer;
            bind.buffer_length = text_length + 1;

            // 重新绑定并获取数据
            mysql_stmt_bind_result(m_stmt, &bind);
            mysql_stmt_fetch(m_stmt);
            buffer[text_length] = '\0';
        }

        // 写入文件
        FILE* fp = fopen(filename.c_str(), "w");
        if (fp == nullptr)
        {
            delete[] buffer;
            m_cda.rc = -1;
            m_cda.message = "fopen failed for " + filename + ": " + strerror(errno);
            return -1;
        }

        if (text_length > 0)
        {
            fwrite(buffer, 1, text_length, fp);
        }
        fclose(fp);
        delete[] buffer;

        return 0;
    }

    const char* sqlstatement::sql()
    {
        return m_sql.c_str();
    }

    int sqlstatement::rc()
    {
        return m_cda.rc;
    }

    unsigned long sqlstatement::rpc()
    {
        return m_cda.rpc;
    }

    std::string sqlstatement::message()
    {
        return m_cda.message;
    }

    // sqlstatement类私有方法
    void sqlstatement::free_bind()
    {
        if (m_bind)
        {
            delete[] m_bind;
            m_bind = nullptr;
        }
        m_param_count = 0;
        m_field_count = 0;
    }

    void sqlstatement::err_report()
    {
        if (m_state == disconnected)
        {
            m_cda.rc = -1;
            m_cda.message = "cursor not open."; // 直接赋值，无需担心缓冲区
            return;
        }

        m_cda.rc = mysql_stmt_errno(m_stmt);
        m_cda.message = mysql_stmt_error(m_stmt); // 直接赋值C字符串，自动转换

        m_conn->m_cda.rc = m_cda.rc;
        m_conn->m_cda.message = m_cda.message; // string之间直接赋值
    }
    // ===========================================================================

} // namespace ol