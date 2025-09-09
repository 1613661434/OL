/**************************************************************************************/
/*
 * 程序名：ol_mysql.cpp
 * 功能描述：开发框架中C++操作MySQL数据库的实现文件
 * 作者：ol
 * 依赖：MySQL C API库
 * 适用标准：C++11及以上
 */
/**************************************************************************************/

#include "ol_mysql.h"

#define DEBUG

namespace ol
{
    void MY__ToUpper(char* str)
    {
        if (str == nullptr) return;
        for (size_t i = 0; i < strlen(str); ++i)
        {
            if ((str[i] >= 'a') && (str[i] <= 'z'))
                str[i] = str[i] - 32;
        }
    }

    void MY__DeleteLChar(char* str, const char chr)
    {
        if (str == nullptr || *str == '\0') return;

        size_t i = 0;
        while (str[i] == chr) ++i;

        if (i > 0) memmove(str, str + i, strlen(str) - i + 1);
    }

    void CDA_DEF::init()
    {
        rc = 0;
        rpc = 0;
        message.clear();
    }

    // connection类实现
    connection::connection() : m_mysql(nullptr), m_autocommitopt(0), m_state(disconnected)
    {
        m_cda.init();
        m_cda.rc = -1;
        m_cda.message = "database not open.";
        m_port = 3306;
    }

    connection::~connection()
    {
        disconnect();
    }

    int connection::connecttodb(const std::string& connstr, const std::string& charset, bool autocommitopt)
    {
        if (m_state == connected) return 0;

        m_cda.init();
        setdbopt(connstr.c_str());

        m_mysql = mysql_init(nullptr);
        if (m_mysql == nullptr)
        {
            m_cda.rc = -1;
            m_cda.message = "mysql_init failed.";
            return -1;
        }

        // 关键修复：添加MYSQL_OPT_RECONNECT选项，确保连接稳定性
        bool reconnect = 1;
        mysql_options(m_mysql, MYSQL_OPT_RECONNECT, &reconnect);

        if (!mysql_real_connect(m_mysql, m_host.c_str(), m_user.c_str(), m_pass.c_str(),
                                m_dbname.c_str(), m_port, m_unix_socket.empty() ? nullptr : m_unix_socket.c_str(), 0))
        {
            m_cda.rc = mysql_errno(m_mysql);
            m_cda.message = mysql_error(m_mysql);
            mysql_close(m_mysql);
            m_mysql = nullptr;
            return -1;
        }

        // 对于二进制数据，强制使用binary字符集
        std::string use_charset = charset.empty() ? "binary" : charset;
        if (mysql_set_character_set(m_mysql, use_charset.c_str()) != 0)
        {
            m_cda.rc = mysql_errno(m_mysql);
            m_cda.message = mysql_error(m_mysql);
            mysql_close(m_mysql);
            m_mysql = nullptr;
            return -1;
        }

        m_autocommitopt = autocommitopt ? 1 : 0;
        mysql_autocommit(m_mysql, m_autocommitopt);

        m_state = connected;
        m_cda.rc = 0;
        m_cda.message.clear();

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
        strsql.resize(len + 1); // 增加1个字节确保null终止符
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

    void connection::character(const char* charset)
    {
        if (charset && m_mysql)
            mysql_set_character_set(m_mysql, charset);
    }

    void connection::setdbopt(const char* connstr)
    {
        std::string str = connstr;
        size_t pos1 = str.find(':');
        size_t pos2 = str.find('@');
        size_t pos3 = str.find(':');
        size_t pos4 = str.find('/');

        if (pos1 != std::string::npos && pos1 < pos2)
            m_user = str.substr(0, pos1);

        if (pos2 != std::string::npos && pos1 < pos2)
            m_pass = str.substr(pos1 + 1, pos2 - pos1 - 1);

        std::string host_part;
        if (pos4 != std::string::npos && pos2 < pos4)
        {
            host_part = str.substr(pos2 + 1, pos4 - pos2 - 1);
            m_dbname = str.substr(pos4 + 1);
        }
        else if (pos2 != std::string::npos)
            host_part = str.substr(pos2 + 1);

        if (!host_part.empty())
        {
            pos3 = host_part.find(':');
            if (pos3 != std::string::npos)
            {
                m_host = host_part.substr(0, pos3);
                m_port = atoi(host_part.substr(pos3 + 1).c_str());
            }
            else
                m_host = host_part;
        }

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

    // sqlstatement类实现
    sqlstatement::sqlstatement() : m_mysql(nullptr), m_stmt(nullptr), m_result(nullptr),
                                   m_bindin(nullptr), m_bindout(nullptr),
                                   m_param_count(0), m_field_count(0),
                                   m_out_lengths(nullptr), m_out_is_null(nullptr),
                                   m_blob_lengths(nullptr), // 初始化新增成员
                                   m_conn(nullptr), m_sqltype(true),
                                   m_autocommitopt(false), m_state(disconnected)
    {
        m_cda.init();
        m_cda.rc = -1;
        m_cda.message = "sqlstatement not connect to connection.";
    }

    sqlstatement::sqlstatement(connection* conn) : m_mysql(nullptr), m_stmt(nullptr), m_result(nullptr),
                                                   m_bindin(nullptr), m_bindout(nullptr),
                                                   m_param_count(0), m_field_count(0),
                                                   m_out_lengths(nullptr), m_out_is_null(nullptr),
                                                   m_blob_lengths(nullptr), // 初始化新增成员
                                                   m_conn(nullptr), m_sqltype(true),
                                                   m_autocommitopt(false), m_state(disconnected)
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
    }

    int sqlstatement::connect(connection* conn)
    {
        if (m_state == connected) return 0;

        m_cda.init();
        m_conn = conn;

        if (!m_conn || m_conn->m_state == connection::disconnected)
        {
            m_cda.rc = -1;
            m_cda.message = "database not open.";
            return -1;
        }

        m_mysql = m_conn->m_mysql;
        m_autocommitopt = m_conn->m_autocommitopt;

        m_stmt = mysql_stmt_init(m_mysql);
        if (!m_stmt)
        {
            err_report();
            return m_cda.rc;
        }

        m_state = connected;
        m_cda.rc = 0;
        m_cda.message.clear();

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

        if (m_result)
        {
            mysql_free_result(m_result);
            m_result = nullptr;
        }

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

        free_bind();
        if (m_result)
        {
            mysql_free_result(m_result);
            m_result = nullptr;
        }

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
        m_sql.resize(len + 1); // 增加1个字节确保null终止符
        vsnprintf(&m_sql[0], len + 1, fmt, ap);
        va_end(ap);

        if (mysql_stmt_prepare(m_stmt, m_sql.c_str(), m_sql.length()) != 0)
        {
            err_report();
            return m_cda.rc;
        }

        m_param_count = mysql_stmt_param_count(m_stmt);
#ifdef DEBUG
        printf("m_param_count = %u\n", m_param_count);
#endif
        if (m_param_count > 0)
        {
            m_bindin = new MYSQL_BIND[m_param_count];
            memset(m_bindin, 0, sizeof(MYSQL_BIND) * m_param_count);

            // 为BLOB字段长度分配内存
            m_blob_lengths = new unsigned long[m_param_count];
            memset(m_blob_lengths, 0, sizeof(unsigned long) * m_param_count);
        }

        // 判断SQL类型
        m_sqltype = true;
        char strtemp[7];
        memset(strtemp, 0, sizeof(strtemp));
        strncpy(strtemp, m_sql.c_str(), 6); // 只取前6个字符判断
        MY__ToUpper(strtemp);
        MY__DeleteLChar(strtemp, ' ');
        if (strncmp(strtemp, "SELECT", 6) == 0 ||
            strncmp(strtemp, "SHOW", 4) == 0 ||
            strncmp(strtemp, "DESC", 3) == 0 || // 修复：DESCRIBE简化为DESC判断
            strncmp(strtemp, "EXPLA", 5) == 0)  // EXPLAIN简化判断
        {
            m_sqltype = false;
        }

        m_cda.rc = 0;
        return 0;
    }

    // 输入参数绑定方法实现
    int sqlstatement::bindin(const unsigned int position, int& value)
    {
        if (position < 1 || position > m_param_count || !m_bindin)
        {
            m_cda.rc = -1;
            m_cda.message = "bindin position out of range or m_bindin is null.";
            return -1;
        }

        MYSQL_BIND* bind = &m_bindin[position - 1];
        memset(bind, 0, sizeof(MYSQL_BIND));
        bind->buffer_type = MYSQL_TYPE_LONG;
        bind->buffer = &value;
        bind->is_unsigned = false;
        bind->is_null = nullptr; // 默认为非NULL

#ifdef DEBUG
        printf("[bindin] 绑定参数%d：类型=MYSQL_TYPE_LONG，值=%d，地址=%p\n",
               position, value, &value);
#endif
        return 0;
    }

    int sqlstatement::bindin(const unsigned int position, long& value)
    {
        if (position < 1 || position > m_param_count || !m_bindin)
        {
            m_cda.rc = -1;
            m_cda.message = "bindin position out of range or m_bindin is null.";
            return -1;
        }

        MYSQL_BIND* bind = &m_bindin[position - 1];
        memset(bind, 0, sizeof(MYSQL_BIND));
        bind->buffer_type = MYSQL_TYPE_LONGLONG;
        bind->buffer = &value;
        bind->is_unsigned = false;
        bind->is_null = nullptr;

#ifdef DEBUG
        printf("[bindin] 绑定参数%d：类型=MYSQL_TYPE_LONGLONG，值=%ld，地址=%p\n",
               position, value, &value);
#endif
        return 0;
    }

    int sqlstatement::bindin(const unsigned int position, unsigned int& value)
    {
        if (position < 1 || position > m_param_count || !m_bindin)
        {
            m_cda.rc = -1;
            m_cda.message = "bindin position out of range or m_bindin is null.";
            return -1;
        }

        MYSQL_BIND* bind = &m_bindin[position - 1];
        memset(bind, 0, sizeof(MYSQL_BIND));
        bind->buffer_type = MYSQL_TYPE_LONG;
        bind->buffer = &value;
        bind->is_unsigned = true;
        bind->is_null = nullptr;

#ifdef DEBUG
        printf("[bindin] 绑定参数%d：类型=MYSQL_TYPE_LONG，值=%u，地址=%p\n",
               position, value, &value);
#endif
        return 0;
    }

    int sqlstatement::bindin(const unsigned int position, unsigned long& value)
    {
        if (position < 1 || position > m_param_count || !m_bindin)
        {
            m_cda.rc = -1;
            m_cda.message = "bindin position out of range or m_bindin is null.";
            return -1;
        }

        MYSQL_BIND* bind = &m_bindin[position - 1];
        memset(bind, 0, sizeof(MYSQL_BIND));
        bind->buffer_type = MYSQL_TYPE_LONGLONG;
        bind->buffer = &value;
        bind->is_unsigned = true;
        bind->is_null = nullptr;

#ifdef DEBUG
        printf("[bindin] 绑定参数%d：类型=MYSQL_TYPE_LONGLONG，值=%lu，地址=%p\n",
               position, value, &value);
#endif
        return 0;
    }

    int sqlstatement::bindin(const unsigned int position, float& value)
    {
        if (position < 1 || position > m_param_count || !m_bindin)
        {
            m_cda.rc = -1;
            m_cda.message = "bindin position out of range or m_bindin is null.";
            return -1;
        }

        MYSQL_BIND* bind = &m_bindin[position - 1];
        memset(bind, 0, sizeof(MYSQL_BIND));
        bind->buffer_type = MYSQL_TYPE_FLOAT;
        bind->buffer = &value;
        bind->is_null = nullptr;

#ifdef DEBUG
        printf("[bindin] 绑定参数%d：类型=MYSQL_TYPE_FLOAT，值=%f，地址=%p\n",
               position, value, &value);
#endif
        return 0;
    }

    int sqlstatement::bindin(const unsigned int position, double& value)
    {
        if (position < 1 || position > m_param_count || !m_bindin)
        {
            m_cda.rc = -1;
            m_cda.message = "bindin position out of range or m_bindin is null.";
            return -1;
        }

        MYSQL_BIND* bind = &m_bindin[position - 1];
        memset(bind, 0, sizeof(MYSQL_BIND));
        bind->buffer_type = MYSQL_TYPE_DOUBLE;
        bind->buffer = &value;
        bind->is_null = nullptr;

#ifdef DEBUG
        printf("[bindin] 绑定参数%d：类型=MYSQL_TYPE_DOUBLE，值=%lf，地址=%p\n",
               position, value, &value);
#endif
        return 0;
    }

    int sqlstatement::bindin(const unsigned int position, char* value, unsigned int len)
    {
        if (position < 1 || position > m_param_count || !m_bindin || !value || len == 0)
        {
            m_cda.rc = -1;
            m_cda.message = "invalid bindin parameters.";
            return -1;
        }

        MYSQL_BIND* bind = &m_bindin[position - 1];
        memset(bind, 0, sizeof(MYSQL_BIND));
        bind->buffer_type = MYSQL_TYPE_STRING;
        bind->buffer = value;
        bind->buffer_length = len;
        bind->is_null = nullptr;

#ifdef DEBUG
        printf("[bindin] 绑定参数%d：类型=MYSQL_TYPE_STRING，值=%s，地址=%p\n",
               position, value, value);
#endif
        return 0;
    }

    int sqlstatement::bindin(const unsigned int position, std::string& value, unsigned int len)
    {
        if (len == 0)
        {
            m_cda.rc = -1;
            m_cda.message = "bindin length cannot be zero.";
            return -1;
        }
        value.resize(len);
        return bindin(position, &value[0], len);
    }

    int sqlstatement::bindin1(const unsigned int position, std::string& value)
    {
        if (value.empty())
        {
            m_cda.rc = -1;
            m_cda.message = "bindin1 value cannot be empty.";
            return -1;
        }
        return bindin(position, &value[0], value.size());
    }

    // 输出参数绑定方法实现
    int sqlstatement::bindout(const unsigned int position, int& value)
    {
        if (m_field_count == 0)
        {
            m_field_count = mysql_stmt_field_count(m_stmt);
            if (m_field_count == 0)
            {
                m_cda.rc = -1;
                m_cda.message = "no fields in result set.";
                return -1;
            }

            // 初始化输出绑定数组和辅助数组
            m_bindout = new MYSQL_BIND[m_field_count];
            memset(m_bindout, 0, sizeof(MYSQL_BIND) * m_field_count);

            m_out_lengths = new unsigned long[m_field_count];
            memset(m_out_lengths, 0, sizeof(unsigned long) * m_field_count);

            m_out_is_null = new bool[m_field_count];
            memset(m_out_is_null, 0, sizeof(bool) * m_field_count);
        }

        if (position < 1 || position > m_field_count || !m_bindout)
        {
            m_cda.rc = -1;
            m_cda.message = "bindout position out of range.";
            return -1;
        }

        MYSQL_BIND* bind = &m_bindout[position - 1];
        memset(bind, 0, sizeof(MYSQL_BIND));
        bind->buffer_type = MYSQL_TYPE_LONG;
        bind->buffer = &value;
        bind->is_unsigned = false;
        bind->length = &m_out_lengths[position - 1];
        bind->is_null = &m_out_is_null[position - 1];

#ifdef DEBUG
        printf("[bindout] 字段%d：类型=MYSQL_TYPE_LONG，地址=%p\n",
               position, &value);
#endif
        return 0;
    }

    int sqlstatement::bindout(const unsigned int position, long& value)
    {
        if (m_field_count == 0)
        {
            m_field_count = mysql_stmt_field_count(m_stmt);
            if (m_field_count == 0)
            {
                m_cda.rc = -1;
                m_cda.message = "no fields in result set.";
                return -1;
            }

            m_bindout = new MYSQL_BIND[m_field_count];
            memset(m_bindout, 0, sizeof(MYSQL_BIND) * m_field_count);

            m_out_lengths = new unsigned long[m_field_count];
            memset(m_out_lengths, 0, sizeof(unsigned long) * m_field_count);

            m_out_is_null = new bool[m_field_count];
            memset(m_out_is_null, 0, sizeof(bool) * m_field_count);
        }

        if (position < 1 || position > m_field_count || !m_bindout)
        {
            m_cda.rc = -1;
            m_cda.message = "bindout position out of range.";
            return -1;
        }

        MYSQL_BIND* bind = &m_bindout[position - 1];
        memset(bind, 0, sizeof(MYSQL_BIND));
        bind->buffer_type = MYSQL_TYPE_LONGLONG;
        bind->buffer = &value;
        bind->is_unsigned = false;
        bind->length = &m_out_lengths[position - 1];
        bind->is_null = &m_out_is_null[position - 1];

#ifdef DEBUG
        printf("[bindout] 字段%d：类型=MYSQL_TYPE_LONGLONG，地址=%p\n",
               position, &value);
#endif
        return 0;
    }

    int sqlstatement::bindout(const unsigned int position, unsigned int& value)
    {
        if (m_field_count == 0)
        {
            m_field_count = mysql_stmt_field_count(m_stmt);
            if (m_field_count == 0)
            {
                m_cda.rc = -1;
                m_cda.message = "no fields in result set.";
                return -1;
            }

            m_bindout = new MYSQL_BIND[m_field_count];
            memset(m_bindout, 0, sizeof(MYSQL_BIND) * m_field_count);

            m_out_lengths = new unsigned long[m_field_count];
            memset(m_out_lengths, 0, sizeof(unsigned long) * m_field_count);

            m_out_is_null = new bool[m_field_count];
            memset(m_out_is_null, 0, sizeof(bool) * m_field_count);
        }

        if (position < 1 || position > m_field_count || !m_bindout)
        {
            m_cda.rc = -1;
            m_cda.message = "bindout position out of range.";
            return -1;
        }

        MYSQL_BIND* bind = &m_bindout[position - 1];
        memset(bind, 0, sizeof(MYSQL_BIND));
        bind->buffer_type = MYSQL_TYPE_LONG;
        bind->buffer = &value;
        bind->is_unsigned = true;
        bind->length = &m_out_lengths[position - 1];
        bind->is_null = &m_out_is_null[position - 1];

        return 0;
    }

    int sqlstatement::bindout(const unsigned int position, unsigned long& value)
    {
        if (m_field_count == 0)
        {
            m_field_count = mysql_stmt_field_count(m_stmt);
            if (m_field_count == 0)
            {
                m_cda.rc = -1;
                m_cda.message = "no fields in result set.";
                return -1;
            }

            m_bindout = new MYSQL_BIND[m_field_count];
            memset(m_bindout, 0, sizeof(MYSQL_BIND) * m_field_count);

            m_out_lengths = new unsigned long[m_field_count];
            memset(m_out_lengths, 0, sizeof(unsigned long) * m_field_count);

            m_out_is_null = new bool[m_field_count];
            memset(m_out_is_null, 0, sizeof(bool) * m_field_count);
        }

        if (position < 1 || position > m_field_count || !m_bindout)
        {
            m_cda.rc = -1;
            m_cda.message = "bindout position out of range.";
            return -1;
        }

        MYSQL_BIND* bind = &m_bindout[position - 1];
        memset(bind, 0, sizeof(MYSQL_BIND));
        bind->buffer_type = MYSQL_TYPE_LONGLONG;
        bind->buffer = &value;
        bind->is_unsigned = true;
        bind->length = &m_out_lengths[position - 1];
        bind->is_null = &m_out_is_null[position - 1];

        return 0;
    }

    int sqlstatement::bindout(const unsigned int position, float& value)
    {
        if (m_field_count == 0)
        {
            m_field_count = mysql_stmt_field_count(m_stmt);
            if (m_field_count == 0)
            {
                m_cda.rc = -1;
                m_cda.message = "no fields in result set.";
                return -1;
            }

            m_bindout = new MYSQL_BIND[m_field_count];
            memset(m_bindout, 0, sizeof(MYSQL_BIND) * m_field_count);

            m_out_lengths = new unsigned long[m_field_count];
            memset(m_out_lengths, 0, sizeof(unsigned long) * m_field_count);

            m_out_is_null = new bool[m_field_count];
            memset(m_out_is_null, 0, sizeof(bool) * m_field_count);
        }

        if (position < 1 || position > m_field_count || !m_bindout)
        {
            m_cda.rc = -1;
            m_cda.message = "bindout position out of range.";
            return -1;
        }

        MYSQL_BIND* bind = &m_bindout[position - 1];
        memset(bind, 0, sizeof(MYSQL_BIND));
        bind->buffer_type = MYSQL_TYPE_FLOAT;
        bind->buffer = &value;
        bind->length = &m_out_lengths[position - 1];
        bind->is_null = &m_out_is_null[position - 1];

        return 0;
    }

    int sqlstatement::bindout(const unsigned int position, double& value)
    {
        if (m_field_count == 0)
        {
            m_field_count = mysql_stmt_field_count(m_stmt);
            if (m_field_count == 0)
            {
                m_cda.rc = -1;
                m_cda.message = "no fields in result set.";
                return -1;
            }

            m_bindout = new MYSQL_BIND[m_field_count];
            memset(m_bindout, 0, sizeof(MYSQL_BIND) * m_field_count);

            m_out_lengths = new unsigned long[m_field_count];
            memset(m_out_lengths, 0, sizeof(unsigned long) * m_field_count);

            m_out_is_null = new bool[m_field_count];
            memset(m_out_is_null, 0, sizeof(bool) * m_field_count);
        }

        if (position < 1 || position > m_field_count || !m_bindout)
        {
            m_cda.rc = -1;
            m_cda.message = "bindout position out of range.";
            return -1;
        }

        MYSQL_BIND* bind = &m_bindout[position - 1];
        memset(bind, 0, sizeof(MYSQL_BIND));
        bind->buffer_type = MYSQL_TYPE_DOUBLE;
        bind->buffer = &value;
        bind->length = &m_out_lengths[position - 1];
        bind->is_null = &m_out_is_null[position - 1];

        return 0;
    }

    int sqlstatement::bindout(const unsigned int position, char* value, unsigned int len)
    {
        if (len == 0 || !value)
        {
            m_cda.rc = -1;
            m_cda.message = "invalid buffer for bindout.";
            return -1;
        }

        if (m_field_count == 0)
        {
            m_field_count = mysql_stmt_field_count(m_stmt);
            if (m_field_count == 0)
            {
                m_cda.rc = -1;
                m_cda.message = "no fields in result set.";
                return -1;
            }

            m_bindout = new MYSQL_BIND[m_field_count];
            memset(m_bindout, 0, sizeof(MYSQL_BIND) * m_field_count);

            m_out_lengths = new unsigned long[m_field_count];
            memset(m_out_lengths, 0, sizeof(unsigned long) * m_field_count);

            m_out_is_null = new bool[m_field_count];
            memset(m_out_is_null, 0, sizeof(bool) * m_field_count);
        }

        if (position < 1 || position > m_field_count || !m_bindout)
        {
            m_cda.rc = -1;
            m_cda.message = "bindout position out of range.";
            return -1;
        }

        MYSQL_BIND* bind = &m_bindout[position - 1];
        memset(bind, 0, sizeof(MYSQL_BIND));
        bind->buffer_type = MYSQL_TYPE_STRING;
        bind->buffer = value;
        bind->buffer_length = len;
        bind->length = &m_out_lengths[position - 1];
        bind->is_null = &m_out_is_null[position - 1];

        return 0;
    }

    int sqlstatement::bindout(const unsigned int position, std::string& value, unsigned int len)
    {
        if (len == 0)
        {
            m_cda.rc = -1;
            m_cda.message = "bindout length cannot be zero.";
            return -1;
        }
        value.resize(len, '\0'); // 确保有足够空间并初始化
        return bindout(position, &value[0], len);
    }

    int sqlstatement::execute()
    {
        m_cda.init();
#ifdef DEBUG
        printf("[execute] 进入execute()方法\n");
#endif

        if (m_state != connected || !m_stmt)
        {
            m_cda.rc = -1;
            m_cda.message = "not connected or m_stmt is null.";
#ifdef DEBUG
            printf("[execute] 失败：%s\n", m_cda.message.c_str());
#endif
            return -1;
        }

        if (mysql_ping(m_mysql) != 0)
        {
            m_cda.rc = mysql_errno(m_mysql);
            m_cda.message = mysql_error(m_mysql);
#ifdef DEBUG
            printf("[execute] MySQL连接已断开：%s\n", m_cda.message.c_str());
#endif
            return -1;
        }

        // 绑定输入参数
        if (m_param_count > 0 && m_bindin)
        {
            if (mysql_stmt_bind_param(m_stmt, m_bindin) != 0)
            {
                err_report();
                m_cda.message = "mysql_stmt_bind_param failed: " + m_cda.message;
#ifdef DEBUG
                printf("[execute] 失败：%s\n", m_cda.message.c_str());
#endif
                return -1;
            }
#ifdef DEBUG
            printf("[execute] 成功绑定%d个输入参数\n", m_param_count);
#endif
        }

        // 执行预处理语句
#ifdef DEBUG
        printf("[execute] 准备执行mysql_stmt_execute\n");
#endif
        int execute_ret = mysql_stmt_execute(m_stmt);
        if (execute_ret != 0)
        {
            err_report();
            m_cda.message = "mysql_stmt_execute failed: " + m_cda.message;
#ifdef DEBUG
            printf("[execute] mysql_stmt_execute失败：err=%d, msg=%s\n", m_cda.rc, m_cda.message.c_str());
#endif
            return -1;
        }
#ifdef DEBUG
        printf("[execute] mysql_stmt_execute成功\n");
#endif

        // 对于查询语句，绑定输出参数并存储结果集
        if (!m_sqltype)
        {
            // 绑定输出参数
            if (m_field_count > 0 && m_bindout)
            {
                if (mysql_stmt_bind_result(m_stmt, m_bindout) != 0)
                {
                    err_report();
                    m_cda.message = "mysql_stmt_bind_result failed: " + m_cda.message;
#ifdef DEBUG
                    printf("[execute] mysql_stmt_bind_result失败：err=%d, msg=%s\n", m_cda.rc, m_cda.message.c_str());
#endif
                    return -1;
                }
#ifdef DEBUG
                printf("[execute] 成功绑定%d个输出参数\n", m_field_count);
#endif
            }

            // 存储结果集
#ifdef DEBUG
            printf("[execute] 准备调用mysql_stmt_store_result\n");
#endif
            if (mysql_stmt_store_result(m_stmt) != 0)
            {
                err_report();
                m_cda.message = "mysql_stmt_store_result failed: " + m_cda.message;
#ifdef DEBUG
                printf("[execute] mysql_stmt_store_result失败：err=%d, msg=%s\n", m_cda.rc, m_cda.message.c_str());
#endif
                return -1;
            }
#ifdef DEBUG
            printf("[execute] mysql_stmt_store_result成功\n");
#endif

            // 获取记录数
            m_cda.rpc = mysql_stmt_num_rows(m_stmt);
#ifdef DEBUG
            printf("[execute] 结果集记录数：%llu\n", (unsigned long long)m_cda.rpc);
#endif
        }
        else
        {
            // 对于非查询语句，获取影响行数
            m_cda.rpc = mysql_stmt_affected_rows(m_stmt);
#ifdef DEBUG
            printf("[execute] 影响行数：%llu\n", (unsigned long long)m_cda.rpc);
#endif
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
        strtmp.resize(len + 1); // 增加1个字节确保null终止符
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

        if (m_cda.rc != 0)
            return m_cda.rc;

        if (m_sqltype)
        {
            m_cda.rc = -1;
            m_cda.message = "no recordset found (not a query statement).";
            return -1;
        }

        // 检查是否有输出绑定
        if (m_field_count > 0 && !m_bindout)
        {
            m_cda.rc = -1;
            m_cda.message = "output parameters not bound.";
            return -1;
        }

        int ret = mysql_stmt_fetch(m_stmt);
        if (ret == 0)
        {
            // 处理字符串的NULL终止
            for (unsigned int i = 0; i < m_field_count; ++i)
            {
                MYSQL_BIND* bind = &m_bindout[i];
                if (bind->buffer_type == MYSQL_TYPE_STRING && bind->buffer &&
                    !*bind->is_null && *bind->length < bind->buffer_length)
                {
                    // 为字符串添加NULL终止符
                    ((char*)bind->buffer)[*bind->length] = '\0';
                }
            }

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
            m_cda.message = "mysql_stmt_fetch failed: " + m_cda.message;
            return m_cda.rc;
        }
    }

    // BLOB相关函数实现（支持输入/输出参数绑定）
    int sqlstatement::bindblob(const unsigned int position, char* buffer, unsigned long length)
    {
        // 基础参数校验
        if (position == 0 || !buffer || length == 0)
        {
            m_cda.rc = -1;
            m_cda.message = "invalid parameters for bindblob (position=0 or buffer=null or length=0)";
            return -1;
        }

        // 区分输入参数（INSERT/UPDATE）和输出参数（SELECT）
        if (m_sqltype)
        {
            // 输入参数绑定（SQL是INSERT/UPDATE，有?占位符）
            if (position > m_param_count || !m_bindin)
            {
                m_cda.rc = -1;
                m_cda.message = "invalid position for bindblob (input parameter)";
                return -1;
            }

            // 存储BLOB长度，绑定输入参数
            m_blob_lengths[position - 1] = length;
            MYSQL_BIND* bind = &m_bindin[position - 1];
            memset(bind, 0, sizeof(MYSQL_BIND));
            bind->buffer_type = MYSQL_TYPE_BLOB;
            bind->buffer = buffer;
            bind->buffer_length = length;
            bind->length = &m_blob_lengths[position - 1];
            bind->is_null = nullptr;
        }
        else
        {
            // 输出参数绑定（SQL是SELECT，无输入参数，校验结果字段数量）
            if (m_field_count == 0)
            {
                m_field_count = mysql_stmt_field_count(m_stmt); // 获取结果字段总数
            }

            // 初始化输出绑定数组（首次绑定输出参数时）
            if (!m_bindout)
            {
                m_bindout = new MYSQL_BIND[m_field_count];
                memset(m_bindout, 0, sizeof(MYSQL_BIND) * m_field_count);

                m_out_lengths = new unsigned long[m_field_count];
                memset(m_out_lengths, 0, sizeof(unsigned long) * m_field_count);

                m_out_is_null = new bool[m_field_count];
                memset(m_out_is_null, 0, sizeof(bool) * m_field_count);
            }

            if (position > m_field_count)
            {
                m_cda.rc = -1;
                m_cda.message = "invalid position for bindblob (output parameter)";
                return -1;
            }

            // 绑定输出参数（SELECT的结果字段）
            MYSQL_BIND* bind = &m_bindout[position - 1];
            memset(bind, 0, sizeof(MYSQL_BIND));
            bind->buffer_type = MYSQL_TYPE_BLOB;
            bind->buffer = buffer;
            bind->buffer_length = length;
            bind->length = &m_out_lengths[position - 1]; // 使用输出长度数组
            bind->is_null = &m_out_is_null[position - 1];
        }

#ifdef DEBUG
        printf("[bindblob] 绑定BLOB %s参数%d：长度=%lu字节\n",
               m_sqltype ? "输入" : "输出", position, length);
#endif
        return 0;
    }

    // 修复filetoblob：添加position参数，支持多参数绑定
    int sqlstatement::filetoblob(const unsigned int position, const std::string& filename)
    {
        FILE* fp = fopen(filename.c_str(), "rb");
        if (!fp)
        {
            m_cda.rc = -1;
            m_cda.message = "fopen failed: " + std::string(strerror(errno));
            return -1;
        }

        // 获取文件大小
        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        if (file_size <= 0)
        {
            fclose(fp);
            m_cda.rc = 0; // 空文件视为成功
            return 0;
        }

        // 分配缓冲区
        char* buffer = new char[file_size];
        if (!buffer)
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
            m_cda.message = "fread failed: " + std::string(strerror(errno));
            return -1;
        }

#ifdef DEBUG
        printf("[filetoblob] 读取文件成功：%s，大小=%ld字节\n", filename.c_str(), file_size);
        printf("[filetoblob] 前16字节: ");
        for (int i = 0; i < 16 && i < file_size; i++)
        {
            printf("%02X ", (unsigned char)buffer[i]);
        }
        printf("\n");
#endif

        // 绑定BLOB数据
        int ret = bindblob(position, buffer, file_size);
        if (ret != 0)
        {
            delete[] buffer;
            return ret;
        }

        // 执行SQL
        ret = execute();
        delete[] buffer; // 执行完成后释放内存
        return ret;
    }

    // 修复blobtofile：完善文件操作逻辑，确保文件正确生成
    int sqlstatement::blobtofile(const unsigned int position, const std::string& filename)
    {
        // 基础状态校验
        if (m_sqltype || m_state != connected)
        {
            m_cda.rc = -1;
            m_cda.message = "no blob data available or not connected.";
            return -1;
        }

        // 获取结果集元数据
        MYSQL_RES* meta_result = mysql_stmt_result_metadata(m_stmt);
        if (!meta_result)
        {
            err_report();
            return m_cda.rc;
        }

        // 校验字段位置合法性
        unsigned int num_fields = mysql_num_fields(meta_result);
        if (position < 1 || position > num_fields)
        {
            mysql_free_result(meta_result);
            m_cda.rc = -1;
            m_cda.message = "invalid BLOB field position.";
            return -1;
        }

        // 校验字段类型是否为BLOB
        MYSQL_FIELD* fields = mysql_fetch_fields(meta_result);
        if (!fields || (fields[position - 1].type != MYSQL_TYPE_BLOB &&
                        fields[position - 1].type != MYSQL_TYPE_LONG_BLOB))
        {
            mysql_free_result(meta_result);
            m_cda.rc = -1;
            m_cda.message = "specified field is not a BLOB type.";
            return -1;
        }

        // 绑定结果集（使用已初始化的m_bindout，而非重新创建bind）
        if (!m_bindout)
        {
            mysql_free_result(meta_result);
            m_cda.rc = -1;
            m_cda.message = "BLOB output buffer not initialized (call bindblob first).";
            return -1;
        }

        // 从绑定的缓冲区中获取数据（复用bindblob的结果）
        MYSQL_BIND* bind = &m_bindout[position - 1];
        unsigned long blob_length = *bind->length; // 实际BLOB数据长度（从bindblob的输出长度获取）
        char* buffer = (char*)bind->buffer;        // 从bindblob绑定的缓冲区获取数据

        if (blob_length == 0 || !buffer)
        {
            mysql_free_result(meta_result);
            m_cda.rc = -1;
            m_cda.message = "BLOB data is empty or buffer not allocated.";
            return -1;
        }

#ifdef DEBUG
        printf("[blobtofile] 准备写入文件：%s，实际数据大小=%lu字节\n", filename.c_str(), blob_length);
        printf("[blobtofile] 前16字节: ");
        for (int i = 0; i < 16 && i < blob_length; i++)
        {
            printf("%02X ", (unsigned char)buffer[i]);
        }
        printf("\n");
#endif

        // 检查目录是否存在，不存在则创建（关键修复：避免路径错误导致文件无法生成）
        size_t last_slash = filename.find_last_of("/\\");
        if (last_slash != std::string::npos)
        {
            std::string dir = filename.substr(0, last_slash);
// Windows下创建目录（需要包含<direct.h>）
#ifdef _WIN32
            if (_mkdir(dir.c_str()) == -1 && errno != EEXIST)
#else
            // Linux/Mac下创建目录（需要包含<sys/stat.h>）
            if (mkdir(dir.c_str(), 0755) == -1 && errno != EEXIST)
#endif
            {
                mysql_free_result(meta_result);
                m_cda.rc = -1;
                m_cda.message = "create directory failed: " + std::string(strerror(errno)) + " (" + dir + ")";
                return -1;
            }
        }

        // 打开文件（二进制写入）
        FILE* fp = fopen(filename.c_str(), "wb");
        if (!fp)
        {
            mysql_free_result(meta_result);
            m_cda.rc = -1;
            m_cda.message = "fopen failed: " + std::string(strerror(errno)) + " (" + filename + ")";
            return -1;
        }

        // 写入文件（使用实际BLOB长度，而非缓冲区大小）
        size_t bytes_written = fwrite(buffer, 1, blob_length, fp);
        if (bytes_written != blob_length)
        {
            fclose(fp);
            mysql_free_result(meta_result);
            m_cda.rc = -1;
            m_cda.message = "fwrite failed: written " + std::to_string(bytes_written) +
                            " of " + std::to_string(blob_length) + " bytes (" + filename + ")";
            remove(filename.c_str()); // 删除不完整文件
            return -1;
        }

        fclose(fp);
        mysql_free_result(meta_result);
        return 0;
    }

    // TEXT相关函数修复
    int sqlstatement::bindtext(const unsigned int position, char* buffer, unsigned long length)
    {
        if (position == 0 || position > m_param_count || !m_bindin || !buffer || length == 0)
        {
            m_cda.rc = -1;
            m_cda.message = "invalid parameters for bindtext";
            return -1;
        }

        // 检查长度是否超过unsigned int的最大值
        if (length > UINT_MAX)
        {
            m_cda.rc = -1;
            m_cda.message = "text length exceeds maximum allowed value.";
            return -1;
        }

        MYSQL_BIND* bind = &m_bindin[position - 1];
        memset(bind, 0, sizeof(MYSQL_BIND));
        bind->buffer_type = MYSQL_TYPE_STRING;
        bind->buffer = buffer;
        bind->buffer_length = static_cast<unsigned int>(length);
        bind->length = &length;
        bind->is_null = nullptr;

        return 0;
    }

    int sqlstatement::bindtext(const unsigned int position, std::string& buffer, unsigned long length)
    {
        // 检查长度是否超过unsigned int的最大值
        if (length > UINT_MAX)
        {
            m_cda.rc = -1;
            m_cda.message = "text length exceeds maximum allowed value.";
            return -1;
        }

        if (length == 0)
        {
            m_cda.rc = -1;
            m_cda.message = "text length cannot be zero.";
            return -1;
        }
        buffer.resize(length);
        return bindtext(position, &buffer[0], length);
    }

    // 1. 首先修复filetotext函数（与filetoblob保持完全一致）
    int sqlstatement::filetotext(const unsigned int position, const std::string& filename, unsigned int chunk_size)
    {
        // 基础校验（与filetoblob完全一致）
        if (position == 0 || position > m_param_count || !m_stmt)
        {
            m_cda.rc = -1;
            m_cda.message = "invalid position for filetotext (position out of range)";
            return -1;
        }

        // 二进制打开文件（与filetoblob一致）
        FILE* fp = fopen(filename.c_str(), "rb");
        if (!fp)
        {
            m_cda.rc = -1;
            m_cda.message = "fopen failed for " + filename + ": " + std::string(strerror(errno));
            return -1;
        }

        // 获取文件大小（与filetoblob一致）
        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        if (file_size < 0)
        {
            fclose(fp);
            m_cda.rc = -1;
            m_cda.message = "ftell failed for " + filename;
            return -1;
        }

        // 初始化缓冲区（与filetoblob一致）
        if (chunk_size < 1024) chunk_size = 1024;
        if (chunk_size > 8 * 1024 * 1024) chunk_size = 8 * 1024 * 1024;
        char* buffer = new (std::nothrow) char[chunk_size];
        if (!buffer)
        {
            fclose(fp);
            m_cda.rc = -1;
            m_cda.message = "memory allocation failed for buffer";
            return -1;
        }

        // 关键：复用BLOB的参数绑定逻辑（只绑定一次，且存入m_bindin）
        if (m_param_count > 0 && m_bindin)
        {
            MYSQL_BIND* bind = &m_bindin[position - 1];
            memset(bind, 0, sizeof(MYSQL_BIND));
            bind->buffer_type = MYSQL_TYPE_BLOB; // 与BLOB完全一致
            bind->is_null = nullptr;
            bind->length = nullptr;
            bind->buffer = nullptr; // 分块传输不需要设置buffer
        }
        else
        {
            delete[] buffer;
            fclose(fp);
            m_cda.rc = -1;
            m_cda.message = "m_bindin is null or m_param_count is 0";
            return -1;
        }

        // 分块传输（与filetoblob完全一致）
        unsigned int param_index = position - 1;
        size_t total_sent = 0;
        bool has_error = false;

        while (total_sent < (size_t)file_size)
        {
            size_t read_size = (file_size - total_sent) < chunk_size ? (file_size - total_sent) : chunk_size;

            size_t bytes_read = fread(buffer, 1, read_size, fp);
            if (bytes_read != read_size)
            {
                m_cda.rc = -1;
                m_cda.message = "fread failed at offset " + std::to_string(total_sent) +
                                ": " + std::string(strerror(errno));
                has_error = true;
                break;
            }

            if (mysql_stmt_send_long_data(m_stmt, param_index, buffer, bytes_read) != 0)
            {
                err_report();
                m_cda.message = "send chunk failed (offset " + std::to_string(total_sent) +
                                "): " + m_cda.message;
                has_error = true;
                break;
            }

            total_sent += bytes_read;
        }

        // 清理资源（与filetoblob一致）
        delete[] buffer;
        fclose(fp);

        if (has_error)
        {
            return -1;
        }

        printf("[filetotext] 复刻BLOB传输完成：总大小=%ld字节，分块数=%d\n",
               file_size, (int)((total_sent + chunk_size - 1) / chunk_size));
        return 0;
    }

    int sqlstatement::texttofile(const unsigned int position, const std::string& filename)
    {
        if (m_sqltype)
        {
            m_cda.rc = -1;
            m_cda.message = "not a query statement.";
            return -1;
        }

        MYSQL_RES* meta_result = mysql_stmt_result_metadata(m_stmt);
        if (!meta_result)
        {
            err_report();
            return m_cda.rc;
        }

        unsigned int num_fields = mysql_num_fields(meta_result);
        if (position < 1 || position > num_fields)
        {
            mysql_free_result(meta_result);
            m_cda.rc = -1;
            m_cda.message = "invalid TEXT field position.";
            return -1;
        }

        MYSQL_FIELD* fields = mysql_fetch_fields(meta_result);
        if (fields[position - 1].type != MYSQL_TYPE_STRING &&
            fields[position - 1].type != MYSQL_TYPE_VAR_STRING &&
            fields[position - 1].type != MYSQL_TYPE_BLOB)
        {
            mysql_free_result(meta_result);
            m_cda.rc = -1;
            m_cda.message = "field is not a TEXT type.";
            return -1;
        }

        // 正确的顺序：先绑定缓冲区，再获取数据
        char* buffer = nullptr;
        unsigned long text_length = 0;
        MYSQL_BIND bind;
        memset(&bind, 0, sizeof(bind));
        bind.buffer_type = MYSQL_TYPE_STRING;
        bind.length = &text_length;

        // 先获取长度
        if (mysql_stmt_bind_result(m_stmt, &bind) != 0)
        {
            err_report();
            mysql_free_result(meta_result);
            return m_cda.rc;
        }

        if (mysql_stmt_fetch(m_stmt) != 0)
        {
            err_report();
            mysql_free_result(meta_result);
            return m_cda.rc;
        }

        // 分配足够的缓冲区
        if (text_length > 0)
        {
            buffer = new char[text_length + 1];
            bind.buffer = buffer;
            bind.buffer_length = text_length + 1;

            // 重新绑定并获取实际数据
            if (mysql_stmt_bind_result(m_stmt, &bind) != 0)
            {
                err_report();
                delete[] buffer;
                mysql_free_result(meta_result);
                return m_cda.rc;
            }

            if (mysql_stmt_fetch(m_stmt) != 0)
            {
                err_report();
                delete[] buffer;
                mysql_free_result(meta_result);
                return m_cda.rc;
            }
            buffer[text_length] = '\0';
        }

        // 写入文件
        FILE* fp = fopen(filename.c_str(), "w");
        if (!fp)
        {
            delete[] buffer;
            mysql_free_result(meta_result);
            m_cda.rc = -1;
            m_cda.message = "fopen failed for " + filename + ": " + std::string(strerror(errno));
            return -1;
        }

        if (text_length > 0 && buffer)
        {
            fwrite(buffer, 1, text_length, fp);
        }
        fclose(fp);
        delete[] buffer;
        mysql_free_result(meta_result);

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

    // 获取字段是否为NULL
    bool sqlstatement::is_null(const unsigned int position)
    {
        if (position < 1 || position > m_field_count || !m_out_is_null)
            return true; // 无效位置视为NULL

        return m_out_is_null[position - 1] != 0;
    }

    // 获取字段实际长度
    unsigned long sqlstatement::length(const unsigned int position)
    {
        if (position < 1 || position > m_field_count || !m_out_lengths)
            return 0;

        return m_out_lengths[position - 1];
    }

    void sqlstatement::free_bind()
    {
        if (m_bindin)
        {
            delete[] m_bindin;
            m_bindin = nullptr;
        }

        if (m_bindout)
        {
            delete[] m_bindout;
            m_bindout = nullptr;
        }

        if (m_out_lengths)
        {
            delete[] m_out_lengths;
            m_out_lengths = nullptr;
        }

        if (m_out_is_null)
        {
            delete[] m_out_is_null;
            m_out_is_null = nullptr;
        }

        // 释放BLOB长度数组
        if (m_blob_lengths)
        {
            delete[] m_blob_lengths;
            m_blob_lengths = nullptr;
        }

        m_param_count = 0;
        m_field_count = 0;
    }

    void sqlstatement::err_report()
    {
        if (m_state == disconnected)
        {
            m_cda.rc = -1;
            m_cda.message = "cursor not open.";
            return;
        }

        m_cda.rc = mysql_stmt_errno(m_stmt);
        m_cda.message = mysql_stmt_error(m_stmt);

        if (m_conn)
        {
            m_conn->m_cda.rc = m_cda.rc;
            m_conn->m_cda.message = m_cda.message;
        }
    }

} // namespace ol