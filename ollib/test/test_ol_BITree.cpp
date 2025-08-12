#include "ol_BITree.h"
#include <cassert>
#include <stdexcept>

using namespace ol;

// 测试辅助宏
#define TEST(condition)                                                                      \
    do                                                                                       \
    {                                                                                        \
        if (!(condition))                                                                    \
        {                                                                                    \
            std::cerr << "Test failed at line " << __LINE__ << ": " #condition << std::endl; \
            assert(false);                                                                   \
        }                                                                                    \
        else                                                                                 \
        {                                                                                    \
            std::cout << "Test passed: " #condition << std::endl;                            \
        }                                                                                    \
    } while (0)

int main()
{
    std::cout << "=== 基本测试 ===" << std::endl;
    BITree<double> bitree({8, 6, 1, 4, 5, 5, 1, 1, 3, 2, 1.3, 4, 9, 0, 7, 4});
    bitree.print();

    bitree.add(0, 3); // 第一个元素+3
    bitree.print();

    std::cout << "Sum of [0, 3]: " << bitree.rangeSum(0, 3) << std::endl;
    std::cout << "Total sum: " << bitree.sum(bitree.size() - 1) << std::endl;
    TEST(bitree.rangeSum(0, 3) == (8 + 3) + 6 + 1 + 4);

    std::cout << "\n=== 边界测试 ===" << std::endl;
    // 测试左边界
    TEST(bitree.rangeSum(0, 0) == 11); // 第一个元素8+3=11
    // 测试右边界
    TEST(bitree.rangeSum(15, 15) == 4); // 最后一个元素
    // 测试越界访问
    TEST(bitree.rangeSum(0, 100) == bitree.sum(bitree.size() - 1)); // 右边界截断
    TEST(bitree.rangeSum(100, 200) == 0);                           // 完全越界
    TEST(bitree.rangeSum(10, 5) == 0);                              // left > right

    // 测试单个元素区间
    TEST(bitree.rangeSum(5, 5) == 5);
    // 测试负值操作
    bitree.add(5, -2.5);
    TEST(bitree.rangeSum(5, 5) == 2.5);

    std::cout << "\n=== 空数组测试 ===" << std::endl;
    BITree<int> empty({});
    TEST(empty.size() == 0);
    empty.add(0, 10); // 应无效果
    TEST(empty.sum(0) == 0);
    TEST(empty.rangeSum(0, 5) == 0);

    std::cout << "\n=== 重置测试 ===" << std::endl;
    bitree.reset({1, 2, 3, 4});
    bitree.print();
    TEST(bitree.sum(3) == 10);
    TEST(bitree.rangeSum(1, 2) == 5);

    // 测试赋值运算符
    std::cout << "\n=== 赋值测试 ===" << std::endl;
    bitree = {10, 20, 30};
    bitree.print();
    TEST(bitree.sum(2) == 60);

    BITree<int> bitree2({8, 6, 1, 4, 9, 0, 7, 4});
    std::vector<int> vec = {5, 10, 15};
    bitree2 = vec;
    bitree2.print();
    TEST(bitree2.rangeSum(0, 1) == 15);

    std::cout << "\n=== 单元素数组测试 ===" << std::endl;
    BITree<int> single({42});
    TEST(single.sum(0) == 42);
    single.add(0, 8);
    TEST(single.sum(0) == 50);
    TEST(single.rangeSum(0, 0) == 50);
    TEST(single.rangeSum(0, 5) == 50); // 右边界截断
    TEST(single.rangeSum(1, 5) == 0);  // 左边界越界

    std::cout << "\n=== 整数数组测试 ===" << std::endl;
    BITree<int> intTree({3, 1, 4, 1, 5, 9, 2, 6});
    TEST(intTree.sum(7) == 31);
    TEST(intTree.rangeSum(2, 5) == 4 + 1 + 5 + 9);
    intTree.add(3, 10); // 第四个元素从1变为11
    TEST(intTree.rangeSum(3, 3) == 11);
    TEST(intTree.sum(7) == 41);

    std::cout << "\n=== 所有测试通过! ===" << std::endl;
    return 0;
}