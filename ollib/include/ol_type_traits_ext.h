/****************************************************************************************/
/*
 * 程序名：ol_type_traits_ext.h
 * 功能描述：提供常用的类型特性扩展和辅助类，包括：
 *          - 不可拷贝/不可移动类特性
 *          - 空类型标记和基础类型工具
 *          - 类型判断与转换的辅助模板
 *          - 模板元编程常用基础结构
 * 作者：ol
 * 适用标准：C++17及以上（需支持constexpr、delete/default等特性）
 */
/****************************************************************************************/

#ifndef OL_TYPE_TRAITS_EXT_H
#define OL_TYPE_TRAITS_EXT_H 1

#include <cstddef>
#include <type_traits>
#include <utility>

namespace ol
{
    /**
     * @brief 空结构体标记，用于模板中需要占位但不占用内存的场景
     * @note 尺寸为1字节（C++标准规定空结构体大小至少为1），但语义上表示"无数据"
     */
    struct TypeEmpty
    {
    };

    /**
     * @brief 不可拷贝标记类，继承此类的派生类将禁用拷贝构造和拷贝赋值
     * @usage class MyClass : public TypeNonCopyable {};
     * @note 构造函数和析构函数为protected，允许派生类正常构造和析构
     */
    class TypeNonCopyable
    {
    public:
        // 禁用拷贝构造
        TypeNonCopyable(const TypeNonCopyable&) = delete;
        // 禁用拷贝赋值
        TypeNonCopyable& operator=(const TypeNonCopyable&) = delete;

    protected:
        // 允许默认构造
        TypeNonCopyable() = default;
        // 允许默认析构
        ~TypeNonCopyable() = default;
    };

    /**
     * @brief 不可移动标记类，继承此类的派生类将禁用移动构造和移动赋值
     * @usage class MyClass : public TypeNonMovable {};
     * @note 通常与TypeNonCopyable结合使用：class MyClass : public TypeNonCopyable, public TypeNonMovable {};
     */
    class TypeNonMovable
    {
    public:
        // 禁用移动构造
        TypeNonMovable(TypeNonMovable&&) = delete;
        // 禁用移动赋值
        TypeNonMovable& operator=(TypeNonMovable&&) = delete;

    protected:
        TypeNonMovable() = default;
        ~TypeNonMovable() = default;
    };

    /**
     * @brief 不可拷贝且不可移动标记类（TypeNonCopyable + TypeNonMovable的组合）
     * @usage class MyClass : public TypeNonCopyableMovable {};
     * @note 适用于单例模式等绝对禁止值语义的场景
     */
    class TypeNonCopyableMovable : public TypeNonCopyable, public TypeNonMovable
    {
    protected:
        TypeNonCopyableMovable() = default;
        ~TypeNonCopyableMovable() = default;
    };

    /**
     * @brief 类型包装器，用于在模板参数中传递类型而不实例化
     * @tparam T 被包装的类型
     * @example using IntWrapper = TypeWrapper<int>;
     */
    template <typename T>
    struct TypeWrapper
    {
        using type = T;
    };

    /**
     * @brief 检查类型是否为ol::TypeEmpty类型
     * @tparam T 待检查类型
     * @value 若T是ol::TypeEmpty则为true，否则为false
     */
    template <typename T>
    struct IsTypeEmpty : std::is_same<std::remove_cv_t<T>, TypeEmpty>
    {
    };

    template <typename T>
    inline constexpr bool IsTypeEmpty_v = IsTypeEmpty<T>::value;

    /**
     * @brief 检查类型是否可迭代（存在begin()和end()）
     * @tparam T 待检查类型
     * @value 若T是可迭代类型则为true，否则为false
     */
    template <typename T, typename = void>
    struct IsIterable : std::false_type
    {
    };

    template <typename T>
    struct IsIterable<T, std::void_t<decltype(std::begin(std::declval<T>())),
                                     decltype(std::end(std::declval<T>()))>>
        : std::true_type
    {
    };

    template <typename T>
    inline constexpr bool IsIterable_v = IsIterable<T>::value;

    /**
     * @brief 条件选择类型，根据条件选择不同类型（编译期分支）
     * @tparam Condition 条件表达式（bool类型）
     * @tparam TrueType 条件为true时的类型
     * @tparam FalseType 条件为false时的类型
     * @note 功能类似std::conditional，但增加更友好的注释和命名
     */
    template <bool Condition, typename TrueType, typename FalseType>
    struct ConditionalType : std::conditional<Condition, TrueType, FalseType>
    {
    };

    template <bool Condition, typename TrueType, typename FalseType>
    using ConditionalType_t = typename ConditionalType<Condition, TrueType, FalseType>::type;

    /**
     * @brief 类型列表容器，用于模板元编程中存储一组类型
     * @tparam Ts 类型列表中的类型参数包
     * @example using MyTypes = TypeList<int, float, std::string>;
     */
    template <typename... Ts>
    struct TypeList
    {
    };

    /**
     * @brief 获取类型列表的长度
     * @tparam List 类型列表（必须是TypeList的实例）
     * @value 类型列表中包含的类型数量
     */
    template <typename List>
    struct TypeListSize : std::integral_constant<size_t, 0>
    {
    };

    template <typename... Ts>
    struct TypeListSize<TypeList<Ts...>> : std::integral_constant<size_t, sizeof...(Ts)>
    {
    };

    template <typename List>
    inline constexpr size_t TypeListSize_v = TypeListSize<List>::value;

} // namespace ol

#endif // !OL_TYPE_TRAITS_EXT_H