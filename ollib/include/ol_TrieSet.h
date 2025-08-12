#ifndef __OL_TRIESET_H
#define __OL_TRIESET_H 1

#include "ol_TrieMap.h"

namespace ol
{

    /**
     * TrieSet 基于 TrieMap 实现的集合，仅存储键而不关注值
     * 内部使用 TrieMap<bool> 作为底层结构，以 true 作为占位值
     */
    class TrieSet
    {
    private:
        TrieMap<bool> map; // 底层依赖 TrieMap，值仅作为存在标记

    public:
        // **** 增 ****

        /**
         * 向集合中添加元素
         * @param key 要添加的元素（字符串）
         */
        void add(const std::string& key)
        {
            map.put(key, true); // 用 true 标记键存在
        }

        // **** 删 ****

        /**
         * 从集合中删除元素
         * @param key 要删除的元素（字符串）
         */
        void remove(const std::string& key)
        {
            map.remove(key);
        }

        // **** 查 ****

        /**
         * 判断元素是否存在于集合中
         * @param key 要检查的元素（字符串）
         * @return 存在返回 true，否则返回 false
         */
        bool contains(const std::string& key)
        {
            return map.has(key); // 复用 TrieMap 的 has 方法
        }

        /**
         * 查找 query 的最短前缀（该前缀需为集合中的元素）
         * @param query 目标字符串
         * @return 最短前缀字符串，不存在则返回空串
         */
        std::string shortestPrefix(const std::string& query)
        {
            return map.shortestPrefix(query);
        }

        /**
         * 查找 query 的最长前缀（该前缀需为集合中的元素）
         * @param query 目标字符串
         * @return 最长前缀字符串，不存在则返回空串
         */
        std::string longestPrefix(const std::string& query)
        {
            return map.longestPrefix(query);
        }

        /**
         * 获取所有以 prefix 为前缀的元素
         * @param prefix 前缀字符串
         * @return 匹配的元素列表
         */
        std::list<std::string> keysByPrefix(const std::string& prefix)
        {
            return map.keysByPrefix(prefix);
        }

        /**
         * 判断集合中是否存在以 prefix 为前缀的元素
         * @param prefix 前缀字符串
         * @return 存在返回 true，否则返回 false
         */
        bool hasPrefix(const std::string& prefix)
        {
            return map.hasPrefix(prefix);
        }

        /**
         * 获取所有匹配模式的元素（支持 '.' 作为通配符，匹配任意字符）
         * @param pattern 模式字符串
         * @return 匹配的元素列表
         */
        std::list<std::string> keysByPattern(const std::string& pattern)
        {
            return map.keysByPattern(pattern);
        }

        /**
         * 判断集合中是否存在匹配模式的元素（支持 '.' 作为通配符）
         * @param pattern 模式字符串
         * @return 存在返回 true，否则返回 false
         */
        bool hasPattern(const std::string& pattern)
        {
            return map.hasPattern(pattern);
        }

        /**
         * 获取集合中元素的个数
         * @return 元素数量
         */
        size_t size()
        {
            return map.size();
        }
    };

} // namespace ol

#endif // !__OL_TRIESET_H