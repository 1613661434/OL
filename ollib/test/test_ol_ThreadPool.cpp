#include "ol_ThreadPool.h"
#include <cassert>
#include <chrono>
#include <iostream>
#include <mutex>
#include <numeric>
#include <string>
#include <vector>

#ifdef __linux__
#include <sys/syscall.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

// 全局输出互斥锁，确保多线程输出不交错
std::mutex g_printMutex;

// 线程安全的格式化打印函数
template <typename... Args>
void safePrint(const char* format, Args&&... args)
{
    std::lock_guard<std::mutex> lock(g_printMutex);
    printf(format, std::forward<Args>(args)...);
    fflush(stdout); // 立即刷新缓冲区
}

// 线程安全的流式打印函数
template <typename T>
void safePrint(const T& content)
{
    std::lock_guard<std::mutex> lock(g_printMutex);
    std::cout << content;
    std::cout.flush(); // 立即刷新缓冲区
}

// 测试任务：无返回值
void printMessage(int id, const std::string& msg)
{
#ifdef __linux__
    pid_t tid = syscall(SYS_gettid);
#elif defined(_WIN32)
    DWORD tid = GetCurrentThreadId();
#else
    auto tid = std::this_thread::get_id();
#endif

    safePrint("任务 %d: %s (线程ID: %llu)\n",
              id, msg.c_str(),
              static_cast<unsigned long long>(tid));

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

// 测试任务：有返回值，计算平方
int square(int x)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return x * x;
}

// 测试任务：可能抛出异常
void riskyTask(int id)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    if (id % 3 == 0)
    {
        throw std::runtime_error("风险任务 " + std::to_string(id) + " 触发了预期异常");
    }
    safePrint("风险任务 %d 成功完成\n", id);
}

// 测试任务：计算总和
long long sumRange(int start, int end)
{
    long long sum = 0;
    for (int i = start; i <= end; ++i)
    {
        sum += i;
    }
    return sum;
}

// 测试stop()方法的任务
void stopTestTask(int id)
{
    safePrint("停止测试任务 %d 开始执行\n", id);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    safePrint("停止测试任务 %d 执行完毕\n", id);
}

int main()
{
    try
    {
        // 创建线程池，3个线程，最大队列大小为10
        ol::ThreadPool pool(3, 10);
        safePrint("创建了包含 %zu 个线程的线程池\n", pool.getThreadCount());

        // 测试添加无返回值任务
        safePrint("\n=== 测试无返回值任务 ===\n");
        for (int i = 0; i < 5; ++i)
        {
            bool success = pool.addTask(std::bind(printMessage, i, "来自线程池的问候"));
            assert(success && "添加任务失败");
        }

        // 等待任务完成
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // 测试添加有返回值的任务
        safePrint("\n=== 测试带返回值的任务 ===\n");
        std::vector<std::future<int>> squareFutures;
        for (int i = 1; i <= 5; ++i)
        {
            squareFutures.emplace_back(pool.submitTask(square, i));
        }

        // 获取并输出结果
        for (int i = 0; i < (int)squareFutures.size(); ++i)
        {
            int result = squareFutures[i].get();
            safePrint("%d 的平方是 %d\n", i + 1, result);
        }

        // 测试异常处理
        safePrint("\n=== 测试异常处理 ===\n");
        for (int i = 0; i < 5; ++i)
        {
            pool.addTask(std::bind(riskyTask, i));
        }

        // 等待任务完成
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // 测试大量任务
        safePrint("\n=== 测试大量任务处理 ===\n");
        const int TASK_COUNT = 8;
        std::vector<std::future<long long>> sumFutures;

        for (int i = 0; i < TASK_COUNT; ++i)
        {
            int start = i * 1000;
            int end = start + 999;
            sumFutures.emplace_back(pool.submitTask(sumRange, start, end));
            safePrint("添加任务: 计算 %d 到 %d 的总和\n", start, end);
        }

        // 获取并验证结果
        for (int i = 0; i < TASK_COUNT; ++i)
        {
            long long result = sumFutures[i].get();
            int start = i * 1000;
            int end = start + 999;
            // 验证结果是否正确 (等差数列求和公式)
            long long expected = (long long)(start + end) * (end - start + 1) / 2;
            safePrint("%d 到 %d 的总和: %lld (预期值: %lld)\n",
                      start, end, result, expected);
            assert(result == expected && "总和计算错误");
        }

        // 测试任务队列限制
        safePrint("\n=== 测试任务队列大小限制 ===\n");
        ol::ThreadPool limitedPool(2, 3); // 2个线程，队列最大3个任务
        int successCount = 0;

        for (int i = 0; i < 10; ++i)
        {
            bool success = limitedPool.addTask([i]()
                                               { 
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                safePrint("完成任务 %d\n", i); });

            if (success)
            {
                successCount++;
                safePrint("已添加任务 %d 到限制线程池\n", i);
            }
            else
            {
                safePrint("添加任务 %d 失败 (队列已满)\n", i);
            }
        }

        safePrint("成功添加了 %d 个任务到限制线程池\n", successCount);

        // 等待所有任务完成
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // 测试stop()方法
        safePrint("\n=== 测试stop()方法 ===\n");
        ol::ThreadPool stopTestPool(2);
        for (int i = 0; i < 3; ++i)
        {
            stopTestPool.addTask(std::bind(stopTestTask, i));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        safePrint("调用 stop(true) 等待所有任务完成...\n");
        stopTestPool.stop(true);
        safePrint("stop(true) 调用完成\n");

        safePrint("\n所有测试均成功完成\n");
    }
    catch (const std::exception& e)
    {
        std::lock_guard<std::mutex> lock(g_printMutex);
        std::cerr << "测试失败: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
// 编译命令: g++ -o test_ol_ThreadPool ol_ThreadPool.cpp test_ol_ThreadPool.cpp -lpthread -std=c++11