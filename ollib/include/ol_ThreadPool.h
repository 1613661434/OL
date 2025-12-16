/****************************************************************************************/
/*
 * 程序名：ol_ThreadPool.h
 * 功能描述：通用线程池模板类的实现，支持以下特性：
 *          - 双模式支持：固定线程数模式（默认）和动态扩缩容模式（通过模板参数控制）
 *          - 任务管理：支持无返回值任务（addTask）和带返回值任务（submitTask）
 *          - 队列策略：任务队列满时可选择拒绝、阻塞等待或超时等待策略
 *          - 线程安全：通过互斥锁和条件变量保证多线程环境下的操作安全性
 *          - 动态特性（当模板参数IsDynamic=true时）：
 *              - 自动根据任务负载扩缩容线程数量（在minThreads和maxThreads范围内）
 *              - 可配置管理者线程检查间隔，平衡响应速度和资源消耗
 * 作者：ol
 * 适用标准：C++17及以上（需支持constexpr if、模板条件类型等特性）
 */
/****************************************************************************************/

#ifndef OL_THREADPOOL_H
#define OL_THREADPOOL_H 1

#include "ol_type_traits.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <utility>

// #define DEBUG

namespace ol
{
    /**
     * @brief 线程池模板类，支持动态/固定两种工作模式
     * @tparam IsDynamic 是否启用动态模式：true为动态扩缩容模式，false为固定线程数模式（默认）
     * @note 动态模式下会根据任务负载自动调整线程数量，固定模式使用初始化时指定的线程数
     * @note 线程安全设计，支持多线程并发添加任务
     * @note 所有线程均通过join模式退出
     */
    template <bool IsDynamic = false>
    class ThreadPool : public TypeNonCopyableMovable
    {
    private:
        // 队列满处理策略
        enum class QueueFullPolicy : char
        {
            kReject, ///< 拒绝新任务
            kBlock,  ///< 阻塞等待
            kTimeout ///< 超时等待
        };

        // 通用成员
        mutable std::mutex m_workersMutex;                                                                                            ///< 保护工作线程集合的互斥锁
        typename std::conditional_t<IsDynamic, std::unordered_map<std::thread::id, std::thread>, std::vector<std::thread>> m_workers; ///< 工作线程集合
        mutable std::mutex m_taskQueueMutex;                                                                                          ///< 保护任务队列的互斥锁
        std::queue<std::function<void()>> m_taskQueue;                                                                                ///< 任务队列
        std::condition_variable m_taskQueueNotEmpty_condVar;                                                                          ///< 任务队列非空条件变量
        std::condition_variable m_taskQueueNotFull_condVar;                                                                           ///< 任务队列非满条件变量
        std::atomic_bool m_stop;                                                                                                      ///< 停止标志
        std::atomic_size_t m_activeWorkers;                                                                                           ///< 追踪活跃工作线程数
        size_t m_maxQueueSize;                                                                                                        ///< 最大队列容量
        QueueFullPolicy m_queueFullPolicy;                                                                                            ///< 队列满策略
        std::chrono::milliseconds m_timeoutMS;                                                                                        ///< 超时时间（毫秒）

        // 动态模式特有成员
        struct DynamicMembers
        {
            size_t minThreads;                              ///< 最小线程数
            size_t maxThreads;                              ///< 最大线程数
            std::atomic_size_t idleThreads;                 ///< 空闲线程数
            std::atomic_size_t workerExitNum;               ///< 工作线程需销毁数
            mutable std::mutex managerMutex;                ///< 管理者线程锁（只是为了事件通知让管理者在睡眠中退出）
            std::condition_variable managerExit_condVar;    ///< 管理者线程退出条件变量
            std::chrono::seconds checkInterval;             ///< 管理者检查间隔（秒）
            std::thread managerThread;                      ///< 管理者线程
            mutable std::mutex workerExitId_dequeMutex;     ///< 保护工作线程退出ID队列的互斥锁
            std::deque<std::thread::id> workerExitId_deque; ///< 工作线程退出ID队列
        };
        typename std::conditional_t<IsDynamic, DynamicMembers, TypeEmpty> m_dynamic; ///< 动态模式成员

    public:
        /**
         * @brief 固定模式构造函数（仅IsDynamic=false时可用）
         * @param threadNum 固定线程数量（必须大于0，否则线程池初始化为停止状态）
         * @param maxQueueSize 任务队列最大容量（0表示无限制，默认0）
         * @note 线程池初始化时会创建指定数量的工作线程
         * @throw 无异常抛出（线程数为0时仅初始化停止状态）
         */
        template <bool D = IsDynamic, typename = std::enable_if_t<!D>>
        ThreadPool(size_t threadNum, size_t maxQueueSize = 0)
            : m_stop(false), m_activeWorkers(0),
              m_maxQueueSize(maxQueueSize),
              m_queueFullPolicy(QueueFullPolicy::kReject), m_timeoutMS(std::chrono::milliseconds(500))
        {
            if (threadNum == 0)
            {
                m_stop = true;
                return;
            }

            // 启动固定数量的工作线程
            m_workers.reserve(threadNum);
            while (threadNum > 0)
            {
                m_activeWorkers.fetch_add(1, std::memory_order_release);
                m_workers.emplace_back(&ThreadPool<IsDynamic>::worker, this);
                --threadNum;
            }
        }

        /**
         * @brief 动态模式构造函数（仅IsDynamic=true时可用）
         * @param minThreadNum 最小线程数（默认0，实际会至少创建1个线程）
         * @param maxThreadNum 最大线程数（默认CPU核心数）
         * @param maxQueueSize 任务队列最大容量（0表示无限制，默认0）
         * @param checkInterval 管理者线程检查间隔（秒，默认1秒）
         * @note 初始化时会创建minThreadNum个线程（若minThreadNum=0则创建1个;若minThreadNum=maxThreadNum=0则线程池初始化为停止状态）
         * @throw std::invalid_argument 当 minThreadNum > maxThreadNum 时抛出
         */
        template <bool D = IsDynamic, typename = std::enable_if_t<D>>
        ThreadPool(size_t minThreadNum = 0,
                   size_t maxThreadNum = std::thread::hardware_concurrency(),
                   size_t maxQueueSize = 0,
                   std::chrono::seconds checkInterval = std::chrono::seconds(1))
            : m_stop(false), m_activeWorkers(0),
              m_maxQueueSize(maxQueueSize),
              m_queueFullPolicy(QueueFullPolicy::kReject), m_timeoutMS(std::chrono::milliseconds(500))
        {
            if (minThreadNum > maxThreadNum)
                throw std::invalid_argument("Invalid thread number range");

            if (minThreadNum == maxThreadNum && minThreadNum == 0)
            {
                m_stop = true;
                return;
            }

            // 初始化动态模式成员
            m_dynamic.minThreads = minThreadNum;
            m_dynamic.maxThreads = maxThreadNum;
            m_dynamic.idleThreads = 0;
            m_dynamic.workerExitNum = 0;
            m_dynamic.checkInterval = checkInterval;

            // 启动最小数量（至少为1）的工作线程
            size_t needThreads = minThreadNum == 0 ? 1 : minThreadNum;

            while (needThreads > 0)
            {
                m_activeWorkers.fetch_add(1, std::memory_order_release);
                std::thread th(&ThreadPool<IsDynamic>::worker, this);
#ifdef DEBUG
                printf("构造函数：新工作线程ID：%zu\n", th.get_id());
#endif
                m_workers.emplace(th.get_id(), std::move(th)); // 移动到哈希表
                --needThreads;
            }

            // 启动管理者线程
            m_dynamic.managerThread = std::thread(&ThreadPool<IsDynamic>::manager<IsDynamic>, this);
#ifdef DEBUG
            printf("构造函数：新管理者线程ID：%zu\n", m_dynamic.managerThread.get_id());
#endif
        }

        /**
         * @brief 析构函数
         * @note 自动调用stop()，等待所有任务完成并清理资源
         */
        ~ThreadPool()
        {
            if (m_stop) return;
            stop(); // 强制join，确保所有线程退出

            // 最终等待活跃线程退出（最多1秒）
            int wait_ms = 0;
            while (m_activeWorkers.load(std::memory_order_acquire) > 0 && wait_ms < 1000)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                wait_ms += 10;
            }
#ifdef DEBUG
            if (m_activeWorkers.load() > 0)
            {
                printf("警告：析构时仍有%d个活跃线程未退出\n", (int)m_activeWorkers.load());
            }
#endif
        }

        /**
         * @brief 停止线程池并清理资源
         * @note 多次调用安全，已停止状态下调用无效果
         * @note 仅支持join模式，等待所有任务完成、所有线程安全退出后返回
         * @warning 确保任务无外部临时资源依赖，避免join等待时出现资源释放竞态
         */
        void stop()
        {
#ifdef DEBUG
            printf("线程池开始stop()\n");
#endif

            // 原子交换，确保仅执行一次stop逻辑 + 内存可见性
            bool expected = false;
            if (!m_stop.compare_exchange_strong(expected, true))
            {
#ifdef DEBUG
                printf("线程池已停止，无需重复操作\n");
#endif
                return;
            }

            // 动态模式：先停止管理者线程（确保其不再修改m_workers）
            if constexpr (IsDynamic)
            {
                // 唤醒管理者线程，使其退出循环
                m_dynamic.managerExit_condVar.notify_one();
                if (m_dynamic.managerThread.joinable())
                {
                    try
                    {
                        auto manager_id = m_dynamic.managerThread.get_id();
                        m_dynamic.managerThread.join();
#ifdef DEBUG
                        printf("动态模式：管理者线程（ID:%zu）已join\n", manager_id);
#endif
                        // 重置管理者线程对象，避免访问无效句柄
                        m_dynamic.managerThread = std::thread();
                    }
                    catch (const std::system_error& e)
                    {
                        fprintf(stderr, "动态模式：管理者线程join失败: %s\n", e.what());
                        // 即使join失败，也重置线程对象
                        m_dynamic.managerThread = std::thread();
                    }
                }

                // 清空工作线程退出队列
                std::lock_guard<std::mutex> lock_exit_deque(m_dynamic.workerExitId_dequeMutex);
                m_dynamic.workerExitId_deque.clear();
#ifdef DEBUG
                printf("动态模式：清空工作线程退出队列\n");
#endif
            }

            // 唤醒所有等待的工作线程
            m_taskQueueNotEmpty_condVar.notify_all();
            m_taskQueueNotFull_condVar.notify_all();

            // 等待活跃线程退出（最多等待5秒，避免死等）
            int wait_ms = 0;
            while (m_activeWorkers.load(std::memory_order_acquire) > 0 && wait_ms < 5000)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                wait_ms += 10;
            }
#ifdef DEBUG
            if (m_activeWorkers.load() > 0)
            {
                printf("警告：仍有%d个活跃线程未退出（已等待%dms）\n",
                       (int)m_activeWorkers.load(), wait_ms);
            }
#endif

            // 处理工作线程
            std::lock_guard<std::mutex> lock(m_workersMutex);
            if constexpr (IsDynamic)
            {
                // 动态模式：哈希表遍历
                for (auto& [id, th] : m_workers)
                {
                    if (th.joinable())
                    {
#ifdef DEBUG
                        printf("动态模式：处理工作线程ID：%zu（强制join）\n", id);
#endif
                        try
                        {
                            th.join();
                        }
                        catch (const std::system_error& e)
                        {
                            fprintf(stderr, "动态模式：线程(ID:%zu)join失败: %s\n", id, e.what());
                        }
                    }
                    else
                    {
#ifdef DEBUG
                        printf("动态模式：线程(ID:%zu)不可join，跳过\n", id);
#endif
                    }
                }
            }
            else
            {
                // 固定模式：向量遍历
                for (auto& th : m_workers)
                {
                    if (th.joinable())
                    {
#ifdef DEBUG
                        printf("固定模式：处理工作线程ID：%zu（强制join）\n", th.get_id());
#endif
                        try
                        {
                            th.join();
                        }
                        catch (const std::system_error& e)
                        {
                            fprintf(stderr, "固定模式：线程join失败: %s\n", e.what());
                        }
                    }
                    else
                    {
#ifdef DEBUG
                        printf("固定模式：线程不可join，跳过\n");
#endif
                    }
                }
            }

            // 清理线程容器
            m_workers.clear();
#ifdef DEBUG
            printf("线程池finish stop()\n");
#endif
        }

        /**
         * @brief 获取当前等待执行的任务数量
         * @return 任务队列中的任务数
         * @note 线程安全，通过互斥锁保护队列访问
         */
        inline size_t getTaskNum() const
        {
            std::lock_guard<std::mutex> lock(m_taskQueueMutex);
            return m_taskQueue.size();
        }

        /**
         * @brief 获取当前工作线程数量
         * @return 工作线程的实时数量
         * @note 线程安全，通过互斥锁保护线程集合访问
         */
        inline size_t getWorkerNum() const
        {
            std::lock_guard<std::mutex> lock(m_workersMutex);
            return m_workers.size();
        }

        /**
         * @brief 动态模式特有：获取当前空闲线程数量
         * @return 空闲线程数
         * @note 仅IsDynamic=true时可用，原子操作确保线程安全
         */
        template <bool D = IsDynamic, typename = std::enable_if_t<D>>
        inline size_t getIdleThreadNum() const
        {
            return m_dynamic.idleThreads;
        }

        /**
         * @brief 设置任务队列满时的拒绝策略（新任务直接被拒绝）
         * @note 线程安全，通过互斥锁保护策略修改
         */
        void setRejectPolicy()
        {
            std::lock_guard<std::mutex> lock(m_taskQueueMutex);
            m_queueFullPolicy = QueueFullPolicy::kReject;
        }

        /**
         * @brief 设置任务队列满时的阻塞策略（等待直到队列有空闲位置）
         * @note 线程安全，通过互斥锁保护策略修改
         */
        void setBlockPolicy()
        {
            std::lock_guard<std::mutex> lock(m_taskQueueMutex);
            m_queueFullPolicy = QueueFullPolicy::kBlock;
        }

        /**
         * @brief 设置任务队列满时的超时等待策略
         * @param timeoutMS 超时时间（毫秒，必须大于0）
         * @throw std::invalid_argument 当timeoutMS <= 0时抛出
         * @note 线程安全，通过互斥锁保护策略和超时时间修改
         */
        void setTimeoutPolicy(std::chrono::milliseconds timeoutMS)
        {
            if (timeoutMS.count() <= 0)
                throw std::invalid_argument("Timeout must be greater than 0");
            std::lock_guard<std::mutex> lock(m_taskQueueMutex);
            m_queueFullPolicy = QueueFullPolicy::kTimeout;
            m_timeoutMS = timeoutMS;
        }

        /**
         * @brief 动态模式特有：设置管理者线程的检查间隔
         * @param interval 检查间隔（秒）
         * @note 仅IsDynamic=true时可用，用于调整扩缩容的响应速度
         */
        template <bool D = IsDynamic, typename = std::enable_if_t<D>>
        void setCheckInterval(std::chrono::seconds interval)
        {
            m_dynamic.checkInterval = interval;
        }

        /**
         * @brief 添加无返回值任务到线程池
         * @param task 待执行的任务（std::function<void()>类型）
         * @return 任务添加成功返回true，失败返回false（线程池已停止或队列满且策略为拒绝/超时）
         * @note 线程安全，根据当前队列策略处理满队列情况
         * @warning 如果任务有异常虽然会将异常输出到错误流，但推荐自己包装一下函数，设置异常处理函数
         */
        bool addTask(std::function<void()> task)
        {
            if (m_stop) return false;

            {
                std::unique_lock<std::mutex> lock(m_taskQueueMutex);

                // 处理队列大小限制
                if (m_maxQueueSize > 0)
                {
                    while (m_taskQueue.size() >= m_maxQueueSize && !m_stop)
                    {
                        switch (m_queueFullPolicy)
                        {
                        case QueueFullPolicy::kReject:
                            return false;
                        case QueueFullPolicy::kBlock:
                            m_taskQueueNotFull_condVar.wait(lock, [this]()
                                                            { return m_taskQueue.size() < m_maxQueueSize || m_stop; });
                            break;
                        case QueueFullPolicy::kTimeout:
                            bool result = m_taskQueueNotFull_condVar.wait_for(lock, m_timeoutMS,
                                                                              [this]()
                                                                              { return m_taskQueue.size() < m_maxQueueSize || m_stop; });
                            if (!result) return false;
                            break;
                        }
                    }
                }

                if (m_stop) return false;
                m_taskQueue.push(std::move(task));
            }

            m_taskQueueNotEmpty_condVar.notify_one();
            return true;
        }

        /**
         * @brief 提交带返回值的任务到线程池
         * @tparam F 任务函数类型
         * @tparam Args 任务函数参数类型
         * @param f 任务函数
         * @param args 任务函数参数
         * @return pair<是否成功添加任务的bool值, 包含任务返回值的std::future对象>
         * @note 若任务添加失败（bool为false），调用future.get()会抛出对应异常（线程池停止/队列满）；
         *       若任务添加成功（bool为true），future.get()会返回任务结果或抛出任务自身的异常。
         * @note 线程安全，内部调用addTask实现任务添加
         */
        template <typename F, typename... Args>
        auto submitTask(F&& f, Args&&... args) -> std::pair<bool, std::future<typename std::invoke_result_t<F, Args...>>>
        {
            using ReturnType = typename std::invoke_result_t<F, Args...>;

            auto task = std::make_shared<std::packaged_task<ReturnType()>>(
                [f = std::forward<F>(f), args = std::make_tuple(std::forward<Args>(args)...)]() mutable
                {
                    return std::apply(std::move(f), std::move(args));
                });

            std::future<ReturnType> result = task->get_future();

            if (!addTask([task]() mutable
                         { (*task)(); }))
            {
                std::promise<ReturnType> promise;
                if (m_stop)
                {
                    promise.set_exception(std::make_exception_ptr(std::runtime_error("ThreadPool has been stopped")));
                    return {false, promise.get_future()};
                }

                switch (m_queueFullPolicy)
                {
                case QueueFullPolicy::kReject:
                    promise.set_exception(std::make_exception_ptr(std::runtime_error("Task queue full (Reject policy)")));
                    return {false, promise.get_future()};
                case QueueFullPolicy::kBlock:
                    // 此时失败一定是因为线程池已停止（否则wait会一直等）
                    promise.set_exception(std::make_exception_ptr(std::runtime_error("Task submission failed in block policy (ThreadPool stopped)")));
                    return {false, promise.get_future()};
                case QueueFullPolicy::kTimeout:
                    promise.set_exception(std::make_exception_ptr(std::runtime_error("Task queue full (Timeout policy)")));
                    return {false, promise.get_future()};
                default:
                    promise.set_exception(std::make_exception_ptr(std::runtime_error("Task submission failed")));
                    return {false, promise.get_future()};
                }
            }

            return {true, std::move(result)};
        }

        /**
         * @brief 检查线程池是否处于运行状态
         * @return 运行中返回true，已停止返回false
         * @note 基于原子变量m_stop的状态判断，线程安全
         */
        bool isRunning() const
        {
            return !m_stop;
        }

    private:
        /**
         * @brief 工作线程主函数
         * @note 循环从任务队列获取并执行任务，直到线程池停止或（动态模式下）收到退出指令
         * @note 动态模式下会维护空闲线程计数，任务执行前后更新状态
         */
        void worker()
        {
            // 动态模式：初始化线程状态
            if constexpr (IsDynamic)
            {
                ++m_dynamic.idleThreads;
            }

            try
            {
                while (!m_stop)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(m_taskQueueMutex);

                        // 等待任务或停止信号
                        auto waitCond = [this]()
                        {
                            if constexpr (IsDynamic)
                                return !m_taskQueue.empty() || m_stop || m_dynamic.workerExitNum > 0;
                            else
                                return !m_taskQueue.empty() || m_stop;
                        };
                        m_taskQueueNotEmpty_condVar.wait(lock, waitCond);

                        if (m_stop) break; // 优先检查停止信号，避免无效操作

                        if constexpr (IsDynamic)
                        {
                            // 原子操作减少workerExitNum，避免竞态
                            if (m_dynamic.workerExitNum.load(std::memory_order_acquire) > 0)
                            {
                                m_dynamic.workerExitNum.fetch_sub(1, std::memory_order_release);
                                break;
                            }
                        }

                        if (m_taskQueue.empty())
                        {
                            lock.unlock();
                            continue;
                        }

                        // 取出任务
                        task = std::move(m_taskQueue.front());
                        m_taskQueue.pop();

                        // 通知可能等待的生产者
                        if (m_queueFullPolicy != QueueFullPolicy::kReject)
                            m_taskQueueNotFull_condVar.notify_one();

                        // 动态模式：更新空闲线程数
                        if constexpr (IsDynamic)
                            --m_dynamic.idleThreads;
                    }

                    // 执行任务
                    try
                    {
                        if (task) task(); // 空任务保护
                    }
                    catch (const std::exception& e)
                    {
                        fprintf(stderr, "Task error: %s\n", e.what());
                    }
                    catch (...)
                    {
                        fprintf(stderr, "Unknown task error\n");
                    }

                    // 动态模式：任务完成，恢复空闲状态
                    if constexpr (IsDynamic)
                    {
                        ++m_dynamic.idleThreads;
                    }
                }
            }
            catch (...)
            {
                fprintf(stderr, "Worker thread(ID:%zu) unexpected exception\n", std::this_thread::get_id());
            }

            // 减少活跃数
            m_activeWorkers.fetch_sub(1, std::memory_order_release);

            // 动态模式：仅当线程池未完全停止时，才添加ID到退出队列
            if constexpr (IsDynamic)
            {
                --m_dynamic.idleThreads;
                // 检查stop标志，避免stop()后仍修改退出队列
                if (!m_stop.load(std::memory_order_acquire))
                {
                    std::unique_lock<std::mutex> lock_exitVector(m_dynamic.workerExitId_dequeMutex);
#ifdef DEBUG
                    printf("线程(ID:%zu)加入退出容器\n", std::this_thread::get_id());
#endif
                    m_dynamic.workerExitId_deque.emplace_back(std::this_thread::get_id());
                }
                else
                {
#ifdef DEBUG
                    printf("线程(ID:%zu)：线程池已停止，跳过加入退出容器\n", std::this_thread::get_id());
#endif
                }
            }

#ifdef DEBUG
            printf("线程已销毁（主动移除，ID:%zu）\n", std::this_thread::get_id());
#endif
        }

        /**
         * @brief 管理者线程主函数（仅动态模式可用）
         * @note 定期执行以下操作：
         *       1. 清理已退出的工作线程对象
         *       2. 根据任务负载和空闲线程数进行扩缩容：
         *          - 扩容：任务数 > 线程数*2 且未达最大线程数时创建新线程
         *          - 缩容：空闲线程 > 线程总数的1/2 且超过最小线程数时销毁多余线程
         */
        template <bool D = IsDynamic, typename = std::enable_if_t<D>>
        void manager()
        {
#ifdef DEBUG
            printf("管理者线程(ID:%zu)启动\n", std::this_thread::get_id());
#endif

            try
            {
                while (!m_stop)
                {
                    // 定期检查（可被stop()唤醒）
                    std::unique_lock<std::mutex> lock_manger(m_dynamic.managerMutex);
                    m_dynamic.managerExit_condVar.wait_for(lock_manger, m_dynamic.checkInterval, [this]()
                                                           { return m_stop.load(std::memory_order_acquire); });
                    if (m_stop) return;

                    // 1. 清理已终止的线程对象
                    {
                        // (1). 先锁定工作线程容器和退出ID队列的锁，确保原子性
                        std::lock_guard<std::mutex> lock_workers(m_workersMutex);
                        std::unique_lock<std::mutex> lock_exitDeque(m_dynamic.workerExitId_dequeMutex);

                        // (2). 复制并清空退出ID队列（避免其他线程并发修改）
                        std::deque<std::thread::id> exitIds;
                        exitIds.swap(m_dynamic.workerExitId_deque); // 用swap避免复制开销
                        lock_exitDeque.unlock();                    // 提前解锁，减少锁持有时间

                        // (3). 逐个清理退出的线程
                        for (const auto& exitId : exitIds)
                        {
#ifdef DEBUG
                            printf("待清理线程ID：%zu\n", exitId);
#endif
                            auto it = m_workers.find(exitId);
                            if (it == m_workers.end())
                            {
#ifdef DEBUG
                                printf("线程ID已被清理，跳过: %zu\n", exitId);
#endif
                                continue;
                            }

                            // (4). 先join线程，再从容器中移除
                            if (it->second.joinable())
                            {
                                try
                                {
                                    it->second.join(); // 确保线程完全退出
#ifdef DEBUG
                                    printf("线程ID已join: %zu\n", exitId);
#endif
                                }
                                catch (const std::exception& e)
                                {
                                    fprintf(stderr, "Worker thread join failure: %s\n", e.what());
                                }
                                catch (...)
                                {
                                    fprintf(stderr, "Unknown Worker thread join error\n");
                                }
                            }

                            m_workers.erase(it); // 移除线程对象
                        }

#ifdef DEBUG
                        printf("线程从容器移除（管理者清理）\n");
#endif
                    }

                    // 2. 扩缩容
                    {
                        std::lock_guard<std::mutex> lock_taskQueue(m_taskQueueMutex);
                        std::lock_guard<std::mutex> lock_workers(m_workersMutex);
                        size_t taskCount = m_taskQueue.size();
                        size_t workerCount = m_workers.size();
                        size_t idleCount = m_dynamic.idleThreads;

                        // (1). 扩容判断：任务数 > 线程数 * 2 且未达最大线程数（乘以2是为了避免轻微负载波动就扩容）
                        if (taskCount > workerCount * 2 && workerCount < m_dynamic.maxThreads)
                        {
                            size_t needThreads = std::min(
                                m_dynamic.maxThreads - workerCount,
                                (taskCount + workerCount - 1) / workerCount // 按当前负载估算需要的线程数（向上取整）
                            );
                            // 每次最多扩容到当前的1.5倍，避免一次性创建过多线程
                            needThreads = std::min(needThreads, workerCount / 2 + 1);

                            while (needThreads > 0)
                            {
                                m_activeWorkers.fetch_add(1, std::memory_order_release);
                                std::thread th(&ThreadPool<IsDynamic>::worker, this);
#ifdef DEBUG
                                printf("新线程（ID: %zu）\n", th.get_id());
#endif
                                m_workers.emplace(th.get_id(), std::move(th)); // 哈希表插入新线程
                                --needThreads;
                            }
// 调试输出：记录扩容操作
#ifdef DEBUG
                            printf("扩容：线程数从 %zu 增加到 %zu（任务数: %zu）\n",
                                   workerCount, m_workers.size(), taskCount);
#endif
                        }
                        // (2). 缩容判断：空闲线程 > 线程总数的1/2 且 线程数超过「最小线程数或1（取较大值）」
                        else if (idleCount > workerCount / 2 && workerCount > std::max(m_dynamic.minThreads, static_cast<size_t>(1)))
                        {
                            // 缩容下限：最多缩减到「最小线程数或1（取较大值）」
                            size_t minKeep = std::max(m_dynamic.minThreads, static_cast<size_t>(1));

                            // 实际缩减数 = 取「可缩减线程数」和「多余空闲线程数」的较小值
                            size_t reduceThreads = std::min(
                                workerCount - minKeep,        // 可缩减线程数 = 当前线程数 - 最低保留数
                                idleCount - (workerCount / 2) // 只销毁超过一半的空闲线程
                            );

#ifdef DEBUG
                            size_t reduceThreads_temp = reduceThreads;
#endif

                            // 销毁线程
                            if (reduceThreads > 0)
                            {
                                m_dynamic.workerExitNum = reduceThreads;
                                do
                                {
                                    m_taskQueueNotEmpty_condVar.notify_one();
                                    --reduceThreads;
                                } while (reduceThreads > 0);
                            }
#ifdef DEBUG
                            printf("缩容：计划销毁 %zu 个线程（当前线程数: %zu, 空闲数: %zu, 保留至少: %zu）\n",
                                   reduceThreads_temp, workerCount, idleCount, minKeep);
#endif
                        }
                    }
                }
            }
            catch (...)
            {
                fprintf(stderr, "Manager thread(ID:%zu) unexpected exception\n", std::this_thread::get_id());
            }

            // 管理者线程退出前：清空退出队列，避免残留ID
            std::lock_guard<std::mutex> lock_exit_deque(m_dynamic.workerExitId_dequeMutex);
            m_dynamic.workerExitId_deque.clear();
#ifdef DEBUG
            printf("管理者线程(ID:%zu)退出，清空退出队列\n", std::this_thread::get_id());
#endif
        }
    };
} // namespace ol

#endif // !OL_THREADPOOL_H