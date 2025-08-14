/****************************************************************************************/
/*
 * 程序名：test_ol_UnionFind.cpp
 * 功能描述：ol_UnionFind.h的测试程序，验证并查集的基本功能：
 *          - 整数类型集合的合并与查询
 *          - 非整数类型（字符串）集合的合并与查询
 *          - 路径压缩和按秩合并的有效性
 *          - 集合数量统计和连接性判断
 * 作者：ol
 * 适用标准：C++11及以上
 */
/****************************************************************************************/
#include "ol_UnionFind.h"
#include <iostream>
#include <string>

using namespace ol;

/**
 * @brief 测试整数类型并查集
 */
void testIntegerUnionFind()
{
    std::cout << "=== 整数类型并查集测试 ===" << std::endl;

    // 初始化包含5个元素的并查集（0-4）
    UnionFind<int> ufInt(5);
    std::cout << "初始集合数量: " << ufInt.countSets() << " (预期: 5)" << std::endl;

    // 执行合并操作
    ufInt.unite(0, 1);
    ufInt.unite(2, 3);
    ufInt.unite(3, 4);
    std::cout << "合并(0-1, 2-3-4)后集合数量: " << ufInt.countSets() << " (预期: 2)" << std::endl;

    // 测试连接性
    std::cout << "0和1是否连通: " << (ufInt.connected(0, 1) ? "是" : "否") << " (预期: 是)" << std::endl;
    std::cout << "2和4是否连通: " << (ufInt.connected(2, 4) ? "是" : "否") << " (预期: 是)" << std::endl;
    std::cout << "0和2是否连通: " << (ufInt.connected(0, 2) ? "是" : "否") << " (预期: 否)" << std::endl;

    // 进一步合并
    ufInt.unite(1, 4);
    std::cout << "合并(1-4)后集合数量: " << ufInt.countSets() << " (预期: 1)" << std::endl;
    std::cout << "0和4是否连通: " << (ufInt.connected(0, 4) ? "是" : "否") << " (预期: 是)" << std::endl;
    std::cout << std::endl;
}

/**
 * @brief 测试字符串类型并查集
 */
void testStringUnionFind()
{
    std::cout << "=== 字符串类型并查集测试 ===" << std::endl;

    // 初始化空的字符串类型并查集
    UnionFind<std::string> ufStr;
    std::cout << "初始集合数量: " << ufStr.countSets() << " (预期: 0)" << std::endl;

    // 动态添加并合并元素
    ufStr.unite("apple", "banana");
    ufStr.unite("cherry", "date");
    std::cout << "合并(apple-banana, cherry-date)后集合数量: " << ufStr.countSets() << " (预期: 2)" << std::endl;

    // 测试连接性
    std::cout << "apple和banana是否连通: " << (ufStr.connected("apple", "banana") ? "是" : "否") << " (预期: 是)" << std::endl;
    std::cout << "cherry和date是否连通: " << (ufStr.connected("cherry", "date") ? "是" : "否") << " (预期: 是)" << std::endl;

    // 进一步合并
    ufStr.unite("banana", "date");
    std::cout << "合并(banana-date)后集合数量: " << ufStr.countSets() << " (预期: 1)" << std::endl;
    std::cout << "apple和cherry是否连通: " << (ufStr.connected("apple", "cherry") ? "是" : "否") << " (预期: 是)" << std::endl;
}

int main()
{
    testIntegerUnionFind();
    testStringUnionFind();
    return 0;
}