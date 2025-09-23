#include "ol_ThreadPool.h"
#include <chrono>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <stdexcept>

#ifdef __linux__
#include <sys/syscall.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

// #define DEBUG

#ifdef DEBUG
static void debug_printf_thread(const char* message)
{
#ifdef __linux__
    printf("%s(%ld).\n", message, syscall(SYS_gettid));
#elif defined(_WIN32)
    printf("%s(%lu).\n", message, GetCurrentThreadId()); // Windows 系统线程 ID
#else
    std::cout << message << '(' << std::this_thread::get_id() << ')' << std::endl;
#endif // __linux__
}
#endif // DEBUG

namespace ol
{
    ThreadPool::ThreadPool(size_t threadNum, size_t maxQueueSize)
        : m_stop(false),
          m_taskCount(0),
          m_maxQueueSize(maxQueueSize),
          m_queuePolicy(QueueFullPolicy::kReject), // 默认策略：拒绝
          m_timeoutUs(1000000)                     // 默认超时1秒（1000000微秒）
    {
        if (threadNum == 0)
        {
            throw std::invalid_argument("Thread number must be greater than 0.");
        }

        // 启动指定数量的工作线程
        for (size_t i = 0; i < threadNum; ++i)
        {
            m_threads.emplace_back([this]
                                   {
#ifdef DEBUG
// 线程创建时的 ID 输出
debug_printf_thread("Created thread");
#endif // DEBUG

                while (!m_stop)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(m_mutex);
                        
                        // 等待任务或停止信号
                        m_condition.wait(lock, [this] { return m_stop || !m_taskQueue.empty(); });

                        if (m_taskQueue.empty())
                        {
                            if(m_stop) return; // 如果线程池已停止且任务队列为空，则退出
                            else continue; // 再次检查队列非空，防止虚假唤醒导致的pop空队列
                        }

                        // 取出任务
                        task = std::move(m_taskQueue.front());
                        m_taskQueue.pop();
                        --m_taskCount;

                        // 任务完成，且队列非空通知工作线程
                        if (!m_taskQueue.empty()) m_condition.notify_one();
                    }

#ifdef DEBUG
// 任务执行时的 ID 输出
debug_printf_thread("Executing task");
#endif // DEBUG

                    try
                    {
                        task(); // 执行任务
                    }
                    catch (const std::exception& e)
                    {
                        fprintf(stderr,"Task execution error: %s\n", e.what());
                    }
                    catch (...)
                    {
                        fprintf(stderr,"Unknown error occurred during task execution.\n");
                    }

#ifdef DEBUG
// 任务完成时的 ID 输出
debug_printf_thread("Complete task");
#endif // DEBUG
                } });
        }
    }

    ThreadPool::~ThreadPool()
    {
        if (!m_stop) // 如果未主动调用stop，则默认安全停止
        {
            stop(true);
        }
    }

    void ThreadPool::stop(bool wait)
    {
        if (m_stop) return; // 避免重复停止

        m_stop = true;
        m_condition.notify_all(); // 唤醒所有阻塞的线程

        if (wait)
        {
            // 等待所有线程执行完剩余任务后退出
            for (std::thread& th : m_threads)
            {
                if (th.joinable())
                {
                    th.join(); // 阻塞等待线程结束
                }
            }
        }
        // 不 wait 时，仅让线程自行退出，不 detach
    }

    void ThreadPool::setRejectPolicy()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queuePolicy = QueueFullPolicy::kReject;
    }

    void ThreadPool::setBlockPolicy()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queuePolicy = QueueFullPolicy::kBlock;
    }

    void ThreadPool::setTimeoutPolicy(size_t timeoutUs)
    {
        if (timeoutUs <= 0)
        {
            throw std::invalid_argument("Timeout must be greater than 0 microseconds");
        }
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queuePolicy = QueueFullPolicy::kTimeout;
        m_timeoutUs = timeoutUs;
    }

    bool ThreadPool::addTask(std::function<void()> task)
    {
        if (m_stop)
        {
            return false;
        }

        {
            std::unique_lock<std::mutex> lock(m_mutex);

            // 处理队列大小限制
            if (m_maxQueueSize > 0)
            {
                // 等待条件：队列有空间 或 线程池停止
                // 注意：使用while循环而非if，防止虚假唤醒
                while (m_taskQueue.size() >= m_maxQueueSize && !m_stop)
                {
                    switch (m_queuePolicy)
                    {
                    case QueueFullPolicy::kReject:
                        return false; // 直接拒绝

                    case QueueFullPolicy::kBlock:
                        m_condition.wait(lock); // 阻塞等待
                        break;

                    case QueueFullPolicy::kTimeout:
                        // 超时等待
                        auto waitDuration = std::chrono::microseconds(m_timeoutUs);
                        bool waitResult = m_condition.wait_for(lock, waitDuration,
                                                               [this]()
                                                               { return m_taskQueue.size() < m_maxQueueSize || m_stop; });
                        if (!waitResult)
                        {
                            return false; // 超时拒绝
                        }
                        break;
                    }
                }
            }

            // 再次检查线程池状态
            if (m_stop)
            {
                return false;
            }

            m_taskQueue.push(std::move(task));
            ++m_taskCount;
        }

        m_condition.notify_one(); // 通知工作线程有新任务
        return true;
    }

    size_t ThreadPool::getTaskCount() const
    {
        return m_taskCount;
    }

    size_t ThreadPool::getThreadCount() const
    {
        return m_threads.size();
    }

    bool ThreadPool::isRunning() const
    {
        return !m_stop;
    }

} // namespace ol