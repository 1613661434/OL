#include "ol_TrieMap.h"
#include <iostream>
#include <string>

using namespace ol;

int main()
{
    TrieMap<std::string> trie;

    // 1. 插入键值对（包含默认值场景）
    trie.put("apple", "A fruit");
    trie.put("app", "Short for application");
    trie.put("application", "A software program");
    trie.put("banana", "Another fruit");
    trie.put("grape", ""); // 插入默认值（空字符串）
    std::cout << "=== 插入完成 ===" << std::endl;

    // 2. 测试 get 方法（std::optional 处理）
    auto getApple = trie.get("apple");
    std::cout << "Get 'apple': " << (getApple.has_value() ? getApple.value() : "null") << std::endl; // A fruit

    auto getGrape = trie.get("grape");
    std::cout << "Get 'grape' (empty value): " << (getGrape.has_value() ? getGrape.value() : "null") << std::endl; // 空字符串

    auto getOrange = trie.get("orange");
    std::cout << "Get 'orange' (not exists): " << (getOrange.has_value() ? getOrange.value() : "null") << std::endl; // null
    std::cout << std::endl;

    // 3. 测试 has 方法
    std::cout << "Has 'apple'? " << (trie.has("apple") ? "Yes" : "No") << std::endl;   // Yes
    std::cout << "Has 'grape'? " << (trie.has("grape") ? "Yes" : "No") << std::endl;   // Yes（即使值为空）
    std::cout << "Has 'orange'? " << (trie.has("orange") ? "Yes" : "No") << std::endl; // No
    std::cout << std::endl;

    // 4. 前缀匹配测试
    std::string prefix = "app";
    auto appKeys = trie.keysByPrefix(prefix);
    std::cout << "Keys with prefix '" << prefix << "': ";
    for (const auto& key : appKeys) std::cout << key << " "; // app apple application
    std::cout << std::endl
              << std::endl;

    // 5. 最短/最长前缀测试
    std::string query = "apparatus";
    std::cout << "Shortest prefix of '" << query << "': " << trie.shortestPrefix(query) << std::endl; // app
    std::cout << "Longest prefix of '" << query << "': " << trie.longestPrefix(query) << std::endl;   // app
    std::cout << std::endl;

    // 6. 删除测试
    trie.remove("app");
    std::cout << "After remove 'app': " << std::endl;
    std::cout << "Has 'app'? " << (trie.has("app") ? "Yes" : "No") << std::endl; // No
    auto appKeysAfterRemove = trie.keysByPrefix("app");
    std::cout << "Keys with prefix 'app' now: ";
    for (const auto& key : appKeysAfterRemove) std::cout << key << " "; // apple application
    std::cout << std::endl
              << std::endl;

    // 7. 模式匹配测试（. 匹配单个字符）
    std::string pattern1 = "a..le"; // 匹配 apple
    auto match1 = trie.keysByPattern(pattern1);
    std::cout << "Keys matching '" << pattern1 << "': ";
    for (const auto& key : match1) std::cout << key << " "; // apple
    std::cout << std::endl;

    std::string pattern2 = "b.n.n."; // 匹配 banana
    auto match2 = trie.keysByPattern(pattern2);
    std::cout << "Keys matching '" << pattern2 << "': ";
    for (const auto& key : match2) std::cout << key << " "; // banana
    std::cout << std::endl;

    std::string pattern3 = "......."; // 7个字符（测试边界）
    auto match3 = trie.keysByPattern(pattern3);
    std::cout << "Keys matching '" << pattern3 << "': ";
    for (const auto& key : match3) std::cout << key << " "; // application（长度7）
    std::cout << std::endl;

    return 0;
}