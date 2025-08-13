#include "ol_TrieMap.h"
#include <iostream>
#include <string>

using namespace ol;

int main()
{
    TrieMap<std::string> trie;

    // 1. �����ֵ�ԣ�����Ĭ��ֵ������
    trie.put("apple", "A fruit");
    trie.put("app", "Short for application");
    trie.put("application", "A software program");
    trie.put("banana", "Another fruit");
    trie.put("grape", ""); // ����Ĭ��ֵ�����ַ�����
    std::cout << "=== ������� ===" << std::endl;

    // 2. ���� get ������std::optional ����
    auto getApple = trie.get("apple");
    std::cout << "Get 'apple': " << (getApple.has_value() ? getApple.value() : "null") << std::endl; // A fruit

    auto getGrape = trie.get("grape");
    std::cout << "Get 'grape' (empty value): " << (getGrape.has_value() ? getGrape.value() : "null") << std::endl; // ���ַ���

    auto getOrange = trie.get("orange");
    std::cout << "Get 'orange' (not exists): " << (getOrange.has_value() ? getOrange.value() : "null") << std::endl; // null
    std::cout << std::endl;

    // 3. ���� has ����
    std::cout << "Has 'apple'? " << (trie.has("apple") ? "Yes" : "No") << std::endl;   // Yes
    std::cout << "Has 'grape'? " << (trie.has("grape") ? "Yes" : "No") << std::endl;   // Yes����ʹֵΪ�գ�
    std::cout << "Has 'orange'? " << (trie.has("orange") ? "Yes" : "No") << std::endl; // No
    std::cout << std::endl;

    // 4. ǰ׺ƥ�����
    std::string prefix = "app";
    auto appKeys = trie.keysByPrefix(prefix);
    std::cout << "Keys with prefix '" << prefix << "': ";
    for (const auto& key : appKeys) std::cout << key << " "; // app apple application
    std::cout << std::endl
              << std::endl;

    // 5. ���/�ǰ׺����
    std::string query = "apparatus";
    std::cout << "Shortest prefix of '" << query << "': " << trie.shortestPrefix(query) << std::endl; // app
    std::cout << "Longest prefix of '" << query << "': " << trie.longestPrefix(query) << std::endl;   // app
    std::cout << std::endl;

    // 6. ɾ������
    trie.remove("app");
    std::cout << "After remove 'app': " << std::endl;
    std::cout << "Has 'app'? " << (trie.has("app") ? "Yes" : "No") << std::endl; // No
    auto appKeysAfterRemove = trie.keysByPrefix("app");
    std::cout << "Keys with prefix 'app' now: ";
    for (const auto& key : appKeysAfterRemove) std::cout << key << " "; // apple application
    std::cout << std::endl
              << std::endl;

    // 7. ģʽƥ����ԣ�. ƥ�䵥���ַ���
    std::string pattern1 = "a..le"; // ƥ�� apple
    auto match1 = trie.keysByPattern(pattern1);
    std::cout << "Keys matching '" << pattern1 << "': ";
    for (const auto& key : match1) std::cout << key << " "; // apple
    std::cout << std::endl;

    std::string pattern2 = "b.n.n."; // ƥ�� banana
    auto match2 = trie.keysByPattern(pattern2);
    std::cout << "Keys matching '" << pattern2 << "': ";
    for (const auto& key : match2) std::cout << key << " "; // banana
    std::cout << std::endl;

    std::string pattern3 = "......."; // 7���ַ������Ա߽磩
    auto match3 = trie.keysByPattern(pattern3);
    std::cout << "Keys matching '" << pattern3 << "': ";
    for (const auto& key : match3) std::cout << key << " "; // application������7��
    std::cout << std::endl;

    return 0;
}