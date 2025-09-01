#include "ol_chrono.h"
#include "ol_sort.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using namespace ol;
using namespace std;

// 使用随机数引擎生成更均匀的随机整数
vector<int> generate_random_ints(size_t size = 50000, int min_val = -1000000, int max_val = 1000000)
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

// 将排序结果保存到英文命名的文件
void save_sort_result(const vector<int>& sorted_data, const string& sort_name)
{
    ofstream out_file(sort_name + "_sorted_result.txt");
    if (!out_file.is_open())
    {
        cerr << "错误：无法创建文件 " << sort_name << "_sorted_result.txt\n";
        return;
    }
    for (const auto& num : sorted_data)
    {
        out_file << num << '\n';
    }
    out_file.close();
    cout << "已保存：" << sort_name << " 结果到 " << sort_name << "_sorted_result.txt\n";
}

// 打印性能对比表格
void print_performance_table(const vector<pair<string, double>>& performance_data)
{
    cout << "\n============================================================\n";
    cout << "                  排序算法性能对比\n";
    cout << "============================================================\n";
    cout << left << setw(15) << "算法名称" << setw(20) << "数据规模" << setw(15) << "时间（秒）\n";
    cout << "------------------------------------------------------------\n";
    const size_t data_size = generate_random_ints().size();
    for (const auto& item : performance_data)
    {
        cout << left << setw(15) << item.first
             << setw(20) << data_size
             << setw(15) << fixed << setprecision(6) << item.second << '\n';
    }
    cout << "============================================================\n";
}

int main()
{
    // 生成原始随机数据
    vector<int> raw_data = generate_random_ints();
    cout << "已生成 " << raw_data.size() << " 个随机整数\n";

    // 存储排序算法名称和对应的耗时
    vector<pair<string, double>> performance_data;

    // 1. 插入排序 (Insertion Sort)
    {
        vector<int> data = raw_data;
        cout << "\n正在执行插入排序...\n";
        ctimer timer;
        insertion_sort(data);
        double time = timer.elapsed();
        save_sort_result(data, "insertion_sort");
        performance_data.emplace_back("插入排序", time);
        cout << "插入排序完成，耗时 " << fixed << setprecision(6) << time << " 秒\n";
    }

    // 2. 希尔排序 (Shell Sort)
    {
        vector<int> data = raw_data;
        cout << "\n正在执行希尔排序...\n";
        ctimer timer;
        shell_sort(data);
        double time = timer.elapsed();
        save_sort_result(data, "shell_sort");
        performance_data.emplace_back("希尔排序", time);
        cout << "希尔排序完成，耗时 " << fixed << setprecision(6) << time << " 秒\n";
    }

    // 3. 起泡排序 (Bubble Sort)
    {
        vector<int> data = raw_data;
        cout << "\n正在执行起泡排序...\n";
        ctimer timer;
        bubble_sort(data);
        double time = timer.elapsed();
        save_sort_result(data, "bubble_sort");
        performance_data.emplace_back("起泡排序", time);
        cout << "起泡排序完成，耗时 " << fixed << setprecision(6) << time << " 秒\n";
    }

    // 4. 快速排序 (Quick Sort)
    {
        vector<int> data = raw_data;
        cout << "\n正在执行快速排序...\n";
        ctimer timer;
        quick_sort(data);
        double time = timer.elapsed();
        save_sort_result(data, "quick_sort");
        performance_data.emplace_back("快速排序", time);
        cout << "快速排序完成，耗时 " << fixed << setprecision(6) << time << " 秒\n";
    }

    // 5. 选择排序 (Selection Sort)
    {
        vector<int> data = raw_data;
        cout << "\n正在执行选择排序...\n";
        ctimer timer;
        selection_sort(data);
        double time = timer.elapsed();
        save_sort_result(data, "selection_sort");
        performance_data.emplace_back("选择排序", time);
        cout << "选择排序完成，耗时 " << fixed << setprecision(6) << time << " 秒\n";
    }

    // 6. 堆排序 (Heap Sort)
    {
        vector<int> data = raw_data;
        cout << "\n正在执行堆排序...\n";
        ctimer timer;
        heap_sort(data);
        double time = timer.elapsed();
        save_sort_result(data, "heap_sort");
        performance_data.emplace_back("堆排序", time);
        cout << "堆排序完成，耗时 " << fixed << setprecision(6) << time << " 秒\n";
    }

    // 7. 归并排序 (Merge Sort)
    {
        vector<int> data = raw_data;
        cout << "\n正在执行归并排序...\n";
        ctimer timer;
        merge_sort(data);
        double time = timer.elapsed();
        save_sort_result(data, "merge_sort");
        performance_data.emplace_back("归并排序", time);
        cout << "归并排序完成，耗时 " << fixed << setprecision(6) << time << " 秒\n";
    }

    // 显示性能对比
    print_performance_table(performance_data);

    // 确定最快的两种算法
    sort(performance_data.begin(), performance_data.end(),
         [](const auto& a, const auto& b)
         { return a.second < b.second; });

    cout << "\n最快的两种算法：\n";
    cout << "1. " << performance_data[0].first << "：" << fixed << setprecision(6) << performance_data[0].second << " 秒\n";
    cout << "2. " << performance_data[1].first << "：" << fixed << setprecision(6) << performance_data[1].second << " 秒\n";

    cout << "\n所有排序测试已完成\n";
    return 0;
}