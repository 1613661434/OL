#include "ol_string.h"
#include <iostream>
#include <string.h>

namespace ol
{

    char* deletelchr(char* str, const char c)
    {
        if (str == nullptr) return nullptr; // ������������ǿյ�ַ��ֱ�ӷ��أ���ֹ���������

        char* p = str;       // ָ���ַ������׵�ַ��
        while (*p == c) ++p; // �����ַ�����p��ָ����ߵ�һ������c���ַ���

        memmove(str, p, strlen(str) - (p - str) + 1); // �ѽ�β��־0Ҳ��������

        return str;
    }

    std::string& deletelchr(std::string& str, const char c)
    {
        auto pos = str.find_first_not_of(c); // ���ַ�������߲��ҵ�һ������c���ַ���λ�á�

        if (pos != 0) str.replace(0, pos, ""); // ��0-pos֮����ַ����滻�ɿա�

        return str;
    }

    char* deleterchr(char* str, const char c)
    {
        if (str == nullptr) return nullptr; // ������������ǿյ�ַ��ֱ�ӷ��أ���ֹ���������

        char* p = str;   // ָ���ַ������׵�ַ��
        char* piscc = 0; // �ұ�ȫ���ַ�c�ĵ�һ��λ�á�

        while (*p != 0) // �����ַ�����
        {
            if (*p == c && piscc == 0) piscc = p; // �����ַ�c�ĵ�һ��λ�á�
            if (*p != c) piscc = 0;               // ֻҪ��ǰ�ַ�����c�����piscc��
            ++p;
        }

        if (piscc != 0) *piscc = 0; // ��pisccλ�õ��ַ���Ϊ0����ʾ�ַ����ѽ�����

        return str;
    }

    std::string& deleterchr(std::string& str, const char c)
    {
        auto pos = str.find_last_not_of(c); // ���ַ������ұ߲��ҵ�һ������c���ַ���λ�á�

        if (pos != 0) str.erase(pos + 1); // ��pos֮����ַ�ɾ����

        return str;
    }

    char* deletelrchr(char* str, const char c)
    {
        deletelchr(str, c);
        deleterchr(str, c);

        return str;
    }

    std::string& deletelrchr(std::string& str, const char c)
    {
        deletelchr(str, c);
        deleterchr(str, c);

        return str;
    }

    char* toupper(char* str)
    {
        if (str == nullptr) return nullptr;

        char* p = str;  // ָ���ַ������׵�ַ��
        while (*p != 0) // �����ַ�����
        {
            if ((*p >= 'a') && (*p <= 'z')) *p = *p - 32;
            ++p;
        }

        return str;
    }

    std::string& toupper(std::string& str)
    {
        for (auto& c : str)
        {
            if ((c >= 'a') && (c <= 'z')) c = c - 32;
        }

        return str;
    }

    char* tolower(char* str)
    {
        if (str == nullptr) return nullptr;

        char* p = str;  // ָ���ַ������׵�ַ��
        while (*p != 0) // �����ַ�����
        {
            if ((*p >= 'A') && (*p <= 'Z')) *p = *p + 32;
            ++p;
        }

        return str;
    }

    std::string& tolower(std::string& str)
    {
        for (auto& c : str)
        {
            if ((c >= 'A') && (c <= 'Z')) c = c + 32;
        }

        return str;
    }

    bool replacestr(std::string& str, const std::string& str1, const std::string& str2, bool bloop)
    {
        // ���ԭ�ַ���str��ɵ�����str1Ϊ�գ�û�����壬��ִ���滻��
        if ((str.length() == 0) || (str1.length() == 0)) return false;

        // ���bloopΪtrue����str2�а�����str1�����ݣ�ֱ�ӷ��أ���Ϊ�������ѭ�������յ����ڴ������
        if ((bloop == true) && (str2.find(str1) != std::string::npos)) return false;

        size_t pstart = 0; // ���bloop==false����һ��ִ���滻�Ŀ�ʼλ�á�
        size_t ppos = 0;   // ������Ҫ�滻��λ�á�

        while (true)
        {
            if (bloop == true)
                ppos = str.find(str1); // ÿ�δ��ַ���������߿�ʼ�����Ӵ�str1��
            else
                ppos = str.find(str1, pstart); // ���ϴ�ִ���滻��λ�ú�ʼ�����Ӵ�str1��

            if (ppos == std::string::npos) break; // ���û���ҵ��Ӵ�str1��

            str.replace(ppos, str1.length(), str2); // ��str1�滻��str2��

            if (bloop == false) pstart = ppos + str2.length(); // ��һ��ִ���滻�Ŀ�ʼλ�������ƶ���
        }

        return true;
    }

    bool replacestr(char* str, const std::string& str1, const std::string& str2, bool bloop)
    {
        if (str == nullptr) return false;

        std::string strtemp(str);

        replacestr(strtemp, str1, str2, bloop);

        strtemp.copy(str, strtemp.length());
        str[strtemp.length()] = 0; // std::string��copy���������C����ַ����Ľ�β��0��

        return true;
    }

    char* picknumber(const std::string& src, char* dest, const bool bsigned, const bool bdot)
    {
        if (dest == nullptr) return nullptr; // �жϿ�ָ�롣

        std::string strtemp = picknumber(src, bsigned, bdot);
        strtemp.copy(dest, strtemp.length());
        dest[strtemp.length()] = 0; // std::string��copy���������C����ַ����Ľ�β��0��

        return dest;
    }

    std::string& picknumber(const std::string& src, std::string& dest, const bool bsigned, const bool bdot)
    {
        // Ϊ��֧��src��dest��ͬһ���������������str��ʱ������
        std::string str;

        for (char c : src)
        {
            // �ж��Ƿ���ȡ���š�
            if ((bsigned == true) && ((c == '+') || (c == '-')))
            {
                str.append(1, c);
                continue;
            }

            // �ж��Ƿ���ȡС���㡣
            if ((bdot == true) && (c == '.'))
            {
                str.append(1, c);
                continue;
            }

            // ��ȡ���֡�
            if (isdigit(c)) str.append(1, c);
        }

        dest = str;

        return dest;
    }

    std::string picknumber(const std::string& src, const bool bsigned, const bool bdot)
    {
        std::string dest;
        picknumber(src, dest, bsigned, bdot);
        return dest;
    }

    bool matchstr(const std::string& str, const std::string& rules)
    {
        // ���ƥ�������ʽ�������ǿյģ�����false��
        if (rules.length() == 0) return false;

        // ������ƥ�������ʽ��������"*"��ֱ�ӷ���true��
        if (rules == "*") return true;

        size_t pos1, pos2;
        ccmdstr cmdstr, cmdsubstr;

        std::string filename = str;
        std::string matchstr = rules;

        // ���ַ�����ת���ɴ�д�������Ƚ�
        toupper(filename);
        toupper(matchstr);

        cmdstr.splittocmd(matchstr, ",");

        for (size_t i = 0, cmdstr_size = cmdstr.size(); i < cmdstr_size; ++i)
        {
            // ���Ϊ�գ���һ��Ҫ����������ͻᱻƥ���ϡ�
            if (cmdstr[i].empty() == true) continue;

            pos1 = pos2 = 0;
            cmdsubstr.splittocmd(cmdstr[i], "*");

            size_t j, cmdsubstr_size = cmdsubstr.size();
            for (j = 0; j < cmdsubstr_size; ++j)
            {
                // ������ļ������ײ�
                if (j == 0)
                    if (filename.substr(0, cmdsubstr[j].length()) != cmdsubstr[j]) break;

                // ������ļ�����β��
                if (j == cmdsubstr_size - 1)
                    if (filename.find(cmdsubstr[j], filename.length() - cmdsubstr[j].length()) == std::string::npos) break;

                pos2 = filename.find(cmdsubstr[j], pos1);

                if (pos2 == std::string::npos) break;

                pos1 = pos2 + cmdsubstr[j].length();
            }

            if (j == cmdsubstr_size) return true;
        }

        return false;
    }

    ccmdstr::ccmdstr(const std::string& buffer, const std::string& sepstr, const bool bdelspace)
    {
        splittocmd(buffer, sepstr, bdelspace);
    }

    // ���ַ�����ֵ�m_cmdstr�����С�
    // buffer������ֵ��ַ�����
    // sepstr��buffer�ַ������ֶ����ݵķָ�����ע�⣬�ָ������ַ�������","��" "��"|"��"~!~"��
    // bdelspace���Ƿ�ɾ����ֺ���ֶ�����ǰ��Ŀո�true-ɾ����false-��ɾ����ȱʡ��ɾ����
    void ccmdstr::splittocmd(const std::string& buffer, const std::string& sepstr, const bool bdelspace)
    {
        // ������еľ�����
        m_cmdstr.clear();

        // �߽�����1��������ֱ�ӷ���
        if (buffer.empty()) return;

        // �߽�����2���շָ����������ַ�����Ϊһ���ֶΣ�
        if (sepstr.empty())
        {
            std::string temp = buffer;        // ����ԭʼ�ַ���
            if (bdelspace) deletelrchr(temp); // ����ȥ�ո�����
            m_cmdstr.push_back(std::move(temp));
            return;
        }

        const size_t sepLen = sepstr.length();
        size_t pos = 0;     // ÿ�δ�buffer�в��ҷָ�������ʼλ�á�
        size_t next = 0;    // ��pos��λ�ÿ�ʼ��������һ���ָ�����λ�á�
        std::string substr; // ���ÿ�β�ֳ������Ӵ���

        // Ԥ�����ڴ棺���ٶ�̬���ݣ�Ԥ���ֶ��������Ԥ����32������������ģ�
        m_cmdstr.reserve(std::min(static_cast<size_t>(32), static_cast<size_t>(buffer.size() / sepLen + 2)));

        while ((next = buffer.find(sepstr, pos)) != std::string::npos) // ��pos��λ�ÿ�ʼ��������һ���ָ�����λ�á�
        {
            substr = buffer.substr(pos, next - pos); // ��buffer�н�ȡ�Ӵ���

            if (bdelspace == true) deletelrchr(substr); // ɾ���Ӵ�ǰ��Ŀո�

            m_cmdstr.push_back(std::move(substr)); // ���Ӵ�����m_cmdstr�����У�����std::string����ƶ����캯����

            pos = next + sepLen; // �´δ�buffer�в��ҷָ�������ʼλ�ú��ơ�
        }

        // �������һ���ֶΣ����һ���ָ���֮������ݣ���
        substr = buffer.substr(pos);

        if (bdelspace == true) deletelrchr(substr);

        m_cmdstr.push_back(std::move(substr));
    }

    bool ccmdstr::getvalue(const size_t i, std::string& value, const size_t len) const
    {
        if (i >= m_cmdstr.size()) return false;

        size_t itmplen = m_cmdstr[i].length();
        if ((len > 0) && (len < itmplen)) itmplen = len;
        value = m_cmdstr[i].substr(0, itmplen);

        return true;
    }

    bool ccmdstr::getvalue(const size_t i, char* value, const size_t len) const
    {
        if ((i >= m_cmdstr.size()) || (value == nullptr)) return false;

        if (len > 0) memset(value, 0, len + 1); // �����߱��뱣֤value�Ŀռ��㹻������������ڴ������

        if ((m_cmdstr[i].length() <= len) || (len == 0))
        {
            m_cmdstr[i].copy(value, m_cmdstr[i].length());
            value[m_cmdstr[i].length()] = '\0'; // std::string��copy���������C����ַ����Ľ�β��'\0'��
        }
        else
        {
            m_cmdstr[i].copy(value, len);
            value[len] = '\0';
        }

        return true;
    }

    bool ccmdstr::getvalue(const size_t i, int& value) const
    {
        if (i >= m_cmdstr.size()) return false;

        try
        {
            value = stoi(picknumber(m_cmdstr[i], true)); // stoi���쳣����Ҫ�����쳣��
        }
        catch (const std::exception&)
        {
            return false;
        }

        return true;
    }

    bool ccmdstr::getvalue(const size_t i, unsigned int& value) const
    {
        if (i >= m_cmdstr.size()) return false;

        try
        {
            value = stoi(picknumber(m_cmdstr[i])); // stoi���쳣����Ҫ�����쳣������ȡ���� + -
        }
        catch (const std::exception&)
        {
            return false;
        }

        return true;
    }

    bool ccmdstr::getvalue(const size_t i, long& value) const
    {
        if (i >= m_cmdstr.size()) return false;

        try
        {
            value = stol(picknumber(m_cmdstr[i], true)); // stol���쳣����Ҫ�����쳣��
        }
        catch (const std::exception&)
        {
            return false;
        }

        return true;
    }

    bool ccmdstr::getvalue(const size_t i, unsigned long& value) const
    {
        if (i >= m_cmdstr.size()) return false;

        try
        {
            value = stoul(picknumber(m_cmdstr[i])); // stoul���쳣����Ҫ�����쳣������ȡ���� + -
        }
        catch (const std::exception&)
        {
            return false;
        }

        return true;
    }

    bool ccmdstr::getvalue(const size_t i, double& value) const
    {
        if (i >= m_cmdstr.size()) return false;

        try
        {
            value = stod(picknumber(m_cmdstr[i], true, true)); // stod���쳣����Ҫ�����쳣����ȡ���ź�С���㡣
        }
        catch (const std::exception&)
        {
            return false;
        }

        return true;
    }

    bool ccmdstr::getvalue(const size_t i, float& value) const
    {
        if (i >= m_cmdstr.size()) return false;

        try
        {
            value = stof(picknumber(m_cmdstr[i], true, true)); // stof���쳣����Ҫ�����쳣����ȡ���ź�С���㡣
        }
        catch (const std::exception&)
        {
            return false;
        }

        return true;
    }

    bool ccmdstr::getvalue(const size_t i, bool& value) const
    {
        if (i >= m_cmdstr.size()) return false;

        std::string str = m_cmdstr[i];
        toupper(str); // ת��Ϊ��д���жϡ�

        if (str == "TRUE")
            value = true;
        else
            value = false;

        return true;
    }

    ccmdstr::~ccmdstr()
    {
        m_cmdstr.clear();
    }

    std::ostream& operator<<(std::ostream& out, const ccmdstr& cmdstr)
    {
        for (size_t i = 0, cmdstr_size = cmdstr.size(); i < cmdstr_size; ++i)
            out << "[" << i << "]=" << cmdstr[i] << std::endl;

        return out;
    }

    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, std::string& value, const size_t len)
    {
        std::string start = "<" + fieldname + ">"; // �����ʼ�ı�ǩ��
        std::string end = "</" + fieldname + ">";  // ����������ı�ǩ��

        size_t startp = xmlbuffer.find(start); // ��xml�в��������ʼ�ı�ǩ��λ�á�
        if (startp == std::string::npos) return false;

        size_t endp = xmlbuffer.find(end); // ��xml�в�������������ı�ǩ��λ�á�
        if (endp == std::string::npos) return false;

        // ��xml�н�ȡ����������ݡ�
        // ��Ƶ�������´��룺
        // value=xmlbuffer.substr(startp+start.length(),endp-startp-start.length());
        // ��Ϊ��
        size_t itmplen = endp - startp - start.length();
        if ((len > 0) && (len < itmplen)) itmplen = len;
        value = xmlbuffer.substr(startp + start.length(), itmplen);

        return true;
    }

    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, char* value, const size_t len)
    {
        if (value == nullptr) return false;

        if (len > 0) memset(value, 0, len + 1); // �����߱��뱣֤value�Ŀռ��㹻������������ڴ������

        std::string str;
        getxmlbuffer(xmlbuffer, fieldname, str);

        if ((str.length() <= len) || (len == 0))
        {
            str.copy(value, str.length());
            value[str.length()] = '\0'; // std::string��copy���������C����ַ����Ľ�β��0��
        }
        else
        {
            str.copy(value, len);
            value[len] = '\0';
        }

        return true;
    }

    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, bool& value)
    {
        std::string str;
        if (getxmlbuffer(xmlbuffer, fieldname, str) == false) return false;

        toupper(str); // ת��Ϊ��д���жϣ�Ҳ����ת��ΪСд��Ч����ͬ����

        if (str == "TRUE")
            value = true;
        else
            value = false;

        return true;
    }

    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, int& value)
    {
        std::string str;

        if (getxmlbuffer(xmlbuffer, fieldname, str) == false) return false;

        try
        {
            value = stoi(picknumber(str, true)); // stoi���쳣����Ҫ�����쳣��
        }
        catch (const std::exception&)
        {
            return false;
        }

        return true;
    }

    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, unsigned int& value)
    {
        std::string str;

        if (getxmlbuffer(xmlbuffer, fieldname, str) == false) return false;

        try
        {
            value = stoi(picknumber(str)); // stoi���쳣����Ҫ�����쳣������ȡ���� + -
        }
        catch (const std::exception&)
        {
            return false;
        }

        return true;
    }

    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, long& value)
    {
        std::string str;

        if (getxmlbuffer(xmlbuffer, fieldname, str) == false) return false;

        try
        {
            value = stol(picknumber(str, true)); // stol���쳣����Ҫ�����쳣��
        }
        catch (const std::exception&)
        {
            return false;
        }

        return true;
    }

    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, unsigned long& value)
    {
        std::string str;

        if (getxmlbuffer(xmlbuffer, fieldname, str) == false) return false;

        try
        {
            value = stoul(picknumber(str)); // stoul���쳣����Ҫ�����쳣������ȡ���� + -
        }
        catch (const std::exception&)
        {
            return false;
        }

        return true;
    }

    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, double& value)
    {
        std::string str;

        if (getxmlbuffer(xmlbuffer, fieldname, str) == false) return false;

        try
        {
            value = stod(picknumber(str, true, true)); // stod���쳣����Ҫ�����쳣����ȡ���ź�С���㡣
        }
        catch (const std::exception&)
        {
            return false;
        }

        return true;
    }

    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, float& value)
    {
        std::string str;

        if (getxmlbuffer(xmlbuffer, fieldname, str) == false) return false;

        try
        {
            value = stof(picknumber(str, true, true)); // stof���쳣����Ҫ�����쳣����ȡ���ź�С���㡣
        }
        catch (const std::exception&)
        {
            return false;
        }

        return true;
    }

    size_t skmp(const std::string& str, const std::string& pattern)
    {
        const size_t n = str.size();
        const size_t m = pattern.size();
        if (m == 0) return 0;

        // ��̬next���飬��ʼ��Ϊδ����״̬
        std::vector<size_t> next(m, SIZE_MAX);
        next[0] = 0; // ��λ�̶�Ϊ0

        size_t len = 0; // ��ǰ�ƥ��ǰ��׺����
        size_t i = 0, j = 0;

        while (i < n)
        {
            if (str[i] == pattern[j])
            {
                // ��̬���㲢����nextֵ
                if (j > 0 && next[j] == SIZE_MAX)
                {
                    next[j] = len; // ���浱ǰ����

                    // ��չ�������������Ҫ��nextֵ
                    size_t k = j, l = len;
                    while (++k < m && pattern[k] == pattern[l])
                    {
                        next[k] = ++l;
                    }
                }

                ++i;
                if (++j == m) return i - m; // ƥ��ɹ�
            }
            else
            {
                if (j > 0)
                {
                    // ʹ���Ѽ����nextֵ����
                    len = next[j - 1];
                    j = (len != SIZE_MAX) ? len : 0;
                }
                else
                {
                    ++i;
                }
                len = j; // ���õ�ǰƥ�䳤��
            }
        }
        return std::string::npos;
    }

} // namespace ol