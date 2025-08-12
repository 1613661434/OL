/****************************************************************************************/
/*   程序名：ol_ftp.h，此程序是开发框架的ftp客户端工具的类的声明文件。                       */
/*   作者：ol                                                                            */
/****************************************************************************************/

#ifndef __OL_FTP_H
#define __OL_FTP_H 1

#include "ftplib.h"
#include "ol_chrono.h"
#include "ol_fstream.h"

namespace ol
{

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

        cftpclient();  // 类的构造函数。
        ~cftpclient(); // 类的析构函数。

        cftpclient(const cftpclient&) = delete;
        cftpclient& operator=(const cftpclient) = delete;

        void initdata(); // 初始化m_size和m_mtime成员变量。

        // 登录ftp服务器。
        // host：ftp服务器ip地址和端口，中间用":"分隔，如"192.168.1.1:21"。
        // username：登录ftp服务器用户名。
        // password：登录ftp服务器的密码。
        // imode：传输模式，1-FTPLIB_PASSIVE是被动模式，2-FTPLIB_PORT是主动模式，缺省是被动模式。
        bool login(const std::string& host, const std::string& username, const std::string& password, const int imode = FTPLIB_PASSIVE);

        // 注销。
        bool logout();

        // 获取ftp服务器上文件的时间。
        // remotefilename：待获取的文件名。
        // 返回值：false-失败；true-成功，获取到的文件时间存放在m_mtime成员变量中。
        bool mtime(const std::string& remotefilename);

        // 获取ftp服务器上文件的大小。
        // remotefilename：待获取的文件名。
        // 返回值：false-失败；true-成功，获取到的文件大小存放在m_size成员变量中。
        bool size(const std::string& remotefilename);

        // 改变ftp服务器的当前工作目录。
        // remotedir：ftp服务器上的目录名。
        // 返回值：true-成功；false-失败。
        bool chdir(const std::string& remotedir);

        // 在ftp服务器上创建目录。
        // remotedir：ftp服务器上待创建的目录名。
        // 返回值：true-成功；false-失败。
        bool mkdir(const std::string& remotedir);

        // 删除ftp服务器上的目录。
        // remotedir：ftp服务器上待删除的目录名。
        // 返回值：true-成功；如果权限不足、目录不存在或目录不为空会返回false。
        bool rmdir(const std::string& remotedir);

        // 发送NLST命令列出ftp服务器目录中的子目录名和文件名。
        // remotedir：ftp服务器的目录名。
        // listfilename：用于保存从服务器返回的目录和文件名列表。
        // 返回值：true-成功；false-失败。
        // 注意：如果列出的是ftp服务器当前目录，remotedir用"","*","."都可以，但是，不规范的ftp服务器可能有差别。
        bool nlist(const std::string& remotedir, const std::string& listfilename);

        // 从ftp服务器上获取文件。
        // remotefilename：待获取ftp服务器上的文件名。
        // localfilename：保存到本地的文件名。
        // bcheckmtime：文件传输完成后，是否核对远程文件传输前后的时间，保证文件的完整性。
        // 返回值：true-成功；false-失败。
        // 注意：文件在传输的过程中，采用临时文件命名的方法，即在localfilename后加".tmp"，在传输
        // 完成后才正式改为localfilename。
        bool get(const std::string& remotefilename, const std::string& localfilename, const bool bcheckmtime = true);

        // 向ftp服务器发送文件。
        // localfilename：本地待发送的文件名。
        // remotefilename：发送到ftp服务器上的文件名。
        // bchecksize：文件传输完成后，是否核对本地文件和远程文件的大小，保证文件的完整性。
        // 返回值：true-成功；false-失败。
        // 注意：文件在传输的过程中，采用临时文件命名的方法，即在remotefilename后加".tmp"，在传输
        // 完成后才正式改为remotefilename。
        bool put(const std::string& localfilename, const std::string& remotefilename, const bool bchecksize = true);

        // 删除ftp服务器上的文件。
        // remotefilename：待删除的ftp服务器上的文件名。
        // 返回值：true-成功；false-失败。
        bool ftpdelete(const std::string& remotefilename);

        // 重命名ftp服务器上的文件。
        // srcremotefilename：ftp服务器上的原文件名。
        // dstremotefilename：ftp服务器上的目标文件名。
        // 返回值：true-成功；false-失败。
        bool ftprename(const std::string& srcremotefilename, const std::string& dstremotefilename);

        // 向ftp服务器发送site命令。
        // command：命令的内容。
        // 返回值：true-成功；false-失败。
        bool site(const std::string& command);

        // 获取服务器最后一次接收响应的返回信息(return a pointer to the last response received)。
        char* response();
    };

} // namespace ol

#endif // !__OL_FTP_H