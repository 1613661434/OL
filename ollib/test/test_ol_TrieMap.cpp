#include "ol_TrieMap.h"
#include <iostream>
#include <string>

using namespace ol;

int main()
{
    TrieMap<std::string> trie;

    // �����ֵ��
    trie.put("apple", "A fruit");
    trie.put("app", "Short for application");
    trie.put("application", "A software program");
    trie.put("banana", "Another fruit");
    trie.put("grape", "A small fruit");

    // ���Ҽ�
    std::cout << "Get 'apple': " << trie.get("apple") << std::endl;   // ���: Get 'apple': A fruit
    std::cout << "Get 'app': " << trie.get("app") << std::endl;       // ���: Get 'app': Short for application
    std::cout << "Get 'banana': " << trie.get("banana") << std::endl; // ���: Get 'banana': Another fruit
    std::cout << "Get 'orange': " << trie.get("orange") << std::endl; // ���: Get 'orange': (empty)

    // ����Ƿ����ĳ����
    std::cout << "Contains 'apple'? " << (trie.has("apple") ? "Yes" : "No") << std::endl;   // ���: Yes
    std::cout << "Contains 'orange'? " << (trie.has("orange") ? "Yes" : "No") << std::endl; // ���: No

    // ������ĳ��ǰ׺��ͷ�����м�
    std::string prefix = "app";
    std::list<std::string> keysByPrefix = trie.keysByPrefix(prefix);
    std::cout << "Keys with prefix '" << prefix << "': ";
    for (const auto& key : keysByPrefix)
    {
        std::cout << key << " ";
    }
    std::cout << std::endl; // ���: Keys with prefix 'app': app apple application

    // �������ǰ׺
    std::string query1 = "apparatus";
    std::cout << "Shortest prefix of '" << query1 << "': " << trie.shortestPrefix(query1) << std::endl; // ���: app

    // �����ǰ׺
    std::cout << "Longest prefix of '" << query1 << "': " << trie.longestPrefix(query1) << std::endl; // ���: app

    // ɾ��ĳ����
    trie.remove("app");
    std::cout << "After removing 'app', contains 'app'? " << (trie.has("app") ? "Yes" : "No") << std::endl; // ���: No
    std::cout << "Get 'app': " << trie.get("app") << std::endl;                                             // ���: Get 'app': (empty)

    // ģʽƥ��
    std::string pattern = "a..le"; // ƥ�� "apple"
    std::list<std::string> keysByPattern = trie.keysByPattern(pattern);
    std::cout << "Keys matching pattern '" << pattern << "': ";
    for (const auto& key : keysByPattern)
    {
        std::cout << key << " ";
    }
    std::cout << std::endl; // ���: Keys matching pattern 'a..le': apple

    return 0;
}