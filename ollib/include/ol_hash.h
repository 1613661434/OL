/****************************************************************************************/
/*
 * 程序名：ol_hash.h
 * 功能描述：哈希工具集，提供通用哈希组合功能，支持以下特性：
 *          - 哈希组合函数（hash_combine）：将单个值的哈希合并到种子中
 *          - 可变参数哈希计算（hash_val）：支持任意类型和数量的参数组合计算哈希值
 *          - 适用于自定义类型的哈希计算场景（如作为unordered_map的哈希函数）
 * 作者：ol
 * 适用标准：C++11及以上（需支持变参模板、std::hash等特性）
 */
/****************************************************************************************/

#include <functional>

#ifndef OL_HASH_H
#define OL_HASH_H 1

namespace ol
{

    /**
     * @brief 哈希组合函数：将单个值的哈希值合并到种子中
     * @tparam T 待哈希的值类型
     * @param seed 哈希种子（会被修改）
     * @param val 待合并的数值
     * @note 采用boost库的哈希组合算法，具有良好的雪崩效应
     */
    template <typename T>
    inline void hash_combine(std::size_t& seed, const T& val)
    {
        seed ^= std::hash<T>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    /**
     * @brief 可变参数哈希计算：单参数版本（递归终止函数）
     * @tparam T 待哈希的值类型
     * @param seed 哈希种子（会被修改）
     * @param val 待哈希的数值
     */
    template <typename T>
    inline void hash_val(std::size_t& seed, const T& val)
    {
        hash_combine(seed, val);
    }

    /**
     * @brief 可变参数哈希计算：多参数递归版本
     * @tparam T 第一个待哈希的值类型
     * @tparam Types 剩余待哈希的值类型列表
     * @param seed 哈希种子（会被修改）
     * @param val 第一个待哈希的数值
     * @param args 剩余待哈希的数值
     * @note 通过递归展开参数包，依次处理所有输入参数
     */
    template <typename T, typename... Types>
    inline void hash_val(std::size_t& seed, const T& val, const Types... args)
    {
        hash_combine(seed, val);
        hash_val(seed, args...); // 递归展开参数包
    }

    /**
     * @brief 可变参数哈希计算：入口函数
     * @tparam Types 待哈希的所有值类型列表
     * @param args 待计算哈希的所有数值
     * @return 组合后的哈希值（std::size_t类型）
     */
    template <typename... Types>
    inline std::size_t hash_val(const Types... args)
    {
        std::size_t seed = 0;
        hash_val(seed, args...); // 处理所有参数
        return seed;
    }

} // namespace ol

#endif // !OL_HASH_H