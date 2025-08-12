#include "../include/ol_fstream.h"
#include <iostream>

namespace ol
{
    ///////////////////////////////////// /////////////////////////////////////
    // ��ƽ̨ ACCESS ����������ļ�/Ŀ¼�����Ի�Ȩ�ޣ�
    // mode: 0=������, 2=дȨ��, 4=��Ȩ��, 6=��дȨ��
    inline int ACCESS(const char* _FileName, int _AccessMode)
    {
#ifdef __linux__
        return access(_FileName, _AccessMode);
#elif defined(_WIN32)
        return _access(_FileName, _AccessMode);
#endif
    }

    // ��ƽ̨ MKDIR ����������Ŀ¼��
    // Linux ��ָ��Ȩ�ޣ��� 0755����Windows ����Ȩ�޲���
    inline int MKDIR(const char* _Path, unsigned int _Mode)
    {
#ifdef __linux__
        return mkdir(_Path, _Mode);
#elif defined(_WIN32)
        // Windows ��֧��Ȩ�޲�����ֱ�ӵ��� _mkdir
        return _mkdir(_Path);
#endif
    }
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // ���ܣ����ݾ���·���𼶴���Ŀ¼��֧�� Windows ���̷�·����
    // ������
    //   pathorfilename - ����·����Linux�� \tmp��\tmp\a.txt��Windows�� D:\a\b\c �� D:\a\b\file.txt��
    //   bisfilename    - true ��ʾ·�������ļ�����false ��ʾ��Ŀ¼·��
    // ����ֵ��true �ɹ���false ʧ��
    bool newdir(const std::string& pathorfilename, bool bisfilename)
    {
        std::string path = pathorfilename;

#ifdef __linux__
        // ���Ŀ¼�Ƿ���ڣ���������ڣ��𼶴�����Ŀ¼
        size_t pos = 1; // ��Ҫ��0��ʼ��0�Ǹ�Ŀ¼/��
#elif defined(_WIN32)
        // 1. ͳһ·���ָ�������Windows ���� \ תΪ  /��
        std::replace(path.begin(), path.end(), '\\', '/');

        size_t pos = 0;

        // ����̷����� D:���Ƿ���Ч
        size_t colon_pos = path.find(':');
        if (colon_pos == 1) // �̷���ʽӦΪ "X:"����ĸ+ð�ţ�λ��1��ð�ţ�
        {
            if (ACCESS(path.substr(0, 2).c_str(), 0) != 0)
            {                   // ����̷��Ƿ����
                errno = ENOENT; // ���Ϊ·��������
                return false;
            }

            // ���̷�����true
            if (path.size() == 2) return true;

            // �����̷�����D:\��һ���ַ���ʼ
            pos = 3;
        }
        else // Windows ����·��������̷����� D:/����������Ϊ��Ч
        {
            errno = EINVAL; // ���Ϊ��Ч����
            return false;
        }
#endif

        while (true)
        {
            size_t pos1 = path.find('/', pos);
            if (pos1 == std::string::npos) break;

            std::string strpathname = path.substr(0, pos1); // ��ȡĿ¼��

            pos = pos1 + 1;                          // λ�ú��ơ�
            if (ACCESS(strpathname.c_str(), 0) != 0) // ���Ŀ¼�����ڣ����������ڶ�����������F_OK��
            {
                // 0755�ǰ˽��ƣ���Ҫд��755��
                if (MKDIR(strpathname.c_str(), 0755) != 0) return false; // ���Ŀ¼�����ڣ���������
            }
        }

        // ���path�����ļ�����Ŀ¼������Ҫ�������һ����Ŀ¼��
        if (bisfilename == false)
        {
            if (ACCESS(path.c_str(), 0) != 0)
            {
                if (MKDIR(path.c_str(), 0755) != 0) return false;
            }
        }

        return true;
    }
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    bool renamefile(const std::string& srcfilename, const std::string& dstfilename)
    {
        // ���ԭ�ļ��Ƿ�����ҿɶ���Linux��R_OK��Windows��_access��4��
#ifdef __linux__
        if (access(srcfilename.c_str(), R_OK) != 0) return false;
#elif defined(_WIN32)
        // Windows��_access�ĵڶ���������4��ʾ���ɶ��ԣ���ӦLinux��R_OK��
        if (_access(srcfilename.c_str(), 4) != 0) return false;
#endif

        // ����Ŀ���ļ���Ŀ¼��
        if (newdir(dstfilename, true) == false) return false;

        // ���ò���ϵͳ�Ŀ⺯��rename�������ļ���
#ifdef __linux__
        if (rename(srcfilename.c_str(), dstfilename.c_str()) == 0) return true;
#elif defined(_WIN32)
        // Windows��rename������Linux�÷���ͬ������ȷ��·����ȷ
        if (rename(srcfilename.c_str(), dstfilename.c_str()) == 0) return true;
#endif

        return false;
    }
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    bool copyfile(const std::string& srcfilename, const std::string& dstfilename)
    {
        // ����Ŀ���ļ���Ŀ¼��
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

        // �����ļ����޸�ʱ������
        std::string strmtime;
        filemtime(srcfilename, strmtime);
        setmtime(dstfilename, strmtime);

        return true;
    }
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    long filesize(const std::string& filename)
    {
#ifdef __linux__
        struct stat st_filestat;
        if (stat(filename.c_str(), &st_filestat) < 0) return -1;
#elif defined(_WIN32)
        struct _stat st_filestat;                                 // Windows��_stat�ṹ��
        if (_stat(filename.c_str(), &st_filestat) < 0) return -1; // Windows��_stat����
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
        // Linux ƽ̨ʹ�� utimbuf
        struct utimbuf stutimbuf;
        stutimbuf.actime = stutimbuf.modtime = strtotime(mtime);
        if (utime(filename.c_str(), &stutimbuf) != 0) return false;
#elif defined(_WIN32)
        // Windows ƽ̨ʹ�� _utimbuf�����»��ߣ�
        struct _utimbuf stutimbuf; // �����ṹ������
        stutimbuf.actime = stutimbuf.modtime = strtotime(mtime);
        if (_utime(filename.c_str(), &stutimbuf) != 0) return false; // ƥ�� Windows ����
#endif
        return true;
    }
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    void cdir::setfmt(const std::string& fmt)
    {
        m_fmt = fmt;
    }

    // ����һ���ݹ麯������opendir()�е��ã�cdir����ⲿ����Ҫ��������
#ifdef __linux__
    // Linux��_opendir���ݹ����Ŀ¼��
    bool cdir::_opendir(const std::string& dirname, const std::string& rules, const size_t maxfiles, const bool bandchild)
    {
        DIR* dir; // Ŀ¼ָ�롣

        // ��Ŀ¼��
        if ((dir = ::opendir(dirname.c_str())) == nullptr) return false; // opendir��⺯����������Ҫ��::

        std::string strffilename; // ȫ·�����ļ�����
        struct dirent* stdir;     // ��Ŵ�Ŀ¼�ж�ȡ�����ݡ�

        // ��ѭ����ȡĿ¼�����ݣ����õ�Ŀ¼�е��ļ�������Ŀ¼��
        while ((stdir = ::readdir(dir)) != 0) // readdir��⺯����������Ҫ��::
        {
            // �ж������е��ļ������Ƿ񳬳�maxfiles������
            if (m_filelist.size() >= maxfiles) break;

            // �ļ�����"."��ͷ���ļ�������.�ǵ�ǰĿ¼��..����һ��Ŀ¼��������.��ͷ�Ķ�������Ŀ¼���ļ���
            if (stdir->d_name[0] == '.') continue;

            // ƴ��ȫ·�����ļ�����
            strffilename = dirname + '/' + stdir->d_name;

            // �����Ŀ¼�����������Ŀ¼��
            if (stdir->d_type == 4)
            {
                if (bandchild == true) // �򿪸�����Ŀ¼��
                {
                    if (_opendir(strffilename, rules, maxfiles, bandchild) == false) // �ݹ����_opendir������
                    {
                        closedir(dir);
                        return false;
                    }
                }
            }

            // �������ͨ�ļ������������С�
            if (stdir->d_type == 8)
            {
                // ����ƥ���ϵ��ļ�����m_filelist�����С�
                if (matchstr(stdir->d_name, rules) == false) continue;

                m_filelist.push_back(std::move(strffilename));
            }
        }

        closedir(dir); // �ر�Ŀ¼��

        return true;
    }
#elif defined(_WIN32)
    // Windows��_opendir���ݹ����Ŀ¼��
    bool cdir::_opendir(const std::string& dirname, const std::string& rules, const size_t maxfiles, const bool bandchild)
    {
        // ��������·����Windows API��Ҫ\�������մ洢��/��
        std::string search_path = dirname;
        std::replace(search_path.begin(), search_path.end(), '/', '\\');
        search_path += "\\*";                                            // Windows����ͨ���

        struct _finddata_t fileinfo;
        intptr_t handle = _findfirst(search_path.c_str(), &fileinfo);
        if (handle == -1)
        {
            return false; // Ŀ¼�����ڣ�Windows API����-1��ʾʧ�ܣ�
        }

        std::string strffilename; // ȫ·�����ļ���
        do {
            // ����.��..
            if (strcmp(fileinfo.name, ".") == 0 || strcmp(fileinfo.name, "..") == 0)
            {
                continue;
            }

            // ƴ��ȫ·��
            strffilename = dirname + '\\' + fileinfo.name;

            // �ж��Ƿ�ΪĿ¼��Windows����_A_SUBDIR��ӦĿ¼��
            bool is_dir = (fileinfo.attrib & _A_SUBDIR) != 0;

            // �����Ŀ¼����Ҫ�ݹ���Ŀ¼
            if (is_dir)
            {
                if (bandchild)
                {
                    if (_opendir(strffilename, rules, maxfiles, bandchild) == false)
                    {
                        _findclose(handle); // �ر�WindowsĿ¼���
                        return false;
                    }
                }
            }
            // �������ͨ�ļ�������������
            else
            {
                if (m_filelist.size() >= maxfiles) break;

                if (true)
                {
                    m_filelist.push_back(std::move(strffilename));
                }
            }
        } while (_findnext(handle, &fileinfo) == 0); // Windows API������һ���ļ�

        _findclose(handle); // �ر�WindowsĿ¼���
        return true;
    }
#endif

    bool cdir::opendir(const std::string& dirname, const std::string& rules, const size_t maxfiles, const bool bandchild, bool bsort)
    {
        m_filelist.clear(); // ����ļ��б�������
        m_pos = 0;          // ���ļ��б����Ѷ�ȡ�ļ���λ�ù�0��

        // ���Ŀ¼�����ڣ���������
        if (newdir(dirname, false) == false) return false;

        // ��Ŀ¼����ȡĿ¼�е��ļ��б������m_filelist�����С�
        bool ret = _opendir(dirname, rules, maxfiles, bandchild);

        if (bsort == true) // ���ļ��б�����
        {
            sort(m_filelist.begin(), m_filelist.end());
        }

        return ret;
    }

    bool cdir::readdir()
    {
        // ����Ѷ��꣬�������
        if (m_pos >= m_filelist.size())
        {
            m_pos = 0;
            m_filelist.clear();
            return false;
        }

        // �ļ�ȫ��������·��
        m_ffilename = m_filelist[m_pos];

        // �Ӿ���·�����ļ����н�����Ŀ¼�����ļ�����
        // ͬʱ����Windows��\��Linux��/�ָ���
        size_t pp = m_ffilename.find_last_of("/\\"); // ͬʱ����/��\

        // �����Ŀ¼�µ��ļ�����D:/file.txt��D:\file.txt��
        if (pp == std::string::npos)
        {
            m_dirname = "";           // û��Ŀ¼��Ϊ��
            m_filename = m_ffilename; // ����·�������ļ���
        }
        else
        {
            m_dirname = m_ffilename.substr(0, pp);
            m_filename = m_ffilename.substr(pp + 1);
        }

        // ��ȡ�ļ���Ϣ������ƽ̨��
#ifdef __linux__
        struct stat st_filestat;
        if (stat(m_ffilename.c_str(), &st_filestat) != 0)  // ���stat�����Ƿ�ɹ�
        {
            m_filesize = 0;
            m_mtime = m_ctime = m_atime = "";
            return false;
        }
#elif defined(_WIN32)
        struct _stat st_filestat;
        // Windows��·�����ܰ���\����Ҫȷ��_stat��ʶ��
        if (_stat(m_ffilename.c_str(), &st_filestat) != 0)
        {
            m_filesize = 0;
            m_mtime = m_ctime = m_atime = "";
            return false;
        }
#endif

        m_filesize = st_filestat.st_size;                  // �ļ���С��
        m_mtime = timetostr1(st_filestat.st_mtime, m_fmt); // �ļ����һ�α��޸ĵ�ʱ�䡣
        m_ctime = timetostr1(st_filestat.st_ctime, m_fmt); // �ļ����ɵ�ʱ�䡣
        m_atime = timetostr1(st_filestat.st_atime, m_fmt); // �ļ����һ�α����ʵ�ʱ�䡣

        ++m_pos; // �Ѷ�ȡ�ļ���λ�ú��ơ�

        return true;
    }

    cdir::~cdir()
    {
        m_filelist.clear();
    }
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    bool cofile::open(const std::string& filename, const bool btmp, const std::ios::openmode mode, const bool benbuffer)
    {
        // ����ļ��Ǵ򿪵�״̬���ȹر�����
        if (fout.is_open()) fout.close();

        m_filename = filename;

        newdir(m_filename, true); // ����ļ���Ŀ¼�����ڣ�����Ŀ¼��

        if (btmp == true)
        { // ������ʱ�ļ��ķ�����
            m_filenametmp = m_filename + ".tmp";
            fout.open(m_filenametmp, mode);
        }
        else
        { // ��������ʱ�ļ��ķ�����
            m_filenametmp.clear();
            fout.open(m_filename, mode);
        }

        // �������ļ���������
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

    // �ر��ļ������Ұ���ʱ�ļ�����Ϊ��ʽ�ļ�����
    bool cofile::closeandrename()
    {
        if (fout.is_open() == false) return false;

        fout.close();

        //  �����������ʱ�ļ��ķ�����
        if (m_filenametmp.empty() == false)
            if (rename(m_filenametmp.c_str(), m_filename.c_str()) != 0) return false;

        return true;
    }

    // �ر��ļ���ɾ����ʱ�ļ���
    void cofile::close()
    {
        if (fout.is_open() == false) return;

        fout.close();

        //  �����������ʱ�ļ��ķ�����
        if (m_filenametmp.empty() == false)
            remove(m_filenametmp.c_str());
    }
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    bool cifile::open(const std::string& filename, const std::ios::openmode mode)
    {
        // ����ļ��Ǵ򿪵�״̬���ȹر�����
        if (fin.is_open()) fin.close();

        m_filename = filename;

        fin.open(m_filename, mode);

        return fin.is_open();
    }

    bool cifile::readline(std::string& buf, const std::string& endbz)
    {
        buf.clear(); // ���buf��

        std::string strline; // ��Ŵ��ļ��ж�ȡ��һ�С�

        while (getline(fin, strline)) // ���ļ��ж�ȡһ�С�
        {
            buf += strline; // �Ѷ�ȡ������ƴ�ӵ�buf�С�

            // ����Ƿ�ﵽ��β��־
            if (endbz == "")
                return true; // �����û�н�β��־��
            else             // ������н�β��־���жϱ����Ƿ�����˽�β��־�����û�У�������������У����ء�
            {
                if (buf.find(endbz, buf.length() - endbz.length()) != std::string::npos) return true;
            }

            buf += "\n"; // getline���ļ��ж�ȡһ�е�ʱ�򣬻�ɾ��\n�����ԣ�����Ҫ����\n����Ϊ���\n��Ӧ�ñ�ɾ����
        }

        return false;
    }

    size_t cifile::read(void* buf, const size_t bufsize)
    {
        // fin.read((char *)buf,bufsize);
        fin.read(static_cast<char*>(buf), bufsize);

        return fin.gcount(); // ���ض�ȡ���ֽ�����
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
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    bool clogfile::open(const std::string& filename, const std::ios::openmode mode, const bool bbackup, const bool benbuffer)
    {
        // �����־�ļ��Ǵ򿪵�״̬���ȹر�����
        if (fout.is_open()) fout.close();

        m_filename = filename;  // ��־�ļ�����
        m_mode = mode;          // ��ģʽ��
        m_backup = bbackup;     // �Ƿ��Զ����ݡ�
        m_enbuffer = benbuffer; // �Ƿ������ļ���������

        newdir(m_filename, true); // �����־�ļ���Ŀ¼�����ڣ���������

        fout.open(m_filename, m_mode); // ����־�ļ���

        if (m_enbuffer == false) fout << std::unitbuf; // �Ƿ������ļ���������

        return fout.is_open();
    }

    bool clogfile::backup()
    {
        // ������
        if (m_backup == false) return true;

        if (fout.is_open() == false) return false;

        const std::streamoff current_pos = fout.tellp();

        // ������ó��ִ���
        if (current_pos == -1) return false;

        // �����ǰ��־�ļ��Ĵ�С����m_maxsize��������־��
        if ((size_t)current_pos > m_maxsize * 1024 * 1024)
        {
            m_splock.lock(); // ������

            fout.close(); // �رյ�ǰ��־�ļ���

            // ƴ�ӱ�����־�ļ�����
            std::string bak_filename = m_filename + "." + ltime1("yyyymmddhh24miss");

            rename(m_filename.c_str(), bak_filename.c_str()); // �ѵ�ǰ��־�ļ�����Ϊ������־�ļ���

            fout.open(m_filename, m_mode); // ���´򿪵�ǰ��־�ļ���

            if (m_enbuffer == false) fout << std::unitbuf; // �ж��Ƿ������ļ���������

            m_splock.unlock(); // ������

            return fout.is_open();
        }

        return true;
    }
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
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
    ///////////////////////////////////// /////////////////////////////////////

} // namespace ol