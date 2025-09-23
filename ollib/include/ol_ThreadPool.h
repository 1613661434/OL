#ifndef __OL_THREADPOOL_H
#define __OL_THREADPOOL_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace ol
{
    class ThreadPool
    {
    private:
        /**
         * @brief 队列满时的处理策略枚举
         * 用于指定当任务队列达到最大容量时，新任务的处理方式
         */
        enum class QueueFullPolicy : char
        {
            kReject, ///< 拒绝新任务，直接返回失败
            kBlock,  ///< 阻塞等待，直到队列有空闲空间
            kTimeout ///< 超时等待，超过指定时间后拒绝任务
        };

        std::vector<std::thread> m_threads;            ///< 线程池中的工作线程集合
        std::queue<std::function<void()>> m_taskQueue; ///< 待执行的任务队列
        std::mutex m_mutex;                            ///< 保护任务队列和策略参数的互斥锁
        std::condition_variable m_condition;           ///< 用于线程同步的条件变量
        std::atomic_bool m_stop;                       ///< 线程池停止标志（原子操作确保线程安全）
        std::atomic_size_t m_taskCount;                ///< 当前等待执行的任务数量（原子操作）
        size_t m_maxQueueSize;                         ///< 任务队列的最大容量（0表示无限制）
        QueueFullPolicy m_queuePolicy;                 ///< 当前采用的队列满处理策略
        size_t m_timeoutUs;                            ///< 超时等待时间（微秒，仅kTimeout策略生效）

    public:
        /**
         * @brief 构造函数，初始化线程池
         * @param threadNum 工作线程的数量（必须大于0）
         * @param maxQueueSize 任务队列的最大容量（0表示无限制，默认值为0）
         */
        ThreadPool(size_t threadNum, size_t maxQueueSize = 0);

        /**
         * @brief 禁止拷贝构造
         * 线程池包含不可拷贝的资源（线程、互斥锁等），因此禁用拷贝
         */
        ThreadPool(const ThreadPool&) = delete;

        /**
         * @brief 禁止赋值操作
         * 线程池包含不可赋值的资源（线程、互斥锁等），因此禁用赋值
         */
        ThreadPool& operator=(const ThreadPool&) = delete;

        /**
         * @brief 析构函数
         * 自动停止线程池并释放所有资源，确保线程安全退出
         */
        ~ThreadPool();

        /**
         * @brief 停止线程池
         * @param wait 是否等待所有任务执行完毕（默认true）
         *        - true：等待所有任务完成后再停止线程（安全退出）
         *        - false：立即停止，丢弃未执行的任务，让正在执行的任务安全完成后退出
         */
        void stop(bool wait = true);

        /**
         * @brief 设置队列满时的"拒绝"策略
         * 当任务队列已满时，新提交的任务会被直接拒绝，addTask返回false，submitTask抛出异常
         */
        void setRejectPolicy();

        /**
         * @brief 设置队列满时的"阻塞"策略
         * 当任务队列已满时，提交任务的线程会阻塞等待，直到队列有空闲空间
         */
        void setBlockPolicy();

        /**
         * @brief 设置队列满时的"超时"策略
         * 当任务队列已满时，提交任务的线程会等待指定时间，超时后拒绝任务
         * @param timeoutUs 超时时间（微秒），必须大于0
         */
        void setTimeoutPolicy(size_t timeoutUs);

        /**
         * @brief 向线程池添加无返回值的任务
         * @param task 待执行的任务（std::function<void()>类型）
         * @return 任务添加成功返回true；失败返回false（线程池已停止或队列满且策略为拒绝/超时）
         */
        bool addTask(std::function<void()> task);

        /**
         * @brief 向线程池添加带返回值的任务
         * @tparam F 任务函数类型
         * @tparam Args 任务函数的参数类型
         * @param f 任务函数
         * @param args 任务函数的参数
         * @return 用于获取任务结果的std::future对象
         * @throw 当线程池已停止或队列满且策略为拒绝/超时时，抛出std::runtime_error异常
         */
        template <typename F, typename... Args>
        auto submitTask(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

        /**
         * @brief 获取当前等待执行的任务数量
         * @return 返回的是当前等待执行的任务数（不包含正在执行的任务）
         */
        size_t getTaskCount() const;

        /**
         * @brief 获取线程池中的工作线程数量
         * @return 线程数量
         */
        size_t getThreadCount() const;

        /**
         * @brief 检查线程池是否处于运行状态
         * @return 运行中返回true，已停止返回false
         */
        bool isRunning() const;
    };

    template <typename F, typename... Args>
    auto ThreadPool::submitTask(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using ReturnType = typename std::result_of<F(Args...)>::type;

        // 包装任务为packaged_task
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<ReturnType> result = task->get_future();

        // 使用addTask添加任务，失败时抛出异常
        bool success = addTask([task]()
                               { (*task)(); });
        if (!success)
        {
            // 加锁读取策略，确保线程安全
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_stop)
            {
                throw std::runtime_error("ThreadPool has been stopped");
            }

            switch (m_queuePolicy)
            {
            case QueueFullPolicy::kReject:
                throw std::runtime_error("Task queue full (Reject policy)");

            case QueueFullPolicy::kBlock:
                // 阻塞策略下addTask返回false，只可能是线程池已停止（上面已判断）
                throw std::runtime_error("Task submission failed in block policy (thread pool stopped)");

            case QueueFullPolicy::kTimeout:
                throw std::runtime_error("Task queue full (Timeout policy)");
            }
        }

        return result;
    }

} // namespace ol

#endif // !__OL_THREADPOOL_H