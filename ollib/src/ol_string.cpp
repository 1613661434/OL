#include "ol_string.h"
#include <iostream>
#include <string.h>

namespace ol
{

    char* deleteLchr(char* str, const char c)
    {
        if (str == nullptr) return nullptr; // 如果传进来的是空地址，直接返回，防止程序崩溃。

        char* p = str;       // 指向字符串的首地址。
        while (*p == c) ++p; // 遍历字符串，p将指向左边第一个不是c的字符。

        memmove(str, p, strlen(str) - (p - str) + 1); // 把结尾标志0也拷过来。

        return str;
    }

    std::string& deleteLchr(std::string& str, const char c)
    {
        auto pos = str.find_first_not_of(c); // 从字符串的左边查找第一个不是c的字符的位置。

        if (pos != 0) str.replace(0, pos, ""); // 把0-pos之间的字符串替换成空。

        return str;
    }

    char* deleteRchr(char* str, const char c)
    {
        if (str == nullptr) return nullptr; // 如果传进来的是空地址，直接返回，防止程序崩溃。

        char* p = str;   // 指向字符串的首地址。
        char* piscc = 0; // 右边全是字符c的第一个位置。

        while (*p != 0) // 遍历字符串。
        {
            if (*p == c && piscc == 0) piscc = p; // 记下字符c的第一个位置。
            if (*p != c) piscc = 0;               // 只要当前字符不是c，清空piscc。
            ++p;
        }

        if (piscc != 0) *piscc = 0; // 把piscc位置的字符置为0，表示字符串已结束。

        return str;
    }

    std::string& deleteRchr(std::string& str, const char c)
    {
        auto pos = str.find_last_not_of(c); // 从字符串的右边查找第一个不是c的字符的位置。

        if (pos != 0) str.erase(pos + 1); // 把pos之后的字符删掉。

        return str;
    }

    char* deleteLRchr(char* str, const char c)
    {
        deleteLchr(str, c);
        deleteRchr(str, c);

        return str;
    }

    std::string& deleteLRchr(std::string& str, const char c)
    {
        deleteLchr(str, c);
        deleteRchr(str, c);

        return str;
    }

    char* toupper(char* str)
    {
        if (str == nullptr) return nullptr;

        char* p = str;  // 指向字符串的首地址。
        while (*p != 0) // 遍历字符串。
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

        char* p = str;  // 指向字符串的首地址。
        while (*p != 0) // 遍历字符串。
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
        // 如果原字符串str或旧的内容str1为空，没有意义，不执行替换。
        if ((str.length() == 0) || (str1.length() == 0)) return false;

        // 如果bloop为true并且str2中包函了str1的内容，直接返回，因为会进入死循环，最终导致内存溢出。
        if ((bloop == true) && (str2.find(str1) != std::string::npos)) return false;

        size_t pstart = 0; // 如果bloop==false，下一次执行替换的开始位置。
        size_t ppos = 0;   // 本次需要替换的位置。

        while (true)
        {
            if (bloop == true)
                ppos = str.find(str1); // 每次从字符串的最左边开始查找子串str1。
            else
                ppos = str.find(str1, pstart); // 从上次执行替换的位置后开始查找子串str1。

            if (ppos == std::string::npos) break; // 如果没有找到子串str1。

            str.replace(ppos, str1.length(), str2); // 把str1替换成str2。

            if (bloop == false) pstart = ppos + str2.length(); // 下一次执行替换的开始位置往右移动。
        }

        return true;
    }

    bool replacestr(char* str, const std::string& str1, const std::string& str2, bool bloop)
    {
        if (str == nullptr) return false;

        std::string strtemp(str);

        replacestr(strtemp, str1, str2, bloop);

        strtemp.copy(str, strtemp.length());
        str[strtemp.length()] = 0; // std::string的copy函数不会给C风格字符串的结尾加0。

        return true;
    }

    char* picknumber(const std::string& src, char* dest, const bool bsigned, const bool bdot)
    {
        if (dest == nullptr) return nullptr; // 判断空指针。

        std::string strtemp = picknumber(src, bsigned, bdot);
        strtemp.copy(dest, strtemp.length());
        dest[strtemp.length()] = 0; // std::string的copy函数不会给C风格字符串的结尾加0。

        return dest;
    }

    std::string& picknumber(const std::string& src, std::string& dest, const bool bsigned, const bool bdot)
    {
        // 为了支持src和dest是同一变量的情况，定义str临时变量。
        std::string str;

        for (char c : src)
        {
            // 判断是否提取符号。
            if ((bsigned == true) && ((c == '+') || (c == '-')))
            {
                str.append(1, c);
                continue;
            }

            // 判断是否提取小数点。
            if ((bdot == true) && (c == '.'))
            {
                str.append(1, c);
                continue;
            }

            // 提取数字。
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
        // 如果匹配规则表达式的内容是空的，返回false。
        if (rules.length() == 0) return false;

        // 如果如果匹配规则表达式的内容是"*"，直接返回true。
        if (rules == "*") return true;

        size_t pos1, pos2;
        ccmdstr cmdstr, cmdsubstr;

        std::string filename = str;
        std::string matchstr = rules;

        // 把字符串都转换成大写后再来比较
        toupper(filename);
        toupper(matchstr);

        cmdstr.split(matchstr, ",");

        for (size_t i = 0, cmdstr_size = cmdstr.size(); i < cmdstr_size; ++i)
        {
            // 如果为空，就一定要跳过，否则就会被匹配上。
            if (cmdstr[i].empty() == true) continue;

            pos1 = pos2 = 0;
            cmdsubstr.split(cmdstr[i], "*");

            size_t j, cmdsubstr_size = cmdsubstr.size();
            for (j = 0; j < cmdsubstr_size; ++j)
            {
                // 如果是文件名的首部
                if (j == 0)
                    if (filename.substr(0, cmdsubstr[j].length()) != cmdsubstr[j]) break;

                // 如果是文件名的尾部
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
        split(buffer, sepstr, bdelspace);
    }

    // 把字符串拆分到m_cmdstr容器中。
    // buffer：待拆分的字符串。
    // sepstr：buffer字符串中字段内容的分隔符，注意，分隔符是字符串，如","、" "、"|"、"~!~"。
    // bdelspace：是否删除拆分后的字段内容前后的空格，true-删除；false-不删除，缺省不删除。
    void ccmdstr::split(const std::string& buffer, const std::string& sepstr, const bool bdelspace)
    {
        // 清除所有的旧数据
        m_cmdstr.clear();

        // 边界条件1：空输入直接返回
        if (buffer.empty()) return;

        // 边界条件2：空分隔符（整个字符串作为一个字段）
        if (sepstr.empty())
        {
            std::string temp = buffer;        // 复制原始字符串
            if (bdelspace) deleteLRchr(temp); // 如需去空格则处理
            m_cmdstr.push_back(std::move(temp));
            return;
        }

        const size_t sepLen = sepstr.length();
        size_t pos = 0;     // 每次从buffer中查找分隔符的起始位置。
        size_t next = 0;    // 从pos的位置开始，查找下一个分隔符的位置。
        std::string substr; // 存放每次拆分出来的子串。

        // 预分配内存：减少动态扩容（预估字段数，最多预分配32个避免过度消耗）
        m_cmdstr.reserve(std::min(static_cast<size_t>(32), static_cast<size_t>(buffer.size() / sepLen + 2)));

        while ((next = buffer.find(sepstr, pos)) != std::string::npos) // 从pos的位置开始，查找下一个分隔符的位置。
        {
            substr = buffer.substr(pos, next - pos); // 从buffer中截取子串。

            if (bdelspace == true) deleteLRchr(substr); // 删除子串前后的空格。

            m_cmdstr.push_back(std::move(substr)); // 把子串放入m_cmdstr容器中，调用std::string类的移动构造函数。

            pos = next + sepLen; // 下次从buffer中查找分隔符的起始位置后移。
        }

        // 处理最后一个字段（最后一个分隔符之后的内容）。
        substr = buffer.substr(pos);

        if (bdelspace == true) deleteLRchr(substr);

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

        if (len > 0) memset(value, 0, len + 1); // 调用者必须保证value的空间足够，否则这里会内存溢出。

        if ((m_cmdstr[i].length() <= len) || (len == 0))
        {
            m_cmdstr[i].copy(value, m_cmdstr[i].length());
            value[m_cmdstr[i].length()] = '\0'; // std::string的copy函数不会给C风格字符串的结尾加'\0'。
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
            value = stoi(picknumber(m_cmdstr[i], true)); // stoi有异常，需要处理异常。
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
            value = stoi(picknumber(m_cmdstr[i])); // stoi有异常，需要处理异常。不提取符号 + -
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
            value = stol(picknumber(m_cmdstr[i], true)); // stol有异常，需要处理异常。
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
            value = stoul(picknumber(m_cmdstr[i])); // stoul有异常，需要处理异常。不提取符号 + -
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
            value = stod(picknumber(m_cmdstr[i], true, true)); // stod有异常，需要处理异常。提取符号和小数点。
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
            value = stof(picknumber(m_cmdstr[i], true, true)); // stof有异常，需要处理异常。提取符号和小数点。
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
        toupper(str); // 转换为大写来判断。

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

    bool getByXml(const std::string& xmlbuffer, const std::string& fieldname, std::string& value, const size_t len)
    {
        std::string start = "<" + fieldname + ">"; // 数据项开始的标签。
        std::string end = "</" + fieldname + ">";  // 数据项结束的标签。

        size_t startp = xmlbuffer.find(start); // 在xml中查找数据项开始的标签的位置。
        if (startp == std::string::npos) return false;

        size_t endp = xmlbuffer.find(end); // 在xml中查找数据项结束的标签的位置。
        if (endp == std::string::npos) return false;

        // 从xml中截取数据项的内容。
        size_t itmplen = endp - startp - start.length();
        if ((len > 0) && (len < itmplen)) itmplen = len;
        value = xmlbuffer.substr(startp + start.length(), itmplen);

        return true;
    }

    bool getByXml(const std::string& xmlbuffer, const std::string& fieldname, char* value, const size_t len)
    {
        if (value == nullptr) return false;

        if (len > 0) memset(value, 0, len + 1); // 调用者必须保证value的空间足够，否则这里会内存溢出。

        std::string str;
        if (getByXml(xmlbuffer, fieldname, str) == false) return false;

        if ((str.length() <= len) || (len == 0))
        {
            str.copy(value, str.length());
            value[str.length()] = '\0'; // std::string的copy函数不会给C风格字符串的结尾加0。
        }
        else
        {
            str.copy(value, len);
            value[len] = '\0';
        }

        return true;
    }

    bool getByXml(const std::string& xmlbuffer, const std::string& fieldname, bool& value)
    {
        std::string str;
        if (getByXml(xmlbuffer, fieldname, str) == false) return false;

        toupper(str); // 转换为大写来判断（也可以转换为小写，效果相同）。

        if (str == "TRUE")
            value = true;
        else
            value = false;

        return true;
    }

    bool getByXml(const std::string& xmlbuffer, const std::string& fieldname, int& value)
    {
        std::string str;

        if (getByXml(xmlbuffer, fieldname, str) == false) return false;

        try
        {
            value = stoi(picknumber(str, true)); // stoi有异常，需要处理异常。
        }
        catch (const std::exception&)
        {
            return false;
        }

        return true;
    }

    bool getByXml(const std::string& xmlbuffer, const std::string& fieldname, unsigned int& value)
    {
        std::string str;

        if (getByXml(xmlbuffer, fieldname, str) == false) return false;

        try
        {
            value = stoi(picknumber(str)); // stoi有异常，需要处理异常。不提取符号 + -
        }
        catch (const std::exception&)
        {
            return false;
        }

        return true;
    }

    bool getByXml(const std::string& xmlbuffer, const std::string& fieldname, long& value)
    {
        std::string str;

        if (getByXml(xmlbuffer, fieldname, str) == false) return false;

        try
        {
            value = stol(picknumber(str, true)); // stol有异常，需要处理异常。
        }
        catch (const std::exception&)
        {
            return false;
        }

        return true;
    }

    bool getByXml(const std::string& xmlbuffer, const std::string& fieldname, unsigned long& value)
    {
        std::string str;

        if (getByXml(xmlbuffer, fieldname, str) == false) return false;

        try
        {
            value = stoul(picknumber(str)); // stoul有异常，需要处理异常。不提取符号 + -
        }
        catch (const std::exception&)
        {
            return false;
        }

        return true;
    }

    bool getByXml(const std::string& xmlbuffer, const std::string& fieldname, double& value)
    {
        std::string str;

        if (getByXml(xmlbuffer, fieldname, str) == false) return false;

        try
        {
            value = stod(picknumber(str, true, true)); // stod有异常，需要处理异常。提取符号和小数点。
        }
        catch (const std::exception&)
        {
            return false;
        }

        return true;
    }

    bool getByXml(const std::string& xmlbuffer, const std::string& fieldname, float& value)
    {
        std::string str;

        if (getByXml(xmlbuffer, fieldname, str) == false) return false;

        try
        {
            value = stof(picknumber(str, true, true)); // stof有异常，需要处理异常。提取符号和小数点。
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

        // 动态next数组，初始化为未计算状态
        std::vector<size_t> next(m, SIZE_MAX);
        next[0] = 0; // 首位固定为0

        size_t len = 0; // 当前最长匹配前后缀长度
        size_t i = 0, j = 0;

        while (i < n)
        {
            if (str[i] == pattern[j])
            {
                // 动态计算并缓存next值
                if (j > 0 && next[j] == SIZE_MAX)
                {
                    next[j] = len; // 缓存当前长度

                    // 扩展计算后续可能需要的next值
                    size_t k = j, l = len;
                    while (++k < m && pattern[k] == pattern[l])
                    {
                        next[k] = ++l;
                    }
                }

                ++i;
                if (++j == m) return i - m; // 匹配成功
            }
            else
            {
                if (j > 0)
                {
                    // 使用已计算的next值回溯
                    len = next[j - 1];
                    j = (len != SIZE_MAX) ? len : 0;
                }
                else
                {
                    ++i;
                }
                len = j; // 重置当前匹配长度
            }
        }
        return std::string::npos;
    }

} // namespace ol