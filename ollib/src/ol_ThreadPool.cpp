#include "ol_ThreadPool.h"

#ifdef __linux__
#include <sys/syscall.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

// #define DEBUG

namespace ol
{

    ThreadPool::ThreadPool(size_t threadNum, size_t maxQueueSize)
        : m_stop(false), m_taskCount(0), m_maxQueueSize(maxQueueSize)
    {
        if (threadNum == 0)
        {
            throw std::invalid_argument("Thread number must be greater than 0.");
        }

        // 启动threadNum个线程
        for (size_t i = 0; i < threadNum; ++i)
        {
            m_threads.emplace_back([this]
                                   {
// 线程创建时的 ID 输出
#ifdef DEBUG
#ifdef __linux__
printf("Created thread(%ld).\n", syscall(SYS_gettid));
#elif defined(_WIN32)
printf("Created thread(%u).\n", GetCurrentThreadId());  // Windows 系统线程 ID
#else
std::cout << "Created thread: " << std::this_thread::get_id() << std::endl;
#endif // __linux__
#endif // DEBUG

                while (!m_stop)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(m_mutex);
                        
                        // 等待任务或停止信号
                        m_condition.wait(lock, [this] { return m_stop || !m_taskQueue.empty(); });

                        // 如果线程池已停止且任务队列为空，则退出
                        if (m_stop && m_taskQueue.empty()) return;

                        // 取出任务
                        task = std::move(m_taskQueue.front());
                        m_taskQueue.pop();
                        --m_taskCount;
                    }

// 任务执行时的 ID 输出
#ifdef DEBUG
#ifdef __linux__
printf("Thread(%ld) is executing task.\n", syscall(SYS_gettid));
#elif defined(_WIN32)
printf("Thread(%u) is executing task.\n", GetCurrentThreadId());  // Windows 系统线程 ID
#else
std::cout << "Thread " << std::this_thread::get_id() << " is executing task." << std::endl;
#endif // __linux__
#endif // DEBUG

                    try
                    {
                        task(); // 执行任务
                    }
                    catch (const std::exception& e)
                    {
                        std::cerr << "Task execution error: " << e.what() << std::endl;
                    }
                    catch (...)
                    {
                        std::cerr << "Unknown error occurred during task execution." << std::endl;
                    }
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
        else
        {
            // 不等待线程，直接 detach（线程会在后台继续执行完当前任务后自动销毁）
            // 注意：detach后无法再控制线程，仅建议在确保任务无需同步结果时使用
            for (std::thread& th : m_threads)
            {
                if (th.joinable())
                {
                    th.detach();
                }
            }
        }
    }

    bool ThreadPool::addTask(std::function<void()> task)
    {
        if (m_stop)
        {
            return false;
        }

        {
            std::unique_lock<std::mutex> lock(m_mutex);

            // 如果队列已满，返回失败
            if (m_maxQueueSize > 0 && m_maxQueueSize <= m_taskQueue.size())
            {
                return false;
            }

            m_taskQueue.push(std::move(task));
            ++m_taskCount;
        }

        m_condition.notify_one();
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