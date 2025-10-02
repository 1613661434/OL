/****************************************************************************************/
/*
 * 程序名：ol_trieset.h
 * 功能描述：基于Trie树的字符串集合类，仅存储键不关注值，特性包括：
 *          - 复用TrieMap实现，底层使用TrieMap<bool>存储（true作为占位值）
 *          - 支持字符串的添加、删除、存在性判断
 *          - 提供前缀匹配（最短/最长前缀、前缀元素列表）和模式匹配（通配符'.'）
 *          - 方法名与TrieMap保持一致，降低使用成本
 * 作者：ol
 * 适用标准：C++17及以上（依赖ol_TrieMap.h及std::list等特性）
 */
/****************************************************************************************/

#ifndef OL_TRIESET_H
#define OL_TRIESET_H 1

#include "ol_TrieMap.h"

namespace ol
{

    /**
     * @brief Trie树实现的字符串集合类（TrieSet）
     * @note 仅存储字符串键，不关联具体值，底层依赖TrieMap<bool>实现
     */
    class TrieSet
    {
    private:
        TrieMap<bool> map; // 底层TrieMap，用true标记键存在

    public:
        // 元素添加
        // ===========================================================================
        /**
         * @brief 向集合中添加字符串（重复添加不影响）
         * @param key 要添加的字符串
         */
        void put(const std::string& key)
        {
            map.put(key, true); // 用true作为占位值，复用TrieMap的添加逻辑
        }
        // ===========================================================================

        // 元素删除
        // ===========================================================================
        /**
         * @brief 从集合中删除字符串
         * @param key 要删除的字符串
         */
        void remove(const std::string& key)
        {
            map.remove(key); // 复用TrieMap的删除逻辑
        }
        // ===========================================================================

        // 元素查询
        // ===========================================================================
        /**
         * @brief 判断字符串是否存在于集合中
         * @param key 要检查的字符串
         * @return 存在返回true，否则返回false
         */
        bool has(const std::string& key)
        {
            return map.has(key); // 复用TrieMap的存在性判断
        }

        /**
         * @brief 查找查询字符串的最短前缀（该前缀必须是集合中的元素）
         * @param query 目标字符串
         * @return 最短前缀字符串，不存在则返回空串
         */
        std::string shortestPrefix(const std::string& query)
        {
            return map.shortestPrefix(query); // 复用TrieMap的前缀查询
        }

        /**
         * @brief 查找查询字符串的最长前缀（该前缀必须是集合中的元素）
         * @param query 目标字符串
         * @return 最长前缀字符串，不存在则返回空串
         */
        std::string longestPrefix(const std::string& query)
        {
            return map.longestPrefix(query); // 复用TrieMap的前缀查询
        }

        /**
         * @brief 获取所有以指定前缀开头的元素
         * @param prefix 前缀字符串
         * @return 匹配的元素列表（std::list<std::string>）
         */
        std::list<std::string> keysByPrefix(const std::string& prefix)
        {
            return map.keysByPrefix(prefix); // 复用TrieMap的前缀匹配
        }

        /**
         * @brief 判断集合中是否存在以指定前缀开头的元素
         * @param prefix 前缀字符串
         * @return 存在返回true，否则返回false
         */
        bool hasPrefix(const std::string& prefix)
        {
            return map.hasPrefix(prefix); // 复用TrieMap的前缀存在性判断
        }

        /**
         * @brief 获取所有匹配模式的元素（支持'.'作为通配符，匹配单个任意字符）
         * @param pattern 模式字符串
         * @return 匹配的元素列表（std::list<std::string>）
         */
        std::list<std::string> keysByPattern(const std::string& pattern)
        {
            return map.keysByPattern(pattern); // 复用TrieMap的模式匹配
        }

        /**
         * @brief 判断集合中是否存在匹配模式的元素（支持'.'作为通配符）
         * @param pattern 模式字符串
         * @return 存在返回true，否则返回false
         */
        bool hasPattern(const std::string& pattern)
        {
            return map.hasPattern(pattern); // 复用TrieMap的模式存在性判断
        }

        /**
         * @brief 获取集合中元素的数量
         * @return 元素总数（size_t）
         */
        size_t size() const
        {
            return map.size(); // 复用TrieMap的计数
        }
        // ===========================================================================
    };

} // namespace ol

#endif // !OL_TRIESET_H