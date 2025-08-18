#include "ol_sort.h"
#include <cmath>
#include <functional>
#include <iostream>
#include <list>
#include <string>
#include <vector>

using namespace ol;
using namespace std;

// 自定义比较器1：按绝对值升序
template <typename T>
struct AbsLess
{
    bool operator()(const T& a, const T& b) const
    {
        return abs(a) < abs(b);
    }
};

// 自定义比较器2：字符串按长度降序
struct StrLenGreater
{
    bool operator()(const string& a, const string& b) const
    {
        return a.size() > b.size();
    }
};

// 自定义比较器3：浮点数按小数部分降序
struct FloatFractionGreater
{
    bool operator()(double a, double b) const
    {
        // 提取小数部分
        double frac_a = a - floor(a);
        double frac_b = b - floor(b);
        return frac_a > frac_b;
    }
};

// 辅助函数：打印分隔线
void print_separator(const string& title)
{
    cout << "\n============================================================" << '\n';
    cout << title << '\n';
    cout << "============================================================" << '\n';
}

// 全局打印函数（避免命名空间冲突）
namespace test_utils
{
    // 通用打印容器
    template <typename Container>
    void print_container(const Container& c)
    {
        for (const auto& elem : c)
        {
            cout << elem << " ";
        }
        cout << endl;
    }

    // pair类型特化打印
    template <typename T1, typename T2>
    void print_container(const vector<pair<T1, T2>>& c)
    {
        for (const auto& elem : c)
        {
            cout << "(" << elem.first << "," << elem.second << ") ";
        }
        cout << endl;
    }

    // 原生数组打印
    template <typename T, size_t N>
    void print_container(const T (&arr)[N])
    {
        for (size_t i = 0; i < N; ++i)
        {
            cout << arr[i] << " ";
        }
        cout << endl;
    }
} // namespace test_utils

int main()
{
    // 1. 基础排序算法测试（原生数组）
    print_separator("1. 基础排序算法测试（原生数组）");
    int arr[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    cout << "原始数组: ";
    test_utils::print_container(arr);

    // 快速排序
    int arr_quick[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    quick_sort(arr_quick);
    cout << "快速排序: ";
    test_utils::print_container(arr_quick);

    // 归并排序
    int arr_merge[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    merge_sort(arr_merge);
    cout << "归并排序: ";
    test_utils::print_container(arr_merge);

    // 堆排序
    int arr_heap[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    heap_sort(arr_heap);
    cout << "堆排序:   ";
    test_utils::print_container(arr_heap);

    // 希尔排序
    int arr_shell[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    shell_sort(arr_shell);
    cout << "希尔排序: ";
    test_utils::print_container(arr_shell);

    // 选择排序
    int arr_select[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    selection_sort(arr_select);
    cout << "选择排序: ";
    test_utils::print_container(arr_select);

    // 2. 浮点型排序测试（希尔排序）
    print_separator("2. 浮点型排序测试（希尔排序）");
    vector<double> vec_double = {3.14, 2.71, 1.618, 0.577, 4.669, 2.718};
    cout << "原始vector<double>: ";
    test_utils::print_container(vec_double);

    shell_sort(vec_double);
    cout << "希尔排序后:        ";
    test_utils::print_container(vec_double);

    // 3. 双向迭代器容器测试（插入排序、冒泡排序）
    print_separator("3. 双向迭代器容器测试（插入排序、冒泡排序）");
    list<int> lst = {15, 12, 18, 11, 19, 13, 17, 14, 16};
    cout << "原始list<int>: ";
    test_utils::print_container(lst);

    insertion_sort(lst);
    cout << "插入排序后:    ";
    test_utils::print_container(lst);

    list<int> lst2 = {15, 12, 18, 11, 19, 13, 17, 14, 16};
    bubble_sort(lst2);
    cout << "冒泡排序后:    ";
    test_utils::print_container(lst2);

    // 4. 自定义排序规则测试（快速排序）
    print_separator("4. 自定义排序规则测试（快速排序）");

    // 4.1 降序排序
    vector<int> vec_desc_int = {5, 2, 8, 1, 9};
    cout << "原始vector (降序测试): ";
    test_utils::print_container(vec_desc_int);
    quick_sort(vec_desc_int, greater<int>());
    cout << "快速排序(降序):        ";
    test_utils::print_container(vec_desc_int);

    // 4.2 绝对值升序
    vector<int> vec_abs = {-3, 1, -5, 2, -4};
    cout << "\n原始vector (绝对值测试): ";
    test_utils::print_container(vec_abs);
    binary_insertion_sort(vec_abs, AbsLess<int>());
    cout << "折半插入排序(绝对值升序): ";
    test_utils::print_container(vec_abs);

    // 4.3 字符串长度降序
    vector<string> vec_str = {"apple", "banana", "cherry", "date", "elderberry"};
    cout << "\n原始vector<string>: ";
    test_utils::print_container(vec_str);
    quick_sort(vec_str, StrLenGreater());
    cout << "快速排序(按长度降序): ";
    test_utils::print_container(vec_str);

    // 5. 计数排序测试（整数专用）
    print_separator("5. 计数排序测试（整数专用）");
    vector<int> vec_count = {4, 2, 2, 8, 3, 3, 1};
    cout << "原始vector: ";
    test_utils::print_container(vec_count);
    counting_sort(vec_count);
    cout << "计数排序后: ";
    test_utils::print_container(vec_count);

    // 6. 基数排序测试（整数，增加升序/降序测试）
    print_separator("6. 基数排序测试（整数）");
    vector<int> vec_radix_asc = {170, 45, 75, 90, 802, 24, 2, 66, -12, -56};
    cout << "原始vector: ";
    test_utils::print_container(vec_radix_asc);

    radix_sort(vec_radix_asc); // 默认升序（std::less）
    cout << "基数排序(升序): ";
    test_utils::print_container(vec_radix_asc);

    vector<int> vec_radix_desc = {170, 45, 75, 90, 802, 24, 2, 66, -12, -56};
    radix_sort(vec_radix_desc, 10, greater<int>()); // 降序测试
    cout << "基数排序(降序): ";
    test_utils::print_container(vec_radix_desc);

    // 7. 桶排序测试（增加自定义比较器测试）
    print_separator("7. 桶排序测试（整数、浮点数及自定义比较器）");

    // 7.1 整数桶排序（降序）
    vector<int> vec_bucket_int = {42, 32, 33, 52, 37, 47, 51};
    cout << "原始整数vector: ";
    test_utils::print_container(vec_bucket_int);
    bucket_sort(vec_bucket_int, 5, greater<int>()); // 整数降序
    cout << "整数桶排序(降序): ";
    test_utils::print_container(vec_bucket_int);

    // 7.2 浮点数桶排序（默认升序）
    vector<double> vec_bucket_float = {0.42, 0.32, 0.33, 0.52, 0.37, 0.47, 0.51};
    cout << "\n原始浮点数vector: ";
    test_utils::print_container(vec_bucket_float);
    bucket_sort(vec_bucket_float, 5, 0.0, 1.0);
    cout << "浮点数桶排序(升序): ";
    test_utils::print_container(vec_bucket_float);

    // 7.3 浮点数桶排序（按小数部分降序，自定义比较器）
    vector<double> vec_bucket_float_custom = {3.14, 2.71, 1.618, 0.577, 4.669, 2.718};
    cout << "\n原始浮点数vector: ";
    test_utils::print_container(vec_bucket_float_custom);
    bucket_sort(vec_bucket_float_custom, 5, 0.0, 5.0, FloatFractionGreater());
    cout << "浮点数桶排序(按小数部分降序): ";
    test_utils::print_container(vec_bucket_float_custom);

    // 7.4 整数桶排序（按绝对值升序，自定义比较器）
    vector<int> vec_bucket_abs = {-5, 3, -2, 4, -1, 6, -3};
    cout << "\n原始整数vector: ";
    test_utils::print_container(vec_bucket_abs);
    bucket_sort(vec_bucket_abs, 5, AbsLess<int>()); // 使用绝对值比较器
    cout << "整数桶排序(按绝对值升序): ";
    test_utils::print_container(vec_bucket_abs);

    // 8. 部分范围排序测试
    print_separator("8. 部分范围排序测试（快速排序）");
    vector<int> vec_part = {10, 5, 8, 3, 1, 9, 2, 7, 4, 6};
    cout << "原始vector:          ";
    test_utils::print_container(vec_part);

    quick_sort(vec_part.begin() + 2, vec_part.end() - 2);
    cout << "部分排序后 [2, end-2): ";
    test_utils::print_container(vec_part);

    // 9. Lambda函数自定义排序测试
    print_separator("9. Lambda函数自定义排序测试（快速排序）");
    vector<pair<int, int>> vec_desc_pair = {{5, 3}, {8, 5}, {8, 20}, {2, 2}, {8, 4}, {1, 5}, {9, 6}, {5, 98}};
    cout << "原始vector: ";
    test_utils::print_container(vec_desc_pair);
    quick_sort(vec_desc_pair, [](const pair<int, int>& a, const pair<int, int>& b)
               { return a.first != b.first ? a.first > b.first : a.second < b.second; });
    cout << "快速排序(按key降序，key相同按value升序): ";
    test_utils::print_container(vec_desc_pair);

    print_separator("所有排序测试完成");
    return 0;
}