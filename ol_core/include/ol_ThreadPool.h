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
    namespace thread_pool_detail
    {
        inline std::mutex& logOutputMutex()
        {
            static std::mutex mutex;
            return mutex;
        }

        inline uint64_t nextLogSequence()
        {
            static std::atomic<uint64_t> sequence{0};
            return sequence.fetch_add(1, std::memory_order_relaxed) + 1;
        }
    } // namespace thread_pool_detail

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

#ifdef OL_DEBUG
        static const char* stateName(State state) noexcept
        {
            switch (state)
            {
            case State::Running:
                return "Running";
            case State::Draining:
                return "Draining";
            case State::Cancelling:
                return "Cancelling";
            case State::Stopped:
                return "Stopped";
            }
            return "Unknown";
        }

        template <typename... Args>
        void debugLog(const char* event, const char* format, Args... args) const
        {
            std::lock_guard<std::mutex> lock(thread_pool_detail::logOutputMutex());
            const uint64_t sequence = thread_pool_detail::nextLogSequence();
            fprintf(stderr,
                    "[ol::ThreadPool][seq=%" PRIu64 "][pool=%p][mode=%s][os_tid=%" PRIu64 "][std_id=%" PRIu64 "][%s] ",
                    sequence,
                    static_cast<const void*>(this),
                    IsDynamic ? "dynamic" : "fixed",
                    getThreadId(),
                    getThreadId(std::this_thread::get_id()),
                    event);
            fprintf(stderr, format, args...);
            fputc('\n', stderr);
            fflush(stderr);
        }
#endif

        template <typename... Args>
        void errorLog(const char* event, const char* format, Args... args) const
        {
            std::lock_guard<std::mutex> lock(thread_pool_detail::logOutputMutex());
            const uint64_t sequence = thread_pool_detail::nextLogSequence();
            fprintf(stderr,
                    "[ol::ThreadPool][seq=%" PRIu64 "][pool=%p][mode=%s][os_tid=%" PRIu64 "][std_id=%" PRIu64 "][%s] ",
                    sequence,
                    static_cast<const void*>(this),
                    IsDynamic ? "dynamic" : "fixed",
                    getThreadId(),
                    getThreadId(std::this_thread::get_id()),
                    event);
            fprintf(stderr, format, args...);
            fputc('\n', stderr);
            fflush(stderr);
        }

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
#ifdef OL_DEBUG
                debugLog("pool-create", "state=Stopped workers=0 max_queue=%zu", m_maxQueueSize);
#endif
                return;
            }

#ifdef OL_DEBUG
            debugLog("pool-create", "state=Running workers=%zu max_queue=%zu", threadNum, m_maxQueueSize);
#endif

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
#ifdef OL_DEBUG
                debugLog("pool-create", "state=Stopped min_workers=0 max_workers=0 max_queue=%zu", m_maxQueueSize);
#endif
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

#ifdef OL_DEBUG
            debugLog("pool-create",
                     "state=Running min_workers=%zu max_workers=%zu max_queue=%zu check_interval_s=%lld",
                     m_dynamic.minThreads,
                     m_dynamic.maxThreads,
                     m_maxQueueSize,
                     static_cast<long long>(m_dynamic.checkInterval.count()));
#endif

            m_workers.reserve(minThreadNum);
            while (minThreadNum > 0)
            {
                m_activeWorkers.fetch_add(1, std::memory_order_acq_rel);
                std::thread th(&ThreadPool<IsDynamic>::worker, this);
                m_workers.emplace(th.get_id(), std::move(th)); // 移动到哈希表
                --minThreadNum;
            }

            // 启动管理者线程
            m_dynamic.managerThread = std::thread(&ThreadPool<IsDynamic>::manager<IsDynamic>, this);
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
                debugLog("invariant-violation",
                         "destructor completed with active_workers=%zu",
                         m_activeWorkers.load(std::memory_order_acquire));
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
            State previousState;
            State nextState;
#ifdef OL_DEBUG
            size_t queuedBefore = 0;
            size_t droppedTasks = 0;
#endif
            {
                std::lock_guard<std::mutex> lock(m_taskQueueMutex);
                previousState = m_state.load(std::memory_order_acquire);
                nextState = previousState;
#ifdef OL_DEBUG
                queuedBefore = m_taskQueue.size();
#endif

                if (mode == ShutdownMode::CancelPending)
                {
                    if (previousState == State::Running || previousState == State::Draining)
                    {
                        nextState = State::Cancelling;
                        m_state.store(nextState, std::memory_order_release);
#ifdef OL_DEBUG
                        droppedTasks = m_taskQueue.size();
#endif
                        cancelledTasks.swap(m_taskQueue);
                        stateChanged = true;
                    }
                }
                else if (previousState == State::Running)
                {
                    nextState = State::Draining;
                    m_state.store(nextState, std::memory_order_release);
                    stateChanged = true;
                }

#ifdef OL_DEBUG
                if (stateChanged)
                {
                    debugLog("state-change",
                             "from=%s to=%s queued_before=%zu dropped=%zu active_workers=%zu",
                             stateName(previousState),
                             stateName(nextState),
                             queuedBefore,
                             droppedTasks,
                             m_activeWorkers.load(std::memory_order_acquire));
                }
#endif
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
            {
#ifdef OL_DEBUG
                debugLog("invalid-call", "wait called from an internal thread");
#endif
                throw std::logic_error("[ol::ThreadPool] wait() cannot be called from an internal thread");
            }

            std::unique_lock<std::mutex> waitLock(m_waitMutex);
            State state = m_state.load(std::memory_order_acquire);
            if (state == State::Stopped) return;
            if (state == State::Running)
            {
#ifdef OL_DEBUG
                debugLog("invalid-call", "wait called while state=Running");
#endif
                throw std::logic_error("[ol::ThreadPool] requestStop() must be called before wait()");
            }

#ifdef OL_DEBUG
            debugLog("wait-begin",
                     "state=%s queued=%zu active_workers=%zu",
                     stateName(state),
                     getTaskNum(),
                     m_activeWorkers.load(std::memory_order_acquire));
#endif

            if constexpr (IsDynamic)
            {
                if (m_dynamic.managerThread.joinable())
                {
                    m_dynamic.managerThread.join();
#ifdef OL_DEBUG
                    debugLog("manager-joined", "state=%s", stateName(m_state.load(std::memory_order_acquire)));
#endif
                }
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

#ifdef OL_DEBUG
            debugLog("worker-join-begin", "count=%zu", workers.size());
#endif
            for (auto& workerThread : workers)
            {
                if (workerThread.joinable()) workerThread.join();
            }

            if constexpr (IsDynamic)
            {
                std::lock_guard<std::mutex> lock(m_dynamic.workerExitId_dequeMutex);
                m_dynamic.workerExitId_deque.clear();
            }

#ifdef OL_DEBUG
            const State shutdownState = m_state.load(std::memory_order_acquire);
#endif
            m_state.store(State::Stopped, std::memory_order_release);
#ifdef OL_DEBUG
            debugLog("wait-end",
                     "from=%s to=Stopped joined=%zu active_workers=%zu",
                     stateName(shutdownState),
                     workers.size(),
                     m_activeWorkers.load(std::memory_order_acquire));
#endif
        }

        /**
         * @brief 优雅停止：拒绝新任务，执行完已排队任务，再等待全部内部线程退出。
         * @note 只能由线程池外部线程调用；多次调用安全。
         */
        void stop()
        {
            if (s_currentThreadPool == this)
            {
#ifdef OL_DEBUG
                debugLog("invalid-call", "stop called from an internal thread; use requestStop");
#endif
                throw std::logic_error("[ol::ThreadPool] stop() cannot be called from an internal thread; use requestStop()");
            }
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
            {
#ifdef OL_DEBUG
                debugLog("invalid-call", "stopNow called from an internal thread; use requestStop(CancelPending)");
#endif
                throw std::logic_error("[ol::ThreadPool] stopNow() cannot be called from an internal thread; use requestStop(CancelPending)");
            }
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
         * @return 操作系统线程标识数值（使用PRIu64进行格式化输出）
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
         * @brief 将std::thread::id转换为可打印哈希值（用于非当前线程的id）
         * @param id 线程id
         * @return 当前进程内用于日志关联的哈希值，不等同于操作系统线程ID
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
#ifdef OL_DEBUG
            const char* exitReason = "unknown";
            debugLog("worker-start",
                     "state=%s active_workers=%zu",
                     stateName(m_state.load(std::memory_order_acquire)),
                     m_activeWorkers.load(std::memory_order_acquire));
#endif

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
                        if (state == State::Cancelling)
                        {
#ifdef OL_DEBUG
                            exitReason = "cancelled";
#endif
                            break;
                        }
                        if (state == State::Stopped)
                        {
#ifdef OL_DEBUG
                            exitReason = "stopped";
#endif
                            break;
                        }
                        if (state == State::Draining && m_taskQueue.empty())
                        {
#ifdef OL_DEBUG
                            exitReason = "drained";
#endif
                            break;
                        }

                        if constexpr (IsDynamic)
                        {
                            if (state == State::Running &&
                                m_dynamic.workerExitNum.load(std::memory_order_acquire) > 0)
                            {
                                m_dynamic.workerExitNum.fetch_sub(1, std::memory_order_acq_rel);
#ifdef OL_DEBUG
                                exitReason = "scale-down";
#endif
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
                        errorLog("task-error", "exception=%s", e.what());
                    }
                    catch (...)
                    {
                        errorLog("task-error", "unknown exception");
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
#ifdef OL_DEBUG
                exitReason = "exception";
#endif
                errorLog("worker-error", "exception=%s", e.what());
            }
            catch (...)
            {
#ifdef OL_DEBUG
                exitReason = "exception";
#endif
                errorLog("worker-error", "unknown exception");
            }

            // 活跃线程数-1
#ifdef OL_DEBUG
            const size_t remainingActiveWorkers = m_activeWorkers.fetch_sub(1, std::memory_order_acq_rel) - 1;
#else
            m_activeWorkers.fetch_sub(1, std::memory_order_acq_rel);
#endif

            // 动态模式：空闲线程数-1
            if constexpr (IsDynamic)
            {
                m_dynamic.idleThreads.fetch_sub(1, std::memory_order_acq_rel);
                // 线程池未停止时，让管理者清理线程
                if (m_state.load(std::memory_order_acquire) == State::Running)
                {
                    std::unique_lock<std::mutex> lock_exitVector(m_dynamic.workerExitId_dequeMutex);
                    m_dynamic.workerExitId_deque.emplace_back(std::this_thread::get_id());
                }
            }

#ifdef OL_DEBUG
            debugLog("worker-exit", "reason=%s active_workers=%zu", exitReason, remainingActiveWorkers);
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
            debugLog("manager-start",
                     "state=%s min_workers=%zu max_workers=%zu",
                     stateName(m_state.load(std::memory_order_acquire)),
                     m_dynamic.minThreads,
                     m_dynamic.maxThreads);
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
                    lock_manger.unlock();

                    // 1. 清理已终止的线程对象
#ifdef OL_DEBUG
                    size_t joinedWorkers = 0;
                    size_t missingWorkers = 0;
#endif
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
                            auto it = m_workers.find(exitId);
                            if (it == m_workers.end())
                            {
#ifdef OL_DEBUG
                                ++missingWorkers;
#endif
                                continue;
                            }

                            if (it->second.joinable())
                            {
                                try
                                {
                                    it->second.join();
#ifdef OL_DEBUG
                                    ++joinedWorkers;
#endif
                                }
                                catch (const std::exception& e)
                                {
                                    errorLog("worker-join-error",
                                             "worker_std_id=%" PRIu64 " exception=%s",
                                             getThreadId(exitId),
                                             e.what());
                                }
                                catch (...)
                                {
                                    errorLog("worker-join-error",
                                             "worker_std_id=%" PRIu64 " unknown exception",
                                             getThreadId(exitId));
                                }
                            }

                            m_workers.erase(it);
                        }
                    }
#ifdef OL_DEBUG
                    if (joinedWorkers > 0 || missingWorkers > 0)
                    {
                        debugLog("worker-cleanup",
                                 "joined=%zu missing=%zu remaining_workers=%zu",
                                 joinedWorkers,
                                 missingWorkers,
                                 getWorkerNum());
                    }
#endif

                    // 2. 扩缩容
                    size_t addedThreads = 0;
                    size_t reducedThreads = 0;
                    size_t taskCountSnapshot = 0;
                    size_t workerCountSnapshot = 0;
                    size_t idleCountSnapshot = 0;
#ifdef OL_DEBUG
                    size_t reducedThreadsForLog = 0;
                    size_t workerCountAfterScale = 0;
#endif
                    {
                        std::lock_guard<std::mutex> lock_taskQueue(m_taskQueueMutex);
                        std::lock_guard<std::mutex> lock_workers(m_workersMutex);
                        taskCountSnapshot = m_taskQueue.size();
                        workerCountSnapshot = m_workers.size();
                        idleCountSnapshot = m_dynamic.idleThreads.load(std::memory_order_acquire);

                        // 扩容判断：任务数 > 线程数 * 2 且未达最大线程数
                        if (taskCountSnapshot > workerCountSnapshot * 2 && workerCountSnapshot < m_dynamic.maxThreads)
                        {
                            addedThreads = std::min(
                                m_dynamic.maxThreads - workerCountSnapshot,
                                (taskCountSnapshot + workerCountSnapshot - 1) / workerCountSnapshot // 按当前负载估算需要的线程数（向上取整）
                            );
                            // 每次最多扩容到当前的1.5倍，避免一次性创建过多线程
                            addedThreads = std::min(addedThreads, workerCountSnapshot / 2 + 1);

                            m_workers.reserve(workerCountSnapshot + addedThreads);
                            for (size_t i = 0; i < addedThreads; ++i)
                            {
                                m_activeWorkers.fetch_add(1, std::memory_order_acq_rel);
                                std::thread th(&ThreadPool<IsDynamic>::worker, this);
                                m_workers.emplace(th.get_id(), std::move(th)); // 哈希表插入新工作线程
                            }
                        }
                        // 缩容判断：空闲线程 > 线程总数的1/2 且 线程数 > 最小线程数
                        else if (idleCountSnapshot > workerCountSnapshot / 2 && workerCountSnapshot > m_dynamic.minThreads)
                        {
                            // 实际缩减数 = 取 可缩减线程数 和 多余空闲线程数 的较小值
                            reducedThreads = std::min(
                                workerCountSnapshot - m_dynamic.minThreads, // 可缩减线程数 = 当前线程数 - 最低保留数
                                idleCountSnapshot - (workerCountSnapshot / 2) // 多余空闲线程数 = 超过一半的空闲线程
                            );
#ifdef OL_DEBUG
                            reducedThreadsForLog = reducedThreads;
#endif

                            // 销毁线程：设置退出数量后逐个唤醒，避免惊群效应
                            if (reducedThreads > 0)
                            {
                                m_dynamic.workerExitNum.fetch_add(reducedThreads, std::memory_order_acq_rel);
                                do
                                {
                                    m_taskQueueNotEmpty_condVar.notify_one();
                                    --reducedThreads;
                                } while (reducedThreads > 0);
                            }
                        }
#ifdef OL_DEBUG
                        workerCountAfterScale = m_workers.size();
#endif
                    }
#ifdef OL_DEBUG
                    if (addedThreads > 0)
                    {
                        debugLog("scale-up",
                                 "from=%zu to=%zu added=%zu queued=%zu idle=%zu",
                                 workerCountSnapshot,
                                 workerCountAfterScale,
                                 addedThreads,
                                 taskCountSnapshot,
                                 idleCountSnapshot);
                    }
                    else if (reducedThreadsForLog > 0)
                    {
                        debugLog("scale-down-request",
                                 "current=%zu requested_exit=%zu queued=%zu idle=%zu min_workers=%zu",
                                 workerCountSnapshot,
                                 reducedThreadsForLog,
                                 taskCountSnapshot,
                                 idleCountSnapshot,
                                 m_dynamic.minThreads);
                    }
#endif
                }
            }
            catch (const std::exception& e)
            {
                errorLog("manager-error", "exception=%s", e.what());
            }
            catch (...)
            {
                errorLog("manager-error", "unknown exception");
            }

            // 清空退出队列
            {
                std::lock_guard<std::mutex> lock_exit_deque(m_dynamic.workerExitId_dequeMutex);
                m_dynamic.workerExitId_deque.clear();
            }
#ifdef OL_DEBUG
            debugLog("manager-exit", "state=%s", stateName(m_state.load(std::memory_order_acquire)));
#endif
        }
    };
} // namespace ol

#endif // !OL_THREADPOOL_H
