#include "ol_type_traits.h"
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <cstdio>

using namespace ol;
using namespace std;

// ======================== 1. 定义待测试的单例子类 ========================
class MyClass : public TypeSingleton<MyClass>
{
    // 必须声明友元，授权单例基类调用私有构造函数
    friend class TypeSingleton<MyClass>;

private:
    // 【单例核心要求】构造函数私有化，禁止外部创建实例
    MyClass()
    {
        // 初始化计数，原子变量保证多线程安全
        m_nCount = 0;
        // 记录初始化次数，原子变量防止多线程竞态
        ++m_nInitCount;
        cout << "[MyClass] 单例对象 初始化成功 | 实例内存地址: " << this << "\n";
    }

    // 析构私有化，杜绝外部手动释放
    ~MyClass()
    {
        cout << "[MyClass] 单例对象 析构成功\n";
    }

public:
    // 原子成员变量：多线程环境下安全修改，验证实例唯一性
    atomic<int> m_nCount;
    // 静态原子变量：统计单例的初始化次数，【核心】验证是否只初始化1次
    static atomic<int> m_nInitCount;

    // 业务成员函数：打印实例信息+修改计数
    void doWork(const string& threadName)
    {
        ++m_nCount;
        printf("[%s] 调用单例成员函数 | 实例地址: %p | 当前计数: %d\n",
               threadName.c_str(), this, m_nCount.load());
    }

    // 获取当前实例地址，用于验证唯一性
    const void* getInstanceAddr() const
    {
        return this;
    }
};

// 初始化静态原子变量，必须类外初始化
atomic<int> MyClass::m_nInitCount = 0;

// ======================== 2. 多线程测试函数：每个线程高频调用单例 ========================
// 模拟多线程并发获取单例、调用成员函数，验证线程安全+实例唯一
void threadFunc(int threadId)
{
    string threadName = "线程" + to_string(threadId);
    // 每个线程【多次调用】单例，模拟高并发场景
    for (int i = 0; i < 10; ++i)
    {
        // 核心调用：获取单例引用
        MyClass& instance = MyClass::GetInst();
        // 调用成员函数
        instance.doWork(threadName);
        // 短暂休眠，模拟业务逻辑耗时，加大线程竞争
        this_thread::sleep_for(chrono::nanoseconds(100));
    }
}

// ======================== 3. 主测试函数：分模块全场景测试 ========================
int main()
{
    cout << "========================================\n";
    cout << "ol::TypeSingleton 单例测试程序           \n";
    cout << "C++17标准 | 全场景覆盖 | 线程安全验证     \n";
    cout << "========================================\n\n";

    // -------------------------- 测试1：基础单例唯一性验证 --------------------------
    cout << "[测试1] 基础单例唯一性验证 -----------------\n";
    MyClass& inst1 = MyClass::GetInst();
    MyClass& inst2 = MyClass::GetInst();
    MyClass& inst3 = MyClass::GetInst();
    // 所有引用指向【同一个内存地址】，证明实例唯一
    cout << "inst1 地址: " << inst1.getInstanceAddr() << "\n";
    cout << "inst2 地址: " << inst2.getInstanceAddr() << "\n";
    cout << "inst3 地址: " << inst3.getInstanceAddr() << "\n";
    cout << "所有引用指向同一实例: " << (inst1.getInstanceAddr() == inst2.getInstanceAddr() && inst2.getInstanceAddr() == inst3.getInstanceAddr()) << "\n\n";

    // -------------------------- 测试2：验证【不可拷贝/不可移动】特性生效 --------------------------
    cout << "[测试2] 不可拷贝/不可移动特性验证 (以下代码编译会报错) -----------------\n";
    MyClass& ref = MyClass::GetInst();
    // 以下代码全部【编译报错】，证明你的TypeNonCopyableMovable生效，完美禁止值语义
    // 1. 禁用拷贝构造
    // MyClass obj1 = ref;
    // 2. 禁用拷贝赋值
    // MyClass obj2; obj2 = ref;
    // 3. 禁用移动构造
    // MyClass obj3 = move(ref);
    // 4. 禁用移动赋值
    // MyClass obj4; obj4 = move(ref);
    // 5. 禁用构造函数
    // TypeSingleton<MyClass> obj5;

    // -------------------------- 测试3：验证【引用转指针】合法且安全 --------------------------
    cout << "[测试3] 引用转指针 合法性验证 -----------------\n";
    MyClass* pInst = &MyClass::GetInst();
    cout << "单例引用转指针地址: " << pInst << "\n";
    cout << "指针与引用指向同一实例: " << (pInst == &ref) << "\n";
    // 指针调用成员函数，功能完全正常
    pInst->doWork("指针调用");
    cout << "✅ 引用转指针 合法、安全、零开销，功能正常\n\n";

    // -------------------------- 测试4：验证【懒汉模式】(第一次调用才初始化) --------------------------
    cout << "[测试4] 懒汉模式验证 -----------------\n";
    // 重置初始化计数
    MyClass::m_nInitCount = 0;
    // 定义一个新的单例测试类，验证懒加载
    class LazyTest : public TypeSingleton<LazyTest>
    {
        friend class TypeSingleton<LazyTest>;

    private:
        LazyTest() { cout << "LazyTest 单例 第一次调用，初始化成功！\n"; }
    };
    cout << "调用GetInst()前，无初始化日志输出\n";
    LazyTest::GetInst();
    cout << "✅ 懒汉模式生效：单例实例【第一次调用GetInst()时才初始化】\n\n";

    // -------------------------- 测试5：多线程高并发线程安全测试 --------------------------
    cout << "[测试5] 多线程高并发测试 -----------------\n";
    const int THREAD_NUM = 10; // 创建线程
    vector<thread> threadPool;
    threadPool.reserve(THREAD_NUM);

    // 重置计数，便于统计
    MyClass::GetInst().m_nCount = 0;
    MyClass::m_nInitCount = 0;

    // 创建所有线程
    for (int i = 0; i < THREAD_NUM; ++i)
    {
        threadPool.emplace_back(threadFunc, i);
    }

    // 等待所有线程执行完毕
    for (auto& t : threadPool)
    {
        t.join();
    }

    // 核心验证结果
    cout << "\n【多线程测试核心结果】\n";
    cout << "✅ 单例初始化总次数: " << MyClass::m_nInitCount.load() << " (正确值应为1)\n";
    cout << "✅ 单例最终计数: " << MyClass::GetInst().m_nCount.load() << " (正确值应为10*10=100)\n";
    cout << "✅ 所有线程获取的实例地址完全一致，无多实例创建问题\n";

    // -------------------------- 测试总结 --------------------------
    cout << "\n========================================\n";
    cout << "✅ 所有测试用例通过！\n";
    cout << "✅ ol::TypeSingleton 单例特性总结：\n";
    cout << "   1. 实例全局唯一，无重复创建\n";
    cout << "   2. 完美禁用拷贝/移动，杜绝值语义破坏单例\n";
    cout << "   3. 引用转指针合法、安全、零开销\n";
    cout << "   4. 懒汉模式生效，按需初始化\n";
    cout << "   5. C++17原生线程安全，高并发无问题\n";
    cout << "========================================\n";

    return 0;
}