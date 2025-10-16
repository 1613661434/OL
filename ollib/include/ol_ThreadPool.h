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
 *          - 灵活停止：支持等待任务完成（join）或立即返回（detach）两种停止模式
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

// #define DEBUG

namespace ol
{
    /**
     * @brief 线程池模板类，支持动态/固定模式
     * @tparam IsDynamic 是否为动态线程池（默认false为固定模式）
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
        std::mutex m_taskQueueMutex;                                                                                                  ///< 保护任务队列的互斥锁
        std::queue<std::function<void()>> m_taskQueue;                                                                                ///< 任务队列
        std::condition_variable m_taskQueueNotEmpty_condVar;                                                                          ///< 任务队列非空条件变量
        std::condition_variable m_taskQueueNotFull_condVar;                                                                           ///< 任务队列非满条件变量
        std::atomic_bool m_stop;                                                                                                      ///< 停止标志
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
            std::mutex managerMutex;                        ///< 管理者线程锁（只是为了事件通知让管理者在睡眠中退出）
            std::condition_variable managerExit_condVar;    ///< 管理者线程退出条件变量
            std::chrono::seconds checkInterval;             ///< 管理者检查间隔（秒）
            std::thread managerThread;                      ///< 管理者线程
            std::mutex workerExitId_dequeMutex;             ///< 保护工作线程退出ID队列的互斥锁
            std::deque<std::thread::id> workerExitId_deque; ///< 工作线程退出ID队列
        };
        typename std::conditional_t<IsDynamic, DynamicMembers, TypeEmpty> m_dynamic; ///< 动态模式成员

    public:
        /**
         * @brief 固定模式构造函数（仅IsDynamic=false时可用）
         * @param threadNum 固定线程数量
         * @param maxQueueSize 最大队列容量（0表示无限制）
         */
        template <bool D = IsDynamic, typename = std::enable_if_t<!D>>
        ThreadPool(size_t threadNum, size_t maxQueueSize = 0)
            : m_stop(false), m_maxQueueSize(maxQueueSize),
              m_queueFullPolicy(QueueFullPolicy::kReject), m_timeoutMS(std::chrono::milliseconds(500))
        {
            static_assert(!IsDynamic, "Fixed constructor only for IsDynamic = false");
            if (threadNum == 0)
            {
                m_stop = true;
                return;
            }

            // 启动固定数量的工作线程
            m_workers.reserve(threadNum);
            while (threadNum > 0)
            {
                m_workers.emplace_back(&ThreadPool::worker, this);
                --threadNum;
            }
        }

        /**
         * @brief 动态模式构造函数（仅IsDynamic=true时可用）
         * @param minThreadNum 最小线程数（默认：0）
         * @param maxThreadNum 最大线程数（默认：CPU核心数）
         * @param maxQueueSize 最大队列容量（0表示无限制）
         * @param checkInterval 管理者检查间隔（秒，默认：1）
         */
        template <bool D = IsDynamic, typename = std::enable_if_t<D>>
        ThreadPool(size_t minThreadNum = 0,
                   size_t maxThreadNum = std::thread::hardware_concurrency(),
                   size_t maxQueueSize = 0,
                   std::chrono::seconds checkInterval = std::chrono::seconds(1))
            : m_stop(false), m_maxQueueSize(maxQueueSize),
              m_queueFullPolicy(QueueFullPolicy::kReject), m_timeoutMS(std::chrono::milliseconds(500))
        {
            static_assert(IsDynamic, "Dynamic constructor only for IsDynamic=true");
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
                std::thread th(&ThreadPool::worker, this);
#ifdef DEBUG
                printf("构造函数：新工作线程ID：%zu\n", th.get_id());
#endif
                m_workers.emplace(th.get_id(), std::move(th)); // 移动到哈希表
                --needThreads;
            }

            // 启动管理者线程
            m_dynamic.managerThread = std::thread(&ThreadPool::manager, this);
#ifdef DEBUG
            printf("构造函数：新管理者线程ID：%zu\n", m_dynamic.managerThread.get_id());
#endif
        }

        /**
         * @brief 析构函数
         */
        ~ThreadPool()
        {
            if (m_stop) return;
            stop(true);
        }

        /**
         * @brief 停止线程池并清理资源
         * @param wait 是否等待线程完成当前任务（true: join等待，false: detach立即返回）
         * @note 多次调用安全，析构时自动调用stop(true)
         * @warning wait = false时，分离的线程若访问已释放资源可能导致崩溃（确保任务完成再进行stop(false)）
         */
        void stop(bool wait = true)
        {
#ifdef DEBUG
            printf("线程池开始stop()\n");
#endif

            if (m_stop) return;

            m_stop = true;

            // 动态模式：停止管理者线程
            if constexpr (IsDynamic)
            {
                m_dynamic.managerExit_condVar.notify_one(); // 发送唤醒信号
                if (m_dynamic.managerThread.joinable())
                    m_dynamic.managerThread.join();
            }

            m_taskQueueNotEmpty_condVar.notify_all();
            m_taskQueueNotFull_condVar.notify_all();

            // 给线程退出的时间
            std::this_thread::sleep_for(std::chrono::milliseconds(200));

            // 处理工作线程
            std::lock_guard<std::mutex> lock(m_workersMutex);
            if constexpr (IsDynamic)
            {
                // 动态模式：哈希表遍历
                for (auto& [id, th] : m_workers)
                {
                    if (th.joinable())
                    {
                        if (wait) // 等待模式：join
                            th.join();
                        else // 不等待模式：detach
                            th.detach();
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
                        if (wait) // 等待模式：join
                            th.join();
                        else // 不等待模式：detach
                            th.detach();
                    }
                }
            }
            m_workers.clear(); // 清理线程容器
#ifdef DEBUG
            printf("线程池finish stop()\n");
#endif
        }

        /**
         * @brief 获取等待任务数量
         */
        inline size_t getTaskNum()
        {
            std::lock_guard<std::mutex> lock(m_taskQueueMutex);
            return m_taskQueue.size();
        }

        /**
         * @brief 获取当前线程数量（有非常大的延迟，不推荐作为依据）
         */
        inline size_t getWorkerNum() const
        {
            std::lock_guard<std::mutex> lock(m_workersMutex);
            return m_workers.size();
        }

        /**
         * @brief 设置队列策略
         */
        void setRejectPolicy()
        {
            std::lock_guard<std::mutex> lock(m_taskQueueMutex);
            m_queueFullPolicy = QueueFullPolicy::kReject;
        }

        void setBlockPolicy()
        {
            std::lock_guard<std::mutex> lock(m_taskQueueMutex);
            m_queueFullPolicy = QueueFullPolicy::kBlock;
        }

        void setTimeoutPolicy(std::chrono::milliseconds timeoutMS)
        {
            if (timeoutMS.count() <= 0)
                throw std::invalid_argument("Timeout must be greater than 0");
            std::lock_guard<std::mutex> lock(m_taskQueueMutex);
            m_queueFullPolicy = QueueFullPolicy::kTimeout;
            m_timeoutMS = timeoutMS;
        }

        /**
         * @brief 设置管理者检查间隔
         * @param interval
         */
        template <bool D = IsDynamic, typename = std::enable_if_t<D>>
        void setCheckInterval(std::chrono::seconds interval)
        {
            m_dynamic.checkInterval = interval;
        }

        /**
         * @brief 添加无返回值任务
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
         * @brief 提交带返回值任务
         */
        template <typename F, typename... Args>
        auto submitTask(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
        {
            using ReturnType = typename std::result_of<F(Args...)>::type;

            auto task = std::make_shared<std::packaged_task<ReturnType()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));

            std::future<ReturnType> result = task->get_future();

            bool success = addTask([task]()
                                   { (*task)(); });
            if (!success)
            {
                std::promise<ReturnType> promise;
                if (m_stop)
                {
                    promise.set_exception(std::make_exception_ptr(std::runtime_error("ThreadPool has been stopped")));
                    return promise.get_future();
                }

                switch (m_queueFullPolicy)
                {
                case QueueFullPolicy::kReject:
                    promise.set_exception(std::make_exception_ptr(std::runtime_error("Task queue full (Reject policy)")));
                    return promise.get_future();
                case QueueFullPolicy::kBlock:
                    // 此时失败一定是因为线程池已停止（否则wait会一直等）
                    promise.set_exception(std::make_exception_ptr(std::runtime_error("Task submission failed in block policy (ThreadPool stopped)")));
                    return promise.get_future();
                case QueueFullPolicy::kTimeout:
                    promise.set_exception(std::make_exception_ptr(std::runtime_error("Task queue full (Timeout policy)")));
                    return promise.get_future();
                default:
                    promise.set_exception(std::make_exception_ptr(std::runtime_error("Task submission failed")));
                    return promise.get_future();
                }
            }

            return result;
        }

        /**
         * @brief 检查线程池是否运行中
         */
        bool isRunning() const
        {
            return !m_stop;
        }

        /**
         * @brief 动态模式特有：获取当前空闲线程数
         */
        template <bool D = IsDynamic, typename = std::enable_if_t<D>>
        size_t getIdleThreadNum() const
        {
            return m_dynamic.idleThreads;
        }

    private:
        /**
         * @brief 工作线程主函数
         */
        void worker()
        {
            // 动态模式：初始化线程状态
            if constexpr (IsDynamic)
            {
                ++m_dynamic.idleThreads;
            }

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

                    if constexpr (IsDynamic)
                    {
                        if (m_dynamic.workerExitNum > 0)
                        {
                            --m_dynamic.workerExitNum;
                            break;
                        }
                    }

                    if (m_taskQueue.empty())
                    {
                        if (m_stop) return;
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
                    task();
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

            // 动态模式：线程退出，更新空闲数，把线程ID加入退出队列
            if constexpr (IsDynamic)
            {
                --m_dynamic.idleThreads;
                std::unique_lock<std::mutex> lock_exitVector(m_dynamic.workerExitId_dequeMutex);
#ifdef DEBUG
                printf("线程(ID:%zu)加入退出容器\n", std::this_thread::get_id());
#endif
                m_dynamic.workerExitId_deque.emplace_back(std::this_thread::get_id());
            }

#ifdef DEBUG
            printf("线程已销毁（主动移除，ID:%zu）\n", std::this_thread::get_id());
#endif
        }

        /**
         * @brief 管理者线程主函数（动态模式特有）
         * 定期检查并调整线程数量，实现平滑扩缩容
         */
        template <bool D = IsDynamic, typename = std::enable_if_t<D>>
        void manager()
        {
            while (!m_stop)
            {
                // 定期检查（可被stop()唤醒）
                std::unique_lock<std::mutex> lock_manger(m_dynamic.managerMutex);
                m_dynamic.managerExit_condVar.wait_for(lock_manger, m_dynamic.checkInterval, [this]
                                                       { return m_stop.load(); });
                if (m_stop) return;

                // 1. 清理已终止的线程对象
                {
                    // 1. 先锁定工作线程容器和退出ID队列的锁，确保原子性
                    std::lock_guard<std::mutex> lock_workers(m_workersMutex);
                    std::unique_lock<std::mutex> lock_exitDeque(m_dynamic.workerExitId_dequeMutex);

                    // 2. 复制并清空退出ID队列（避免其他线程并发修改）
                    std::deque<std::thread::id> exitIds;
                    exitIds.swap(m_dynamic.workerExitId_deque); // 用swap避免复制开销
                    lock_exitDeque.unlock();                    // 提前解锁，减少锁持有时间

                    // 3. 逐个清理退出的线程
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

                        // 4. 先join线程，再从容器中移除
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

                    // (1). 扩容判断：任务数 > 线程数 * 2 且未达最大线程数
                    //    （乘以2是为了避免轻微负载波动就扩容）
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
                            std::thread th(&ThreadPool::worker, this);
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
    };

} // namespace ol

#endif // !OL_THREADPOOL_H