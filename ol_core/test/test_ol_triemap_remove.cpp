#include "ol_TrieMap.h"

#include <iostream>
#include <string>

namespace
{
    bool require(bool condition, const char* message)
    {
        if (condition) return true;
        std::cerr << message << '\n';
        return false;
    }
}

int main()
{
    ol::TrieMap<int> trie;

    trie.put("app", 1);
    trie.put("apple", 2);
    trie.remove("apple");

    if (!require(trie.has("app"), "removing a child key removed its valid parent")) return 1;
    if (!require(!trie.hasPrefix("apple"), "removed leaf path was not pruned")) return 1;

    trie.remove("app");
    if (!require(trie.size() == 0, "trie size was not decremented to zero")) return 1;
    if (!require(!trie.hasPrefix("a"), "last key left a dangling prefix")) return 1;
    if (!require(!trie.hasPrefix(""), "empty trie still reports an empty prefix")) return 1;

    trie.put("again", 3);
    if (!require(trie.has("again"), "trie could not be reused after pruning its root")) return 1;

    std::cout << "TrieMap remove regression test passed\n";
    return 0;
}
