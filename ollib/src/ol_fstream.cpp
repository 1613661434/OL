#include "ol_fstream.h"
#include <iostream>

namespace ol
{
    // ===========================================================================
    // 跨平台 ACCESS 函数（检查文件/目录存在性或权限）
    // mode: 0=检查存在, 2=写权限, 4=读权限, 6=读写权限
    inline int ACCESS(const char* _FileName, int _AccessMode)
    {
#ifdef __linux__
        return access(_FileName, _AccessMode);
#elif defined(_WIN32)
        return _access(_FileName, _AccessMode);
#endif
    }

    // 跨平台 MKDIR 函数（创建目录）
    // Linux 需指定权限（如 0755），Windows 忽略权限参数
    inline int MKDIR(const char* _Path, unsigned int _Mode)
    {
#ifdef __linux__
        return mkdir(_Path, _Mode);
#elif defined(_WIN32)
        // Windows 不支持权限参数，直接调用 _mkdir
        return _mkdir(_Path);
#endif
    }
    // ===========================================================================

    // ===========================================================================
    // 功能：根据绝对路径逐级创建目录（支持 Windows 带盘符路径）
    // 参数：
    //   pathorfilename - 绝对路径（Linux如 \tmp或\tmp\a.txt；Windows如 D:\a\b\c 或 D:\a\b\file.txt）
    //   bisfilename    - true 表示路径包含文件名，false 表示纯目录路径
    // 返回值：true 成功，false 失败
    bool newdir(const std::string& pathorfilename, bool bisfilename)
    {
        std::string path = pathorfilename;

#ifdef __linux__
        // 检查目录是否存在，如果不存在，逐级创建子目录
        size_t pos = 1; // 不要从0开始，0是根目录/。
#elif defined(_WIN32)
        // 1. 统一路径分隔符（将Windows 风格的 \ 转为  /）
        std::replace(path.begin(), path.end(), '\\', '/');

        size_t pos = 0;

        // 检查盘符（如 D:）是否有效
        size_t colon_pos = path.find(':');
        if (colon_pos == 1) // 盘符格式应为 "X:"（字母+冒号，位置1是冒号）
        {
            if (ACCESS(path.substr(0, 2).c_str(), 0) != 0)
            {                   // 检查盘符是否存在
                errno = ENOENT; // 标记为路径不存在
                return false;
            }

            // 纯盘符返回true
            if (path.size() == 2) return true;

            // 跳过盘符，从D:\下一个字符开始
            pos = 3;
        }
        else // Windows 绝对路径必须带盘符（如 D:/），否则视为无效
        {
            errno = EINVAL; // 标记为无效参数
            return false;
        }
#endif

        while (true)
        {
            size_t pos1 = path.find('/', pos);
            if (pos1 == std::string::npos) break;

            std::string strpathname = path.substr(0, pos1); // 截取目录。

            pos = pos1 + 1;                          // 位置后移。
            if (ACCESS(strpathname.c_str(), 0) != 0) // 如果目录不存在，创建它，第二个参数即是F_OK。
            {
                // 0755是八进制，不要写成755。
                if (MKDIR(strpathname.c_str(), 0755) != 0) return false; // 如果目录不存在，创建它。
            }
        }

        // 如果path不是文件，是目录，还需要创建最后一级子目录。
        if (bisfilename == false)
        {
            if (ACCESS(path.c_str(), 0) != 0)
            {
                if (MKDIR(path.c_str(), 0755) != 0) return false;
            }
        }

        return true;
    }
    // ===========================================================================

    // ===========================================================================
    bool renamefile(const std::string& srcfilename, const std::string& dstfilename)
    {
        // 检查原文件是否存在且可读（Linux用R_OK，Windows用_access的4）
#ifdef __linux__
        if (access(srcfilename.c_str(), R_OK) != 0) return false;
#elif defined(_WIN32)
        // Windows中_access的第二个参数：4表示检查可读性（对应Linux的R_OK）
        if (_access(srcfilename.c_str(), 4) != 0) return false;
#endif

        // 创建目标文件的目录。
        if (newdir(dstfilename, true) == false) return false;

        // 调用操作系统的库函数rename重命名文件。
#ifdef __linux__
        if (rename(srcfilename.c_str(), dstfilename.c_str()) == 0) return true;
#elif defined(_WIN32)
        // Windows的rename函数与Linux用法相同，但需确保路径正确
        if (rename(srcfilename.c_str(), dstfilename.c_str()) == 0) return true;
#endif

        return false;
    }
    // ===========================================================================

    // ===========================================================================
    bool copyfile(const std::string& srcfilename, const std::string& dstfilename)
    {
        // 创建目标文件的目录。
        if (newdir(dstfilename, true) == false) return false;

        cifile ifile;
        cofile ofile;
        size_t ifilesize = filesize(srcfilename);

        size_t total_bytes = 0;
        size_t onread = 0;
        char buffer[5000];

        if (ifile.open(srcfilename, std::ios::in | std::ios::binary) == false) return false;

        if (ofile.open(dstfilename, std::ios::out | std::ios::binary) == false) return false;

        while (true)
        {
            if ((ifilesize - total_bytes) > 5000)
                onread = 5000;
            else
                onread = ifilesize - total_bytes;

            memset(buffer, 0, sizeof(buffer));
            ifile.read(buffer, onread);
            ofile.write(buffer, onread);

            total_bytes = total_bytes + onread;

            if (total_bytes == ifilesize) break;
        }

        ifile.close();
        ofile.closeandrename();

        // 更改文件的修改时间属性
        std::string strmtime;
        filemtime(srcfilename, strmtime);
        setmtime(dstfilename, strmtime);

        return true;
    }
    // ===========================================================================

    // ===========================================================================
    long filesize(const std::string& filename)
    {
#ifdef __linux__
        struct stat st_filestat;
        if (stat(filename.c_str(), &st_filestat) < 0) return -1;
#elif defined(_WIN32)
        struct _stat st_filestat;                                 // Windows用_stat结构体
        if (_stat(filename.c_str(), &st_filestat) < 0) return -1; // Windows用_stat函数
#endif
        return st_filestat.st_size;
    }

    bool filemtime(const std::string& filename, std::string& mtime, const std::string& fmt)
    {
#ifdef __linux__
        struct stat st_filestat;
        if (stat(filename.c_str(), &st_filestat) < 0) return false;
#elif defined(_WIN32)
        struct _stat st_filestat;
        if (_stat(filename.c_str(), &st_filestat) < 0) return false;
#endif
        timetostr(st_filestat.st_mtime, mtime, fmt);
        return true;
    }

    bool filemtime(const std::string& filename, char* mtime, const std::string& fmt)
    {
#ifdef __linux__
        struct stat st_filestat;
        if (stat(filename.c_str(), &st_filestat) < 0) return false;
#elif defined(_WIN32)
        struct _stat st_filestat;
        if (_stat(filename.c_str(), &st_filestat) < 0) return false;
#endif
        timetostr(st_filestat.st_mtime, mtime, fmt);
        return true;
    }

    bool setmtime(const std::string& filename, const std::string& mtime)
    {
#ifdef __linux__
        // Linux 平台使用 utimbuf
        struct utimbuf stutimbuf;
        stutimbuf.actime = stutimbuf.modtime = strtotime(mtime);
        if (utime(filename.c_str(), &stutimbuf) != 0) return false;
#elif defined(_WIN32)
        // Windows 平台使用 _utimbuf（带下划线）
        struct _utimbuf stutimbuf; // 修正结构体名称
        stutimbuf.actime = stutimbuf.modtime = strtotime(mtime);
        if (_utime(filename.c_str(), &stutimbuf) != 0) return false; // 匹配 Windows 函数
#endif
        return true;
    }
    // ===========================================================================

    // ===========================================================================
    void cdir::setfmt(const std::string& fmt)
    {
        m_fmt = fmt;
    }

    // 这是一个递归函数，在opendir()中调用，cdir类的外部不需要调用它。
#ifdef __linux__
    // Linux的_opendir（递归遍历目录）
    bool cdir::_opendir(const std::string& dirname, const std::string& rules, const size_t maxfiles, const bool bandchild, const bool bwithDotFiles)
    {
        DIR* dir; // 目录指针。

        // 打开目录。
        if ((dir = ::opendir(dirname.c_str())) == nullptr) return false; // opendir与库函数重名，需要加::

        std::string strffilename; // 全路径的文件名。
        struct dirent* stdir;     // 存放从目录中读取的内容。

        // 用循环读取目录的内容，将得到目录中的文件名和子目录。
        while ((stdir = ::readdir(dir)) != 0) // readdir与库函数重名，需要加::
        {
            // 判断容器中的文件数量是否超出maxfiles参数。
            if (m_filelist.size() >= maxfiles) break;

            // 跳过.和..（.是当前目录，..是上一级目录）
            if (strcmp(stdir->d_name, ".") == 0 || strcmp(stdir->d_name, "..") == 0) continue;

            // .开头的特殊目录和文件。
            if (!bwithDotFiles && stdir->d_name[0] == '.') continue;

            // 拼接全路径的文件名。
            strffilename = dirname + '/' + stdir->d_name;

            // 如果是目录，处理各级子目录。
            if (stdir->d_type == 4)
            {
                if (bandchild == true) // 打开各级子目录。
                {
                    if (_opendir(strffilename, rules, maxfiles, bandchild, bwithDotFiles) == false) // 递归调用_opendir函数。
                    {
                        closedir(dir);
                        return false;
                    }
                }
            }

            // 如果是普通文件，放入容器中。
            if (stdir->d_type == 8)
            {
                // 把能匹配上的文件放入m_filelist容器中。
                if (matchstr(stdir->d_name, rules) == false) continue;

                m_filelist.push_back(std::move(strffilename));
            }
        }

        closedir(dir); // 关闭目录。

        return true;
    }
#elif defined(_WIN32)
    // Windows的_opendir（递归遍历目录）
    bool cdir::_opendir(const std::string& dirname, const std::string& rules, const size_t maxfiles, const bool bandchild, const bool bwithDotFiles)
    {
        // 构建搜索路径
        std::string search_path = dirname;
        std::replace(search_path.begin(), search_path.end(), '/', '\\');
        search_path += "\\*"; // Windows搜索通配符

        struct _finddata_t fileinfo;
        intptr_t handle = _findfirst(search_path.c_str(), &fileinfo);
        if (handle == -1)
        {
            return false; // 目录不存在（Windows API返回-1表示失败）
        }

        std::string strffilename; // 全路径的文件名
        do {
            // 跳过.和..（.是当前目录，..是上一级目录）
            if (strcmp(fileinfo.name, ".") == 0 || strcmp(fileinfo.name, "..") == 0) continue;

            // .开头的特殊目录和文件。
            if (!bwithDotFiles && fileinfo.name[0] == '.') continue;

            // 拼接全路径
            strffilename = dirname + '\\' + fileinfo.name;

            // 判断是否为目录（Windows属性_A_SUBDIR对应目录）
            bool is_dir = (fileinfo.attrib & _A_SUBDIR) != 0;

            // 如果是目录且需要递归子目录
            if (is_dir)
            {
                if (bandchild)
                {
                    if (_opendir(strffilename, rules, maxfiles, bandchild, bwithDotFiles) == false)
                    {
                        _findclose(handle); // 关闭Windows目录句柄
                        return false;
                    }
                }
            }
            // 如果是普通文件，放入容器中
            else
            {
                if (m_filelist.size() >= maxfiles) break;

                // 把能匹配上的文件放入m_filelist容器中
                if (matchstr(fileinfo.name, rules) == false) continue;

                m_filelist.push_back(std::move(strffilename));
            }
        } while (_findnext(handle, &fileinfo) == 0); // Windows API遍历下一个文件

        _findclose(handle); // 关闭Windows目录句柄
        return true;
    }
#endif

    bool cdir::opendir(const std::string& dirname, const std::string& rules, const size_t maxfiles, const bool bandchild, bool bsort, const bool bwithDotFiles)
    {
        m_filelist.clear(); // 清空文件列表容器。
        m_pos = 0;          // 从文件列表中已读取文件的位置归0。

        // 如果目录不存在，创建它。
        if (newdir(dirname, false) == false) return false;

        // 打开目录，获取目录中的文件列表，存放在m_filelist容器中。
        bool ret = _opendir(dirname, rules, maxfiles, bandchild, bwithDotFiles);

        if (bsort == true) // 对文件列表排序。
        {
            sort(m_filelist.begin(), m_filelist.end());
        }

        return ret;
    }

    bool cdir::readdir()
    {
        // 如果已读完，清空容器
        if (m_pos >= m_filelist.size())
        {
            m_pos = 0;
            m_filelist.clear();
            return false;
        }

        // 文件全名，包括路径
        m_ffilename = m_filelist[m_pos];

        // 从绝对路径的文件名中解析出目录名和文件名。
        // 同时处理Windows的\和Linux的/分隔符
        size_t pp = m_ffilename.find_last_of("/\\"); // 同时查找'/'和'\'

        // 处理根目录下的文件（如D:/file.txt或D:\file.txt）
        if (pp == std::string::npos)
        {
            m_dirname = "";           // 没有目录，为空
            m_filename = m_ffilename; // 整个路径就是文件名
        }
        else
        {
            m_dirname = m_ffilename.substr(0, pp);
            m_filename = m_ffilename.substr(pp + 1);
        }

        // 获取文件信息（区分平台）
#ifdef __linux__
        struct stat st_filestat;
        if (stat(m_ffilename.c_str(), &st_filestat) != 0) // 检查stat调用是否成功
        {
            m_filesize = 0;
            m_mtime = m_ctime = m_atime = "";
            return false;
        }
#elif defined(_WIN32)
        struct _stat st_filestat;
        // Windows下路径可能包含\，需要确保_stat能识别
        if (_stat(m_ffilename.c_str(), &st_filestat) != 0)
        {
            m_filesize = 0;
            m_mtime = m_ctime = m_atime = "";
            return false;
        }
#endif

        m_filesize = st_filestat.st_size;                  // 文件大小。
        m_mtime = timetostr1(st_filestat.st_mtime, m_fmt); // 文件最后一次被修改的时间。
        m_ctime = timetostr1(st_filestat.st_ctime, m_fmt); // 文件生成的时间。
        m_atime = timetostr1(st_filestat.st_atime, m_fmt); // 文件最后一次被访问的时间。

        ++m_pos; // 已读取文件的位置后移。

        return true;
    }

    cdir::~cdir()
    {
        m_filelist.clear();
    }
    // ===========================================================================

    // ===========================================================================
    bool cofile::open(const std::string& filename, const bool btmp, const std::ios::openmode mode, const bool benbuffer)
    {
        // 如果文件是打开的状态，先关闭它。
        if (fout.is_open()) fout.close();

        m_filename = filename;

        newdir(m_filename, true); // 如果文件的目录不存在，创建目录。

        if (btmp == true)
        { // 采用临时文件的方案。
            m_filenametmp = m_filename + ".tmp";
            fout.open(m_filenametmp, mode);
        }
        else
        { // 不采用临时文件的方案。
            m_filenametmp.clear();
            fout.open(m_filename, mode);
        }

        // 不启用文件缓冲区。
        if (benbuffer == false) fout << std::unitbuf;

        return fout.is_open();
    }

    bool cofile::write(void* buf, int bufsize)
    {
        if (fout.is_open() == false) return false;

        // fout.write((char *)buf,bufsize);
        fout.write(static_cast<char*>(buf), bufsize);

        return fout.good();
    }

    // 关闭文件，并且把临时文件名改为正式文件名。
    bool cofile::closeandrename()
    {
        if (fout.is_open() == false) return false;

        fout.close();

        //  如果采用了临时文件的方案。
        if (m_filenametmp.empty() == false)
            if (rename(m_filenametmp.c_str(), m_filename.c_str()) != 0) return false;

        return true;
    }

    // 关闭文件，删除临时文件。
    void cofile::close()
    {
        if (fout.is_open() == false) return;

        fout.close();

        //  如果采用了临时文件的方案。
        if (m_filenametmp.empty() == false)
            remove(m_filenametmp.c_str());
    }
    // ===========================================================================

    // ===========================================================================
    bool cifile::open(const std::string& filename, const std::ios::openmode mode)
    {
        // 如果文件是打开的状态，先关闭它。
        if (fin.is_open()) fin.close();

        m_filename = filename;

        fin.open(m_filename, mode);

        return fin.is_open();
    }

    bool cifile::readline(std::string& buf, const std::string& endbz)
    {
        buf.clear(); // 清空buf。

        std::string strline; // 存放从文件中读取的一行。

        while (getline(fin, strline)) // 从文件中读取一行。
        {
            buf += strline; // 把读取的内容拼接到buf中。

            // 检查是否达到结尾标志
            if (endbz == "")
                return true; // 如果行没有结尾标志。
            else             // 如果行有结尾标志，判断本次是否读到了结尾标志，如果没有，继续读；如果有，返回。
            {
                if (buf.find(endbz, buf.length() - endbz.length()) != std::string::npos) return true;
            }

            buf += '\n'; // getline从文件中读取一行的时候，会删除\n，所以，这里要补上\n，因为这个\n不应该被删除。
        }

        return false;
    }

    size_t cifile::read(void* buf, const size_t bufsize)
    {
        // fin.read((char *)buf,bufsize);
        fin.read(static_cast<char*>(buf), bufsize);

        return fin.gcount(); // 返回读取的字节数。
    }

    bool cifile::closeandremove()
    {
        if (fin.is_open() == false) return false;

        fin.close();

        if (remove(m_filename.c_str()) != 0) return false;

        return true;
    }

    void cifile::close()
    {
        if (fin.is_open() == false) return;

        fin.close();
    }
    // ===========================================================================

    // ===========================================================================
    bool clogfile::open(const std::string& filename, const std::ios::openmode mode, const bool bbackup, const bool benbuffer)
    {
        // 如果日志文件是打开的状态，先关闭它。
        if (fout.is_open()) fout.close();

        m_filename = filename;  // 日志文件名。
        m_mode = mode;          // 打开模式。
        m_backup = bbackup;     // 是否自动备份。
        m_enbuffer = benbuffer; // 是否启用文件缓冲区。

        newdir(m_filename, true); // 如果日志文件的目录不存在，创建它。

        fout.open(m_filename, m_mode); // 打开日志文件。

        if (m_enbuffer == false) fout << std::unitbuf; // 是否启用文件缓冲区。

        return fout.is_open();
    }

    bool clogfile::backup()
    {
        // 不备份
        if (m_backup == false) return true;

        if (fout.is_open() == false) return false;

        const std::streamoff current_pos = fout.tellp();

        // 如果调用出现错误
        if (current_pos == -1) return false;

        // 如果当前日志文件的大小超过m_maxsize，备份日志。
        if ((size_t)current_pos > m_maxsize * 1024 * 1024)
        {
            m_splock.lock(); // 加锁。

            fout.close(); // 关闭当前日志文件。

            // 拼接备份日志文件名。
            std::string bak_filename = m_filename + "." + ltime1("yyyymmddhh24miss");

            rename(m_filename.c_str(), bak_filename.c_str()); // 把当前日志文件改名为备份日志文件。

            fout.open(m_filename, m_mode); // 重新打开当前日志文件。

            if (m_enbuffer == false) fout << std::unitbuf; // 判断是否启动文件缓冲区。

            m_splock.unlock(); // 解锁。

            return fout.is_open();
        }

        return true;
    }
    // ===========================================================================

    // ===========================================================================
    std::ostream& nl(std::ostream& os)
    {
        return os << '\n';
    }

    std::ostream& operator<<(std::ostream& os, const binary_t& b)
    {
        return os << std::bitset<32>(b.value);
    }

    std::istream& clearbuf(std::istream& is)
    {
        return is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    // ===========================================================================

} // namespace ol