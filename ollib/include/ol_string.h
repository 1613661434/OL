/****************************************************************************************/
/*
 * 程序名：ol_string.h
 * 功能描述：字符串处理工具类及函数集合，提供丰富的字符串操作功能，特性包括：
 *          - 字符串修整（删除首尾指定字符、大小写转换等）
 *          - 字符串替换、提取数字、正则匹配等基础操作
 *          - 命令行字符串拆分类（ccmdstr），支持多分隔符和类型转换
 *          - XML格式字符串解析函数，支持多种数据类型提取
 *          - 格式化输出函数（sformat），兼容C风格格式符并支持std::string
 *          - KMP算法实现的高效子串查找
 * 作者：ol
 * 适用标准：C++11及以上（需支持变参模板、类型萃取等特性）
 */
/****************************************************************************************/

#ifndef OL_STRING_H
#define OL_STRING_H 1

#include <cstdio>
#include <string>
#include <type_traits>
#include <vector>

namespace ol
{

    // ===========================================================================
    /**
     * @brief 删除字符串左边指定字符
     * @param str 待处理的字符串（C字符串会被直接修改，std::string为引用）
     * @param c 要删除的字符（默认空格' '）
     * @return 修改后的字符串（C字符串返回指针，std::string返回引用）
     */
    char* deleteLchr(char* str, const char c = ' ');               // C字符串版本
    std::string& deleteLchr(std::string& str, const char c = ' '); // std::string版本

    /**
     * @brief 删除字符串右边指定字符
     * @param str 待处理的字符串（C字符串会被直接修改，std::string为引用）
     * @param c 要删除的字符（默认空格' '）
     * @return 修改后的字符串（C字符串返回指针，std::string返回引用）
     */
    char* deleteRchr(char* str, const char c = ' ');               // C字符串版本
    std::string& deleteRchr(std::string& str, const char c = ' '); // std::string版本

    /**
     * @brief 删除字符串左右两边指定字符
     * @param str 待处理的字符串（C字符串会被直接修改，std::string为引用）
     * @param c 要删除的字符（默认空格' '）
     * @return 修改后的字符串（C字符串返回指针，std::string返回引用）
     */
    char* deleteLRchr(char* str, const char c = ' ');               // C字符串版本
    std::string& deleteLRchr(std::string& str, const char c = ' '); // std::string版本

    /**
     * @brief 将字符串中的小写字母转换为大写（非字母字符不变）
     * @param str 待转换的字符串（C字符串会被直接修改，std::string为引用）
     * @return 修改后的字符串（C字符串返回指针，std::string返回引用）
     */
    char* toupper(char* str);               // C字符串版本
    std::string& toupper(std::string& str); // std::string版本

    /**
     * @brief 将字符串中的大写字母转换为小写（非字母字符不变）
     * @param str 待转换的字符串（C字符串会被直接修改，std::string为引用）
     * @return 修改后的字符串（C字符串返回指针，std::string返回引用）
     */
    char* tolower(char* str);               // C字符串版本
    std::string& tolower(std::string& str); // std::string版本

    /**
     * @brief 字符串替换
     * @param str 待处理的字符串（C字符串会被直接修改，std::string为引用）
     * @param str1 要替换的旧子串
     * @param str2 替换的新子串
     * @param bloop 是否循环替换（默认false）
     * @return true-替换成功，false-替换失败（如存在逻辑错误）
     * @note 1、如果str2比str1要长，替换后str会变长，C字符串需保证足够空间（std::string无此问题）。
     *       2、如果str2中包含str1且bloop为true，存在逻辑错误，函数将不执行操作。
     *       3、如果str2为空，表示删除str中所有str1的内容。
     */
    bool replacestr(char* str, const std::string& str1, const std::string& str2, const bool bloop = false);        // C字符串版本
    bool replacestr(std::string& str, const std::string& str1, const std::string& str2, const bool bloop = false); // std::string版本

    /**
     * @brief 从字符串中提取数字相关字符
     * @param src 源字符串
     * @param dest 存储结果的目标变量（C字符串/ std::string引用，仅前两个版本需要）
     * @param bsigned 是否提取符号（+/-，默认false）
     * @param bdot 是否提取小数点（.，默认false）
     * @return 提取结果（C字符串返回指针，std::string返回引用或新字符串）
     * @note src和dest可指向同一变量（前两个版本）
     */
    char* picknumber(const std::string& src, char* dest, const bool bsigned = false, const bool bdot = false);               // C字符串输出版本
    std::string& picknumber(const std::string& src, std::string& dest, const bool bsigned = false, const bool bdot = false); // std::string输出版本
    std::string picknumber(const std::string& src, const bool bsigned = false, const bool bdot = false);                     // 返回新字符串版本

    /**
     * @brief 正则匹配字符串（支持通配符*，匹配多个任意字符）
     * @param str 待匹配的字符串（精确内容）
     * @param rules 匹配规则（用*表示多个任意字符，多规则用半角的逗号分隔，如"*.h,*.cpp"）
     * @return true-匹配成功，false-匹配失败
     * @note 1）str参数不需要支持"*"，rules参数支持"*"；
     *       2）函数在判断str是否匹配rules的时候，会忽略字母的大小写。
     */
    bool matchstr(const std::string& str, const std::string& rules);
    // ===========================================================================

    // ===========================================================================
    // ccmdstr类，命令行字符串拆分类，用于解析带分隔符的结构化字符串。
    // 字符串的格式为：字段内容1+分隔符+字段内容2+分隔符+字段内容3+分隔符+...+字段内容n。
    // 例如："messi,10,striker,30,1.72,68.5,Barcelona"，这是足球运动员梅西的资料。
    // 包括：姓名、球衣号码、场上位置、年龄、身高、体重和效力的俱乐部，字段之间用半角的逗号分隔。
    class ccmdstr
    {
    private:
        std::vector<std::string> m_cmdstr; // 拆分后的字段容器

        ccmdstr(const ccmdstr&) = delete;            // 禁用拷贝构造函数
        ccmdstr& operator=(const ccmdstr&) = delete; // 禁用赋值函数
    public:
        // 默认构造函数
        ccmdstr()
        {
        }

        /**
         * @brief 带参构造函数，直接拆分字符串
         * @param buffer 待拆分的字符串
         * @param sepstr 分隔符（支持多字符，如",,"、" | "）
         * @param bdelspace 是否删除字段前后空格（默认false）
         */
        ccmdstr(const std::string& buffer, const std::string& sepstr, const bool bdelspace = false);

        /**
         * @brief 重载[]运算符，访问拆分后的字段(m_cmdstr成员)
         * @param i 字段索引（从0开始）
         * @return 字段内容的常量引用
         * @note 索引越界会抛出异常
         */
        const std::string& operator[](int i) const
        {
            return m_cmdstr[i];
        }

        /**
         * @brief 拆分字符串并存储到内部容器
         * @param buffer 待拆分的字符串
         * @param sepstr 分隔符（支持多字符，注意，sepstr参数的数据类型不是字符，是字符串，如","、" "、"|"、"~!~"）
         * @param bdelspace 是否删除字段前后空格（默认false）
         * @note 空字段会被保留（如",test"拆分为["", "test"]）
         */
        void split(const std::string& buffer, const std::string& sepstr, const bool bdelspace = false);

        /**
         * @brief 获取拆分后的字段数量
         * @return 字段总数（m_cmdstr的大小）
         */
        size_t size() const
        {
            return m_cmdstr.size();
        }

        /**
         * @brief 从字段容器（m_cmdstr）中获取指定索引的字段内容并转换为目标类型
         * @param i 字段索引（从0开始）
         * @param value 存储结果的变量引用
         * @param len 仅字符串类型有效，指定截取长度（默认0表示不截取）
         * @return true-成功（索引有效且转换成功），false-失败（索引越界或转换失败）
         */
        bool getvalue(const size_t i, std::string& value, const size_t len = 0) const; // std::string版本
        bool getvalue(const size_t i, char* value, const size_t len = 0) const;        // C字符串版本（自动添加'\0'）
        bool getvalue(const size_t i, int& value) const;                               // 转换为int
        bool getvalue(const size_t i, unsigned int& value) const;                      // 转换为unsigned int
        bool getvalue(const size_t i, long& value) const;                              // 转换为long
        bool getvalue(const size_t i, unsigned long& value) const;                     // 转换为unsigned long
        bool getvalue(const size_t i, double& value) const;                            // 转换为double
        bool getvalue(const size_t i, float& value) const;                             // 转换为float
        bool getvalue(const size_t i, bool& value) const;                              // 转换为bool（"true"/"1"为true）

        // 析构函数
        ~ccmdstr();
    };

    /**
     * @brief 重载<<运算符，输出ccmdstr的字段内容（调试用）
     * @param out 输出流
     * @param cmdstr ccmdstr对象
     * @return 输出流引用
     */
    std::ostream& operator<<(std::ostream& out, const ccmdstr& cmdstr);
    // ===========================================================================

    // ===========================================================================
    /**
     * @brief 解析XML格式字符串，提取指定标签的内容并转换为目标类型
     * @param xmlbuffer 待解析的XML格式字符串（如"<tag>value</tag>..."）
     * @param fieldname 要提取的字段标签名（如"filename"对应<filename>标签）
     * @param value 存储结果的变量引用/指针
     * @param len 仅字符串类型有效，指定内容最大长度（默认0表示不限长度）
     * @return true-成功（标签存在且转换成功），false-失败（标签不存在或转换失败）
     * @note 当value为char[]时，需保证数组内存充足，避免溢出
     * @example <filename>/tmp/_public.h</filename><mtime>2020-01-01 12:20:35</mtime><size>18348</size>
     */
    bool getByXml(const std::string& xmlbuffer, const std::string& fieldname, std::string& value, const size_t len = 0); // 提取为std::string
    bool getByXml(const std::string& xmlbuffer, const std::string& fieldname, char* value, const size_t len = 0);        // 提取为C字符串（自动添加'\0'）
    bool getByXml(const std::string& xmlbuffer, const std::string& fieldname, bool& value);                              // 转换为bool
    bool getByXml(const std::string& xmlbuffer, const std::string& fieldname, int& value);                               // 转换为int
    bool getByXml(const std::string& xmlbuffer, const std::string& fieldname, unsigned int& value);                      // 转换为unsigned int
    bool getByXml(const std::string& xmlbuffer, const std::string& fieldname, long& value);                              // 转换为long
    bool getByXml(const std::string& xmlbuffer, const std::string& fieldname, unsigned long& value);                     // 转换为unsigned long
    bool getByXml(const std::string& xmlbuffer, const std::string& fieldname, double& value);                            // 转换为double
    bool getByXml(const std::string& xmlbuffer, const std::string& fieldname, float& value);                             // 转换为float
    // ===========================================================================

    // ===========================================================================
    // C++格式化输出函数模板。
    // 辅助函数：自动转换 std::string 到 const char*
    namespace base
    {
        /**
         * @brief 格式化参数辅助函数，自动将std::string转换为const char*
         * @tparam T 参数类型
         * @param arg 待转换的参数
         * @return 转换后的参数（const char*或原类型）
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
    } // namespace base

    /**
     * @brief 格式化输出函数（写入已有字符串）
     * @tparam Types 可变参数类型列表
     * @param str 存储结果的字符串引用
     * @param fmt 格式字符串（C风格）
     * @param args 待格式化的参数
     * @return true-格式化成功，false-失败
     */
    template <typename... Types>
    bool sformat(std::string& str, const char* fmt, Types&&... args)
    {
        // 计算长度
        int len = std::snprintf(nullptr, 0, fmt, base::format_arg(std::forward<Types>(args))...);
        if (len < 0) return false;

        if (len == 0)
        {
            str.clear();
            return true;
        }

        // 执行格式化
        str.resize(len);
        std::snprintf(&str[0], len + 1, fmt, base::format_arg(std::forward<Types>(args))...);
        return true;
    }

    /**
     * @brief 格式化输出函数（返回新字符串）
     * @tparam Types 可变参数类型列表
     * @param fmt 格式字符串（C风格）
     * @param args 待格式化的参数
     * @return 格式化后的新字符串
     */
    template <typename... Types>
    std::string sformat(const char* fmt, Types&&... args)
    {
        std::string str;
        int len = std::snprintf(nullptr, 0, fmt, base::format_arg(std::forward<Types>(args))...);
        if (len <= 0) return str;

        str.resize(len);
        std::snprintf(&str[0], len + 1, fmt, base::format_arg(std::forward<Types>(args))...);
        return str;
    }
    // ===========================================================================

    // ===========================================================================
    /**
     * @brief KMP算法查找子串
     * @param str 主字符串
     * @param pattern 待查找的子串（模式串）
     * @return 子串在主串中首次出现的位置（从0开始），未找到返回std::string::npos
     */
    size_t skmp(const std::string& str, const std::string& pattern);
    // ===========================================================================
} // namespace ol

#endif // !OL_STRING_H