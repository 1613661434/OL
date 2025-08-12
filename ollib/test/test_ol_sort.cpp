#define DEBUG
#include "ol_sort.h"
#include <iostream>

using namespace ol;
using namespace std;

int main()
{
    // 测试原生数组 - 快速排序
    int arr[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    cout << "int arr[] Original array: ";
    print_container(arr);
    quick_sort(arr); // 直接调用快速排序
    cout << "Quick sorted array: ";
    print_container(arr);
    cout << "\n";

    // 测试原生数组 - 插入排序
    int arr2[] = {9, 3, 7, 5, 1, 8, 2, 6, 4};
    cout << "int arr2[] Original array: ";
    print_container(arr2);
    insertion_sort(arr2); // 直接调用插入排序
    cout << "Insertion sorted array: ";
    print_container(arr2);
    cout << "\n";

    // 测试vector - 快速排序
    vector<double> vec = {3.14, 2.71, 1.618, 0.577, 4.669, 2.718};
    cout << "vector<double> vec Original vector: ";
    print_container(vec);
    quick_sort(vec); // 直接调用快速排序
    cout << "Quick sorted vector: ";
    print_container(vec);
    cout << "\n";

    // 测试vector - 插入排序
    vector<int> vec2 = {10, 5, 8, 3, 1, 9, 2, 7, 4, 6};
    cout << "vector<int> vec2 Original vector: ";
    print_container(vec2);
    insertion_sort(vec2); // 直接调用插入排序
    cout << "Insertion sorted vector: ";
    print_container(vec2);
    cout << "\n";

    // 测试list - 插入排序
    list<int> lst = {15, 12, 18, 11, 19, 13, 17, 14, 16};
    cout << "list<int> lst Original list: ";
    print_container(lst);
    insertion_sort(lst); // 链表使用插入排序
    cout << "Insertion sorted list: ";
    print_container(lst);
    cout << "\n";

    // 会报错
    //  测试list - 快速排序
    /*
    list<int> lst2 = { 14, 2, 6, 11, 9, 13, 22, 33, 16 };
    cout << "list<int> lst2 Original list: ";
    print_container(lst2);
    quick_sort(lst2); // 链表使用插入排序
    cout << "Insertion sorted list: ";
    print_container(lst2);
    cout << "\n";
    */

    // 测试部分范围排序 - 快速排序
    vector<int> numbers = {10, 5, 8, 3, 1, 9, 2, 7, 4, 6};
    cout << "vector<int> numbers Original numbers: ";
    print_container(numbers);
    quick_sort(numbers.begin() + 2, numbers.end() - 2); // 排序部分范围
    cout << "Partially quick sorted: ";
    print_container(numbers);
    cout << "\n";

    // 测试部分范围排序 - 插入排序
    vector<int> numbers2 = {10, 5, 8, 3, 1, 9, 2, 7, 4, 6};
    cout << "vector<int> numbers2 Original numbers: ";
    print_container(numbers2);
    insertion_sort(numbers2.begin() + 1, numbers2.end() - 1); // 排序部分范围
    cout << "Partially insertion sorted: ";
    print_container(numbers2);

    return 0;
}