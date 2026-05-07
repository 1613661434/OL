#include <chrono>
#include <future>
#include <iostream>
#include <thread>
#include <type_traits>
#include <mutex>
#include <vector>
#include "ol_mutex.h"

using namespace std;

// 全局共享资源（用于多线程测试）
int g_shared_counter = 0;

// 多线程测试：用 ol::spin_mutex 保护共享资源
void thread_task_spin(ol::spin_mutex& smtx, int loop_count)
{
    for (int i = 0; i < loop_count; ++i)
    {
        // 使用 std::unique_lock 加锁
        unique_lock<ol::spin_mutex> lock(smtx);
        // 临界区：修改共享计数器
        g_shared_counter++;
        // 模拟临界区耗时（让线程竞争更明显）
        this_thread::sleep_for(chrono::microseconds(1));
    }
}

// 多线程测试：用 ol::recursive_spin_mutex 保护共享资源
void thread_task_recursive(ol::recursive_spin_mutex& rsmtx, int loop_count, int recursion_depth)
{
    // 递归辅助函数：验证递归锁的可重入性
    auto recursive_func = [&](auto&& self, int depth) -> void
    {
        if (depth <= 0) return;

        // 递归加锁（同一线程多次加锁，递归锁不会死锁）
        unique_lock<ol::recursive_spin_mutex> lock(rsmtx);
        g_shared_counter++;
        self(self, depth - 1); // 递归调用，再次加锁
    };

    for (int i = 0; i < loop_count; ++i)
    {
        recursive_func(recursive_func, recursion_depth);
        this_thread::sleep_for(chrono::microseconds(1));
    }
}

int main()
{
    cout << "==================================== 测试1：spin_mutex 单线程基础用法 ====================================" << endl;
    {
        ol::spin_mutex smtx;
        cout << "--- 测试 std::lock_guard 自动加锁/解锁 ---" << endl;
        // lock_guard 构造自动加锁，析构自动解锁
        lock_guard<ol::spin_mutex> lock1(smtx);
        cout << "lock_guard 持有锁中，临界区执行完成" << endl;
    } // 此处 lock1 析构，自动解锁

    {
        ol::spin_mutex smtx;
        cout << "\n--- 测试 std::unique_lock 手动控制加锁/解锁 ---" << endl;
        unique_lock<ol::spin_mutex> lock2(smtx); // 默认构造：自动加锁
        cout << "初始加锁后，是否持有锁：" << (lock2.owns_lock() ? "是（1）" : "否（0）") << endl;

        lock2.unlock(); // 手动解锁
        cout << "手动解锁后，是否持有锁：" << (lock2.owns_lock() ? "是（1）" : "否（0）") << endl;

        lock2.lock(); // 手动再次加锁
        cout << "手动再次加锁后，是否持有锁：" << (lock2.owns_lock() ? "是（1）" : "否（0）") << endl;

        lock2.unlock(); // 手动解锁
        cout << "手动解锁后，尝试无阻塞加锁（try_lock）：" << endl;
        bool try_ok = lock2.try_lock(); // 无阻塞尝试加锁
        cout << "try_lock 结果：" << (try_ok ? "成功（1）" : "失败（0）") << "，当前是否持有锁：" << (lock2.owns_lock() ? "是（1）" : "否（0）") << endl;
    }

    cout << "\n==================================== 测试2：spin_mutex unique_lock 标签高级用法 ====================================" << endl;
    {
        ol::spin_mutex smtx;

        // 1. 测试 defer_lock：小{}包裹，确保析构释放锁
        cout << "--- 测试 std::defer_lock （延迟加锁）---" << endl;
        {
            unique_lock<ol::spin_mutex> lock_defer(smtx, std::defer_lock); // 延迟加锁，构造时不加锁
            cout << "构造后（未加锁），是否持有锁：" << (lock_defer.owns_lock() ? "是（1）" : "否（0）") << endl;
            lock_defer.lock(); // 手动加锁
            cout << "手动加锁后，是否持有锁：" << (lock_defer.owns_lock() ? "是（1）" : "否（0）") << endl;
        } // lock_defer 析构，自动解锁

        // 2. 测试 try_to_lock：小{}包裹，确保析构释放锁
        cout << "\n--- 测试 std::try_to_lock （无阻塞尝试加锁）---" << endl;
        {
            unique_lock<ol::spin_mutex> lock_try(smtx, std::try_to_lock); // 无阻塞尝试加锁
            cout << "try_to_lock 结果：" << (lock_try.owns_lock() ? "成功（1）" : "失败（0）") << endl;
        } // lock_try 析构，自动解锁

        // 3. 测试 adopt_lock：此时锁已释放，可成功手动加锁
        cout << "\n--- 测试 std::adopt_lock （采用已加锁）---" << endl;
        smtx.lock(); // 手动先加锁（成功，无阻塞）
        {
            unique_lock<ol::spin_mutex> lock_adopt(smtx, std::adopt_lock); // 采用已加锁的锁
            cout << "采用已加锁的锁后，是否持有锁：" << (lock_adopt.owns_lock() ? "是（1）" : "否（0）") << endl;
        } // lock_adopt 析构，自动解锁
    }

    cout << "\n==================================== 测试3：spin_mutex 多线程竞争测试 ====================================" << endl;
    {
        ol::spin_mutex smtx;
        const int thread_num = 5;   // 线程数
        const int loop_count = 100; // 每个线程循环次数
        vector<thread> threads;

        g_shared_counter = 0; // 重置共享计数器
        cout << "启动 " << thread_num << " 个线程，每个线程修改共享计数器 " << loop_count << " 次..." << endl;

        // 启动所有线程
        for (int i = 0; i < thread_num; ++i)
        {
            threads.emplace_back(thread_task_spin, ref(smtx), loop_count);
        }

        // 等待所有线程执行完成
        for (auto& t : threads)
        {
            t.join();
        }

        // 验证共享计数器结果（预期：thread_num * loop_count）
        int expected = thread_num * loop_count;
        cout << "共享计数器最终值：" << g_shared_counter << "，预期值：" << expected << endl;
        cout << (g_shared_counter == expected ? "测试通过：多线程竞争无数据混乱" : "测试失败：存在数据竞争") << endl;
    }

    cout << "\n==================================== 测试4：recursive_spin_mutex 递归与多线程测试 ====================================" << endl;
    {
        ol::recursive_spin_mutex rsmtx;
        const int thread_num = 3;      // 线程数
        const int loop_count = 50;     // 每个线程循环次数
        const int recursion_depth = 3; // 递归深度（每个循环递归加锁3次）
        vector<thread> threads;

        g_shared_counter = 0; // 重置共享计数器
        cout << "启动 " << thread_num << " 个线程，每个线程递归修改共享计数器 " << loop_count << " 次（递归深度 " << recursion_depth << "）..." << endl;

        // 启动所有线程
        for (int i = 0; i < thread_num; ++i)
        {
            threads.emplace_back(thread_task_recursive, ref(rsmtx), loop_count, recursion_depth);
        }

        // 等待所有线程执行完成
        for (auto& t : threads)
        {
            t.join();
        }

        // 验证共享计数器结果（预期：thread_num * loop_count * recursion_depth）
        int expected = thread_num * loop_count * recursion_depth;
        cout << "共享计数器最终值：" << g_shared_counter << "，预期值：" << expected << endl;
        cout << (g_shared_counter == expected ? "测试通过：递归锁可重入，多线程无数据混乱" : "测试失败：递归锁异常或存在数据竞争") << endl;
    }

    cout << "\n==================================== 测试5：recursive_spin_mutex 异常场景测试（非持有者解锁） ====================================" << endl;
    {
        ol::recursive_spin_mutex rsmtx;
        // 先让主线程获取锁
        unique_lock<ol::recursive_spin_mutex> main_lock(rsmtx);
        cout << "主线程已获取递归锁，启动子线程尝试解锁..." << endl;

        // 子线程（非锁持有者）尝试解锁，预期抛出 std::logic_error
        thread err_thread([&]()
                          {
            try
            {
                rsmtx.unlock(); // 非持有者解锁，抛出异常
            }
            catch (const logic_error& e)
            {
                cout << "捕获到预期异常：" << e.what() << endl;
            } });

        err_thread.join();
        cout << "异常场景测试完成" << endl;
    }

    cout << "\n==================================== 所有测试执行完毕 ====================================" << endl;
    return 0;
}