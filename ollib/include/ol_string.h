#ifndef __OL_STRING_H
#define __OL_STRING_H 1

#include <cstdio>
#include <string>
#include <type_traits>
#include <vector>

namespace ol
{

    ///////////////////////////////////// /////////////////////////////////////
    // C++����ַ������������ɺ�����
    // ɾ���ַ������ָ�����ַ���
    // str����������ַ�����
    // c����Ҫɾ�����ַ���ȱʡɾ���ո�
    char* deletelchr(char* str, const int c = ' ');
    std::string& deletelchr(std::string& str, const int c = ' ');

    // ɾ���ַ����ұ�ָ�����ַ���
    // str����������ַ�����
    // c����Ҫɾ�����ַ���ȱʡɾ���ո�
    char* deleterchr(char* str, const int c = ' ');
    std::string& deleterchr(std::string& str, const int c = ' ');

    // ɾ���ַ�����������ָ�����ַ���
    // str����������ַ�����
    // chr����Ҫɾ�����ַ���ȱʡɾ���ո�
    char* deletelrchr(char* str, const int c = ' ');
    std::string& deletelrchr(std::string& str, const int c = ' ');

    // ���ַ����е�Сд��ĸת���ɴ�д�����Բ�����ĸ���ַ���
    // str����ת�����ַ�����
    char* toupper(char* str);
    std::string& toupper(std::string& str);

    // ���ַ����еĴ�д��ĸת����Сд�����Բ�����ĸ���ַ���
    // str����ת�����ַ�����
    char* tolower(char* str);
    std::string& tolower(std::string& str);

    // �ַ����滻������
    // ���ַ���str�У���������ַ���str1�����滻Ϊ�ַ���str2��
    // str����������ַ�����
    // str1���ɵ����ݡ�
    // str2���µ����ݡ�
    // bloop���Ƿ�ѭ��ִ���滻��
    // ע�⣺
    // 1�����str2��str1Ҫ�����滻��str��䳤�����Ա��뱣֤str���㹻�Ŀռ䣬�����ڴ�������C++����ַ���������������⣩��
    // 2�����str2�а�����str1�����ݣ���bloopΪtrue���������������߼�����replacestr��ʲôҲ������
    // 3�����str2Ϊ�գ���ʾɾ��str��str1�����ݡ�
    bool replacestr(char* str, const std::string& str1, const std::string& str2, const bool bloop = false);
    bool replacestr(std::string& str, const std::string& str1, const std::string& str2, const bool bloop = false);

    // ��һ���ַ�������ȡ�����֡����ź�С���㣬��ŵ���һ���ַ����С�
    // src��ԭ�ַ�����
    // dest��Ŀ���ַ�����
    // bsigned���Ƿ���ȡ���ţ�+��-����true-������false-��������
    // bdot���Ƿ���ȡС���㣨.����true-������false-��������
    // ע�⣺src��dest������ͬһ��������
    char* picknumber(const std::string& src, char* dest, const bool bsigned = false, const bool bdot = false);
    std::string& picknumber(const std::string& src, std::string& dest, const bool bsigned = false, const bool bdot = false);
    std::string picknumber(const std::string& src, const bool bsigned = false, const bool bdot = false);

    // ������ʽ���ж�һ���ַ����Ƿ�ƥ����һ���ַ�����
    // str����Ҫ�жϵ��ַ������Ǿ�ȷ��ʾ�ģ����ļ���"_public.cpp"��
    // rules��ƥ�����ı��ʽ�����Ǻ�"*"���������ַ���������ʽ֮���ð�ǵĶ��ŷָ�����"*.h,*.cpp"��
    // ע�⣺1��str��������Ҫ֧��"*"��rules����֧��"*"��2���������ж�str�Ƿ�ƥ��rules��ʱ�򣬻������ĸ�Ĵ�Сд��
    bool matchstr(const std::string& str, const std::string& rules);
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // ccmdstr�����ڲ���зָ������ַ�����
    // �ַ����ĸ�ʽΪ���ֶ�����1+�ָ���+�ֶ�����2+�ָ���+�ֶ�����3+�ָ���+...+�ֶ�����n��
    // ���磺"messi,10,striker,30,1.72,68.5,Barcelona"�����������˶�Ա÷�������ϡ�
    // ���������������º��롢����λ�á����䡢��ߡ����غ�Ч���ľ��ֲ����ֶ�֮���ð�ǵĶ��ŷָ���
    class ccmdstr
    {
    private:
        std::vector<std::string> m_cmdstr; // ��Ų�ֺ���ֶ����ݡ�

        ccmdstr(const ccmdstr&) = delete;            // ���ÿ������캯����
        ccmdstr& operator=(const ccmdstr&) = delete; // ���ø�ֵ������
    public:
        ccmdstr()
        {
        } // ���캯����
        ccmdstr(const std::string& buffer, const std::string& sepstr, const bool bdelspace = false);

        const std::string& operator[](int i) const // ����[]��������������������һ������m_cmdstr��Ա��
        {
            return m_cmdstr[i];
        }

        // ���ַ�����ֵ�m_cmdstr�����У�ע�⣺���ַ���Ҳ����룬�磺",asd"���ָ���Ϊ","�������[0]=""��[1]="asd"��
        // buffer������ֵ��ַ�����
        // sepstr��buffer�в��õķָ�����ע�⣬sepstr�������������Ͳ����ַ������ַ�������","��" "��"|"��"~!~"��
        // bdelspace����ֺ��Ƿ�ɾ���ֶ�����ǰ��Ŀո�true-ɾ����false-��ɾ����ȱʡ��ɾ����
        void splittocmd(const std::string& buffer, const std::string& sepstr, const bool bdelspace = false);

        // ��ȡ��ֺ��ֶεĸ�������m_cmdstr�����Ĵ�С��
        size_t size() const
        {
            return m_cmdstr.size();
        }

        // ��m_cmdstr������ȡ�ֶ����ݡ�
        // i���ֶε�˳��ţ�����������±꣬��0��ʼ��
        // value����������ĵ�ַ�����ڴ���ֶ����ݡ�
        // ����ֵ��true-�ɹ������i��ȡֵ������m_cmdstr�����Ĵ�С������ʧ�ܡ�
        bool getvalue(const size_t i, std::string& value, const size_t len = 0) const; // C++����ַ�����
        bool getvalue(const size_t i, char* value, const size_t len = 0) const;        // C����ַ�����lenȱʡֵΪ0-ȫ�����ȣ���������len������'\0'
        bool getvalue(const size_t i, int& value) const;                               // int������
        bool getvalue(const size_t i, unsigned int& value) const;                      // unsigned int������
        bool getvalue(const size_t i, long& value) const;                              // long������
        bool getvalue(const size_t i, unsigned long& value) const;                     // unsigned long������
        bool getvalue(const size_t i, double& value) const;                            // double˫���ȡ�
        bool getvalue(const size_t i, float& value) const;                             // float�����ȡ�
        bool getvalue(const size_t i, bool& value) const;                              // bool�͡�

        ~ccmdstr(); // ����������
    };

    // ����<<����������ccmdstr::m_cmdstr�е����ݣ�������ԡ�
    std::ostream& operator<<(std::ostream& out, const ccmdstr& cmdstr);
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // ����xml��ʽ�ַ����ĺ����塣
    // xml��ʽ���ַ������������£�
    // <filename>/tmp/_public.h</filename><mtime>2020-01-01 12:20:35</mtime><size>18348</size>
    // <filename>/tmp/_public.cpp</filename><mtime>2020-01-01 10:10:15</mtime><size>50945</size>
    // xmlbuffer����������xml��ʽ�ַ�����
    // fieldname���ֶεı�ǩ����
    // value����������ĵ�ַ�����ڴ���ֶ����ݣ�֧��bool��int��insigned int��long��
    //       unsigned long��double��char[]��
    // ע�⣺��value��������������Ϊchar []ʱ�����뱣֤value������ڴ��㹻��������ܷ����ڴ���������⣬
    //           Ҳ������len�����޶���ȡ�ֶ����ݵĳ��ȣ�len��ȱʡֵΪ0����ʾ���޳��ȡ�
    // ����ֵ��true-�ɹ������fieldname����ָ���ı�ǩ�������ڣ�����ʧ�ܡ�
    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, std::string& value, const size_t len = 0);
    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, char* value, const size_t len = 0);
    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, bool& value);
    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, int& value);
    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, unsigned int& value);
    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, long& value);
    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, unsigned long& value);
    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, double& value);
    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, float& value);
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // C++��ʽ���������ģ�塣
    // �����������Զ�ת�� std::string �� const char*
    namespace detail
    {
        template <typename T>
        auto format_arg(T&& arg) -> decltype(auto)
        {
            if constexpr (std::is_same_v<std::decay_t<T>, std::string>)
            {
                return arg.c_str();
            }
            else
            {
                return std::forward<T>(arg);
            }
        }
    } // namespace detail

    // �汾1�������д������ string
    template <typename... Args>
    bool sformat(std::string& str, const char* fmt, Args&&... args)
    {
        // ���㳤��
        int len = std::snprintf(nullptr, 0, fmt, detail::format_arg(std::forward<Args>(args))...);
        if (len < 0) return false;

        if (len == 0)
        {
            str.clear();
            return true;
        }

        // ִ�и�ʽ��
        str.resize(len);
        std::snprintf(&str[0], len + 1, fmt, detail::format_arg(std::forward<Args>(args))...);
        return true;
    }

    // �汾2���������ַ���
    template <typename... Args>
    std::string sformat(const char* fmt, Args&&... args)
    {
        std::string str;
        int len = std::snprintf(nullptr, 0, fmt, detail::format_arg(std::forward<Args>(args))...);
        if (len <= 0) return str;

        str.resize(len);
        std::snprintf(&str[0], len + 1, fmt, detail::format_arg(std::forward<Args>(args))...);
        return str;
    }
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // KMP�㷨�����Ӵ�
    size_t skmp(const std::string& str, const std::string& substr);
    ///////////////////////////////////// /////////////////////////////////////
} // namespace ol

#endif // !__OL_STRING_H