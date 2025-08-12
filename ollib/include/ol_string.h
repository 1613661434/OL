#ifndef __OL_STRING_H
#define __OL_STRING_H 1

#include <cstdio>
#include <string>
#include <type_traits>
#include <vector>

namespace ol
{

    ///////////////////////////////////// /////////////////////////////////////
    // C++风格字符串操作的若干函数。
    // 删除字符串左边指定的字符。
    // str：待处理的字符串。
    // c：需要删除的字符，缺省删除空格。
    char* deletelchr(char* str, const int c = ' ');
    std::string& deletelchr(std::string& str, const int c = ' ');

    // 删除字符串右边指定的字符。
    // str：待处理的字符串。
    // c：需要删除的字符，缺省删除空格。
    char* deleterchr(char* str, const int c = ' ');
    std::string& deleterchr(std::string& str, const int c = ' ');

    // 删除字符串左右两边指定的字符。
    // str：待处理的字符串。
    // chr：需要删除的字符，缺省删除空格。
    char* deletelrchr(char* str, const int c = ' ');
    std::string& deletelrchr(std::string& str, const int c = ' ');

    // 把字符串中的小写字母转换成大写，忽略不是字母的字符。
    // str：待转换的字符串。
    char* toupper(char* str);
    std::string& toupper(std::string& str);

    // 把字符串中的大写字母转换成小写，忽略不是字母的字符。
    // str：待转换的字符串。
    char* tolower(char* str);
    std::string& tolower(std::string& str);

    // 字符串替换函数。
    // 在字符串str中，如果存在字符串str1，就替换为字符串str2。
    // str：待处理的字符串。
    // str1：旧的内容。
    // str2：新的内容。
    // bloop：是否循环执行替换。
    // 注意：
    // 1、如果str2比str1要长，替换后str会变长，所以必须保证str有足够的空间，否则内存会溢出（C++风格字符串不存在这个问题）。
    // 2、如果str2中包含了str1的内容，且bloop为true，这种做法存在逻辑错误，replacestr将什么也不做。
    // 3、如果str2为空，表示删除str中str1的内容。
    bool replacestr(char* str, const std::string& str1, const std::string& str2, const bool bloop = false);
    bool replacestr(std::string& str, const std::string& str1, const std::string& str2, const bool bloop = false);

    // 从一个字符串中提取出数字、符号和小数点，存放到另一个字符串中。
    // src：原字符串。
    // dest：目标字符串。
    // bsigned：是否提取符号（+和-），true-包括；false-不包括。
    // bdot：是否提取小数点（.），true-包括；false-不包括。
    // 注意：src和dest可以是同一个变量。
    char* picknumber(const std::string& src, char* dest, const bool bsigned = false, const bool bdot = false);
    std::string& picknumber(const std::string& src, std::string& dest, const bool bsigned = false, const bool bdot = false);
    std::string picknumber(const std::string& src, const bool bsigned = false, const bool bdot = false);

    // 正则表达式，判断一个字符串是否匹配另一个字符串。
    // str：需要判断的字符串，是精确表示的，如文件名"_public.cpp"。
    // rules：匹配规则的表达式，用星号"*"代表任意字符，多个表达式之间用半角的逗号分隔，如"*.h,*.cpp"。
    // 注意：1）str参数不需要支持"*"，rules参数支持"*"；2）函数在判断str是否匹配rules的时候，会忽略字母的大小写。
    bool matchstr(const std::string& str, const std::string& rules);
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // ccmdstr类用于拆分有分隔符的字符串。
    // 字符串的格式为：字段内容1+分隔符+字段内容2+分隔符+字段内容3+分隔符+...+字段内容n。
    // 例如："messi,10,striker,30,1.72,68.5,Barcelona"，这是足球运动员梅西的资料。
    // 包括：姓名、球衣号码、场上位置、年龄、身高、体重和效力的俱乐部，字段之间用半角的逗号分隔。
    class ccmdstr
    {
    private:
        std::vector<std::string> m_cmdstr; // 存放拆分后的字段内容。

        ccmdstr(const ccmdstr&) = delete;            // 禁用拷贝构造函数。
        ccmdstr& operator=(const ccmdstr&) = delete; // 禁用赋值函数。
    public:
        ccmdstr()
        {
        } // 构造函数。
        ccmdstr(const std::string& buffer, const std::string& sepstr, const bool bdelspace = false);

        const std::string& operator[](int i) const // 重载[]运算符，可以像访问数组一样访问m_cmdstr成员。
        {
            return m_cmdstr[i];
        }

        // 把字符串拆分到m_cmdstr容器中，注意：空字符串也会存入，如：",asd"，分隔符为","，结果：[0]=""，[1]="asd"。
        // buffer：待拆分的字符串。
        // sepstr：buffer中采用的分隔符，注意，sepstr参数的数据类型不是字符，是字符串，如","、" "、"|"、"~!~"。
        // bdelspace：拆分后是否删除字段内容前后的空格，true-删除；false-不删除，缺省不删除。
        void splittocmd(const std::string& buffer, const std::string& sepstr, const bool bdelspace = false);

        // 获取拆分后字段的个数，即m_cmdstr容器的大小。
        size_t size() const
        {
            return m_cmdstr.size();
        }

        // 从m_cmdstr容器获取字段内容。
        // i：字段的顺序号，类似数组的下标，从0开始。
        // value：传入变量的地址，用于存放字段内容。
        // 返回值：true-成功；如果i的取值超出了m_cmdstr容器的大小，返回失败。
        bool getvalue(const size_t i, std::string& value, const size_t len = 0) const; // C++风格字符串。
        bool getvalue(const size_t i, char* value, const size_t len = 0) const;        // C风格字符串，len缺省值为0-全部长度，会在索引len处设置'\0'
        bool getvalue(const size_t i, int& value) const;                               // int整数。
        bool getvalue(const size_t i, unsigned int& value) const;                      // unsigned int整数。
        bool getvalue(const size_t i, long& value) const;                              // long整数。
        bool getvalue(const size_t i, unsigned long& value) const;                     // unsigned long整数。
        bool getvalue(const size_t i, double& value) const;                            // double双精度。
        bool getvalue(const size_t i, float& value) const;                             // float单精度。
        bool getvalue(const size_t i, bool& value) const;                              // bool型。

        ~ccmdstr(); // 析构函数。
    };

    // 重载<<运算符，输出ccmdstr::m_cmdstr中的内容，方便调试。
    std::ostream& operator<<(std::ostream& out, const ccmdstr& cmdstr);
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // 解析xml格式字符串的函数族。
    // xml格式的字符串的内容如下：
    // <filename>/tmp/_public.h</filename><mtime>2020-01-01 12:20:35</mtime><size>18348</size>
    // <filename>/tmp/_public.cpp</filename><mtime>2020-01-01 10:10:15</mtime><size>50945</size>
    // xmlbuffer：待解析的xml格式字符串。
    // fieldname：字段的标签名。
    // value：传入变量的地址，用于存放字段内容，支持bool、int、insigned int、long、
    //       unsigned long、double和char[]。
    // 注意：当value参数的数据类型为char []时，必须保证value数组的内存足够，否则可能发生内存溢出的问题，
    //           也可以用len参数限定获取字段内容的长度，len的缺省值为0，表示不限长度。
    // 返回值：true-成功；如果fieldname参数指定的标签名不存在，返回失败。
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
    // C++格式化输出函数模板。
    // 辅助函数：自动转换 std::string 到 const char*
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

    // 版本1：将结果写入已有 string
    template <typename... Args>
    bool sformat(std::string& str, const char* fmt, Args&&... args)
    {
        // 计算长度
        int len = std::snprintf(nullptr, 0, fmt, detail::format_arg(std::forward<Args>(args))...);
        if (len < 0) return false;

        if (len == 0)
        {
            str.clear();
            return true;
        }

        // 执行格式化
        str.resize(len);
        std::snprintf(&str[0], len + 1, fmt, detail::format_arg(std::forward<Args>(args))...);
        return true;
    }

    // 版本2：返回新字符串
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
    // KMP算法查找子串
    size_t skmp(const std::string& str, const std::string& substr);
    ///////////////////////////////////// /////////////////////////////////////
} // namespace ol

#endif // !__OL_STRING_H