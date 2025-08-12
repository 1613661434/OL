#include "ol_BITree.h"
#include <cassert>
#include <stdexcept>

using namespace ol;

// ���Ը�����
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
    std::cout << "=== �������� ===" << std::endl;
    BITree<double> bitree({8, 6, 1, 4, 5, 5, 1, 1, 3, 2, 1.3, 4, 9, 0, 7, 4});
    bitree.print();

    bitree.add(0, 3); // ��һ��Ԫ��+3
    bitree.print();

    std::cout << "Sum of [0, 3]: " << bitree.rangeSum(0, 3) << std::endl;
    std::cout << "Total sum: " << bitree.sum(bitree.size() - 1) << std::endl;
    TEST(bitree.rangeSum(0, 3) == (8 + 3) + 6 + 1 + 4);

    std::cout << "\n=== �߽���� ===" << std::endl;
    // ������߽�
    TEST(bitree.rangeSum(0, 0) == 11); // ��һ��Ԫ��8+3=11
    // �����ұ߽�
    TEST(bitree.rangeSum(15, 15) == 4); // ���һ��Ԫ��
    // ����Խ�����
    TEST(bitree.rangeSum(0, 100) == bitree.sum(bitree.size() - 1)); // �ұ߽�ض�
    TEST(bitree.rangeSum(100, 200) == 0);                           // ��ȫԽ��
    TEST(bitree.rangeSum(10, 5) == 0);                              // left > right

    // ���Ե���Ԫ������
    TEST(bitree.rangeSum(5, 5) == 5);
    // ���Ը�ֵ����
    bitree.add(5, -2.5);
    TEST(bitree.rangeSum(5, 5) == 2.5);

    std::cout << "\n=== ��������� ===" << std::endl;
    BITree<int> empty({});
    TEST(empty.size() == 0);
    empty.add(0, 10); // Ӧ��Ч��
    TEST(empty.sum(0) == 0);
    TEST(empty.rangeSum(0, 5) == 0);

    std::cout << "\n=== ���ò��� ===" << std::endl;
    bitree.reset({1, 2, 3, 4});
    bitree.print();
    TEST(bitree.sum(3) == 10);
    TEST(bitree.rangeSum(1, 2) == 5);

    // ���Ը�ֵ�����
    std::cout << "\n=== ��ֵ���� ===" << std::endl;
    bitree = {10, 20, 30};
    bitree.print();
    TEST(bitree.sum(2) == 60);

    BITree<int> bitree2({8, 6, 1, 4, 9, 0, 7, 4});
    std::vector<int> vec = {5, 10, 15};
    bitree2 = vec;
    bitree2.print();
    TEST(bitree2.rangeSum(0, 1) == 15);

    std::cout << "\n=== ��Ԫ��������� ===" << std::endl;
    BITree<int> single({42});
    TEST(single.sum(0) == 42);
    single.add(0, 8);
    TEST(single.sum(0) == 50);
    TEST(single.rangeSum(0, 0) == 50);
    TEST(single.rangeSum(0, 5) == 50); // �ұ߽�ض�
    TEST(single.rangeSum(1, 5) == 0);  // ��߽�Խ��

    std::cout << "\n=== ����������� ===" << std::endl;
    BITree<int> intTree({3, 1, 4, 1, 5, 9, 2, 6});
    TEST(intTree.sum(7) == 31);
    TEST(intTree.rangeSum(2, 5) == 4 + 1 + 5 + 9);
    intTree.add(3, 10); // ���ĸ�Ԫ�ش�1��Ϊ11
    TEST(intTree.rangeSum(3, 3) == 11);
    TEST(intTree.sum(7) == 41);

    std::cout << "\n=== ���в���ͨ��! ===" << std::endl;
    return 0;
}