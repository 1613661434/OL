/****************************************************************************************/
/*
 * 程序名：ol_enum_char_ops.h
 * 功能描述：为底层类型为 char 的强类型枚举（enum class : char）提供通用运算符重载，
 *          支持 int 类型常用的算术/比较操作，兼容 C++11 及以上标准，无外部依赖
 *          - 自增/自减：前置++、后置++、前置--、后置--
 *          - 比较运算符：<、>、<=、>=、==、!=（完整大小比较）
 *          - 算术运算符：+、-（枚举值 ± 整数偏移量）
 *          - 赋值运算符：+=、-=（枚举值自增/自减指定偏移量）
 *          所有重载仅作用于「底层为 char 的强类型枚举」，避免全局作用域污染
 * 作者：ol
 * 适用标准：C++11及以上（需支持 type_traits 头文件及相关类型萃取特性）
 */
/****************************************************************************************/

#ifndef OL_ENUM_CHAR_OPS_H
#define OL_ENUM_CHAR_OPS_H 1

#include <type_traits>

namespace ol
{
    /**
     * @brief 模板类型限制：仅匹配「底层为 char 的强类型枚举（enum class）」
     * @tparam E 枚举类型
     */
    template <typename E>
    using EnableIfCharEnum = typename std::enable_if<
        std::is_enum<E>::value &&                                              // 类型 E 是枚举类型
            !std::is_convertible<E, int>::value &&                             // 是强类型枚举（enum class，普通枚举可隐式转int）
            std::is_same<typename std::underlying_type<E>::type, char>::value, // 底层类型为 char
        void>::type;

    // ====================== 自增运算符 ======================
    /**
     * @brief 重载前置 ++ 运算符
     * @tparam E 底层为 char 的强类型枚举
     * @tparam = 类型限制占位符（仅匹配符合条件的枚举）
     * @param e 待自增的枚举值（引用）
     * @return E& 自增后的枚举值引用（支持链式操作）
     * @note C++11 要求 inline 模板函数避免链接冲突
     */
    template <typename E, typename = EnableIfCharEnum<E>>
    inline E& operator++(E& e)
    {
        e = static_cast<E>(static_cast<char>(e) + 1);
        return e;
    }

    /**
     * @brief 重载后置 ++ 运算符
     * @tparam E 底层为 char 的强类型枚举
     * @tparam = 类型限制占位符（仅匹配符合条件的枚举）
     * @param e 待自增的枚举值（引用）
     * @param int 后置运算符标记（无实际意义）
     * @return E 自增前的枚举值副本
     */
    template <typename E, typename = EnableIfCharEnum<E>>
    inline E operator++(E& e, int)
    {
        E temp = e;
        ++e;
        return temp;
    }

    // ====================== 自减运算符 ======================
    /**
     * @brief 重载前置 -- 运算符
     * @tparam E 底层为 char 的强类型枚举
     * @tparam = 类型限制占位符（仅匹配符合条件的枚举）
     * @param e 待自减的枚举值（引用）
     * @return E& 自减后的枚举值引用（支持链式操作）
     */
    template <typename E, typename = EnableIfCharEnum<E>>
    inline E& operator--(E& e)
    {
        e = static_cast<E>(static_cast<char>(e) - 1);
        return e;
    }

    /**
     * @brief 重载后置 -- 运算符
     * @tparam E 底层为 char 的强类型枚举
     * @tparam = 类型限制占位符（仅匹配符合条件的枚举）
     * @param e 待自减的枚举值（引用）
     * @param int 后置运算符标记（无实际意义）
     * @return E 自减前的枚举值副本
     */
    template <typename E, typename = EnableIfCharEnum<E>>
    inline E operator--(E& e, int)
    {
        E temp = e;
        --e;
        return temp;
    }

    // ====================== 比较运算符 ======================
    /**
     * @brief 重载小于运算符 <
     * @tparam E 底层为 char 的强类型枚举
     * @tparam = 类型限制占位符（仅匹配符合条件的枚举）
     * @param lhs 左操作数（枚举值）
     * @param rhs 右操作数（枚举值）
     * @return bool true-左操作数小于右操作数，false-反之
     */
    template <typename E, typename = EnableIfCharEnum<E>>
    inline bool operator<(const E& lhs, const E& rhs)
    {
        return static_cast<char>(lhs) < static_cast<char>(rhs);
    }

    /**
     * @brief 重载大于运算符 >
     * @tparam E 底层为 char 的强类型枚举
     * @tparam = 类型限制占位符（仅匹配符合条件的枚举）
     * @param lhs 左操作数（枚举值）
     * @param rhs 右操作数（枚举值）
     * @return bool true-左操作数大于右操作数，false-反之
     */
    template <typename E, typename = EnableIfCharEnum<E>>
    inline bool operator>(const E& lhs, const E& rhs)
    {
        return static_cast<char>(lhs) > static_cast<char>(rhs);
    }

    /**
     * @brief 重载小于等于运算符 <=
     * @tparam E 底层为 char 的强类型枚举
     * @tparam = 类型限制占位符（仅匹配符合条件的枚举）
     * @param lhs 左操作数（枚举值）
     * @param rhs 右操作数（枚举值）
     * @return bool true-左操作数小于等于右操作数，false-反之
     */
    template <typename E, typename = EnableIfCharEnum<E>>
    inline bool operator<=(const E& lhs, const E& rhs)
    {
        return static_cast<char>(lhs) <= static_cast<char>(rhs);
    }

    /**
     * @brief 重载大于等于运算符 >=
     * @tparam E 底层为 char 的强类型枚举
     * @tparam = 类型限制占位符（仅匹配符合条件的枚举）
     * @param lhs 左操作数（枚举值）
     * @param rhs 右操作数（枚举值）
     * @return bool true-左操作数大于等于右操作数，false-反之
     */
    template <typename E, typename = EnableIfCharEnum<E>>
    inline bool operator>=(const E& lhs, const E& rhs)
    {
        return static_cast<char>(lhs) >= static_cast<char>(rhs);
    }

    /**
     * @brief 重载等于运算符 ==
     * @tparam E 底层为 char 的强类型枚举
     * @tparam = 类型限制占位符（仅匹配符合条件的枚举）
     * @param lhs 左操作数（枚举值）
     * @param rhs 右操作数（枚举值）
     * @return bool true-两枚举值相等，false-反之
     */
    template <typename E, typename = EnableIfCharEnum<E>>
    inline bool operator==(const E& lhs, const E& rhs)
    {
        return static_cast<char>(lhs) == static_cast<char>(rhs);
    }

    /**
     * @brief 重载不等于运算符 !=
     * @tparam E 底层为 char 的强类型枚举
     * @tparam = 类型限制占位符（仅匹配符合条件的枚举）
     * @param lhs 左操作数（枚举值）
     * @param rhs 右操作数（枚举值）
     * @return bool true-两枚举值不相等，false-反之
     */
    template <typename E, typename = EnableIfCharEnum<E>>
    inline bool operator!=(const E& lhs, const E& rhs)
    {
        return !(lhs == rhs);
    }

    // ====================== 算术运算符 ======================
    /**
     * @brief 重载加法运算符 +（枚举值 + 整数偏移量）
     * @tparam E 底层为 char 的强类型枚举
     * @tparam = 类型限制占位符（仅匹配符合条件的枚举）
     * @param e 枚举值（原数值）
     * @param offset 整数偏移量（可正可负）
     * @return E 偏移后的枚举值
     */
    template <typename E, typename = EnableIfCharEnum<E>>
    inline E operator+(const E& e, int offset)
    {
        return static_cast<E>(static_cast<char>(e) + offset);
    }

    /**
     * @brief 重载减法运算符 -（枚举值 - 整数偏移量）
     * @tparam E 底层为 char 的强类型枚举
     * @tparam = 类型限制占位符（仅匹配符合条件的枚举）
     * @param e 枚举值（原数值）
     * @param offset 整数偏移量（可正可负）
     * @return E 偏移后的枚举值
     */
    template <typename E, typename = EnableIfCharEnum<E>>
    inline E operator-(const E& e, int offset)
    {
        return static_cast<E>(static_cast<char>(e) - offset);
    }

    // ====================== 赋值运算符 ======================
    /**
     * @brief 重载赋值加法运算符 +=（枚举值自增指定偏移量）
     * @tparam E 底层为 char 的强类型枚举
     * @tparam = 类型限制占位符（仅匹配符合条件的枚举）
     * @param e 待操作的枚举值（引用）
     * @param offset 整数偏移量（可正可负）
     * @return E& 操作后的枚举值引用（支持链式操作）
     */
    template <typename E, typename = EnableIfCharEnum<E>>
    inline E& operator+=(E& e, int offset)
    {
        e = e + offset;
        return e;
    }

    /**
     * @brief 重载赋值减法运算符 -=（枚举值自减指定偏移量）
     * @tparam E 底层为 char 的强类型枚举
     * @tparam = 类型限制占位符（仅匹配符合条件的枚举）
     * @param e 待操作的枚举值（引用）
     * @param offset 整数偏移量（可正可负）
     * @return E& 操作后的枚举值引用（支持链式操作）
     */
    template <typename E, typename = EnableIfCharEnum<E>>
    inline E& operator-=(E& e, int offset)
    {
        e = e - offset;
        return e;
    }

} // namespace ol

// 全局导出运算符
using ol::operator++;
using ol::operator--;
using ol::operator<;
using ol::operator>;
using ol::operator<=;
using ol::operator>=;
using ol::operator==;
using ol::operator!=;
using ol::operator+;
using ol::operator-;
using ol::operator+=;
using ol::operator-=;

#endif // !OL_ENUM_CHAR_OPS_H