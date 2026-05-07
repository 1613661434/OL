#include "ol_hash.h"
#include <functional>
#include <iostream>
#include <string>
#include <unordered_set>

using namespace ol;

// 自定义类型：Customer（用于测试万能哈希）
class Customer
{
public:
    std::string fname; // 名
    std::string lname; // 姓
    long no;           // 编号

public:
    // 构造函数
    Customer(std::string in_fname, std::string in_lname, long in_no)
        : fname(in_fname), lname(in_lname), no(in_no)
    {
    }

    // 重载==运算符：用于unordered_set判断元素相等性
    bool operator==(const Customer& other) const
    {
        return (fname == other.fname) &&
               (lname == other.lname) &&
               (no == other.no);
    }
};

// 为Customer定义哈希函数（使用万能哈希）
class CustomerHash
{
public:
    std::size_t operator()(const Customer& c) const
    {
        return hash_val(c.fname, c.lname, c.no); // 组合所有成员的哈希值
    }
};

// 辅助函数：打印Customer对象的哈希值
void show_hash_code(const Customer& c)
{
    std::cout << "哈希值: " << hash_val(c.fname, c.lname, c.no) << "\n";
}

// 主函数：测试万能哈希的功能
int main()
{
    // 1. 测试基础类型的哈希组合
    std::cout << "=== 基础类型哈希测试 ===" << std::endl;
    int a = 10;
    std::string b = "test";
    double c = 3.14;
    std::cout << "组合哈希(a, b, c): " << hash_val(a, b, c) << std::endl
              << std::endl;

    // 2. 测试自定义类型Customer的哈希
    std::cout << "=== 自定义类型哈希测试 ===" << std::endl;
    Customer cust1("Ace", "Hou", 1L);
    Customer cust2("Ace", "Hou", 1L); // 与cust1内容相同
    Customer cust3("Bob", "Smith", 2L);

    show_hash_code(cust1); // 哈希值相同
    show_hash_code(cust2); // 哈希值相同
    show_hash_code(cust3); // 哈希值不同

    // 3. 测试unordered_set中的去重功能
    std::cout << "\n=== unordered_set去重测试 ===" << std::endl;
    std::unordered_set<Customer, CustomerHash> cust_set;
    cust_set.insert(cust1);
    cust_set.insert(cust2); // 插入失败（与cust1相等）
    cust_set.insert(cust3);

    std::cout << "集合中元素数量: " << cust_set.size() << std::endl; // 输出2（去重成功）

    return 0;
}