#include "ol_TrieMap.h"
#include <iostream>
#include <string>

using namespace ol;

int main()
{
    TrieMap<std::string> trie;

    // 插入键值对
    trie.put("apple", "A fruit");
    trie.put("app", "Short for application");
    trie.put("application", "A software program");
    trie.put("banana", "Another fruit");
    trie.put("grape", "A small fruit");

    // 查找键
    std::cout << "Get 'apple': " << trie.get("apple") << std::endl;   // 输出: Get 'apple': A fruit
    std::cout << "Get 'app': " << trie.get("app") << std::endl;       // 输出: Get 'app': Short for application
    std::cout << "Get 'banana': " << trie.get("banana") << std::endl; // 输出: Get 'banana': Another fruit
    std::cout << "Get 'orange': " << trie.get("orange") << std::endl; // 输出: Get 'orange': (empty)

    // 检查是否存在某个键
    std::cout << "Contains 'apple'? " << (trie.has("apple") ? "Yes" : "No") << std::endl;   // 输出: Yes
    std::cout << "Contains 'orange'? " << (trie.has("orange") ? "Yes" : "No") << std::endl; // 输出: No

    // 查找以某个前缀开头的所有键
    std::string prefix = "app";
    std::list<std::string> keysByPrefix = trie.keysByPrefix(prefix);
    std::cout << "Keys with prefix '" << prefix << "': ";
    for (const auto& key : keysByPrefix)
    {
        std::cout << key << " ";
    }
    std::cout << std::endl; // 输出: Keys with prefix 'app': app apple application

    // 查找最短前缀
    std::string query1 = "apparatus";
    std::cout << "Shortest prefix of '" << query1 << "': " << trie.shortestPrefix(query1) << std::endl; // 输出: app

    // 查找最长前缀
    std::cout << "Longest prefix of '" << query1 << "': " << trie.longestPrefix(query1) << std::endl; // 输出: app

    // 删除某个键
    trie.remove("app");
    std::cout << "After removing 'app', contains 'app'? " << (trie.has("app") ? "Yes" : "No") << std::endl; // 输出: No
    std::cout << "Get 'app': " << trie.get("app") << std::endl;                                             // 输出: Get 'app': (empty)

    // 模式匹配
    std::string pattern = "a..le"; // 匹配 "apple"
    std::list<std::string> keysByPattern = trie.keysByPattern(pattern);
    std::cout << "Keys matching pattern '" << pattern << "': ";
    for (const auto& key : keysByPattern)
    {
        std::cout << key << " ";
    }
    std::cout << std::endl; // 输出: Keys matching pattern 'a..le': apple

    return 0;
}