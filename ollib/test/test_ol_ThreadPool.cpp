#include "ol_ThreadPool.h"
#include <cassert>
#include <chrono>
#include <cmath>
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

// 测试任务：无返回值，打印信息
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

// 测试任务：有返回值，计算立方（用于策略测试）
int cube(int x)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return x * x * x;
}

// 测试任务：可能抛出异常，验证异常处理机制
void riskyTask(int id)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    if (id % 3 == 0)
    {
        throw std::runtime_error("风险任务 " + std::to_string(id) + " 触发了预期异常");
    }
    safePrint("风险任务 %d 成功完成\n", id);
}

// 测试任务：计算区间总和，验证返回值正确性
long long sumRange(int start, int end)
{
    long long sum = 0;
    for (int i = start; i <= end; ++i)
    {
        sum += i;
    }
    return sum;
}

// 测试任务：用于验证stop()方法的行为
void stopTestTask(int id)
{
    safePrint("停止测试任务 %d 开始执行\n", id);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    safePrint("停止测试任务 %d 执行完毕\n", id);
}

// 测试任务：长时间运行的任务，用于验证队列策略
void longRunningTask(int id)
{
    safePrint("长任务 %d 开始执行\n", id);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    safePrint("长任务 %d 执行完毕\n", id);
}

int main()
{
    try
    {
        // 1. 基本功能测试：创建线程池
        ol::ThreadPool pool(3, 10); // 3个线程，队列最大10个任务
        safePrint("=== 基本功能测试 ===\n");
        safePrint("创建了包含 %zu 个线程的线程池（预期3个）\n", pool.getThreadCount());
        assert(pool.getThreadCount() == 3 && "线程数量初始化错误");
        assert(pool.isRunning() && "线程池未处于运行状态");

        // 2. 测试无返回值任务
        safePrint("\n=== 无返回值任务测试 ===\n");
        for (int i = 0; i < 5; ++i)
        {
            bool success = pool.addTask(std::bind(printMessage, i, "来自线程池的问候"));
            assert(success && "无返回值任务添加失败");
        }
        safePrint("当前等待执行的任务数: %zu\n", pool.getTaskCount());
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 等待任务执行
        assert(pool.getTaskCount() == 0 && "无返回值任务未正确执行完毕");

        // 3. 测试带返回值的任务
        safePrint("\n=== 带返回值任务测试 ===\n");
        std::vector<std::future<int>> squareFutures;
        for (int i = 1; i <= 5; ++i)
        {
            squareFutures.emplace_back(pool.submitTask(square, i));
        }

        // 获取并验证结果
        for (int i = 0; i < (int)squareFutures.size(); ++i)
        {
            int result = squareFutures[i].get();
            int expected = (i + 1) * (i + 1);
            safePrint("%d 的平方是 %d（预期%d）\n", i + 1, result, expected);
            assert(result == expected && "平方计算结果错误");
        }

        // 4. 测试异常处理机制
        safePrint("\n=== 异常处理测试 ===\n");
        for (int i = 0; i < 5; ++i)
        {
            pool.addTask(std::bind(riskyTask, i));
        }
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 等待任务执行
        // 验证线程池仍在运行（异常未导致线程崩溃）
        assert(pool.isRunning() && "异常导致线程池停止");

        // 5. 测试大量任务处理能力
        safePrint("\n=== 大量任务处理测试 ===\n");
        const int TASK_COUNT = 8;
        std::vector<std::future<long long>> sumFutures;

        for (int i = 0; i < TASK_COUNT; ++i)
        {
            int start = i * 1000;
            int end = start + 999;
            sumFutures.emplace_back(pool.submitTask(sumRange, start, end));
            safePrint("添加任务: 计算 %d 到 %d 的总和\n", start, end);
        }

        // 验证计算结果
        for (int i = 0; i < TASK_COUNT; ++i)
        {
            long long result = sumFutures[i].get();
            int start = i * 1000;
            int end = start + 999;
            // 等差数列求和公式验证
            long long expected = (long long)(start + end) * (end - start + 1) / 2;
            safePrint("%d 到 %d 的总和: %lld（预期: %lld）\n",
                      start, end, result, expected);
            assert(result == expected && "区间求和结果错误");
        }

        // 6. 测试队列满策略：拒绝策略（默认）
        safePrint("\n=== 队列满策略测试：拒绝策略 ===\n");
        ol::ThreadPool rejectPool(2, 3); // 2个线程，队列容量3
        rejectPool.setRejectPolicy();    // 显式设置拒绝策略
        int rejectSuccessAdd = 0;
        int rejectSuccessSubmit = 0;

        // 测试addTask在拒绝策略下的行为
        for (int i = 0; i < 5; ++i)
        {
            bool success = rejectPool.addTask(std::bind(longRunningTask, i));
            if (success)
            {
                rejectSuccessAdd++;
                safePrint("拒绝策略-add: 任务 %d 添加成功\n", i);
            }
            else
            {
                safePrint("拒绝策略-add: 任务 %d 添加失败（队列已满）\n", i);
            }
        }

        // 测试submitTask在拒绝策略下的行为
        std::vector<std::future<int>> rejectFutures;
        for (int i = 10; i < 15; ++i)
        {
            try
            {
                auto future = rejectPool.submitTask(cube, i);
                rejectFutures.push_back(std::move(future));
                rejectSuccessSubmit++;
                safePrint("拒绝策略-submit: 任务 %d 添加成功\n", i);
            }
            catch (const std::exception& e)
            {
                safePrint("拒绝策略-submit: 任务 %d 添加失败（%s）\n", i, e.what());
            }
        }

        safePrint("拒绝策略：add成功 %d 个，submit成功 %d 个（预期共5个）\n",
                  rejectSuccessAdd, rejectSuccessSubmit);
        std::this_thread::sleep_for(std::chrono::seconds(3)); // 等待所有任务完成

        // 验证结果
        for (auto& future : rejectFutures)
        {
            int result = future.get();
            int x = std::cbrt(result); // 计算立方根
            assert(result == x * x * x && "立方计算错误");
        }
        assert(rejectPool.getTaskCount() == 0 && "拒绝策略任务未执行完毕");

        // 7. 测试队列满策略：阻塞策略
        safePrint("\n=== 队列满策略测试：阻塞策略 ===\n");
        ol::ThreadPool blockPool(2, 3); // 2个线程，队列容量3
        blockPool.setBlockPolicy();     // 设置阻塞策略
        std::vector<std::thread> blockTestThreads;
        int blockSuccessSubmit = 0;
        std::atomic_int tasksAdded(0); // 原子变量跟踪已添加的任务数

        // 多线程添加任务，测试阻塞行为
        for (int i = 0; i < 5; ++i)
        {
            blockTestThreads.emplace_back([&, i]()
                                          {
        try {
            auto future = blockPool.submitTask(cube, i + 20);

            // 等待任务完成并验证结果
            int result = future.get();
            int x = i + 20;
            assert(result == x * x * x && "阻塞策略-立方计算错误");
            
            std::lock_guard<std::mutex> lock(g_printMutex);
            blockSuccessSubmit++;
        }
        catch (const std::exception& e) {
            safePrint("阻塞策略：线程 %d 添加任务失败（%s）\n", i, e.what());
        } });
        }

        // 等待所有添加线程完成
        for (auto& t : blockTestThreads)
        {
            if (t.joinable())
            {
                t.join();
            }
        }

        safePrint("阻塞策略：submit成功添加并执行 %d 个任务（预期5个）\n", blockSuccessSubmit);
        std::this_thread::sleep_for(std::chrono::seconds(2)); // 等待任务执行
        assert(blockPool.getTaskCount() == 0 && "阻塞策略任务未执行完毕");

        // 8. 测试队列满策略：超时策略
        safePrint("\n=== 队列满策略测试：超时策略 ===\n");
        ol::ThreadPool timeoutPool(2, 3);     // 2个线程，队列容量3
        timeoutPool.setTimeoutPolicy(100000); // 超时100ms
        int timeoutSuccessAdd = 0;
        int timeoutSuccessSubmit = 0;

        // 先添加一些任务填满队列
        for (int i = 0; i < 3; ++i)
        {
            timeoutPool.addTask(std::bind(longRunningTask, i + 50));
            timeoutSuccessAdd++;
        }

        // 测试submitTask在超时策略下的行为
        std::vector<std::future<int>> timeoutFutures;
        for (int i = 60; i < 65; ++i)
        {
            try
            {
                auto future = timeoutPool.submitTask(cube, i);
                timeoutFutures.push_back(std::move(future));
                timeoutSuccessSubmit++;
                safePrint("超时策略-submit: 任务 %d 添加成功\n", i);
            }
            catch (const std::exception& e)
            {
                safePrint("超时策略-submit: 任务 %d 添加失败（%s）\n", i, e.what());
            }
        }

        safePrint("超时策略：add成功 %d 个，submit成功 %d 个\n",
                  timeoutSuccessAdd, timeoutSuccessSubmit);
        std::this_thread::sleep_for(std::chrono::seconds(3)); // 等待任务执行

        // 验证成功提交的任务结果
        for (auto& future : timeoutFutures)
        {
            int result = future.get();
            int x = std::cbrt(result); // 计算立方根
            assert(result == x * x * x && "超时策略-立方计算错误");
        }
        assert(timeoutPool.getTaskCount() == 0 && "超时策略任务未执行完毕");

        // 9. 测试stop()方法
        safePrint("\n=== stop()方法测试 ===\n");
        ol::ThreadPool stopTestPool(2);
        for (int i = 0; i < 3; ++i)
        {
            stopTestPool.addTask(std::bind(stopTestTask, i));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        safePrint("调用 stop(true) 等待所有任务完成...\n");
        stopTestPool.stop(true);
        assert(!stopTestPool.isRunning() && "stop(true) 未正确停止线程池");
        safePrint("stop(true) 调用完成\n");

        // 10. 测试已停止的线程池行为
        safePrint("\n=== 已停止线程池行为测试 ===\n");
        // 创建一个新线程池并主动停止它，用于测试
        ol::ThreadPool stoppedPool(1);
        stoppedPool.stop(); // 确保线程池已停止

        // 尝试向已停止的线程池添加任务
        bool addResult = stoppedPool.addTask([]()
                                             { safePrint("此任务不应执行\n"); });
        assert(!addResult && "已停止线程池不应接受addTask任务");

        // 尝试向已停止的线程池提交任务
        bool submitThrew = false;
        try
        {
            stoppedPool.submitTask(square, 5);
        }
        catch (const std::exception& e)
        {
            submitThrew = true;
            safePrint("已停止线程池：submitTask正确抛出异常 - %s\n", e.what());
        }
        assert(submitThrew && "已停止线程池应拒绝submitTask任务并抛出异常");

        safePrint("\n所有测试均成功完成!\n");
    }
    catch (const std::exception& e)
    {
        std::lock_guard<std::mutex> lock(g_printMutex);
        std::cerr << "测试失败: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

// 编译命令:
// Linux: g++ -o test_ol_ThreadPool ol_ThreadPool.cpp test_ol_ThreadPool.cpp -lpthread -std=c++11
// Windows: mingw32-make.exe -o test_ol_ThreadPool.exe ol_ThreadPool.cpp test_ol_ThreadPool.cpp -std=c++11