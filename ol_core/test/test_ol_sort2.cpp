#include "ol_chrono.h"
#include "ol_sort.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <list>
#include <random>
#include <string>
#include <vector>

using namespace ol;
using namespace std;

// 自定义比较器
template <typename T>
struct AbsLess
{
    bool operator()(const T& a, const T& b) const
    {
        return abs(a) < abs(b);
    }
};

struct StrLenGreater
{
    bool operator()(const string& a, const string& b) const
    {
        return a.size() > b.size();
    }
};

struct FloatFractionGreater
{
    bool operator()(double a, double b) const
    {
        double frac_a = a - floor(a);
        double frac_b = b - floor(b);
        return frac_a > frac_b;
    }
};

// 生成随机数据
vector<int> generate_random_ints(size_t size, int min_val, int max_val)
{
    vector<int> result(size);

    // 使用随机数引擎和均匀分布生成更均匀的随机数
    random_device rd;                                     // 用于获取种子
    mt19937 gen(rd());                                    // 梅森旋转算法随机数生成器
    uniform_int_distribution<int> dist(min_val, max_val); // 均匀整数分布

    for (size_t i = 0; i < size; ++i)
    {
        result[i] = dist(gen);
    }
    return result;
}

vector<double> generate_random_floats(size_t size, double min_val, double max_val)
{
    vector<double> result(size);

    // 使用随机数引擎和均匀分布生成更均匀的随机数
    random_device rd;                                         // 用于获取种子
    mt19937 gen(rd());                                        // 梅森旋转算法随机数生成器
    uniform_real_distribution<double> dist(min_val, max_val); // 定义浮点数分布

    for (size_t i = 0; i < size; ++i)
    {
        result[i] = dist(gen);
    }
    return result;
}

// 辅助函数：打印分隔线
void print_separator(const string& title)
{
    cout << "\n============================================================" << '\n';
    cout << title << '\n';
    cout << "============================================================" << '\n';
}

// 打印容器工具
namespace test_utils
{
    template <typename Container>
    void print_container(const Container& c)
    {
        size_t size = c.size();
        if (size <= 20)
        {
            for (const auto& elem : c) cout << elem << " ";
        }
        else
        {
            auto it = c.begin();
            for (size_t i = 0; i < 10; ++i, ++it) cout << *it << " ";
            cout << "... ";
            it = c.end();
            for (size_t i = 0; i < 10; ++i) --it;
            for (size_t i = 0; i < 10; ++i, ++it) cout << *it << " ";
        }
        cout << endl;
    }

    template <typename T1, typename T2>
    void print_container(const vector<pair<T1, T2>>& c)
    {
        size_t size = c.size();
        if (size <= 20)
        {
            for (const auto& elem : c) cout << "(" << elem.first << "," << elem.second << ") ";
        }
        else
        {
            auto it = c.begin();
            for (size_t i = 0; i < 10; ++i, ++it) cout << "(" << it->first << "," << it->second << ") ";
            cout << "... ";
            it = c.end();
            for (size_t i = 0; i < 10; ++i) --it;
            for (size_t i = 0; i < 10; ++i, ++it) cout << "(" << it->first << "," << it->second << ") ";
        }
        cout << endl;
    }
} // namespace test_utils

// 日期工具
namespace date_utils
{
    string normalize_date(const string& date)
    {
        string result;
        for (char c : date)
        {
            if (c != '-') result += c;
        }
        return result;
    }
} // namespace date_utils

int main()
{
    const size_t LARGE_DATA_SIZE = 100000;
    const size_t MEDIUM_DATA_SIZE = 10000;
    const size_t SMALL_DATA_SIZE = 100;

    // 1. 基础排序算法与STL比较
    print_separator("1. 基础排序算法与STL std::sort比较（大型数组）");
    auto large_ints = generate_random_ints(LARGE_DATA_SIZE, -1000000, 1000000);

    // STL排序
    vector<int> vec_stl = large_ints;
    ctimer timer;
    sort(vec_stl.begin(), vec_stl.end());
    double stl_time = timer.elapsed();
    cout << "STL std::sort (" << LARGE_DATA_SIZE << "元素): " << stl_time << " 秒" << endl;
    cout << "前10个元素: ";
    for (size_t i = 0; i < 10; ++i) cout << vec_stl[i] << " ";
    cout << endl;

    // 快速排序
    vector<int> vec_quick = large_ints;
    timer.start();
    quick_sort(vec_quick);
    double quick_time = timer.elapsed();
    cout << "自定义快速排序 (" << LARGE_DATA_SIZE << "元素): " << quick_time << " 秒" << endl;
    cout << "相对速度: " << (quick_time / stl_time) << "x, 前10个元素: ";
    for (size_t i = 0; i < 10; ++i) cout << vec_quick[i] << " ";
    cout << endl;

    // 归并排序
    vector<int> vec_merge = large_ints;
    timer.start();
    merge_sort(vec_merge);
    double merge_time = timer.elapsed();
    cout << "自定义归并排序 (" << LARGE_DATA_SIZE << "元素): " << merge_time << " 秒" << endl;
    cout << "相对速度: " << (merge_time / stl_time) << "x, 前10个元素: ";
    for (size_t i = 0; i < 10; ++i) cout << vec_merge[i] << " ";
    cout << endl;

    // 堆排序
    vector<int> vec_heap = large_ints;
    timer.start();
    heap_sort(vec_heap);
    double heap_time = timer.elapsed();
    cout << "自定义堆排序   (" << LARGE_DATA_SIZE << "元素): " << heap_time << " 秒" << endl;
    cout << "相对速度: " << (heap_time / stl_time) << "x, 前10个元素: ";
    for (size_t i = 0; i < 10; ++i) cout << vec_heap[i] << " ";
    cout << endl;

    // 希尔排序
    vector<int> vec_shell = large_ints;
    timer.start();
    shell_sort(vec_shell);
    double shell_time = timer.elapsed();
    cout << "自定义希尔排序 (" << LARGE_DATA_SIZE << "元素): " << shell_time << " 秒" << endl;
    cout << "相对速度: " << (shell_time / stl_time) << "x, 前10个元素: ";
    for (size_t i = 0; i < 10; ++i) cout << vec_shell[i] << " ";
    cout << endl;

    // 选择排序（小数据集）
    auto small_ints = generate_random_ints(SMALL_DATA_SIZE, -10000, 10000);
    vector<int> vec_select = small_ints;
    timer.start();
    selection_sort(vec_select);
    double select_time = timer.elapsed();

    vector<int> vec_stl_small = small_ints;
    timer.start();
    sort(vec_stl_small.begin(), vec_stl_small.end());
    double stl_small_time = timer.elapsed();

    cout << "自定义选择排序 (" << SMALL_DATA_SIZE << "元素): " << select_time << " 秒" << endl;
    cout << "相对速度: " << (select_time / stl_small_time) << "x, 前10个元素: ";
    for (size_t i = 0; i < 10; ++i) cout << vec_select[i] << " ";
    cout << endl;

    // 2. 浮点型排序与STL比较
    print_separator("2. 浮点型排序与STL比较");
    auto large_floats = generate_random_floats(MEDIUM_DATA_SIZE, -100000.0, 100000.0);
    cout << "原始vector<double> (" << MEDIUM_DATA_SIZE << "元素): ";
    test_utils::print_container(large_floats);

    // 自定义希尔排序
    vector<double> vec_shell_float = large_floats;
    timer.start();
    shell_sort(vec_shell_float);
    double float_shell_time = timer.elapsed();
    cout << "自定义希尔排序后: " << float_shell_time << " 秒" << endl;

    // STL排序
    vector<double> vec_stl_float = large_floats;
    timer.start();
    sort(vec_stl_float.begin(), vec_stl_float.end());
    double stl_float_time = timer.elapsed();
    cout << "STL排序耗时: " << stl_float_time << " 秒, 相对速度: " << (float_shell_time / stl_float_time) << "x" << endl;
    test_utils::print_container(vec_shell_float);

    // 3. 双向迭代器容器排序与STL比较
    print_separator("3. 双向迭代器容器排序与STL比较");
    auto medium_ints = generate_random_ints(MEDIUM_DATA_SIZE / 10, -10000, 10000);
    list<int> lst(medium_ints.begin(), medium_ints.end());
    cout << "原始list<int> (" << lst.size() << "元素): ";
    test_utils::print_container(lst);

    // 插入排序
    list<int> lst_insert = lst;
    timer.start();
    insertion_sort(lst_insert);
    double insert_time = timer.elapsed();
    cout << "自定义插入排序后: " << insert_time << " 秒" << endl;

    // STL list排序
    list<int> lst_stl = lst;
    timer.start();
    lst_stl.sort();
    double stl_list_time = timer.elapsed();
    cout << "STL list::sort耗时: " << stl_list_time << " 秒, 相对速度: " << (insert_time / stl_list_time) << "x" << endl;
    test_utils::print_container(lst_insert);

    // 冒泡排序
    list<int> lst_bubble = lst;
    timer.start();
    bubble_sort(lst_bubble);
    double bubble_time = timer.elapsed();
    cout << "自定义冒泡排序后: " << bubble_time << " 秒" << endl;
    cout << "相对速度: " << (bubble_time / stl_list_time) << "x" << endl;
    test_utils::print_container(lst_bubble);

    // 4. 计数排序测试（补充）
    print_separator("4. 计数排序测试");
    vector<int> vec_count = generate_random_ints(MEDIUM_DATA_SIZE, 0, 1000);
    cout << "原始vector (" << MEDIUM_DATA_SIZE << "元素): ";
    test_utils::print_container(vec_count);

    timer.start();
    counting_sort(vec_count);
    double count_time = timer.elapsed();
    cout << "计数排序后: " << count_time << " 秒, ";
    test_utils::print_container(vec_count);

    // STL对比
    vector<int> vec_stl_count = generate_random_ints(MEDIUM_DATA_SIZE, 0, 1000);
    timer.start();
    sort(vec_stl_count.begin(), vec_stl_count.end());
    double stl_count_time = timer.elapsed();
    cout << "STL排序耗时: " << stl_count_time << " 秒, 相对速度: " << (count_time / stl_count_time) << "x" << endl;

    // 5. 基数排序LSD测试（补充）
    print_separator("5. 基数排序LSD测试");
    vector<int> vec_radix_lsd = generate_random_ints(MEDIUM_DATA_SIZE, -100000, 100000);
    cout << "原始vector (" << MEDIUM_DATA_SIZE << "元素): ";
    test_utils::print_container(vec_radix_lsd);

    timer.start();
    radix_sort_lsd(vec_radix_lsd);
    double radix_lsd_time = timer.elapsed();
    cout << "基数排序LSD(升序): " << radix_lsd_time << " 秒, ";
    test_utils::print_container(vec_radix_lsd);

    // STL对比
    vector<int> vec_stl_radix_lsd = generate_random_ints(MEDIUM_DATA_SIZE, -100000, 100000);
    timer.start();
    sort(vec_stl_radix_lsd.begin(), vec_stl_radix_lsd.end());
    double stl_radix_lsd_time = timer.elapsed();
    cout << "STL排序耗时: " << stl_radix_lsd_time << " 秒, 相对速度: " << (radix_lsd_time / stl_radix_lsd_time) << "x" << endl;

    // 6. 基数排序MSD测试与分组功能（恢复）
    print_separator("6. 基数排序MSD测试与分组功能");
    vector<string> vec_str = {"apple", "banana", "cherry", "date", "elderberry",
                              "fig", "grape", "kiwi", "lemon", "mango", "nectarine",
                              "orange", "pear", "quince", "raspberry", "strawberry",
                              "tangerine", "watermelon", "blueberry", "blackberry",
                              "apricot", "avocado", "blackcurrant", "boysenberry"};
    vector<string> large_strs;
    for (int i = 0; i < 500; ++i)
    {
        large_strs.push_back(vec_str[i % vec_str.size()] + to_string(i));
    }

    cout << "原始vector<string> (" << large_strs.size() << "元素): ";
    test_utils::print_container(large_strs);

    // MSD排序
    vector<string> vec_msd = large_strs;
    timer.start();
    radix_sort_msd(vec_msd);
    double radix_msd_time = timer.elapsed();
    cout << "基数排序MSD后: " << radix_msd_time << " 秒, ";
    test_utils::print_container(vec_msd);

    // STL对比
    vector<string> vec_stl_msd = large_strs;
    timer.start();
    sort(vec_stl_msd.begin(), vec_stl_msd.end());
    double stl_msd_time = timer.elapsed();
    cout << "STL排序耗时: " << stl_msd_time << " 秒, 相对速度: " << (radix_msd_time / stl_msd_time) << "x" << endl;

    // 恢复前2位分组功能
    cout << "\n基数排序前2位分组测试: " << endl;
    timer.start();
    auto groups = radix_group_by_prefix(large_strs, 2);
    double group_time = timer.elapsed();
    cout << "按前2个字符分组结果(" << group_time << " 秒): " << endl;
    cout << "  共 " << groups.size() << " 个分组" << endl;
    for (size_t i = 0; i < min(static_cast<size_t>(3), groups.size()); ++i)
    {
        cout << "  分组 " << i << " (" << groups[i].size() << "元素): ";
        test_utils::print_container(groups[i]);
    }
    if (groups.size() > 3)
    {
        cout << "  ... 省略 " << groups.size() - 3 << " 个分组" << endl;
    }

    // 7. 桶排序全测试
    print_separator("7. 桶排序全测试");
    // 7.1 整数桶排序
    vector<int> vec_bucket_int = generate_random_ints(MEDIUM_DATA_SIZE, 0, 10000);
    cout << "原始整数vector (" << MEDIUM_DATA_SIZE << "元素): ";
    test_utils::print_container(vec_bucket_int);

    timer.start();
    bucket_sort(vec_bucket_int, 10, greater<int>());
    double bucket_int_time = timer.elapsed();
    cout << "整数桶排序(降序): " << bucket_int_time << " 秒, ";
    test_utils::print_container(vec_bucket_int);

    // STL对比
    vector<int> vec_stl_bucket_int = generate_random_ints(MEDIUM_DATA_SIZE, 0, 10000);
    timer.start();
    sort(vec_stl_bucket_int.begin(), vec_stl_bucket_int.end(), greater<int>());
    double stl_bucket_int_time = timer.elapsed();
    cout << "STL排序耗时: " << stl_bucket_int_time << " 秒, 相对速度: " << (bucket_int_time / stl_bucket_int_time) << "x" << endl;

    // 7.2 浮点数桶排序（小数部分）
    vector<double> vec_bucket_float = generate_random_floats(MEDIUM_DATA_SIZE, 0.0, 1000.0);
    cout << "\n原始浮点数vector (" << MEDIUM_DATA_SIZE << "元素): ";
    test_utils::print_container(vec_bucket_float);

    timer.start();
    bucket_sort(vec_bucket_float, 10, 0.0, 1000.0, FloatFractionGreater());
    double bucket_float_time = timer.elapsed();
    cout << "浮点数桶排序(按小数部分降序): " << bucket_float_time << " 秒, ";
    test_utils::print_container(vec_bucket_float);

    // STL对比
    vector<double> vec_stl_bucket_float = generate_random_floats(MEDIUM_DATA_SIZE, 0.0, 1000.0);
    timer.start();
    sort(vec_stl_bucket_float.begin(), vec_stl_bucket_float.end(), FloatFractionGreater());
    double stl_bucket_float_time = timer.elapsed();
    cout << "STL排序耗时: " << stl_bucket_float_time << " 秒, 相对速度: " << (bucket_float_time / stl_bucket_float_time) << "x" << endl;

    // 8. 折半插入排序测试（补充）
    print_separator("8. 折半插入排序测试");
    vector<int> vec_binary_insert = generate_random_ints(MEDIUM_DATA_SIZE / 10, -10000, 10000);
    cout << "原始vector (" << vec_binary_insert.size() << "元素): ";
    test_utils::print_container(vec_binary_insert);

    timer.start();
    binary_insertion_sort(vec_binary_insert);
    double binary_insert_time = timer.elapsed();
    cout << "折半插入排序后: " << binary_insert_time << " 秒, ";
    test_utils::print_container(vec_binary_insert);

    // STL对比
    vector<int> vec_stl_binary = generate_random_ints(MEDIUM_DATA_SIZE / 10, -10000, 10000);
    timer.start();
    sort(vec_stl_binary.begin(), vec_stl_binary.end());
    double stl_binary_time = timer.elapsed();
    cout << "STL排序耗时: " << stl_binary_time << " 秒, 相对速度: " << (binary_insert_time / stl_binary_time) << "x" << endl;

    // 9. 部分范围排序与STL比较
    print_separator("9. 部分范围排序与STL比较");
    vector<int> vec_part = generate_random_ints(MEDIUM_DATA_SIZE, 0, 10000);
    cout << "原始vector (" << MEDIUM_DATA_SIZE << "元素): ";
    test_utils::print_container(vec_part);

    timer.start();
    quick_sort(vec_part.begin() + MEDIUM_DATA_SIZE / 4, vec_part.end() - MEDIUM_DATA_SIZE / 4);
    double partial_time = timer.elapsed();
    cout << "自定义部分排序后 [1/4, 3/4): " << partial_time << " 秒" << endl;

    vector<int> vec_stl_part = generate_random_ints(MEDIUM_DATA_SIZE, 0, 10000);
    timer.start();
    sort(vec_stl_part.begin() + MEDIUM_DATA_SIZE / 4, vec_stl_part.end() - MEDIUM_DATA_SIZE / 4);
    double stl_part_time = timer.elapsed();
    cout << "STL部分排序耗时: " << stl_part_time << " 秒, 相对速度: " << (partial_time / stl_part_time) << "x" << endl;
    test_utils::print_container(vec_part);

    // 10. Lambda函数自定义排序与STL比较
    print_separator("10. Lambda函数自定义排序与STL比较");
    vector<pair<int, int>> vec_lambda;
    for (size_t i = 0; i < MEDIUM_DATA_SIZE; ++i)
    {
        vec_lambda.emplace_back(rand() % 100, rand() % 1000);
    }
    cout << "原始vector (" << MEDIUM_DATA_SIZE << "元素): ";
    test_utils::print_container(vec_lambda);

    timer.start();
    quick_sort(vec_lambda, [](const pair<int, int>& a, const pair<int, int>& b)
               { return a.first != b.first ? a.first > b.first : a.second < b.second; });
    double lambda_time = timer.elapsed();
    cout << "自定义Lambda排序后: " << lambda_time << " 秒" << endl;

    vector<pair<int, int>> vec_stl_lambda;
    for (size_t i = 0; i < MEDIUM_DATA_SIZE; ++i)
    {
        vec_stl_lambda.emplace_back(rand() % 100, rand() % 1000);
    }
    timer.start();
    sort(vec_stl_lambda.begin(), vec_stl_lambda.end(), [](const pair<int, int>& a, const pair<int, int>& b)
         { return a.first != b.first ? a.first > b.first : a.second < b.second; });
    double stl_lambda_time = timer.elapsed();
    cout << "STL Lambda排序耗时: " << stl_lambda_time << " 秒, 相对速度: " << (lambda_time / stl_lambda_time) << "x" << endl;
    test_utils::print_container(vec_lambda);

    print_separator("所有排序测试完成");
    return 0;
}