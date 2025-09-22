/****************************************************************************************/
/*
 * 程序名：ol_ThreadPool.h
 * 功能描述：线程池封装类，提供高效的多线程任务调度能力，支持以下特性：
 *          - 固定数量线程的创建与管理，自动回收线程资源
 *          - 支持无返回值任务（addTask）和带返回值任务（submitTask）提交
 *          - 线程安全的任务队列操作，通过互斥锁与条件变量实现同步
 *          - 任务队列大小限制，防止内存溢出
 *          - 任务执行异常捕获与处理，避免线程因任务异常终止
 *          - 跨平台线程ID获取支持（Linux与Windows兼容）
 *          - 提供线程数量、任务数量等状态查询接口
 * 作者：ol
 * 适用标准：C++11及以上（需支持std::thread、std::future、lambda表达式等特性）
 */
/****************************************************************************************/

#ifndef __OL_THREADPOOL_H
#define __OL_THREADPOOL_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace ol
{

    /**
     * @brief 线程池类，用于管理和复用线程，执行异步任务
     *
     * 线程池在构造时创建指定数量的线程，这些线程会等待任务队列中的任务并执行。
     * 支持添加无返回值和有返回值的任务，并通过future获取任务结果。
     * 线程池在析构时会等待所有任务执行完毕后再销毁线程。
     */
    class ThreadPool
    {
    private:
        std::vector<std::thread> m_threads;            ///< 线程池中的工作线程
        std::queue<std::function<void()>> m_taskQueue; ///< 任务队列，存储等待执行的任务
        std::mutex m_mutex;                            ///< 保护任务队列的互斥锁
        std::condition_variable m_condition;           ///< 用于线程同步的条件变量
        std::atomic_bool m_stop;                       ///< 线程池停止标志
        std::atomic_size_t m_taskCount;                ///< 当前等待执行的任务数量
        size_t m_maxQueueSize;                         ///< 任务队列的最大容量，0表示无限制

    public:
        /**
         * @brief 构造函数，创建线程池并初始化指定数量的线程
         * @param threadNum 线程池中线程的数量
         * @param maxQueueSize 任务队列的最大容量，默认为0（无限制）
         */
        ThreadPool(size_t threadNum, size_t maxQueueSize = 0);

        /**
         * @brief 禁止拷贝构造函数
         */
        ThreadPool(const ThreadPool&) = delete;

        /**
         * @brief 禁止赋值操作符
         */
        ThreadPool& operator=(const ThreadPool&) = delete;

        /**
         * @brief 析构函数，停止线程池并等待所有线程退出
         */
        ~ThreadPool();

        /**
         * @brief 主动停止线程池
         * @param wait 是否等待所有线程执行完当前任务后再退出（默认true）
         *             - true：等待所有任务完成和线程退出（安全退出）
         *             - false：立即停止，不等待线程（可能导致任务中断）
         */
        void stop(bool wait = true);

        /**
         * @brief 向线程池添加无返回值的任务
         * @param task 要执行的任务，类型为std::function<void()>
         * @return 若任务添加成功返回true，若线程池已停止或队列已满返回false
         */
        bool addTask(std::function<void()> task);

        /**
         * @brief 向线程池添加带参数和返回值的任务
         * @tparam F 任务函数类型
         * @tparam Args 任务函数参数类型
         * @param f 任务函数
         * @param args 任务函数的参数
         * @return 返回一个std::future对象，用于获取任务执行结果
         */
        template <typename F, typename... Args>
        auto submitTask(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

        /**
         * @brief 获取当前等待执行的任务数量
         * @return 任务队列中的任务数量
         */
        size_t getTaskCount() const;

        /**
         * @brief 获取线程池中的线程数量
         * @return 线程数量
         */
        size_t getThreadCount() const;

        /**
         * @brief 检查线程池是否正在运行
         * @return 若线程池未停止则返回true，否则返回false
         */
        bool isRunning() const;
    };

    template <typename F, typename... Args>
    auto ThreadPool::submitTask(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using ReturnType = typename std::result_of<F(Args...)>::type;

        // 如果线程池已停止，抛出异常
        if (m_stop)
        {
            throw std::runtime_error("ThreadPool is stopped, cannot submit task.");
        }

        // 创建包装任务
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<ReturnType> res = task->get_future();
        std::function<void()> wrapper = [task]()
        {
            try
            {
                (*task)();
            }
            catch (...)
            {
                // 捕获异常，防止线程因未处理的异常而终止
                std::cerr << "Task execution threw an uncaught exception." << std::endl;
            }
        };

        {
            std::unique_lock<std::mutex> lock(m_mutex);

            // 如果队列已满，等待有空间
            if (m_maxQueueSize > 0)
            {
                m_condition.wait(lock, [this]()
                                 { return m_taskQueue.size() < m_maxQueueSize || m_stop; });
            }

            // 如果线程池已停止，抛出异常
            if (m_stop)
            {
                throw std::runtime_error("ThreadPool is stopped, cannot submit task.");
            }

            m_taskQueue.push(wrapper);
            ++m_taskCount;
        }

        m_condition.notify_one();
        return res;
    }

} // namespace ol

#endif // !__OL_THREADPOOL_H