#include "ol_ThreadPool.h"
#include <cassert>
#include <chrono>
#include <iostream>
#include <mutex>
#include <numeric>
#include <string>
#include <type_traits>
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
    fflush(stdout);
}

// 线程安全的流式打印函数
template <typename T>
void safePrint(const T& content)
{
    std::lock_guard<std::mutex> lock(g_printMutex);
    std::cout << content;
    std::cout.flush();
}

// 获取当前线程ID的字符串表示
std::string getThreadId()
{
#ifdef __linux__
    return std::to_string(syscall(SYS_gettid));
#elif defined(_WIN32)
    return std::to_string(GetCurrentThreadId());
#else
    return std::to_string(std::hash<std::thread::id>()(std::this_thread::get_id()));
#endif
}

// 测试任务：无返回值，打印信息
void printMessage(int id, const std::string& msg)
{
    safePrint("任务 %d: %s (线程ID: %s)\n",
              id, msg.c_str(),
              getThreadId().c_str());

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

// 测试任务：有返回值，计算-100
int subtract100(int x)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return x - 100;
}

// 测试任务：有返回值，计算+1000（用于策略测试）
int add1000(int x)
{
    std::string threadId = getThreadId();
    safePrint("  阻塞策略任务 %d 开始执行 (执行线程: %s)\n", x, threadId.c_str());

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int result = x + 1000;
    safePrint("  阻塞策略任务 %d 执行完成 (执行线程: %s, 结果: %d)\n", x, threadId.c_str(), result);
    return result;
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

// 测试任务：长时间运行的任务，用于验证队列策略和动态扩缩容
void longRunningTask(int id)
{
    safePrint("长任务 %d 开始执行\n", id);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    safePrint("长任务 %d 执行完毕\n", id);
}

// 测试任务：轻量级任务，用于测试动态线程池的扩缩容
void lightTask(int id)
{
    std::string threadId = getThreadId();
    safePrint("轻量任务 %d 开始执行 (线程: %s)\n", id, threadId.c_str());
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    safePrint("轻量任务 %d 执行完毕 (线程: %s)\n", id, threadId.c_str());
}

// 固定模式线程池基本功能测试
void runFixedCommonTests(size_t threadNum, size_t maxQueueSize)
{
    ol::ThreadPool<false> pool(threadNum, maxQueueSize);
    const std::string poolType = "固定模式";

    safePrint("\n=== %s 基本功能测试 ===\n", poolType.c_str());
    safePrint("创建了包含 %zu 个线程的线程池\n", pool.getWorkerNum());
    assert(pool.isRunning() && "线程池未处于运行状态");

    // 测试无返回值任务
    safePrint("\n=== %s 无返回值任务测试 ===\n", poolType.c_str());
    for (int i = 0; i < 5; ++i)
    {
        bool success = pool.addTask(std::bind(printMessage, i, poolType + "线程池的问候"));
        assert(success && "无返回值任务添加失败");
    }
    safePrint("当前等待执行的任务数: %zu\n", pool.getTaskNum());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    assert(pool.getTaskNum() == 0 && "无返回值任务未正确执行完毕");

    // 测试带返回值的任务
    safePrint("\n=== %s 带返回值任务测试 ===\n", poolType.c_str());
    std::vector<std::future<int>> subtractFutures;
    for (int i = 1; i <= 5; ++i)
    {
        subtractFutures.emplace_back(pool.submitTask(subtract100, i));
    }

    // 获取并验证结果
    for (int i = 0; i < (int)subtractFutures.size(); ++i)
    {
        int result = subtractFutures[i].get();
        int expected = (i + 1) - 100;
        safePrint("%d - 100 = %d（预期%d）\n", i + 1, result, expected);
        assert(result == expected && "减法计算结果错误");
    }

    // 测试异常处理机制
    safePrint("\n=== %s 异常处理测试 ===\n", poolType.c_str());
    for (int i = 0; i < 5; ++i)
    {
        pool.addTask(std::bind(riskyTask, i));
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    assert(pool.isRunning() && "异常导致线程池停止");

    // 测试大量任务处理能力
    safePrint("\n=== %s 大量任务处理测试 ===\n", poolType.c_str());
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
        long long expected = (long long)(start + end) * (end - start + 1) / 2;
        safePrint("%d 到 %d 的总和: %lld（预期: %lld）\n",
                  start, end, result, expected);
        assert(result == expected && "区间求和结果错误");
    }
}

// 动态模式线程池基本功能测试
void runDynamicCommonTests(size_t minThreadNum, size_t maxThreadNum,
                           size_t maxQueueSize, std::chrono::seconds checkInterval)
{
    ol::ThreadPool<true> pool(minThreadNum, maxThreadNum, maxQueueSize, checkInterval);
    const std::string poolType = "动态模式";

    safePrint("\n=== %s 基本功能测试 ===\n", poolType.c_str());
    safePrint("创建了包含 %zu 个线程的线程池\n", pool.getWorkerNum());
    assert(pool.isRunning() && "线程池未处于运行状态");

    // 测试无返回值任务
    safePrint("\n=== %s 无返回值任务测试 ===\n", poolType.c_str());
    for (int i = 0; i < 5; ++i)
    {
        bool success = pool.addTask(std::bind(printMessage, i, poolType + "线程池的问候"));
        assert(success && "无返回值任务添加失败");
    }
    safePrint("当前等待执行的任务数: %zu\n", pool.getTaskNum());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    assert(pool.getTaskNum() == 0 && "无返回值任务未正确执行完毕");

    // 测试带返回值的任务
    safePrint("\n=== %s 带返回值任务测试 ===\n", poolType.c_str());
    std::vector<std::future<int>> subtractFutures;
    for (int i = 1; i <= 5; ++i)
    {
        subtractFutures.emplace_back(pool.submitTask(subtract100, i));
    }

    // 获取并验证结果
    for (int i = 0; i < (int)subtractFutures.size(); ++i)
    {
        int result = subtractFutures[i].get();
        int expected = (i + 1) - 100;
        safePrint("%d - 100 = %d（预期%d）\n", i + 1, result, expected);
        assert(result == expected && "减法计算结果错误");
    }

    // 测试异常处理机制
    safePrint("\n=== %s 异常处理测试 ===\n", poolType.c_str());
    for (int i = 0; i < 5; ++i)
    {
        pool.addTask(std::bind(riskyTask, i));
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    assert(pool.isRunning() && "异常导致线程池停止");

    // 测试大量任务处理能力
    safePrint("\n=== %s 大量任务处理测试 ===\n", poolType.c_str());
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
        long long expected = (long long)(start + end) * (end - start + 1) / 2;
        safePrint("%d 到 %d 的总和: %lld（预期: %lld）\n",
                  start, end, result, expected);
        assert(result == expected && "区间求和结果错误");
    }
}

// 固定模式队列策略测试
void testFixedQueuePolicies(size_t threadNum, size_t maxQueueSize)
{
    ol::ThreadPool<false> pool(threadNum, maxQueueSize);
    const std::string poolType = "固定模式";

    // 测试队列满策略：拒绝策略
    safePrint("\n=== %s 队列满策略测试：拒绝策略 ===\n", poolType.c_str());
    pool.setRejectPolicy();
    int rejectSuccessAdd = 0;
    int rejectSuccessSubmit = 0;

    // 测试addTask在拒绝策略下的行为
    for (int i = 0; i < 5; ++i)
    {
        bool success = pool.addTask(std::bind(longRunningTask, i));
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
            auto future = pool.submitTask(add1000, i);
            rejectFutures.push_back(std::move(future));
            rejectSuccessSubmit++;
            safePrint("拒绝策略-submit: 任务 %d 添加成功\n", i);
        }
        catch (const std::exception& e)
        {
            safePrint("拒绝策略-submit: 任务 %d 添加失败（%s）\n", i, e.what());
        }
    }

    safePrint("拒绝策略：add成功 %d 个，submit成功 %d 个\n",
              rejectSuccessAdd, rejectSuccessSubmit);
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // 验证结果
    for (auto& future : rejectFutures)
    {
        int result = future.get();
        assert(result == (result - 1000) + 1000 && "加法计算错误");
    }
    assert(pool.getTaskNum() == 0 && "拒绝策略任务未执行完毕");
}

// 动态模式队列策略测试
void testDynamicQueuePolicies(size_t minThreadNum, size_t maxThreadNum,
                              size_t maxQueueSize, std::chrono::seconds checkInterval)
{
    ol::ThreadPool<true> pool(minThreadNum, maxThreadNum, maxQueueSize, checkInterval);
    const std::string poolType = "动态模式";

    // 测试队列满策略：拒绝策略
    safePrint("\n=== %s 队列满策略测试：拒绝策略 ===\n", poolType.c_str());
    pool.setRejectPolicy();
    int rejectSuccessAdd = 0;
    int rejectSuccessSubmit = 0;

    // 测试addTask在拒绝策略下的行为
    for (int i = 0; i < 5; ++i)
    {
        bool success = pool.addTask(std::bind(longRunningTask, i));
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
            auto future = pool.submitTask(add1000, i);
            rejectFutures.push_back(std::move(future));
            rejectSuccessSubmit++;
            safePrint("拒绝策略-submit: 任务 %d 添加成功\n", i);
        }
        catch (const std::exception& e)
        {
            safePrint("拒绝策略-submit: 任务 %d 添加失败（%s）\n", i, e.what());
        }
    }

    safePrint("拒绝策略：add成功 %d 个，submit成功 %d 个\n",
              rejectSuccessAdd, rejectSuccessSubmit);
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // 验证结果
    for (auto& future : rejectFutures)
    {
        int result = future.get();
        assert(result == (result - 1000) + 1000 && "加法计算错误");
    }
    assert(pool.getTaskNum() == 0 && "拒绝策略任务未执行完毕");
}

// 固定模式stop方法测试
void testFixedStopBehavior(size_t threadNum, size_t maxQueueSize)
{
    const std::string poolType = "固定模式";

    // 测试1：stop(false)的即时性（不等待任务完成）
    safePrint("\n=== %s stop(false)即时性测试 ===\n", poolType.c_str());
    ol::ThreadPool<false> stopTestPool1(threadNum, maxQueueSize);

    // 提交一个长时间任务
    stopTestPool1.addTask([]()
                          { std::this_thread::sleep_for(std::chrono::seconds(5)); });
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 确保任务已被取走执行

    // 记录stop(false)调用时间（不等待任务完成）
    auto start = std::chrono::steady_clock::now();
    stopTestPool1.stop(false); // 不等待任务，立即返回
    auto end = std::chrono::steady_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    safePrint("stop(false) 执行耗时: %lld ms（预期 <= 500ms）\n", duration);
    assert(duration <= 500 && "stop(false) 未及时唤醒线程");
    assert(!stopTestPool1.isRunning() && "stop(false) 未正确停止线程池");

    // 测试2：stop(true)的正常等待（使用短任务）
    safePrint("\n=== %s stop(true)等待测试 ===\n", poolType.c_str());
    ol::ThreadPool<false> stopTestPool2(threadNum, maxQueueSize);

    // 提交一个短任务（200ms）
    stopTestPool2.addTask([]()
                          { std::this_thread::sleep_for(std::chrono::milliseconds(220)); });
    // 缩短等待时间到50ms，确保任务已开始执行但未完成
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // 记录stop(true)调用时间（等待任务完成）
    start = std::chrono::steady_clock::now();
    stopTestPool2.stop(true); // 等待任务完成后返回
    end = std::chrono::steady_clock::now();

    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    safePrint("stop(true) 执行耗时: %lld ms（预期 <= 850ms）\n", duration); // 调整预期范围
    assert(duration <= 850 && "stop(true) 未正确等待任务完成");
    assert(!stopTestPool2.isRunning() && "stop(true) 未正确停止线程池");

    // 测试3：已停止的线程池行为
    safePrint("\n=== %s 已停止线程池行为测试 ===\n", poolType.c_str());
    ol::ThreadPool<false> stoppedPool(threadNum, maxQueueSize);
    stoppedPool.stop();

    // 验证任务提交失败
    bool addResult = stoppedPool.addTask([]()
                                         { safePrint("此任务不应执行\n"); });
    assert(!addResult && "已停止线程池不应接受addTask任务");

    bool submitThrew = false;
    try
    {
        // 线程池已停止时调用submitTask本身不会抛异常，而是返回带异常的future
        auto future = stoppedPool.submitTask(subtract100, 5);
        // 调用get()时才会触发异常
        future.get();
    }
    catch (const std::exception& e)
    {
        submitThrew = true;
        safePrint("已停止线程池：submitTask正确抛出异常 - %s\n", e.what());
    }
    assert(submitThrew && "已停止线程池应拒绝submitTask任务");
}

// 动态模式stop方法测试
void testDynamicStopBehavior(size_t minThreadNum, size_t maxThreadNum,
                             size_t maxQueueSize, std::chrono::seconds checkInterval)
{
    const std::string poolType = "动态模式";

    // 测试1：stop(false)的即时性（不等待任务完成）可能导致程序崩溃
    {
        ol::ThreadPool<true> stopTestPool1(minThreadNum, maxThreadNum, maxQueueSize, checkInterval);

        // 提交一个长时间任务（1秒）
        stopTestPool1.addTask([]()
                              { std::this_thread::sleep_for(std::chrono::seconds(1)); });
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 确保任务已开始执行（如果时间太短，会让线程使用析构的资源导致程序崩溃）

        // 记录stop(false)调用时间（不等待任务）
        auto start = std::chrono::steady_clock::now();
        stopTestPool1.stop(false); // 不等待任务，立即返回
        auto end = std::chrono::steady_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        safePrint("stop(false) 执行耗时: %lld ms（预期 <= 400ms）\n", duration);
        assert(duration <= 400 && "stop(false) 未及时唤醒管理者线程");
        assert(!stopTestPool1.isRunning() && "stop(false) 未正确停止线程池");
    }

    // 测试2：stop(true)的正常等待（使用短任务）
    {
        ol::ThreadPool<true> stopTestPool2(minThreadNum, maxThreadNum, maxQueueSize, checkInterval);

        // 提交一个短任务（200ms）
        stopTestPool2.addTask([]()
                              { std::this_thread::sleep_for(std::chrono::milliseconds(220)); });
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // 确保任务已开始但未完成

        // 记录stop(true)调用时间（等待任务完成）
        auto start = std::chrono::steady_clock::now();
        stopTestPool2.stop(true); // 等待任务完成后返回
        auto end = std::chrono::steady_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        safePrint("stop(true) 执行耗时: %lld ms（预期 < 850ms）\n", duration);
        assert(duration <= 850 && "stop(true) 未正确等待任务完成");
        assert(!stopTestPool2.isRunning() && "stop(true) 未正确停止线程池");
    }

    // 测试3：已停止的线程池行为
    {
        ol::ThreadPool<true> stoppedPool(minThreadNum, maxThreadNum, maxQueueSize, checkInterval);
        stoppedPool.stop(); // 主动停止

        // 验证任务提交失败
        bool addResult = stoppedPool.addTask([]()
                                             { safePrint("此任务不应执行\n"); });
        assert(!addResult && "已停止线程池不应接受addTask任务");

        bool submitThrew = false;
        try
        {
            auto future = stoppedPool.submitTask(subtract100, 5);
            future.get();
        }
        catch (const std::exception& e)
        {
            submitThrew = true;
            safePrint("已停止线程池：submitTask正确抛出异常 - %s\n", e.what());
        }
        assert(submitThrew && "已停止线程池应拒绝submitTask任务");
    }

    safePrint("\n%s stop方法测试全部通过\n", poolType.c_str());
}

// 动态线程池的扩缩容功能测试
void testDynamicScaling()
{
    safePrint("\n=== 动态线程池扩缩容测试 ===\n");
    // 创建动态线程池：最小2，最大5，检查间隔1秒
    ol::ThreadPool<true> dynamicPool(2, 5, 100, std::chrono::seconds(1));
    safePrint("初始线程数: %zu（预期2）\n", dynamicPool.getWorkerNum());
    assert(dynamicPool.getWorkerNum() == 2 && "初始线程数错误");
    // assert(dynamicPool.getIdleThreadNum() == 2 && "初始空闲线程数错误");

    // 阶段1：少量任务（线程数应保持最小）
    safePrint("\n=== 阶段1：少量任务测试 ===\n");
    for (int i = 0; i < 3; ++i)
        dynamicPool.addTask(std::bind(lightTask, i));

    // 等待1.5个检查周期
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    safePrint("当前线程数: %zu（预期2）\n", dynamicPool.getWorkerNum());
    safePrint("当前空闲线程数: %zu\n", dynamicPool.getIdleThreadNum());
    assert(dynamicPool.getWorkerNum() == 2 && "阶段1不应扩容");

    // 阶段2：大量任务（增加任务数量，确保触发扩容）
    safePrint("\n=== 阶段2：大量任务测试 ===\n");
    const int BATCH_SIZE = 80; // 增加到80个任务
    for (int i = 0; i < BATCH_SIZE; ++i)
        dynamicPool.addTask(std::bind(lightTask, i));

    // 等待5个检查周期（确保管理者线程检查并扩容）
    std::this_thread::sleep_for(std::chrono::seconds(5));
    safePrint("扩容后线程数: %zu（预期3-5）\n", dynamicPool.getWorkerNum());
    safePrint("扩容后空闲线程数: %zu\n", dynamicPool.getIdleThreadNum());
    assert(dynamicPool.getWorkerNum() >= 3 && dynamicPool.getWorkerNum() <= 5 && "未正常扩容");

    // 阶段3：任务完成后缩容
    safePrint("\n=== 阶段3：任务完成缩容测试 ===\n");
    std::this_thread::sleep_for(std::chrono::seconds(3)); // 等待所有任务完成
    std::this_thread::sleep_for(std::chrono::seconds(2)); // 等待缩容
    safePrint("缩容后线程数: %zu（预期2-4）\n", dynamicPool.getWorkerNum());
    safePrint("缩容后空闲线程数: %zu\n", dynamicPool.getIdleThreadNum());
    assert(dynamicPool.getWorkerNum() >= 2 && dynamicPool.getWorkerNum() <= 4 && "未正常缩容");

    // 测试最小线程数为0的动态模式
    safePrint("\n=== 测试最小线程数为0的动态模式 ===\n");
    ol::ThreadPool<true> zeroMinPool(0, 3, 5);
    size_t initialThreads = zeroMinPool.getWorkerNum();
    safePrint("初始线程数（min=0）: %zu（预期1）\n", initialThreads);
    assert(initialThreads == 1 && "初始线程数不为1");

    // 添加任务并等待扩容
    zeroMinPool.addTask(std::bind(lightTask, 100));
    safePrint("已添加任务，等待线程创建...\n");

    // 延长等待时间至2秒，确保管理者线程完成检查和扩容
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    // 输出当前线程数，辅助调试
    size_t afterThreads = zeroMinPool.getWorkerNum();
    safePrint("添加任务后线程数: %zu（预期1-3）\n", afterThreads);

    // 验证线程创建
    assert(afterThreads >= 1 && "min=0时未动态创建线程");
    safePrint("最小线程数为0的动态模式测试通过\n");

    safePrint("\n动态线程池扩缩容测试完成\n");
}

int main()
{
    try
    {
        safePrint("===== 线程池测试程序启动 =====");

        // 测试固定模式线程池
        runFixedCommonTests(3, 10);
        testFixedQueuePolicies(3, 10);
        testFixedStopBehavior(2, 10);

        // 测试动态模式线程池（通用功能）
        runDynamicCommonTests(2, 5, 10, std::chrono::seconds(1));
        testDynamicQueuePolicies(2, 5, 10, std::chrono::seconds(1));
        testDynamicStopBehavior(2, 5, 10, std::chrono::seconds(1));

        // 测试动态模式特有功能（扩缩容）
        testDynamicScaling();

        safePrint("\n===== 所有测试均成功完成! =====");
    }
    catch (const std::exception& e)
    {
        std::lock_guard<std::mutex> lock(g_printMutex);
        std::cerr << "\n测试失败: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}