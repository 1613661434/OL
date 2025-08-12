#define DEBUG
#include "ol_sort.h"
#include <iostream>

using namespace ol;
using namespace std;

int main()
{
    // ����ԭ������ - ��������
    int arr[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    cout << "int arr[] Original array: ";
    print_container(arr);
    quick_sort(arr); // ֱ�ӵ��ÿ�������
    cout << "Quick sorted array: ";
    print_container(arr);
    cout << "\n";

    // ����ԭ������ - ��������
    int arr2[] = {9, 3, 7, 5, 1, 8, 2, 6, 4};
    cout << "int arr2[] Original array: ";
    print_container(arr2);
    insertion_sort(arr2); // ֱ�ӵ��ò�������
    cout << "Insertion sorted array: ";
    print_container(arr2);
    cout << "\n";

    // ����vector - ��������
    vector<double> vec = {3.14, 2.71, 1.618, 0.577, 4.669, 2.718};
    cout << "vector<double> vec Original vector: ";
    print_container(vec);
    quick_sort(vec); // ֱ�ӵ��ÿ�������
    cout << "Quick sorted vector: ";
    print_container(vec);
    cout << "\n";

    // ����vector - ��������
    vector<int> vec2 = {10, 5, 8, 3, 1, 9, 2, 7, 4, 6};
    cout << "vector<int> vec2 Original vector: ";
    print_container(vec2);
    insertion_sort(vec2); // ֱ�ӵ��ò�������
    cout << "Insertion sorted vector: ";
    print_container(vec2);
    cout << "\n";

    // ����list - ��������
    list<int> lst = {15, 12, 18, 11, 19, 13, 17, 14, 16};
    cout << "list<int> lst Original list: ";
    print_container(lst);
    insertion_sort(lst); // ����ʹ�ò�������
    cout << "Insertion sorted list: ";
    print_container(lst);
    cout << "\n";

    // �ᱨ��
    //  ����list - ��������
    /*
    list<int> lst2 = { 14, 2, 6, 11, 9, 13, 22, 33, 16 };
    cout << "list<int> lst2 Original list: ";
    print_container(lst2);
    quick_sort(lst2); // ����ʹ�ò�������
    cout << "Insertion sorted list: ";
    print_container(lst2);
    cout << "\n";
    */

    // ���Բ��ַ�Χ���� - ��������
    vector<int> numbers = {10, 5, 8, 3, 1, 9, 2, 7, 4, 6};
    cout << "vector<int> numbers Original numbers: ";
    print_container(numbers);
    quick_sort(numbers.begin() + 2, numbers.end() - 2); // ���򲿷ַ�Χ
    cout << "Partially quick sorted: ";
    print_container(numbers);
    cout << "\n";

    // ���Բ��ַ�Χ���� - ��������
    vector<int> numbers2 = {10, 5, 8, 3, 1, 9, 2, 7, 4, 6};
    cout << "vector<int> numbers2 Original numbers: ";
    print_container(numbers2);
    insertion_sort(numbers2.begin() + 1, numbers2.end() - 1); // ���򲿷ַ�Χ
    cout << "Partially insertion sorted: ";
    print_container(numbers2);

    return 0;
}