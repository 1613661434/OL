/****************************************************************************************/
/*
 * 程序名：ol_oci.cpp
 * 功能描述：C++ Oracle数据库操作实现（复用ol_core工具函数）
 * 作者：ol
 * 标准：C++17 及以上
 */
/****************************************************************************************/

#include "ol_oci.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>

namespace ol
{
    namespace oracle
    {
        // LOB分块传输缓冲区大小
        static constexpr size_t LOB_CHUNK_SIZE = 10240;

        // ===================== DBResult 实现 =====================
        void DBResult::init()
        {
            code = 0;
            affected_rows = 0;
            error_msg.clear();
        }

        // ===================== DBConn 实现 =====================
        DBConn::DBConn()
            : m_autocommitopt(false),
              m_state(ConnState::Disconnected)
        {
            m_result.init();
            m_result.code = -1;
            m_result.error_msg = "database not open.";
        }

        DBConn::~DBConn()
        {
            disconnect();
        }

        void DBConn::setConnectParam(const std::string& connstr, const std::string& charset, bool autocommit)
        {
            parseConnStr(connstr);
            m_charset = charset;
            m_autocommitopt = autocommit;
        }

        void DBConn::parseConnStr(const std::string& connstr)
        {
            // 格式：username/password@tnsname
            size_t atPos = connstr.find('@');
            size_t slashPos = connstr.find('/');

            // 解析 TNS 服务名
            if (atPos != std::string::npos)
            {
                m_tnsname = connstr.substr(atPos + 1);
            }

            // 解析用户名和密码
            std::string credPart = (atPos != std::string::npos)
                                       ? connstr.substr(0, atPos)
                                       : connstr;

            if (slashPos != std::string::npos && slashPos < atPos)
            {
                m_user = credPart.substr(0, slashPos);
                m_pass = credPart.substr(slashPos + 1);
            }
            else
            {
                m_user = credPart;
            }
        }

        void DBConn::setCharset(const char* charset)
        {
            if (!charset || charset[0] == '\0') return;

#ifdef __unix__
            setenv("NLS_LANG", charset, 1);
            setenv("NLS_DATE_FORMAT", "yyyy-mm-dd hh24:mi:ss", 1);
#elif defined(_WIN32)
            char buf[128];
            snprintf(buf, sizeof(buf), "NLS_LANG=%s", charset);
            putenv(buf);
            putenv("NLS_DATE_FORMAT=yyyy-mm-dd hh24:mi:ss");
#endif
        }

        int DBConn::ociEnvInit()
        {
            int oci_ret = OCIEnvCreate(&m_envhp, OCI_DEFAULT, nullptr, nullptr, nullptr, nullptr, 0, nullptr);

            if (oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO)
            {
                ociEnvClose();
                return -1;
            }

            return 0;
        }

        int DBConn::ociEnvClose()
        {
            if (m_envhp)
            {
                OCIHandleFree(m_envhp, OCI_HTYPE_ENV);
                m_envhp = nullptr;
            }
            return 0;
        }

        int DBConn::ociContextCreate()
        {
            if (!m_envhp) return -1;

            int oci_ret = OCIHandleAlloc(m_envhp, (dvoid**)&m_errhp, OCI_HTYPE_ERROR, 0, nullptr);

            if (oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO)
            {
                ociContextClose();
                return -1;
            }

            // 多线程同时OCILogon会导致段错误，必须加锁
            static std::mutex logonMutex;
            {
                std::lock_guard<std::mutex> lock(logonMutex);
                oci_ret = OCILogon(m_envhp, m_errhp, &m_svchp,
                                   (OraText*)m_user.c_str(), (ub4)m_user.size(),
                                   (OraText*)m_pass.c_str(), (ub4)m_pass.size(),
                                   (OraText*)m_tnsname.c_str(), (ub4)m_tnsname.size());
            }

            if (oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO)
            {
                ociContextClose();
                return -1;
            }

            return 0;
        }

        int DBConn::ociContextClose()
        {
            if (m_svchp)
            {
                OCILogoff(m_svchp, m_errhp);
                OCIHandleFree(m_svchp, OCI_HTYPE_SVCCTX);
                m_svchp = nullptr;
            }

            if (m_errhp)
            {
                OCIHandleFree(m_errhp, OCI_HTYPE_ERROR);
                m_errhp = nullptr;
            }

            return 0;
        }

        bool DBConn::connect()
        {
            if (m_state == ConnState::Connected) return true;

            m_result.init();

            // 设置字符集
            setCharset(m_charset.c_str());

            // 初始化OCI环境
            if (ociEnvInit() != 0)
            {
                m_result.code = -1;
                m_result.error_msg = "initialize oracle call interface failed.";
                return false;
            }

            // 创建上下文并登录
            if (ociContextCreate() != 0)
            {
                ociEnvClose();
                m_result.code = 1017;
                m_result.error_msg = "ORA-01017: invalid username/password, logon denied.";
                return false;
            }

            m_state = ConnState::Connected;
            m_result.init();
            return true;
        }

        void DBConn::disconnect()
        {
            if (m_state == ConnState::Disconnected) return;

            if (!m_autocommitopt) rollback();

            ociContextClose();
            ociEnvClose();

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

        bool DBConn::reconnect()
        {
            disconnect();
            return connect();
        }

        bool DBConn::beginTransaction()
        {
            // Oracle在首条DML时隐式开启事务，只需确保autocommit关闭
            m_autocommitopt = false;
            return true;
        }

        bool DBConn::commit()
        {
            m_result.init();

            if (m_state == ConnState::Disconnected)
            {
                m_result.code = -1;
                m_result.error_msg = "database not open.";
                return false;
            }

            int oci_ret = OCITransCommit(m_svchp, m_errhp, OCI_DEFAULT);

            if (oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO)
            {
                errReport();
                return false;
            }

            return true;
        }

        bool DBConn::rollback()
        {
            m_result.init();

            if (m_state == ConnState::Disconnected)
            {
                m_result.code = -1;
                m_result.error_msg = "database not open.";
                return false;
            }

            int oci_ret = OCITransRollback(m_svchp, m_errhp, OCI_DEFAULT);

            if (oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO)
            {
                errReport();
                return false;
            }

            return true;
        }

        std::unique_ptr<DBStmt> DBConn::createStmt()
        {
            return std::make_unique<DBStmt>(*this);
        }

        int DBConn::execute(const char* fmt, ...)
        {
            m_result.init();

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
                m_result = stmt.m_result;
                return -1;
            }

            if (!stmt.execute())
            {
                m_result = stmt.m_result;
                return -1;
            }

            m_result = stmt.m_result;
            return 0;
        }

        int DBConn::code() const { return m_result.code; }
        size_t DBConn::affectedRows() const { return m_result.affected_rows; }
        std::string DBConn::errorMsg() const { return m_result.error_msg; }

        void DBConn::errReport()
        {
            if (m_state == ConnState::Disconnected)
            {
                m_result.code = -1;
                m_result.error_msg = "database not open.";
                return;
            }

            m_result.init();
            m_result.code = -1;
            m_result.error_msg = "call err_report failed.";

            if (m_errhp)
            {
                char errBuf[2048] = {0};
                if (OCIErrorGet(m_errhp, 1, nullptr, &m_result.code, (OraText*)errBuf, sizeof(errBuf), OCI_HTYPE_ERROR)
                    == OCI_NO_DATA)
                {
                    m_result.init();
                    return;
                }
                m_result.error_msg = errBuf;
            }
        }

        // ===================== DBStmt 实现 =====================
        DBStmt::DBStmt(DBConn& conn)
            : m_conn(conn),
              m_autocommitopt(conn.m_autocommitopt)
        {
            m_result.init();
            m_result.code = -1;
            m_result.error_msg = "cursor not open.";

            ociStmtCreate();
        }

        DBStmt::~DBStmt()
        {
            freelob();
            ociStmtClose();
        }

        bool DBStmt::isOpen() const
        {
            return m_smthp != nullptr;
        }

        int DBStmt::ociStmtCreate()
        {
            if (!m_envhp && m_conn.m_envhp)
            {
                m_envhp = m_conn.m_envhp;
                m_svchp = m_conn.m_svchp;
                m_errhp = m_conn.m_errhp;
            }

            if (!m_envhp) return -1;

            int oci_ret = OCIHandleAlloc(m_envhp, (dvoid**)&m_smthp, OCI_HTYPE_STMT, 0, nullptr);

            if (oci_ret == OCI_SUCCESS || oci_ret == OCI_SUCCESS_WITH_INFO)
            {
                return 0;
            }

            return oci_ret;
        }

        int DBStmt::ociStmtClose()
        {
            if (m_smthp)
            {
                OCIHandleFree(m_smthp, OCI_HTYPE_STMT);
                m_smthp = nullptr;
            }
            return 0;
        }

        bool DBStmt::prepare(const char* sql)
        {
            if (!isOpen())
            {
                m_result.code = -1;
                m_result.error_msg = "cursor not open.";
                return false;
            }

            m_result.init();
            m_sql = sql;

            int oci_ret = OCIStmtPrepare(m_smthp, m_errhp, (OraText*)m_sql.c_str(), (ub4)m_sql.size(),
                                         OCI_NTV_SYNTAX, OCI_DEFAULT);

            if (oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO)
            {
                errReport();
                return false;
            }

            // 判断是否为查询语句
            m_isQuery = false;

            std::string head = m_sql.substr(0, 30);
            ol::toUpper(head);
            ol::deleteLchr(head, ' ');

            if (head.compare(0, 6, "SELECT") == 0) m_isQuery = true;

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
            int oci_ret = OCIBindByPos(m_smthp, &m_bindhp, m_errhp, (ub4)pos, &value, sizeof(value),
                                        SQLT_INT, nullptr, nullptr, nullptr, 0, nullptr, OCI_DEFAULT);
            if (oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO)
            {
                errReport();
                return m_result.code;
            }
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindin(unsigned int pos, long& value)
        {
            int oci_ret = OCIBindByPos(m_smthp, &m_bindhp, m_errhp, (ub4)pos, &value, sizeof(value),
                                        SQLT_INT, nullptr, nullptr, nullptr, 0, nullptr, OCI_DEFAULT);
            if (oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO)
            {
                errReport();
                return m_result.code;
            }
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindin(unsigned int pos, unsigned int& value)
        {
            int oci_ret = OCIBindByPos(m_smthp, &m_bindhp, m_errhp, (ub4)pos, &value, sizeof(value),
                                        SQLT_INT, nullptr, nullptr, nullptr, 0, nullptr, OCI_DEFAULT);
            if (oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO)
            {
                errReport();
                return m_result.code;
            }
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindin(unsigned int pos, unsigned long& value)
        {
            int oci_ret = OCIBindByPos(m_smthp, &m_bindhp, m_errhp, (ub4)pos, &value, sizeof(value),
                                        SQLT_INT, nullptr, nullptr, nullptr, 0, nullptr, OCI_DEFAULT);
            if (oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO)
            {
                errReport();
                return m_result.code;
            }
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindin(unsigned int pos, float& value)
        {
            int oci_ret = OCIBindByPos(m_smthp, &m_bindhp, m_errhp, (ub4)pos, &value, sizeof(value),
                                        SQLT_FLT, nullptr, nullptr, nullptr, 0, nullptr, OCI_DEFAULT);
            if (oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO)
            {
                errReport();
                return m_result.code;
            }
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindin(unsigned int pos, double& value)
        {
            int oci_ret = OCIBindByPos(m_smthp, &m_bindhp, m_errhp, (ub4)pos, &value, sizeof(value),
                                        SQLT_FLT, nullptr, nullptr, nullptr, 0, nullptr, OCI_DEFAULT);
            if (oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO)
            {
                errReport();
                return m_result.code;
            }
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindin(unsigned int pos, char* value, unsigned int len)
        {
            if (!value || len == 0)
            {
                m_result.code = -1;
                m_result.error_msg = "bindin failed: invalid params (null/zero length)";
                return -1;
            }
            int oci_ret = OCIBindByPos(m_smthp, &m_bindhp, m_errhp, (ub4)pos, value, len + 1,
                                        SQLT_STR, nullptr, nullptr, nullptr, 0, nullptr, OCI_DEFAULT);
            if (oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO)
            {
                errReport();
                return m_result.code;
            }
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
            return bindin(pos, &value[0], len);
        }

        // ===================== 输出绑定 =====================
        int DBStmt::bindout(unsigned int pos, int& value)
        {
            int oci_ret = OCIDefineByPos(m_smthp, &m_defhp, m_errhp, (ub4)pos, &value, sizeof(value),
                                          SQLT_INT, nullptr, nullptr, nullptr, OCI_DEFAULT);
            if (oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO)
            {
                errReport();
                return m_result.code;
            }
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindout(unsigned int pos, long& value)
        {
            int oci_ret = OCIDefineByPos(m_smthp, &m_defhp, m_errhp, (ub4)pos, &value, sizeof(value),
                                          SQLT_INT, nullptr, nullptr, nullptr, OCI_DEFAULT);
            if (oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO)
            {
                errReport();
                return m_result.code;
            }
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindout(unsigned int pos, unsigned int& value)
        {
            int oci_ret = OCIDefineByPos(m_smthp, &m_defhp, m_errhp, (ub4)pos, &value, sizeof(value),
                                          SQLT_INT, nullptr, nullptr, nullptr, OCI_DEFAULT);
            if (oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO)
            {
                errReport();
                return m_result.code;
            }
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindout(unsigned int pos, unsigned long& value)
        {
            int oci_ret = OCIDefineByPos(m_smthp, &m_defhp, m_errhp, (ub4)pos, &value, sizeof(value),
                                          SQLT_INT, nullptr, nullptr, nullptr, OCI_DEFAULT);
            if (oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO)
            {
                errReport();
                return m_result.code;
            }
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindout(unsigned int pos, float& value)
        {
            int oci_ret = OCIDefineByPos(m_smthp, &m_defhp, m_errhp, (ub4)pos, &value, sizeof(value),
                                          SQLT_FLT, nullptr, nullptr, nullptr, OCI_DEFAULT);
            if (oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO)
            {
                errReport();
                return m_result.code;
            }
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindout(unsigned int pos, double& value)
        {
            int oci_ret = OCIDefineByPos(m_smthp, &m_defhp, m_errhp, (ub4)pos, &value, sizeof(value),
                                          SQLT_FLT, nullptr, nullptr, nullptr, OCI_DEFAULT);
            if (oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO)
            {
                errReport();
                return m_result.code;
            }
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindout(unsigned int pos, char* value, unsigned int len)
        {
            if (!value || len == 0)
            {
                m_result.code = -1;
                m_result.error_msg = "bindout failed: invalid params (null/zero length)";
                return -1;
            }
            int oci_ret = OCIDefineByPos(m_smthp, &m_defhp, m_errhp, (ub4)pos, value, len + 1,
                                          SQLT_STR, nullptr, nullptr, nullptr, OCI_DEFAULT);
            if (oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO)
            {
                errReport();
                return m_result.code;
            }
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
            return bindout(pos, &value[0], len);
        }

        // ===================== 执行与结果 =====================
        bool DBStmt::execute()
        {
            if (!isOpen())
            {
                m_result.code = -1;
                m_result.error_msg = "cursor not open.";
                return false;
            }

            m_result.init();

            ub4 mode = OCI_DEFAULT;
            // 非查询语句 + 自动提交 → OCI_COMMIT_ON_SUCCESS
            if (!m_isQuery && m_autocommitopt) mode = OCI_COMMIT_ON_SUCCESS;

            // iters: DML用1，查询用0
            ub4 iters = m_isQuery ? 0 : 1;

            int oci_ret = OCIStmtExecute(m_svchp, m_smthp, m_errhp, iters, 0, nullptr, nullptr, mode);

            if (oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO)
            {
                errReport();
                return false;
            }

            // 非查询语句：获取影响行数
            if (!m_isQuery)
            {
                OCIAttrGet((CONST dvoid*)m_smthp, OCI_HTYPE_STMT, (dvoid*)&m_result.affected_rows, nullptr,
                           OCI_ATTR_ROW_COUNT, m_errhp);
                m_conn.m_result.affected_rows = m_result.affected_rows;
            }

            m_result.code = 0;
            return true;
        }

        int DBStmt::next()
        {
            if (!isOpen())
            {
                m_result.code = -1;
                m_result.error_msg = "cursor not open.";
                return -1;
            }

            // 非查询语句，无法fetch
            if (!m_isQuery)
            {
                m_result.code = -1;
                m_result.error_msg = "no recordset found.";
                return -1;
            }

            int oci_ret = OCIStmtFetch(m_smthp, m_errhp, 1, OCI_FETCH_NEXT, OCI_DEFAULT);

            if (oci_ret == OCI_NO_DATA)
            {
                return OCI_NO_DATA; // 100
            }

            if (oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO)
            {
                errReport();

                // ORA-01405(FETCHED COLUMN VALUE IS NULL) 和 ORA-01406(FETCHED COLUMN VALUE WAS TRUNCATED) 不算错误
                if (m_result.code != 1405 && m_result.code != 1406) return m_result.code;

                m_result.code = 0;
            }

            // 更新行计数
            OCIAttrGet((CONST dvoid*)m_smthp, OCI_HTYPE_STMT, (dvoid*)&m_result.affected_rows, nullptr,
                       OCI_ATTR_ROW_COUNT, m_errhp);
            m_conn.m_result.affected_rows = m_result.affected_rows;

            return 0;
        }

        // ===================== LOB 操作 =====================
        int DBStmt::alloclob()
        {
            if (m_lob) return 0;

            return OCIDescriptorAlloc(m_envhp, (dvoid**)&m_lob, OCI_DTYPE_LOB, 0, nullptr);
        }

        void DBStmt::freelob()
        {
            if (m_lob)
            {
                OCIDescriptorFree(m_lob, OCI_DTYPE_LOB);
                m_lob = nullptr;
            }
        }

        int DBStmt::bindblob(unsigned int pos)
        {
            if (alloclob() != 0)
            {
                m_result.code = -1;
                m_result.error_msg = "alloclob failed";
                return -1;
            }

            int oci_ret = OCIDefineByPos(m_smthp, &m_defhp, m_errhp, (ub4)pos, (dvoid*)&m_lob, -1,
                                          SQLT_BLOB, nullptr, nullptr, nullptr, OCI_DEFAULT);
            if (oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO)
            {
                errReport();
                return m_result.code;
            }
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        int DBStmt::bindclob(unsigned int pos)
        {
            if (alloclob() != 0)
            {
                m_result.code = -1;
                m_result.error_msg = "alloclob failed";
                return -1;
            }

            int oci_ret = OCIDefineByPos(m_smthp, &m_defhp, m_errhp, (ub4)pos, (dvoid*)&m_lob, -1,
                                          SQLT_CLOB, nullptr, nullptr, nullptr, OCI_DEFAULT);
            if (oci_ret != OCI_SUCCESS && oci_ret != OCI_SUCCESS_WITH_INFO)
            {
                errReport();
                return m_result.code;
            }
            m_result.code = 0;
            m_result.error_msg.clear();
            return 0;
        }

        static ub4 fileLength(FILE* fp)
        {
            fseek(fp, 0, SEEK_END);
            return (ub4)ftell(fp);
        }

        int DBStmt::filetolobInternal(FILE* fp)
        {
            ub4 offset = 1;
            ub4 loblen = 0;
            std::vector<char> buf(LOB_CHUNK_SIZE);
            ub4 amtp;
            ub1 piece;
            sword retval;
            ub4 nbytes;
            ub4 remainder;

            ub4 filelen = fileLength(fp);

            if (filelen == 0) return 0;

            amtp = filelen;
            remainder = filelen;

            OCILobGetLength(m_svchp, m_errhp, m_lob, &loblen);
            fseek(fp, 0, SEEK_SET);

            nbytes = (filelen > LOB_CHUNK_SIZE) ? LOB_CHUNK_SIZE : filelen;

            if (fread(buf.data(), 1, nbytes, fp) != nbytes)
            {
                m_result.code = -1;
                m_result.error_msg = "fread failed";
                return -1;
            }

            remainder -= nbytes;

            if (remainder == 0)
            {
                // 只有一块
                retval = OCILobWrite(m_svchp, m_errhp, m_lob, &amtp, offset, buf.data(),
                                     nbytes, OCI_ONE_PIECE, nullptr,
                                     nullptr, 0, SQLCS_IMPLICIT);
                if (retval != OCI_SUCCESS)
                {
                    errReport();
                    return m_result.code;
                }
            }
            else
            {
                // 多块分片写入
                retval = OCILobWrite(m_svchp, m_errhp, m_lob, &amtp, offset, buf.data(),
                                     LOB_CHUNK_SIZE, OCI_FIRST_PIECE, nullptr,
                                     nullptr, 0, SQLCS_IMPLICIT);
                if (retval != OCI_NEED_DATA)
                {
                    errReport();
                    return m_result.code;
                }

                piece = OCI_NEXT_PIECE;

                do
                {
                    nbytes = (remainder > LOB_CHUNK_SIZE) ? LOB_CHUNK_SIZE : remainder;
                    if (remainder <= LOB_CHUNK_SIZE) piece = OCI_LAST_PIECE;

                    if (fread(buf.data(), 1, nbytes, fp) != nbytes)
                    {
                        m_result.code = -1;
                        m_result.error_msg = "fread failed";
                        return -1;
                    }

                    retval = OCILobWrite(m_svchp, m_errhp, m_lob, &amtp, offset, buf.data(),
                                         nbytes, piece, nullptr, nullptr, 0, SQLCS_IMPLICIT);
                    remainder -= nbytes;

                } while (retval == OCI_NEED_DATA && !feof(fp));
            }

            if (retval != OCI_SUCCESS)
            {
                errReport();
                return m_result.code;
            }

            OCILobGetLength(m_svchp, m_errhp, m_lob, &loblen);
            return 0;
        }

        int DBStmt::lobtofileInternal(FILE* fp)
        {
            ub4 offset = 1;
            ub4 loblen = 0;
            std::vector<char> buf(LOB_CHUNK_SIZE);
            ub4 amtp = 0;
            sword retval;

            OCILobGetLength(m_svchp, m_errhp, m_lob, &loblen);

            if (loblen == 0) return 0;

            amtp = loblen;

            ub4 readLen = (loblen < LOB_CHUNK_SIZE) ? loblen : LOB_CHUNK_SIZE;

            retval = OCILobRead(m_svchp, m_errhp, m_lob, &amtp, offset, buf.data(),
                                readLen, nullptr, nullptr, 0, SQLCS_IMPLICIT);

            switch (retval)
            {
            case OCI_SUCCESS:
                fwrite(buf.data(), 1, amtp, fp);
                break;

            case OCI_NEED_DATA:
                fwrite(buf.data(), 1, amtp, fp);

                while (true)
                {
                    amtp = 0;
                    retval = OCILobRead(m_svchp, m_errhp, m_lob, &amtp, offset, buf.data(),
                                        LOB_CHUNK_SIZE, nullptr, nullptr, 0, SQLCS_IMPLICIT);

                    if (amtp > 0) fwrite(buf.data(), 1, amtp, fp);

                    if (retval != OCI_NEED_DATA) break;
                }
                break;

            case OCI_ERROR:
                errReport();
                return m_result.code;
            }

            return 0;
        }

        int DBStmt::filetoblob(unsigned int pos, const std::string& filename)
        {
            FILE* fp = fopen(filename.c_str(), "rb");
            if (!fp)
            {
                m_result.code = -1;
                m_result.error_msg = "fopen failed";
                return -1;
            }

            if (bindblob(pos) != 0)
            {
                fclose(fp);
                return -1;
            }

            int ret = filetolobInternal(fp);
            fclose(fp);
            return ret;
        }

        int DBStmt::filetoclob(unsigned int pos, const std::string& filename)
        {
            FILE* fp = fopen(filename.c_str(), "rb");
            if (!fp)
            {
                m_result.code = -1;
                m_result.error_msg = "fopen failed";
                return -1;
            }

            if (bindclob(pos) != 0)
            {
                fclose(fp);
                return -1;
            }

            int ret = filetolobInternal(fp);
            fclose(fp);
            return ret;
        }

        int DBStmt::blobtofile(unsigned int pos, const std::string& filename)
        {
            FILE* fp = fopen(filename.c_str(), "wb");
            if (!fp)
            {
                m_result.code = -1;
                m_result.error_msg = "fopen failed";
                return -1;
            }

            if (bindblob(pos) != 0)
            {
                fclose(fp);
                return -1;
            }

            int ret = lobtofileInternal(fp);
            fclose(fp);

            // 如果导出过程中发生错误，删除不完整的文件
            if (ret != 0) remove(filename.c_str());

            return ret;
        }

        int DBStmt::clobtofile(unsigned int pos, const std::string& filename)
        {
            FILE* fp = fopen(filename.c_str(), "wb");
            if (!fp)
            {
                m_result.code = -1;
                m_result.error_msg = "fopen failed";
                return -1;
            }

            if (bindclob(pos) != 0)
            {
                fclose(fp);
                return -1;
            }

            int ret = lobtofileInternal(fp);
            fclose(fp);

            if (ret != 0) remove(filename.c_str());

            return ret;
        }

        // ===================== 工具方法 =====================
        const char* DBStmt::sql() const { return m_sql.c_str(); }
        int DBStmt::code() const { return m_result.code; }
        size_t DBStmt::affectedRows() const { return m_result.affected_rows; }
        std::string DBStmt::errorMsg() const { return m_result.error_msg; }

        void DBStmt::errReport()
        {
            if (!isOpen())
            {
                m_result.code = -1;
                m_result.error_msg = "cursor not open.";
                return;
            }

            m_result.code = -1;
            m_result.error_msg = "call err_report failed.";

            if (m_errhp)
            {
                char errBuf[2048] = {0};
                if (OCIErrorGet(m_errhp, 1, nullptr, &m_result.code, (OraText*)errBuf, sizeof(errBuf), OCI_HTYPE_ERROR)
                    == OCI_NO_DATA)
                {
                    m_result.code = 0;
                    m_result.error_msg.clear();
                    return;
                }
                m_result.error_msg = errBuf;
            }

            m_conn.errReport();
        }

    } // namespace oracle
} // namespace ol
