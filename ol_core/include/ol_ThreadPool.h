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
#include <cinttypes>
#include <queue>
#include <deque>
#include <functional>
#include <future>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <utility>

#ifdef __unix__
#include <sys/syscall.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

// #define OL_DEBUG

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
    public:
        enum class State : char
        {
            Running,    ///< 接受并执行任务。
            Draining,   ///< 拒绝新任务，执行完已排队任务后退出。
            Cancelling, ///< 拒绝新任务，丢弃已排队任务，等待正在执行的任务结束。
            Stopped     ///< 全部内部线程已经join。
        };

        enum class ShutdownMode : char
        {
            Drain,
            CancelPending
        };

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
        mutable std::mutex m_waitMutex;                                                                                               ///< 保证只有一个外部线程执行join。
        mutable std::mutex m_taskQueueMutex;                                                                                          ///< 保护任务队列的互斥锁
        std::queue<std::function<void()>> m_taskQueue;                                                                                ///< 任务队列
        std::condition_variable m_taskQueueNotEmpty_condVar;                                                                          ///< 任务队列非空条件变量
        std::condition_variable m_taskQueueNotFull_condVar;                                                                           ///< 任务队列非满条件变量
        std::atomic<State> m_state;                                                                                                   ///< 当前运行/关闭状态。
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

        inline static thread_local ThreadPool* s_currentThreadPool = nullptr;

        class ThreadContextGuard
        {
        private:
            ThreadPool* m_previous;

        public:
            explicit ThreadContextGuard(ThreadPool* pool)
                : m_previous(s_currentThreadPool)
            {
                s_currentThreadPool = pool;
            }

            ~ThreadContextGuard()
            {
                s_currentThreadPool = m_previous;
            }
        };

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
            : m_state(State::Running), m_activeWorkers(0),
              m_maxQueueSize(maxQueueSize),
              m_queueFullPolicy(QueueFullPolicy::kReject), m_timeoutMS(std::chrono::milliseconds(500))
        {
            if (threadNum == 0)
            {
                m_state.store(State::Stopped, std::memory_order_release);
                return;
            }

            // 启动固定数量的工作线程
            m_workers.reserve(threadNum);
            while (threadNum > 0)
            {
                m_activeWorkers.fetch_add(1, std::memory_order_acq_rel);
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
            : m_state(State::Running), m_activeWorkers(0),
              m_maxQueueSize(maxQueueSize),
              m_queueFullPolicy(QueueFullPolicy::kReject), m_timeoutMS(std::chrono::milliseconds(500))
        {
            if (minThreadNum > maxThreadNum) throw std::invalid_argument("[ol::ThreadPool] Invalid thread number range");

            if (minThreadNum == maxThreadNum && minThreadNum == 0)
            {
                m_state.store(State::Stopped, std::memory_order_release);
                return;
            }

            // 最小线程数至少为1
            minThreadNum = std::max(minThreadNum, static_cast<size_t>(1));

            // 初始化动态模式成员
            m_dynamic.minThreads = minThreadNum;
            m_dynamic.maxThreads = maxThreadNum;
            m_dynamic.idleThreads = 0;
            m_dynamic.workerExitNum = 0;
            m_dynamic.checkInterval = checkInterval;

            m_workers.reserve(minThreadNum);
            while (minThreadNum > 0)
            {
                m_activeWorkers.fetch_add(1, std::memory_order_acq_rel);
                std::thread th(&ThreadPool<IsDynamic>::worker, this);
#ifdef OL_DEBUG
                printf("构造函数：新工作线程(ID:%llu)\n", getThreadId(th.get_id()));
#endif
                m_workers.emplace(th.get_id(), std::move(th)); // 移动到哈希表
                --minThreadNum;
            }

            // 启动管理者线程
            m_dynamic.managerThread = std::thread(&ThreadPool<IsDynamic>::manager<IsDynamic>, this);
#ifdef OL_DEBUG
            printf("构造函数：新管理者线程(ID:%llu)\n", getThreadId(m_dynamic.managerThread.get_id()));
#endif
        }

        /**
         * @brief 析构函数
         * @note 自动调用stop()，等待所有任务完成并清理资源
         * @warning ThreadPool对象必须由线程池外部线程销毁。
         */
        ~ThreadPool()
        {
            if (m_state.load(std::memory_order_acquire) == State::Stopped) return;
            stop();
#ifdef OL_DEBUG
            if (m_activeWorkers.load() > 0)
            {
                printf("警告：析构时仍有%d个活跃线程未退出\n", (int)m_activeWorkers.load());
            }
#endif
        }

        /**
         * @brief 请求关闭线程池，不等待内部线程退出。
         * @param mode Drain-执行完排队任务；CancelPending-丢弃排队任务。
         * @note 可以在线程池自己的工作线程中调用；多次调用安全，Drain可以升级为CancelPending。
         */
        void requestStop(ShutdownMode mode = ShutdownMode::Drain)
        {
            std::queue<std::function<void()>> cancelledTasks;
            bool stateChanged = false;
            {
                std::lock_guard<std::mutex> lock(m_taskQueueMutex);
                const State state = m_state.load(std::memory_order_acquire);

                if (mode == ShutdownMode::CancelPending)
                {
                    if (state == State::Running || state == State::Draining)
                    {
                        m_state.store(State::Cancelling, std::memory_order_release);
                        cancelledTasks.swap(m_taskQueue);
                        stateChanged = true;
                    }
                }
                else if (state == State::Running)
                {
                    m_state.store(State::Draining, std::memory_order_release);
                    stateChanged = true;
                }
            }

            if (!stateChanged) return;

            if constexpr (IsDynamic) m_dynamic.managerExit_condVar.notify_all();
            m_taskQueueNotEmpty_condVar.notify_all();
            m_taskQueueNotFull_condVar.notify_all();
        }

        /**
         * @brief 等待管理者线程和全部工作线程退出并执行join。
         * @throws std::logic_error 当前线程属于此线程池，或尚未调用requestStop()。
         * @note 只能由线程池外部线程调用；多次调用安全。
         */
        void wait()
        {
            if (s_currentThreadPool == this)
                throw std::logic_error("[ol::ThreadPool] wait() cannot be called from an internal thread");

            std::unique_lock<std::mutex> waitLock(m_waitMutex);
            State state = m_state.load(std::memory_order_acquire);
            if (state == State::Stopped) return;
            if (state == State::Running)
                throw std::logic_error("[ol::ThreadPool] requestStop() must be called before wait()");

            if constexpr (IsDynamic)
            {
                if (m_dynamic.managerThread.joinable()) m_dynamic.managerThread.join();
            }

            std::vector<std::thread> workers;
            {
                std::lock_guard<std::mutex> lock(m_workersMutex);
                workers.reserve(m_workers.size());
                if constexpr (IsDynamic)
                {
                    for (auto& [id, workerThread] : m_workers)
                    {
                        (void)id;
                        workers.push_back(std::move(workerThread));
                    }
                    m_workers.clear();
                }
                else
                {
                    workers.swap(m_workers);
                }
            }

            for (auto& workerThread : workers)
            {
                if (workerThread.joinable()) workerThread.join();
            }

            if constexpr (IsDynamic)
            {
                std::lock_guard<std::mutex> lock(m_dynamic.workerExitId_dequeMutex);
                m_dynamic.workerExitId_deque.clear();
            }

            m_state.store(State::Stopped, std::memory_order_release);
        }

        /**
         * @brief 优雅停止：拒绝新任务，执行完已排队任务，再等待全部内部线程退出。
         * @note 只能由线程池外部线程调用；多次调用安全。
         */
        void stop()
        {
            if (s_currentThreadPool == this)
                throw std::logic_error("[ol::ThreadPool] stop() cannot be called from an internal thread; use requestStop()");
            requestStop(ShutdownMode::Drain);
            wait();
        }

        /**
         * @brief 快速停止：拒绝新任务，丢弃排队任务，等待正在执行的任务结束。
         * @note 只能由线程池外部线程调用；多次调用安全。
         * @note 被丢弃的submitTask任务，其future将报告std::future_errc::broken_promise。
         */
        void stopNow()
        {
            if (s_currentThreadPool == this)
                throw std::logic_error("[ol::ThreadPool] stopNow() cannot be called from an internal thread; use requestStop(CancelPending)");
            requestStop(ShutdownMode::CancelPending);
            wait();
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
         * @brief 获取当前线程的可打印标识（Linux返回内核TID，Windows返回线程ID）
         * @return 线程标识数值（可安全用于printf("%llu")等格式化输出）
         * @note Linux下返回syscall(SYS_gettid)，与top -H输出一致；Windows下返回GetCurrentThreadId()
         */
        static uint64_t getThreadId()
        {
#ifdef __unix__
            return static_cast<uint64_t>(syscall(SYS_gettid));
#else
            return static_cast<uint64_t>(GetCurrentThreadId());
#endif
        }

        /**
         * @brief 将std::thread::id转换为可打印数值（用于非当前线程的id）
         * @param id 线程id
         * @return 可打印数值
         */
        static uint64_t getThreadId(const std::thread::id& id)
        {
            return std::hash<std::thread::id>{}(id);
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
                throw std::invalid_argument("[ol::ThreadPool] Timeout must be greater than 0");
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
            if (m_state.load(std::memory_order_acquire) != State::Running) return false;

            {
                std::unique_lock<std::mutex> lock(m_taskQueueMutex);

                // 处理队列大小限制
                if (m_maxQueueSize > 0)
                {
                    while (m_taskQueue.size() >= m_maxQueueSize &&
                           m_state.load(std::memory_order_acquire) == State::Running)
                    {
                        switch (m_queueFullPolicy)
                        {
                        case QueueFullPolicy::kReject:
                            return false;
                        case QueueFullPolicy::kBlock:
                            m_taskQueueNotFull_condVar.wait(lock, [this]()
                                                            { return m_taskQueue.size() < m_maxQueueSize || m_state.load(std::memory_order_acquire) != State::Running; });
                            break;
                        case QueueFullPolicy::kTimeout:
                            bool result = m_taskQueueNotFull_condVar.wait_for(lock, m_timeoutMS,
                                                                              [this]()
                                                                              { return m_taskQueue.size() < m_maxQueueSize || m_state.load(std::memory_order_acquire) != State::Running; });
                            if (!result) return false;
                            break;
                        }
                    }
                }

                if (m_state.load(std::memory_order_acquire) != State::Running) return false;
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
                State state;
                QueueFullPolicy queueFullPolicy;
                {
                    std::lock_guard<std::mutex> lock(m_taskQueueMutex);
                    state = m_state.load(std::memory_order_acquire);
                    queueFullPolicy = m_queueFullPolicy;
                }

                if (state != State::Running)
                {
                    promise.set_exception(std::make_exception_ptr(std::runtime_error("[ol::ThreadPool] ThreadPool is not accepting tasks")));
                    return {false, promise.get_future()};
                }

                switch (queueFullPolicy)
                {
                case QueueFullPolicy::kReject:
                    promise.set_exception(std::make_exception_ptr(std::runtime_error("[ol::ThreadPool] Task queue full (Reject policy)")));
                    return {false, promise.get_future()};
                case QueueFullPolicy::kBlock:
                    // 此时失败一定是因为线程池已停止（否则wait会一直等）
                    promise.set_exception(std::make_exception_ptr(std::runtime_error("[ol::ThreadPool] Task submission failed in block policy (ThreadPool stopped)")));
                    return {false, promise.get_future()};
                case QueueFullPolicy::kTimeout:
                    promise.set_exception(std::make_exception_ptr(std::runtime_error("[ol::ThreadPool] Task queue full (Timeout policy)")));
                    return {false, promise.get_future()};
                default:
                    promise.set_exception(std::make_exception_ptr(std::runtime_error("[ol::ThreadPool] Task submission failed")));
                    return {false, promise.get_future()};
                }
            }

            return {true, std::move(result)};
        }

        /**
         * @brief 检查线程池是否处于运行状态
         * @return 仍接受新任务时返回true，否则返回false。
         * @note 只有Running状态返回true，Draining/Cancelling/Stopped均返回false。
         */
        bool isRunning() const noexcept { return m_state.load(std::memory_order_acquire) == State::Running; }

        // 返回精确的运行/关闭状态。
        State getState() const noexcept { return m_state.load(std::memory_order_acquire); }

    private:
        /**
         * @brief 工作线程主函数
         * @note 循环从任务队列获取并执行任务，直到线程池停止或（动态模式下）收到退出指令
         * @note 动态模式下会维护空闲线程计数，任务执行前后更新状态
         */
        void worker()
        {
            ThreadContextGuard threadContext(this);

            // 动态模式：初始化线程状态
            if constexpr (IsDynamic)
            {
                m_dynamic.idleThreads.fetch_add(1, std::memory_order_release);
            }

            try
            {
                while (true)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(m_taskQueueMutex);

                        // 等待任务或停止信号
                        auto waitCond = [this]()
                        {
                            if constexpr (IsDynamic)
                                return !m_taskQueue.empty() || m_state.load(std::memory_order_acquire) != State::Running || m_dynamic.workerExitNum.load(std::memory_order_acquire) > 0;
                            else
                                return !m_taskQueue.empty() || m_state.load(std::memory_order_acquire) != State::Running;
                        };
                        m_taskQueueNotEmpty_condVar.wait(lock, waitCond);

                        const State state = m_state.load(std::memory_order_acquire);
                        if (state == State::Cancelling || state == State::Stopped) break;
                        if (state == State::Draining && m_taskQueue.empty()) break;

                        if constexpr (IsDynamic)
                        {
                            if (state == State::Running &&
                                m_dynamic.workerExitNum.load(std::memory_order_acquire) > 0)
                            {
                                m_dynamic.workerExitNum.fetch_sub(1, std::memory_order_acq_rel);
                                break;
                            }
                        }

                        if (m_taskQueue.empty()) continue;

                        // 取出任务
                        task = std::move(m_taskQueue.front());
                        m_taskQueue.pop();

                        // 通知可能等待的生产者
                        if (m_queueFullPolicy != QueueFullPolicy::kReject) m_taskQueueNotFull_condVar.notify_one();

                        // 动态模式：空闲线程数-1
                        if constexpr (IsDynamic) m_dynamic.idleThreads.fetch_sub(1, std::memory_order_acq_rel);
                    }

                    // 执行任务
                    try
                    {
                        if (task) task(); // 空任务保护
                    }
                    catch (const std::exception& e)
                    {
                        fprintf(stderr, "[ol::ThreadPool] Worker thread(ID:%llu) Task error: %s\n", getThreadId(), e.what());
                    }
                    catch (...)
                    {
                        fprintf(stderr, "[ol::ThreadPool] Worker thread(ID:%llu) Unknown task error\n", getThreadId());
                    }

                    // 动态模式：任务完成，恢复空闲状态
                    if constexpr (IsDynamic)
                    {
                        m_dynamic.idleThreads.fetch_add(1, std::memory_order_release);
                    }
                }
            }
            catch (const std::exception& e)
            {
                fprintf(stderr, "[ol::ThreadPool] Worker thread(ID:%llu) exception: %s\n", getThreadId(), e.what());
            }
            catch (...)
            {
                fprintf(stderr, "[ol::ThreadPool] Worker thread(ID:%llu) unexpected exception\n", getThreadId());
            }

            // 活跃线程数-1
            m_activeWorkers.fetch_sub(1, std::memory_order_acq_rel);

            // 动态模式：空闲线程数-1
            if constexpr (IsDynamic)
            {
                m_dynamic.idleThreads.fetch_sub(1, std::memory_order_acq_rel);
                // 线程池未停止时，让管理者清理线程
                if (m_state.load(std::memory_order_acquire) == State::Running)
                {
                    std::unique_lock<std::mutex> lock_exitVector(m_dynamic.workerExitId_dequeMutex);
#ifdef OL_DEBUG
                    printf("[worker] 线程(ID:%llu)加入退出容器\n", getThreadId());
#endif
                    m_dynamic.workerExitId_deque.emplace_back(std::this_thread::get_id());
                }
#ifdef OL_DEBUG
                else
                {
                    printf("[worker] 线程(ID:%llu)：线程池已停止，跳过加入退出容器\n", getThreadId());
                }
#endif
            }

#ifdef OL_DEBUG
            printf("[worker] 线程(ID:%llu)已销毁（主动移除）\n", getThreadId());
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
            ThreadContextGuard threadContext(this);

#ifdef OL_DEBUG
            printf("[manager] 管理者线程(ID:%llu)启动\n", getThreadId());
#endif

            try
            {
                std::deque<std::thread::id> exitIds;
                while (m_state.load(std::memory_order_acquire) == State::Running)
                {
                    // 定期检查（可被stop()唤醒）
                    std::unique_lock<std::mutex> lock_manger(m_dynamic.managerMutex);
                    m_dynamic.managerExit_condVar.wait_for(lock_manger, m_dynamic.checkInterval, [this]()
                                                           { return m_state.load(std::memory_order_acquire) != State::Running; });
                    if (m_state.load(std::memory_order_acquire) != State::Running) break;

                    // 1. 清理已终止的线程对象
                    {
                        // 上锁
                        std::lock_guard<std::mutex> lock_workers(m_workersMutex);
                        std::unique_lock<std::mutex> lock_exitDeque(m_dynamic.workerExitId_dequeMutex);

                        // 交换退出线程ID队列
                        exitIds.clear();
                        exitIds.swap(m_dynamic.workerExitId_deque);
                        lock_exitDeque.unlock(); // 解锁

                        // 清理退出的线程
                        for (const auto& exitId : exitIds)
                        {
#ifdef OL_DEBUG
                            printf("[manager] 待清理线程(ID：%llu)\n", getThreadId(exitId));
#endif
                            auto it = m_workers.find(exitId);
                            if (it == m_workers.end())
                            {
#ifdef OL_DEBUG
                                printf("[manager] 线程(ID:%llu)已被清理，跳过\n", getThreadId(exitId));
#endif
                                continue;
                            }

                            if (it->second.joinable())
                            {
                                try
                                {
                                    it->second.join();
#ifdef OL_DEBUG
                                    printf("[manager] 线程(ID:%llu)已join\n", getThreadId(exitId));
#endif
                                }
                                catch (const std::exception& e)
                                {
                                    fprintf(stderr, "[ol::ThreadPool] Worker thread(ID:%llu) join failure: %s\n", getThreadId(exitId), e.what());
                                }
                                catch (...)
                                {
                                    fprintf(stderr, "[ol::ThreadPool] Worker thread(ID:%llu) Unknown join error\n", getThreadId(exitId));
                                }
                            }

                            m_workers.erase(it);
                        }
                    }

                    // 2. 扩缩容
                    {
                        std::lock_guard<std::mutex> lock_taskQueue(m_taskQueueMutex);
                        std::lock_guard<std::mutex> lock_workers(m_workersMutex);
                        size_t taskCount = m_taskQueue.size();
                        size_t workerCount = m_workers.size();
                        size_t idleCount = m_dynamic.idleThreads.load(std::memory_order_acquire);

                        // 扩容判断：任务数 > 线程数 * 2 且未达最大线程数
                        if (taskCount > workerCount * 2 && workerCount < m_dynamic.maxThreads)
                        {
                            size_t needThreads = std::min(
                                m_dynamic.maxThreads - workerCount,
                                (taskCount + workerCount - 1) / workerCount // 按当前负载估算需要的线程数（向上取整）
                            );
                            // 每次最多扩容到当前的1.5倍，避免一次性创建过多线程
                            needThreads = std::min(needThreads, workerCount / 2 + 1);

                            m_workers.reserve(workerCount + needThreads);
                            while (needThreads > 0)
                            {
                                m_activeWorkers.fetch_add(1, std::memory_order_acq_rel);
                                std::thread th(&ThreadPool<IsDynamic>::worker, this);
#ifdef OL_DEBUG
                                printf("[manager] 新工作线程(ID:%llu)\n", getThreadId(th.get_id()));
#endif
                                m_workers.emplace(th.get_id(), std::move(th)); // 哈希表插入新工作线程
                                --needThreads;
                            }
#ifdef OL_DEBUG
                            printf("[manager] 扩容：线程数从 %zu 增加到 %zu（任务数: %zu）\n",
                                   workerCount, m_workers.size(), taskCount);
#endif
                        }
                        // 缩容判断：空闲线程 > 线程总数的1/2 且 线程数 > 最小线程数
                        else if (idleCount > workerCount / 2 && workerCount > m_dynamic.minThreads)
                        {

                            // 实际缩减数 = 取 可缩减线程数 和 多余空闲线程数 的较小值
                            size_t reduceThreads = std::min(
                                workerCount - m_dynamic.minThreads, // 可缩减线程数 = 当前线程数 - 最低保留数
                                idleCount - (workerCount / 2)       // 多余空闲线程数 = 超过一半的空闲线程
                            );

#ifdef OL_DEBUG
                            size_t reduceThreads_temp = reduceThreads;
#endif

                            // 销毁线程：设置退出数量后逐个唤醒，避免惊群效应
                            if (reduceThreads > 0)
                            {
                                m_dynamic.workerExitNum.fetch_add(reduceThreads, std::memory_order_acq_rel);
                                do
                                {
                                    m_taskQueueNotEmpty_condVar.notify_one();
                                    --reduceThreads;
                                } while (reduceThreads > 0);
                            }
#ifdef OL_DEBUG
                            printf("[manager] 缩容：计划销毁 %zu 个线程（当前线程数: %zu, 空闲数: %zu, 保留至少: %zu）\n",
                                   reduceThreads_temp, workerCount, idleCount, m_dynamic.minThreads);
#endif
                        }
                    }
                }
            }
            catch (const std::exception& e)
            {
                fprintf(stderr, "[ol::ThreadPool] Manager thread(ID:%llu) exception: %s\n", getThreadId(), e.what());
            }
            catch (...)
            {
                fprintf(stderr, "[ol::ThreadPool] Manager thread(ID:%llu) unexpected exception\n", getThreadId());
            }

            // 清空退出队列
            std::lock_guard<std::mutex> lock_exit_deque(m_dynamic.workerExitId_dequeMutex);
            m_dynamic.workerExitId_deque.clear();
#ifdef OL_DEBUG
            printf("[manager] 管理者线程(ID:%llu)退出，清空退出队列\n", getThreadId());
#endif
        }
    };
} // namespace ol

#endif // !OL_THREADPOOL_H
