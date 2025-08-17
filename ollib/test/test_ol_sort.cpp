#include "ol_sort.h"
#include <functional> // 用于 std::greater 等标准比较器
#include <iostream>
#include <list>
#include <string>
#include <vector>

using namespace ol;
using namespace std;

// 自定义比较器1：按绝对值升序（满足严格弱序）
template <typename T>
struct AbsLess
{
    bool operator()(const T& a, const T& b) const
    {
        return abs(a) < abs(b);
    }
};

// 自定义比较器2：字符串按长度降序（满足严格弱序）
struct StrLenGreater
{
    bool operator()(const string& a, const string& b) const
    {
        return a.size() > b.size();
    }
};

// 辅助函数：打印分隔线
void print_separator(const string& title)
{
    cout << "\n============================================================" << '\n';
    cout << title << '\n';
    cout << "============================================================" << '\n';
}

// 针对pair类型的打印特化（如果头文件中没有提供）
template <typename T1, typename T2>
void print_container(const vector<pair<T1, T2>>& c)
{
    for (const auto& elem : c)
    {
        cout << "(" << elem.first << "," << elem.second << ") ";
    }
    cout << endl;
}

int main()
{
    // 1. 基础测试：原生数组 + 多种排序算法
    print_separator("1. 原生数组排序测试");
    int arr[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    cout << "原始数组: ";
    ol::print_container(arr); // 明确指定使用ol命名空间中的函数

    // 测试快速排序
    int arr_quick[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    quick_sort(arr_quick);
    cout << "快速排序: ";
    ol::print_container(arr_quick);

    // 测试归并排序
    int arr_merge[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    merge_sort(arr_merge);
    cout << "归并排序: ";
    ol::print_container(arr_merge);

    // 测试堆排序
    int arr_heap[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    heap_sort(arr_heap);
    cout << "堆排序:   ";
    ol::print_container(arr_heap);

    // 2. STL容器测试：vector + 基础类型
    print_separator("2. STL容器排序测试");
    vector<double> vec_double = {3.14, 2.71, 1.618, 0.577, 4.669, 2.718};
    cout << "原始vector<double>: ";
    ol::print_container(vec_double);

    shell_sort(vec_double); // 测试希尔排序
    cout << "希尔排序后:        ";
    ol::print_container(vec_double);

    // 3. 双向迭代器容器测试：list
    print_separator("3. 双向迭代器容器（list）测试");
    list<int> lst = {15, 12, 18, 11, 19, 13, 17, 14, 16};
    cout << "原始list<int>: ";
    ol::print_container(lst);

    insertion_sort(lst); // list为双向迭代器，适配插入排序
    cout << "插入排序后:    ";
    ol::print_container(lst);

    // 测试冒泡排序
    list<int> lst2 = {15, 12, 18, 11, 19, 13, 17, 14, 16};
    bubble_sort(lst2);
    cout << "冒泡排序后:    ";
    ol::print_container(lst2);

    // 4. 自定义排序规则测试
    print_separator("4. 自定义排序规则测试");

    // 4.1 降序排序（使用std::greater）
    vector<int> vec_desc_int = {5, 2, 8, 1, 9};
    cout << "原始vector (降序测试): ";
    ol::print_container(vec_desc_int);
    quick_sort(vec_desc_int, greater<int>()); // 快速排序-降序
    cout << "快速排序(降序):        ";
    ol::print_container(vec_desc_int);

    // 4.2 按绝对值升序（自定义比较器AbsLess）
    vector<int> vec_abs = {-3, 1, -5, 2, -4};
    cout << "\n原始vector (绝对值测试): ";
    ol::print_container(vec_abs);
    binary_insertion_sort(vec_abs, AbsLess<int>()); // 折半插入排序-按绝对值升序
    cout << "折半插入排序(绝对值升序): ";
    ol::print_container(vec_abs);

    // 4.3 字符串按长度降序（自定义比较器StrLenGreater）
    vector<string> vec_str = {"apple", "banana", "cherry", "date", "elderberry"};
    cout << "\n原始vector<string>: ";
    ol::print_container(vec_str);
    quick_sort(vec_str, StrLenGreater()); // 快速排序-按字符串长度降序
    cout << "快速排序(按长度降序): ";
    ol::print_container(vec_str);

    // 5. 计数排序测试（仅适用于整数）
    print_separator("5. 计数排序测试（整数专用）");
    vector<int> vec_count = {4, 2, 2, 8, 3, 3, 1};
    cout << "原始vector: ";
    ol::print_container(vec_count);
    counting_sort(vec_count);
    cout << "计数排序后: ";
    ol::print_container(vec_count);

    // 6. 部分范围排序测试
    print_separator("6. 部分范围排序测试");
    vector<int> vec_part = {10, 5, 8, 3, 1, 9, 2, 7, 4, 6};
    cout << "原始vector:          ";
    ol::print_container(vec_part);

    // 排序区间：第3个元素（索引2）到倒数第2个元素（索引8）
    quick_sort(vec_part.begin() + 2, vec_part.end() - 2);
    cout << "部分排序后 [2, end-2): ";
    ol::print_container(vec_part);

    // 7. 选择排序测试
    print_separator("7. 选择排序测试");
    vector<int> vec_select = {9, 7, 8, 3, 2, 1, 5, 4, 6};
    cout << "原始vector: ";
    ol::print_container(vec_select);
    selection_sort(vec_select);
    cout << "选择排序后: ";
    ol::print_container(vec_select);

    // 8. 基数排序测试
    print_separator("8. 基数排序测试");
    vector<int> vec_radix = {170, 45, 75, 90, 802, 24, 2, 66, -12, -56};
    cout << "原始vector: ";
    ol::print_container(vec_radix);
    radix_sort(vec_radix);
    cout << "基数排序后: ";
    ol::print_container(vec_radix);

    // 9. Lambda函数自定义排序测试
    print_separator("9. Lambda函数自定义排序测试");
    vector<std::pair<int, int>> vec_desc_pair = {{5, 3}, {8, 5}, {8, 20}, {2, 2}, {8, 4}, {1, 5}, {9, 6}, {5, 98}};
    cout << "原始vector: ";
    print_container(vec_desc_pair); // 使用我们自定义的pair特化版本
    quick_sort(vec_desc_pair, [](const pair<int, int>& a, const pair<int, int>& b)
               { return a.first != b.first ? a.first > b.first : a.second < b.second; });
    cout << "快速排序(规则按key降序，key相同则按value升序): ";
    print_container(vec_desc_pair);

    print_separator("所有排序测试完成");
    return 0;
}