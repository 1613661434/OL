/****************************************************************************************/
/*
 * 程序名：ol_triemap.h
 * 功能描述：Trie树（字典树）的键值对实现类，支持以下特性：
 *          - 存储字符串键与任意类型值的映射关系
 *          - 支持键的插入、删除、查询及存在性判断
 *          - 提供前缀匹配功能（最短/最长前缀、前缀键列表）
 *          - 支持通配符'.'的模式匹配（匹配任意单个字符）
 *          - 内部使用shared_ptr管理节点内存，避免内存泄漏
 * 作者：ol
 * 适用标准：C++17及以上（需支持std::optional、std::shared_ptr等特性）
 */
/****************************************************************************************/

#ifndef OL_TRIEMAP_H
#define OL_TRIEMAP_H 1

#include <list>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace ol
{

    /**
     * @brief Trie树的节点类，存储单个字符对应的状态
     * @tparam V 节点存储的值类型
     */
    template <typename V>
    class TrieNode
    {
    public:
        V val = V();                                                              // 节点存储的值（默认初始化）
        bool isValid = false;                                                     // 标记该节点是否为某个键的终点（存储有效数据）
        std::unordered_map<unsigned char, std::shared_ptr<TrieNode<V>>> children; // 子节点映射（键：字符，值：子节点指针）

        // 构造函数（默认初始化，无需预分配空间）
        TrieNode()
        {
        }
    };

    /**
     * @brief Trie树键值对映射类（TrieMap）
     * @tparam V 存储的值类型
     */
    template <typename V>
    class TrieMap
    {
    private:
        std::shared_ptr<TrieNode<V>> root; // Trie 树的根节点
        size_t count;                      // 当前存在的键值对总数

        /**
         * @brief 遍历Trie树，收集所有有效键
         * @param node 起始节点
         * @param path 当前路径（已遍历的字符）
         * @param res 存储结果的列表
         */
        void traverse(std::shared_ptr<TrieNode<V>> node, std::string& path, std::list<std::string>& res)
        {
            if (!node) return;

            if (node->isValid) // 用标记位判断是否为有效键
            {
                res.push_back(path);
            }

            for (auto& [c, child] : node->children)
            {
                path.push_back(c);
                traverse(child, path, res);
                path.pop_back();
            }
        }

        /**
         * @brief 按模式匹配遍历Trie树（支持'.'作为通配符，匹配单个任意字符）
         * @param node 起始节点
         * @param path 当前匹配路径
         * @param pattern 模式字符串
         * @param i 当前匹配位置
         * @param res 存储匹配结果的列表
         */
        void traverseByPattern(std::shared_ptr<TrieNode<V>> node, std::string& path,
                               const std::string& pattern, size_t i, std::list<std::string>& res)
        {
            if (!node) return; // 节点不存在，匹配失败

            // 模式匹配完成
            if (i == pattern.length())
            {
                // 如果当前节点为有效键
                if (node->isValid)
                {
                    res.push_back(path);
                }
                return;
            }

            unsigned char c = pattern[i];
            if (c == '.')
            {
                // 通配符匹配任意字符，尝试所有可能的子节点
                for (auto& [ch, child] : node->children)
                {
                    path.push_back(ch);
                    traverseByPattern(child, path, pattern, i + 1, res);
                    path.pop_back();
                }
            }
            else
            {
                // 普通字符，查找对应的子节点
                auto it = node->children.find(c);
                if (it != node->children.end())
                {
                    path.push_back(c);
                    traverseByPattern(it->second, path, pattern, i + 1, res);
                    path.pop_back();
                }
            }
        }

        /**
         * @brief 递归插入键值对
         * @param node 当前节点
         * @param key 插入的键
         * @param val 插入的值
         * @param i 当前处理位置
         * @return 处理后的节点
         */
        std::shared_ptr<TrieNode<V>> put(std::shared_ptr<TrieNode<V>> node, const std::string& key, V val, size_t i)
        {
            if (!node)
            {
                node = std::make_shared<TrieNode<V>>();
            }

            if (i == key.length())
            {
                node->val = val;
                node->isValid = true; // 标记为有效节点（无论val是否为默认值）
                return node;
            }

            unsigned char c = key[i];
            node->children[c] = put(node->children[c], key, val, i + 1);
            return node;
        }

        /**
         * @brief 递归删除键
         * @param node 当前节点
         * @param key 删除的键
         * @param i 当前处理位置
         * @return 处理后的节点（可能为nullptr）
         */
        std::shared_ptr<TrieNode<V>> remove(std::shared_ptr<TrieNode<V>> node, const std::string& key, size_t i)
        {
            if (!node) return nullptr; // 节点不存在，直接返回

            // 到达键的末尾，标记为无效
            if (i == key.length())
            {
                node->isValid = false;
            }
            else
            {
                unsigned char c = key[i];
                // 递归删除子节点
                node->children[c] = remove(node->children[c], key, i + 1);
            }

            // 后序处理：如果节点有效（isValid=true），保留节点
            if (node->isValid)
            {
                return node;
            }

            // 如果没有子节点，删除当前节点
            if (node->children.empty())
            {
                return nullptr;
            }

            // 有子节点但无值，保留节点作为中间路径
            return node;
        }

        /**
         * @brief 检查模式是否匹配
         * @param node 当前节点
         * @param pattern 模式字符串
         * @param i 当前处理位置
         * @return 是否存在匹配的键
         */
        bool hasPattern(std::shared_ptr<TrieNode<V>> node, const std::string& pattern, size_t i)
        {
            if (!node) return false; // 节点不存在，匹配失败

            // 模式处理完毕，检查当前节点是否有值
            if (i == pattern.length())
            {
                return node->isValid;
            }

            unsigned char c = pattern[i];
            if (c != '.')
            {
                // 普通字符，查找对应子节点并递归匹配
                auto it = node->children.find(c);
                return (it != node->children.end()) && hasPattern(it->second, pattern, i + 1);
            }
            else
            {
                // 通配符，尝试所有子节点
                for (auto& [ch, child] : node->children)
                {
                    if (hasPattern(child, pattern, i + 1))
                    {
                        return true;
                    }
                }
                return false;
            }
        }

        /**
         * @brief 查找键对应的节点
         * @param node 起始节点
         * @param key 查找的键
         * @return 键终点对应的节点（不存在则返回nullptr）
         */
        std::shared_ptr<TrieNode<V>> findNode(std::shared_ptr<TrieNode<V>> node, const std::string& key)
        {
            auto current = node;
            for (size_t i = 0; i < key.length(); ++i)
            {
                if (!current) return nullptr; // 路径中断，返回null

                // 查找下一个字符对应的子节点
                auto it = current->children.find(key[i]);
                if (it == current->children.end())
                {
                    return nullptr; // 子节点不存在，返回null
                }

                current = it->second;
            }
            return current; // 返回键末尾对应的节点
        }

    public:
        // 构造函数，初始化根节点和计数
        TrieMap() : root(std::make_shared<TrieNode<V>>()), count(0)
        {
        }

        /**
         * @brief 插入或更新键值对
         * @param key 键字符串
         * @param val 对应的值
         */
        void put(const std::string& key, V val)
        {
            // 如果是新键，增加计数
            if (!has(key))
            {
                ++count;
            }
            root = put(root, key, val, 0);
        }

        /**
         * @brief 删除键值对
         * @param key 要删除的键
         */
        void remove(const std::string& key)
        {
            if (!has(key))
            {
                return;
            }
            // 递归删除并更新根节点
            root = remove(root, key, 0);
            --count;
        }

        /**
         * @brief 获取键对应的值，通过 std::optional 明确表示值是否存在
         * @param key 要查询的键
         * @return 若键存在，返回包含对应值的 std::optional<V>；若键不存在，返回 std::nullopt
         * @note 可通过 has_value() 判断键是否存在，通过 value() 获取值（键存在时）
         */
        std::optional<V> get(const std::string& key)
        {
            auto node = findNode(root, key);
            if (node && node->isValid)
            {
                return node->val;
            }
            return std::nullopt;
        }

        /**
         * @brief 检查键是否存在
         * @param key 要检查的键
         * @return 键存在返回true，否则返回false
         */
        bool has(const std::string& key)
        {
            auto node = findNode(root, key);
            return (node && node->isValid);
        }

        /**
         * @brief 检查是否存在以指定前缀开头的键
         * @param prefix 前缀字符串
         * @return 存在返回true，否则返回false
         */
        bool hasPrefix(const std::string& prefix)
        {
            return findNode(root, prefix) != nullptr;
        }

        /**
         * @brief 查找查询字符串的最短前缀键
         * @param query 查询字符串
         * @return 最短前缀键（不存在返回空串）
         */
        std::string shortestPrefix(const std::string& query)
        {
            auto current = root;
            for (size_t i = 0; i < query.length(); ++i)
            {
                if (!current) break;

                // 基于isValid判断，而非val是否为默认值
                if (current->isValid)
                {
                    return query.substr(0, i);
                }

                auto it = current->children.find(query[i]);
                if (it == current->children.end()) break;
                current = it->second;
            }

            if (current && current->isValid)
            {
                return query;
            }

            return "";
        }

        /**
         * @brief 查找查询字符串的最长前缀键
         * @param query 查询字符串
         * @return 最长前缀键（不存在返回空串）
         */
        std::string longestPrefix(const std::string& query)
        {
            auto current = root;
            size_t max_len = 0; // 记录最长有效前缀的长度
            size_t i;           // 循环变量（在循环外定义，用于判断是否遍历完所有字符）

            for (i = 0; i < query.length(); ++i)
            {
                if (!current) break; // 路径中断，退出循环

                // 如果当前节点是有效键，更新最长前缀长度
                if (current->isValid)
                {
                    max_len = i;
                }

                // 查找下一个字符对应的子节点
                auto it = current->children.find(query[i]);
                if (it == current->children.end()) break; // 字符不匹配，退出循环
                current = it->second;                     // 移动到子节点
            }

            // 只有当：1. 遍历完查询字符串的所有字符；2. 最终节点是有效键
            // 才返回整个查询字符串（说明查询字符串本身就是已插入的键）
            if (i == query.length() && current && current->isValid)
            {
                return query;
            }

            // 否则返回最长有效前缀
            return query.substr(0, max_len);
        }

        /**
         * @brief 获取所有以指定前缀开头的键
         * @param prefix 前缀字符串
         * @return 匹配的键列表
         */
        std::list<std::string> keysByPrefix(const std::string& prefix)
        {
            std::list<std::string> res;
            auto node = findNode(root, prefix);
            if (!node)
            {
                return res;
            }

            // 从前缀节点开始遍历所有子节点
            std::string path = prefix;
            traverse(node, path, res);
            return res;
        }

        /**
         * @brief 获取所有匹配模式的键（支持'.'作为通配符，匹配单个任意字符）
         * @param pattern 模式字符串，其中'.'代表匹配任意单个字符
         * @return 匹配的键列表
         */
        std::list<std::string> keysByPattern(const std::string& pattern)
        {
            std::list<std::string> res;
            std::string path;
            traverseByPattern(root, path, pattern, 0, res);
            return res;
        }

        /**
         * @brief 检查是否存在匹配模式的键（支持'.'作为通配符，匹配单个任意字符）
         * @param pattern 模式字符串
         * @return 存在返回true，否则返回false
         */
        bool hasPattern(const std::string& pattern)
        {
            return hasPattern(root, pattern, 0);
        }

        /**
         * @brief 获取当前键值对的数量
         * @return 键值对数量
         */
        size_t size() const
        {
            return count;
        }
    };

} // namespace ol

#endif // !OL_TRIEMAP_H