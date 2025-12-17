/****************************************************************************************/
/*
 * 程序名：ol_ftp.h
 * 功能描述：FTP客户端工具类，基于ftplib实现，支持FTP服务器的各类操作，特性包括：
 *          - 支持主动/被动模式连接FTP服务器
 *          - 提供文件上传（put）、下载（get）、删除、重命名等基础操作
 *          - 支持目录创建、删除、切换及列表获取
 *          - 提供文件大小、修改时间获取及完整性校验功能
 *          - 记录操作失败原因（连接失败、登录失败、模式设置失败等）
 *          - 仅支持 Linux 平台
 * 作者：ol
 * 适用标准：C++11及以上（需依赖ftplib库）
 */
/****************************************************************************************/

#ifndef OL_FTP_H
#define OL_FTP_H 1

#ifdef __linux__
#include "third_party/ftplib/ftplib.h"
#include "ol_chrono.h"
#include "ol_fstream.h"
#endif // __linux__

namespace ol
{

#ifdef __linux__
    // FTP客户端类，封装FTP服务器的连接及文件操作
    class cftpclient
    {
    private:
        netbuf* m_ftpconn; // ftp连接句柄。
    public:
        unsigned int m_size; // 文件的大小，单位：字节。
        std::string m_mtime; // 文件的修改时间，格式：yyyymmddhh24miss。

        // 以下三个成员变量用于存放login方法登录失败的原因。
        bool m_connectfailed; // 如果网络连接失败，该成员的值为true。
        bool m_loginfailed;   // 如果登录失败，用户名和密码不正确，或没有登录权限，该成员的值为true。
        bool m_optionfailed;  // 如果设置传输模式失败，该成员变量的值为true。

        // 构造函数
        cftpclient();

        // 析构函数
        ~cftpclient();

        // 禁用拷贝构造和赋值（避免连接句柄重复释放）
        cftpclient(const cftpclient&) = delete;
        cftpclient& operator=(const cftpclient) = delete;

        // 初始化m_size和m_mtime成员变量。
        void initdata();

        /**
         * @brief 登录FTP服务器
         * @param host FTP服务器地址和端口（格式："ip:port"，如"192.168.1.1:21"）
         * @param username 登录用户名
         * @param password 登录密码
         * @param imode 传输模式（1-FTPLIB_PASSIVE被动模式，2-FTPLIB_PORT主动模式，默认被动模式）
         * @return true-登录成功，false-失败（失败原因见m_connectfailed/m_loginfailed/m_optionfailed）
         */
        bool login(const std::string& host, const std::string& username, const std::string& password, const int imode = FTPLIB_PASSIVE);

        /**
         * @brief 从FTP服务器注销并断开连接
         * @return true-成功，false-失败
         */
        bool logout();

        /**
         * @brief 获取FTP服务器上文件的修改时间
         * @param remotefilename 远程文件名
         * @return true-成功（结果存于m_mtime），false-失败
         */
        bool mtime(const std::string& remotefilename);

        /**
         * @brief 获取FTP服务器上文件的大小
         * @param remotefilename 远程文件名
         * @return true-成功（结果存于m_size），false-失败
         */
        bool size(const std::string& remotefilename);

        /**
         * @brief 切换FTP服务器的当前工作目录
         * @param remotedir 远程目录名
         * @return true-成功，false-失败
         */
        bool chdir(const std::string& remotedir);

        /**
         * @brief 在FTP服务器上创建目录
         * @param remotedir 待创建的远程目录名
         * @return true-成功，false-失败
         */
        bool mkdir(const std::string& remotedir);

        /**
         * @brief 删除FTP服务器上的目录
         * @param remotedir 待删除的远程目录名
         * @return true-成功，false-失败（权限不足、目录不存在或非空）
         */
        bool rmdir(const std::string& remotedir);

        /**
         * @brief 列出FTP服务器目录中的文件和子目录（发送NLST命令）
         * @param remotedir 远程目录名（空串、"*"或"."表示当前目录）
         * @param listfilename 本地文件路径，用于保存列表结果
         * @return true-成功，false-失败
         * @note 如果列出的是ftp服务器当前目录，remotedir用"","*","."都可以，但是，不规范的ftp服务器可能有差别
         */
        bool nlist(const std::string& remotedir, const std::string& listfilename);

        /**
         * @brief 从FTP服务器下载文件
         * @param remotefilename 远程文件名
         * @param localfilename 本地保存路径
         * @param bcheckmtime 是否校验文件修改时间（确保传输完整性，默认true）
         * @return true-成功，false-失败
         * @note 传输过程中使用临时文件（localfilename.tmp），完成后重命名
         */
        bool get(const std::string& remotefilename, const std::string& localfilename, const bool bcheckmtime = true);

        /**
         * @brief 向FTP服务器上传文件
         * @param localfilename 本地文件名
         * @param remotefilename 远程保存路径
         * @param bchecksize 是否校验文件大小（确保传输完整性，默认true）
         * @return true-成功，false-失败
         * @note 传输过程中使用临时文件（remotefilename.tmp），完成后重命名
         */
        bool put(const std::string& localfilename, const std::string& remotefilename, const bool bchecksize = true);

        /**
         * @brief 删除FTP服务器上的文件
         * @param remotefilename 待删除的远程文件名
         * @return true-成功，false-失败
         */
        bool ftpdelete(const std::string& remotefilename);

        /**
         * @brief 重命名FTP服务器上的文件
         * @param srcremotefilename 远程原文件名
         * @param dstremotefilename 远程目标文件名
         * @return true-成功，false-失败
         */
        bool ftprename(const std::string& srcremotefilename, const std::string& dstremotefilename);

        /**
         * @brief 向FTP服务器发送SITE命令（站点特定命令）
         * @param command 命令内容
         * @return true-成功，false-失败
         */
        bool site(const std::string& command);

        /**
         * @brief 获取服务器最后一次响应信息(return a pointer to the last response received)
         * @return 响应信息字符串指针
         */
        char* response();
    };
#endif // __linux__

} // namespace ol

#endif // !OL_FTP_H