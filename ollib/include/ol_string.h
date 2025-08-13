/****************************************************************************************/
/*
 * ��������ol_string.h
 * �����������ַ����������༰�������ϣ��ṩ�ḻ���ַ����������ܣ����԰�����
 *          - �ַ���������ɾ����βָ���ַ�����Сдת���ȣ�
 *          - �ַ����滻����ȡ���֡�����ƥ��Ȼ�������
 *          - �������ַ�������ࣨccmdstr����֧�ֶ�ָ���������ת��
 *          - XML��ʽ�ַ�������������֧�ֶ�������������ȡ
 *          - ��ʽ�����������sformat��������C����ʽ����֧��std::string
 *          - KMP�㷨ʵ�ֵĸ�Ч�Ӵ�����
 * ���ߣ�ol
 * ���ñ�׼��C++11�����ϣ���֧�ֱ��ģ�塢������ȡ�����ԣ�
 */
/****************************************************************************************/

#ifndef __OL_STRING_H
#define __OL_STRING_H 1

#include <cstdio>
#include <string>
#include <type_traits>
#include <vector>

namespace ol
{

    // ===========================================================================
    /**
     * ɾ���ַ������ָ���ַ���C�ַ����汾��
     * @param str �������C�ַ������ᱻֱ���޸ģ�
     * @param c Ҫɾ�����ַ���Ĭ�Ͽո�' '��
     * @return �޸ĺ���ַ���ָ��
     */
    char* deletelchr(char* str, const char c = ' ');

    /**
     * ɾ���ַ������ָ���ַ���std::string�汾��
     * @param str ��������ַ�������
     * @param c Ҫɾ�����ַ���Ĭ�Ͽո�' '��
     * @return �޸ĺ���ַ�������
     */
    std::string& deletelchr(std::string& str, const char c = ' ');

    /**
     * ɾ���ַ����ұ�ָ���ַ���C�ַ����汾��
     * @param str �������C�ַ������ᱻֱ���޸ģ�
     * @param c Ҫɾ�����ַ���Ĭ�Ͽո�' '��
     * @return �޸ĺ���ַ���ָ��
     */
    char* deleterchr(char* str, const char c = ' ');

    /**
     * ɾ���ַ����ұ�ָ���ַ���std::string�汾��
     * @param str ��������ַ�������
     * @param c Ҫɾ�����ַ���Ĭ�Ͽո�' '��
     * @return �޸ĺ���ַ�������
     */
    std::string& deleterchr(std::string& str, const char c = ' ');

    /**
     * ɾ���ַ�����������ָ���ַ���C�ַ����汾��
     * @param str �������C�ַ������ᱻֱ���޸ģ�
     * @param c Ҫɾ�����ַ���Ĭ�Ͽո�' '��
     * @return �޸ĺ���ַ���ָ��
     */
    char* deletelrchr(char* str, const char c = ' ');

    /**
     * ɾ���ַ�����������ָ���ַ���std::string�汾��
     * @param str ��������ַ�������
     * @param c Ҫɾ�����ַ���Ĭ�Ͽո�' '��
     * @return �޸ĺ���ַ�������
     */
    std::string& deletelrchr(std::string& str, const char c = ' ');

    /**
     * ���ַ����е�Сд��ĸת��Ϊ��д��C�ַ����汾��
     * @param str ��ת����C�ַ������ᱻֱ���޸ģ�
     * @return �޸ĺ���ַ���ָ�루����ĸ�ַ����䣩
     */
    char* toupper(char* str);

    /**
     * ���ַ����е�Сд��ĸת��Ϊ��д��std::string�汾��
     * @param str ��ת�����ַ�������
     * @return �޸ĺ���ַ������ã�����ĸ�ַ����䣩
     */
    std::string& toupper(std::string& str);

    /**
     * ���ַ����еĴ�д��ĸת��ΪСд��C�ַ����汾��
     * @param str ��ת����C�ַ������ᱻֱ���޸ģ�
     * @return �޸ĺ���ַ���ָ�루����ĸ�ַ����䣩
     */
    char* tolower(char* str);

    /**
     * ���ַ����еĴ�д��ĸת��ΪСд��std::string�汾��
     * @param str ��ת�����ַ�������
     * @return �޸ĺ���ַ������ã�����ĸ�ַ����䣩
     */
    std::string& tolower(std::string& str);

    /**
     * �ַ����滻��C�ַ����汾��
     * @param str �������C�ַ������ᱻֱ���޸ģ�
     * @param str1 Ҫ�滻�ľ��Ӵ�
     * @param str2 �滻�����Ӵ�
     * @param bloop �Ƿ�ѭ���滻��Ĭ��false��
     * @return true-�滻�ɹ���false-�滻ʧ�ܣ�������߼�����
     * @note 1�����str2��str1Ҫ�����滻��str��䳤�����Ա��뱣֤str���㹻�Ŀռ䣬�����ڴ�������C++����ַ���������������⣩��
     *       2�����str2�а�����str1�����ݣ���bloopΪtrue���������������߼�����replacestr��ʲôҲ������
     *       3�����str2Ϊ�գ���ʾɾ��str��str1�����ݡ�
     */
    bool replacestr(char* str, const std::string& str1, const std::string& str2, const bool bloop = false);

    /**
     * �ַ����滻��std::string�汾��
     * @param str ��������ַ�������
     * @param str1 Ҫ�滻�ľ��Ӵ�
     * @param str2 �滻�����Ӵ�
     * @param bloop �Ƿ�ѭ���滻��Ĭ��false��
     * @return true-�滻�ɹ���false-�滻ʧ�ܣ�������߼�����
     * @note 1�����str2��str1Ҫ�����滻��str��䳤�����Ա��뱣֤str���㹻�Ŀռ䣬�����ڴ�������C++����ַ���������������⣩��
     *       2�����str2�а�����str1�����ݣ���bloopΪtrue���������������߼�����replacestr��ʲôҲ������
     *       3�����str2Ϊ�գ���ʾɾ��str��str1�����ݡ�
     */
    bool replacestr(std::string& str, const std::string& str1, const std::string& str2, const bool bloop = false);

    /**
     * ���ַ�������ȡ��������ַ���C�ַ�������汾��
     * @param src Դ�ַ���
     * @param dest Ŀ��C�ַ������洢��ȡ�����
     * @param bsigned �Ƿ���ȡ���ţ�+/-��Ĭ��false��
     * @param bdot �Ƿ���ȡС���㣨.��Ĭ��false��
     * @return Ŀ���ַ���ָ�루src��dest����ͬ��
     */
    char* picknumber(const std::string& src, char* dest, const bool bsigned = false, const bool bdot = false);

    /**
     * ���ַ�������ȡ��������ַ���std::string����汾��
     * @param src Դ�ַ���
     * @param dest Ŀ���ַ������ã��洢��ȡ�����
     * @param bsigned �Ƿ���ȡ���ţ�+/-��Ĭ��false��
     * @param bdot �Ƿ���ȡС���㣨.��Ĭ��false��
     * @return Ŀ���ַ�������
     */
    std::string& picknumber(const std::string& src, std::string& dest, const bool bsigned = false, const bool bdot = false);

    /**
     * ���ַ�������ȡ��������ַ����������ַ����汾��
     * @param src Դ�ַ���
     * @param bsigned �Ƿ���ȡ���ţ�+/-��Ĭ��false��
     * @param bdot �Ƿ���ȡС���㣨.��Ĭ��false��
     * @return ��ȡ��������ַ���
     */
    std::string picknumber(const std::string& src, const bool bsigned = false, const bool bdot = false);

    /**
     * ����ƥ���ַ�����֧��ͨ���*��ƥ���������ַ���
     * @param str ��ƥ����ַ�������ȷ���ݣ�
     * @param rules ƥ�������*��ʾ��������ַ���������ð�ǵĶ��ŷָ�����"*.h,*.cpp"��
     * @return true-ƥ��ɹ���false-ƥ��ʧ��
     * @note 1��str��������Ҫ֧��"*"��rules����֧��"*"��
     *       2���������ж�str�Ƿ�ƥ��rules��ʱ�򣬻������ĸ�Ĵ�Сд��
     */
    bool matchstr(const std::string& str, const std::string& rules);
    // ===========================================================================

    // ===========================================================================
    // ccmdstr�࣬�������ַ�������࣬���ڽ������ָ����Ľṹ���ַ�����
    // �ַ����ĸ�ʽΪ���ֶ�����1+�ָ���+�ֶ�����2+�ָ���+�ֶ�����3+�ָ���+...+�ֶ�����n��
    // ���磺"messi,10,striker,30,1.72,68.5,Barcelona"�����������˶�Ա÷�������ϡ�
    // ���������������º��롢����λ�á����䡢��ߡ����غ�Ч���ľ��ֲ����ֶ�֮���ð�ǵĶ��ŷָ���
    class ccmdstr
    {
    private:
        std::vector<std::string> m_cmdstr; // ��ֺ���ֶ�����

        ccmdstr(const ccmdstr&) = delete;            // ���ÿ������캯��
        ccmdstr& operator=(const ccmdstr&) = delete; // ���ø�ֵ����
    public:
        // Ĭ�Ϲ��캯��
        ccmdstr()
        {
        }

        /**
         * ���ι��캯����ֱ�Ӳ���ַ���
         * @param buffer ����ֵ��ַ���
         * @param sepstr �ָ�����֧�ֶ��ַ�����",,"��" | "��
         * @param bdelspace �Ƿ�ɾ���ֶ�ǰ��ո�Ĭ��false��
         */
        ccmdstr(const std::string& buffer, const std::string& sepstr, const bool bdelspace = false);

        /**
         * ����[]����������ʲ�ֺ���ֶ�(m_cmdstr��Ա)
         * @param i �ֶ���������0��ʼ��
         * @return �ֶ����ݵĳ�������
         * @note ����Խ����׳��쳣
         */
        const std::string& operator[](int i) const
        {
            return m_cmdstr[i];
        }

        // ���ַ�����ֵ�m_cmdstr�����У�ע�⣺���ַ���Ҳ����룬�磺",asd"���ָ���Ϊ","�������[0]=""��[1]="asd"��
        // buffer������ֵ��ַ�����
        // sepstr��buffer�в��õķָ�����ע�⣬sepstr�������������Ͳ����ַ������ַ�������","��" "��"|"��"~!~"��
        // bdelspace����ֺ��Ƿ�ɾ���ֶ�����ǰ��Ŀո�true-ɾ����false-��ɾ����ȱʡ��ɾ����

        /**
         * ����ַ������洢���ڲ�����
         * @param buffer ����ֵ��ַ���
         * @param sepstr �ָ�����֧�ֶ��ַ���ע�⣬sepstr�������������Ͳ����ַ������ַ�������","��" "��"|"��"~!~"��
         * @param bdelspace �Ƿ�ɾ���ֶ�ǰ��ո�Ĭ��false��
         * @note ���ֶλᱻ��������",test"���Ϊ["", "test"]��
         */
        void splittocmd(const std::string& buffer, const std::string& sepstr, const bool bdelspace = false);

        /**
         * ��ȡ��ֺ���ֶ�����
         * @return �ֶ�������m_cmdstr�Ĵ�С��
         */
        size_t size() const
        {
            return m_cmdstr.size();
        }

        /**
         * ���ֶ�������m_cmdstr���л�ȡָ���������ֶ����ݲ�ת��ΪĿ������
         * @param i �ֶ���������0��ʼ��
         * @param value �洢����ı�������
         * @param len ���ַ���������Ч��ָ����ȡ���ȣ�Ĭ��0��ʾ����ȡ��
         * @return true-�ɹ���������Ч��ת���ɹ�����false-ʧ�ܣ�����Խ���ת��ʧ�ܣ�
         */
        bool getvalue(const size_t i, std::string& value, const size_t len = 0) const; // std::string�汾
        bool getvalue(const size_t i, char* value, const size_t len = 0) const;        // C�ַ����汾���Զ����'\0'��
        bool getvalue(const size_t i, int& value) const;                               // ת��Ϊint
        bool getvalue(const size_t i, unsigned int& value) const;                      // ת��Ϊunsigned int
        bool getvalue(const size_t i, long& value) const;                              // ת��Ϊlong
        bool getvalue(const size_t i, unsigned long& value) const;                     // ת��Ϊunsigned long
        bool getvalue(const size_t i, double& value) const;                            // ת��Ϊdouble
        bool getvalue(const size_t i, float& value) const;                             // ת��Ϊfloat
        bool getvalue(const size_t i, bool& value) const;                              // ת��Ϊbool��"true"/"1"Ϊtrue��

        // ��������
        ~ccmdstr();
    };

    /**
     * ����<<����������ccmdstr���ֶ����ݣ������ã�
     * @param out �����
     * @param cmdstr ccmdstr����
     * @return ���������
     */
    std::ostream& operator<<(std::ostream& out, const ccmdstr& cmdstr);
    // ===========================================================================

    // ===========================================================================
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

    /**
     * ����XML��ʽ�ַ�������ȡָ����ǩ�����ݲ�ת��ΪĿ������
     * @param xmlbuffer ��������XML��ʽ�ַ�������"<tag>value</tag>..."��
     * @param fieldname Ҫ��ȡ���ֶα�ǩ������"filename"��Ӧ<filename>��ǩ��
     * @param value �洢����ı�������/ָ��
     * @param len ���ַ���������Ч��ָ��������󳤶ȣ�Ĭ��0��ʾ���޳��ȣ�
     * @return true-�ɹ�����ǩ������ת���ɹ�����false-ʧ�ܣ���ǩ�����ڻ�ת��ʧ�ܣ�
     * @note ��valueΪchar[]ʱ���豣֤�����ڴ���㣬�������
     * @example <filename>/tmp/_public.h</filename><mtime>2020-01-01 12:20:35</mtime><size>18348</size>
     */
    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, std::string& value, const size_t len = 0); // ��ȡΪstd::string
    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, char* value, const size_t len = 0);        // ��ȡΪC�ַ������Զ����'\0'��
    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, bool& value);                              // ת��Ϊbool
    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, int& value);                               // ת��Ϊint
    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, unsigned int& value);                      // ת��Ϊunsigned int
    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, long& value);                              // ת��Ϊlong
    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, unsigned long& value);                     // ת��Ϊunsigned long
    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, double& value);                            // ת��Ϊdouble
    bool getxmlbuffer(const std::string& xmlbuffer, const std::string& fieldname, float& value);                             // ת��Ϊfloat
    // ===========================================================================

    // ===========================================================================
    // C++��ʽ���������ģ�塣
    // �����������Զ�ת�� std::string �� const char*
    namespace detail
    {
        /**
         * ��ʽ�����������������Զ���std::stringת��Ϊconst char*
         * @tparam T ��������
         * @param arg ��ת���Ĳ���
         * @return ת����Ĳ�����const char*��ԭ���ͣ�
         */
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

    /**
     * ��ʽ�����������д�������ַ�����
     * @tparam Types �ɱ���������б�
     * @param str �洢������ַ�������
     * @param fmt ��ʽ�ַ�����C���
     * @param args ����ʽ���Ĳ���
     * @return true-��ʽ���ɹ���false-ʧ��
     */
    template <typename... Types>
    bool sformat(std::string& str, const char* fmt, Types&&... args)
    {
        // ���㳤��
        int len = std::snprintf(nullptr, 0, fmt, detail::format_arg(std::forward<Types>(args))...);
        if (len < 0) return false;

        if (len == 0)
        {
            str.clear();
            return true;
        }

        // ִ�и�ʽ��
        str.resize(len);
        std::snprintf(&str[0], len + 1, fmt, detail::format_arg(std::forward<Types>(args))...);
        return true;
    }

    /**
     * ��ʽ������������������ַ�����
     * @tparam Types �ɱ���������б�
     * @param fmt ��ʽ�ַ�����C���
     * @param args ����ʽ���Ĳ���
     * @return ��ʽ��������ַ���
     */
    template <typename... Types>
    std::string sformat(const char* fmt, Types&&... args)
    {
        std::string str;
        int len = std::snprintf(nullptr, 0, fmt, detail::format_arg(std::forward<Types>(args))...);
        if (len <= 0) return str;

        str.resize(len);
        std::snprintf(&str[0], len + 1, fmt, detail::format_arg(std::forward<Types>(args))...);
        return str;
    }
    // ===========================================================================

    // ===========================================================================
    /**
     * KMP�㷨�����Ӵ�
     * @param str ���ַ���
     * @param pattern �����ҵ��Ӵ���ģʽ����
     * @return �Ӵ����������״γ��ֵ�λ�ã���0��ʼ����δ�ҵ�����std::string::npos
     */
    size_t skmp(const std::string& str, const std::string& pattern);
    // ===========================================================================
} // namespace ol

#endif // !__OL_STRING_H