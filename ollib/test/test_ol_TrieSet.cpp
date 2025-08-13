#include "ol_TrieSet.h"
#include <iostream>
#include <string>

using namespace ol;

int main()
{
    TrieSet trieSet;

    // 1. 插入元素
    trieSet.put("apple");
    trieSet.put("app");
    trieSet.put("application");
    trieSet.put("banana");
    trieSet.put("grape");
    trieSet.put("grape"); // 重复插入（幂等性测试）
    std::cout << "=== 插入完成 ===" << std::endl;
    std::cout << "Size after insert: " << trieSet.size() << std::endl; // 5
    std::cout << std::endl;

    // 2. 测试 has 方法
    std::cout << "Has 'apple'? " << (trieSet.has("apple") ? "Yes" : "No") << std::endl;   // Yes
    std::cout << "Has 'app'? " << (trieSet.has("app") ? "Yes" : "No") << std::endl;       // Yes
    std::cout << "Has 'orange'? " << (trieSet.has("orange") ? "Yes" : "No") << std::endl; // No
    std::cout << std::endl;

    // 3. 前缀匹配测试
    std::string prefix = "app";
    auto appKeys = trieSet.keysByPrefix(prefix);
    std::cout << "Keys with prefix '" << prefix << "': ";
    for (const auto& key : appKeys) std::cout << key << " "; // app apple application
    std::cout << std::endl
              << std::endl;

    // 4. 最短/最长前缀测试
    std::string query = "appetite";
    std::cout << "Shortest prefix of '" << query << "': " << trieSet.shortestPrefix(query) << std::endl; // app
    std::cout << "Longest prefix of '" << query << "': " << trieSet.longestPrefix(query) << std::endl;   // app
    std::cout << std::endl;

    // 5. 删除测试
    trieSet.remove("app");
    std::cout << "After remove 'app': " << std::endl;
    std::cout << "Has 'app'? " << (trieSet.has("app") ? "Yes" : "No") << std::endl; // No
    std::cout << "Size after remove: " << trieSet.size() << std::endl;              // 4
    auto appKeysAfterRemove = trieSet.keysByPrefix("app");
    std::cout << "Keys with prefix 'app' now: ";
    for (const auto& key : appKeysAfterRemove) std::cout << key << " "; // apple application
    std::cout << std::endl
              << std::endl;

    // 6. 模式匹配测试
    std::string pattern1 = "a..le"; // 匹配 apple
    auto match1 = trieSet.keysByPattern(pattern1);
    std::cout << "Keys matching '" << pattern1 << "': ";
    for (const auto& key : match1) std::cout << key << " "; // apple
    std::cout << std::endl;

    std::string pattern2 = "gr.pe"; // 匹配 grape
    auto match2 = trieSet.keysByPattern(pattern2);
    std::cout << "Keys matching '" << pattern2 << "': ";
    for (const auto& key : match2) std::cout << key << " "; // grape
    std::cout << std::endl;

    return 0;
}