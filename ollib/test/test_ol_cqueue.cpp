#include "ol_cqueue.h"
#include <string>

using namespace ol;
using namespace std;

// 自定义测试类，用于验证复杂类型的处理（含资源管理，便于观察clear时的析构）
class TestClass
{
private:
    int m_id;
    string m_name;

public:
    // 默认构造函数
    TestClass() : m_id(0), m_name("default")
    {
        cout << "Default constructor: " << m_name << endl;
    }

    // 带参数的构造函数
    TestClass(int id, const string& name) : m_id(id), m_name(name)
    {
        cout << "Constructor: " << m_name << endl;
    }

    // 拷贝构造函数
    TestClass(const TestClass& other) : m_id(other.m_id), m_name(other.m_name)
    {
        cout << "Copy constructor: " << m_name << endl;
    }

    // 移动构造函数
    TestClass(TestClass&& other) noexcept : m_id(other.m_id), m_name(move(other.m_name))
    {
        cout << "Move constructor: " << m_name << endl;
    }

    // 析构函数（非平凡析构，clear时需显式调用，此处打印以验证）
    ~TestClass()
    {
        cout << "Destructor: " << m_name << " (资源释放)" << endl;
    }

    // 拷贝赋值运算符
    TestClass& operator=(const TestClass& other)
    {
        if (this != &other)
        {
            m_id = other.m_id;
            m_name = other.m_name;
            cout << "Copy assignment: " << m_name << endl;
        }
        return *this;
    }

    // 移动赋值运算符
    TestClass& operator=(TestClass&& other) noexcept
    {
        if (this != &other)
        {
            m_id = other.m_id;
            m_name = move(other.m_name);
            cout << "Move assignment: " << m_name << endl;
        }
        return *this;
    }

    // 获取信息
    string info() const
    {
        return "ID: " + to_string(m_id) + ", Name: " + m_name;
    }
};

// 辅助函数：打印分隔线
void printSeparator(const string& title = "")
{
    cout << "\n===== " << title << " =====" << endl;
}

int main()
{
    printSeparator("测试基本整数队列操作");
    {
        cqueue<int, 3> intQueue;

        cout << "入队操作测试:" << endl;
        intQueue.push(10);
        intQueue.push(20);
        intQueue.push(30);
        cout << "队列已满，尝试入队40:" << endl;
        if (!intQueue.push(40))
        {
            cout << "入队失败，队列已满" << endl;
        }

        cout << "\n队列状态:" << endl;
        cout << "队列长度: " << intQueue.size() << endl;
        cout << "队列元素:" << endl;
        intQueue.print();

        cout << "\n出队操作测试:" << endl;
        while (!intQueue.empty())
        {
            cout << "队首元素: " << intQueue.front() << endl;
            intQueue.pop();
        }
        cout << "队列已空，尝试出队:" << endl;
        if (!intQueue.pop())
        {
            cout << "出队失败，队列为空" << endl;
        }
    }

    printSeparator("测试异常处理");
    {
        cqueue<int, 2> queue;

        try
        {
            cout << "尝试从空队列获取队首元素..." << endl;
            queue.front();
        }
        catch (const out_of_range& e)
        {
            cout << "捕获异常: " << e.what() << endl;
        }

        queue.push(1);
        queue.push(2);
        cout << "\n队列已满，尝试入队3..." << endl;
        if (!queue.push(3))
        {
            cout << "入队失败，队列已满" << endl;
        }
    }

    printSeparator("测试自定义类型");
    {
        cqueue<TestClass, 2> objQueue;

        cout << "\n使用push入队临时对象:" << endl;
        objQueue.push(TestClass(1, "Object1"));

        cout << "\n使用emplace原地构造对象:" << endl;
        objQueue.emplace(2, "Object2");

        cout << "\n队列元素:" << endl;
        while (!objQueue.empty())
        {
            cout << objQueue.front().info() << endl;
            objQueue.pop();
        }
    }

    printSeparator("测试移动语义");
    {
        cqueue<string, 2> strQueue;

        cout << "\n使用push入队右值引用:" << endl;
        strQueue.push("Hello");
        strQueue.push(string("World"));

        cout << "\n队列元素:" << endl;
        strQueue.print();

        cout << "\n移动构造新队列:" << endl;
        cqueue<string, 2> movedQueue(move(strQueue));
        cout << "移动后原队列状态:" << endl;
        cout << "原队列是否为空: " << (strQueue.empty() ? "是" : "否") << endl;
        cout << "移动后新队列元素:" << endl;
        movedQueue.print();
    }

    printSeparator("测试init()方法");
    {
        cqueue<int, 3> queue;
        queue.push(1);
        queue.push(2);
        cout << "初始化前队列长度: " << queue.size() << endl;

        cout << "调用init()重新初始化队列..." << endl;
        queue.init();
        cout << "初始化后队列长度: " << queue.size() << endl;

        cout << "重新入队元素..." << endl;
        queue.push(100);
        queue.push(200);
        queue.print();
    }

    printSeparator("测试队列满和空的边界条件");
    {
        cqueue<int, 1> queue;

        cout << "入队元素1..." << endl;
        queue.push(1);
        cout << "队列是否已满: " << (queue.full() ? "是" : "否") << endl;

        cout << "尝试再次入队元素2..." << endl;
        if (!queue.push(2))
        {
            cout << "入队失败，队列已满" << endl;
        }

        cout << "出队元素..." << endl;
        queue.pop();
        cout << "队列是否为空: " << (queue.empty() ? "是" : "否") << endl;
    }

    printSeparator("测试clear()函数");
    {
        // 1. 测试POD类型（int）的clear：仅重置状态，无需析构
        printSeparator("1. POD类型（int）的clear");
        cqueue<int, 3> podQueue;
        podQueue.push(10);
        podQueue.push(20);
        podQueue.push(30);

        cout << "clear前 - 队列长度: " << podQueue.size() << ", 元素: ";
        podQueue.print();

        cout << "调用clear()..." << endl;
        podQueue.clear();

        cout << "clear后 - 队列长度: " << podQueue.size() << ", 是否为空: " << (podQueue.empty() ? "是" : "否") << endl;
        cout << "clear后重新入队元素（验证复用）:" << endl;
        podQueue.push(100);
        podQueue.push(200);
        podQueue.print();

        // 2. 测试非POD类型（TestClass）的clear：需显式析构元素（观察Destructor打印）
        printSeparator("2. 非POD类型（TestClass）的clear");
        cqueue<TestClass, 2> nonPodQueue;
        nonPodQueue.emplace(1, "ClearTest1"); // 原地构造
        nonPodQueue.emplace(2, "ClearTest2");

        cout << "clear前 - 队列长度: " << nonPodQueue.size() << ", 元素信息:" << endl;
        cout << nonPodQueue.front().info() << endl;
        nonPodQueue.pop(); // 先弹出第一个，留一个在队列中便于观察clear的析构
        cout << "弹出一个元素后，剩余队列长度: " << nonPodQueue.size() << endl;

        cout << "调用clear()（观察下方'资源释放'打印，验证析构）..." << endl;
        nonPodQueue.clear();

        cout << "clear后 - 队列长度: " << nonPodQueue.size() << ", 是否为空: " << (nonPodQueue.empty() ? "是" : "否") << endl;
        cout << "clear后重新入队（验证复用）:" << endl;
        nonPodQueue.emplace(3, "ClearTest3"); // 重新构造元素
        cout << "复用后队列元素: " << nonPodQueue.front().info() << endl;

        // 3. 测试空队列调用clear：无操作（边界场景）
        printSeparator("3. 空队列调用clear（边界场景）");
        cqueue<string, 2> emptyQueue;
        cout << "空队列调用clear()前 - 是否为空: " << (emptyQueue.empty() ? "是" : "否") << endl;
        emptyQueue.clear(); // 调用无副作用
        cout << "空队列调用clear()后 - 是否为空: " << (emptyQueue.empty() ? "是" : "否") << endl;
    }

    return 0;
}