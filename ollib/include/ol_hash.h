#include <functional>

#ifndef __OL_HASH_H
#define __OL_HASH_H 1

// ol_hash.h - OL库的哈希工具集
// 提供通用哈希组合函数 hash_combine 和 hash_val，
// 支持任意类型和数量的参数组合哈希值计算。

namespace ol
{

    // 万能哈希函数核心实现
    // 1. 哈希组合函数：将单个值的哈希合并到种子中
    template <typename T>
    inline void hash_combine(std::size_t& seed, const T& val)
    {
        seed ^= std::hash<T>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    // 2. 可变参数哈希计算：单参数版本
    template <typename T>
    inline void hash_val(std::size_t& seed, const T& val)
    {
        hash_combine(seed, val);
    }

    // 3. 可变参数哈希计算：多参数递归版本
    template <typename T, typename... Args>
    inline void hash_val(std::size_t& seed, const T& val, const Args... args)
    {
        hash_combine(seed, val);
        hash_val(seed, args...); // 递归展开参数包
    }

    // 4. 可变参数哈希计算：入口函数
    template <typename... Args>
    inline std::size_t hash_val(const Args... args)
    {
        std::size_t seed = 0;
        hash_val(seed, args...); // 处理所有参数
        return seed;
    }

} // namespace ol

#endif // !__OL_HASH_H