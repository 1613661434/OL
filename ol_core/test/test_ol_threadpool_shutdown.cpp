#include "ol_ThreadPool.h"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <future>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <type_traits>

namespace
{
    using FixedPool = ol::ThreadPool<false>;
    using namespace std::chrono_literals;

    static_assert(std::is_same_v<std::underlying_type_t<FixedPool::State>, char>,
                  "ThreadPool::State must use char as its underlying type");
    static_assert(sizeof(FixedPool::State) == sizeof(char),
                  "ThreadPool::State must occupy one byte");

    void require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }

    void testGracefulRequestAndWait()
    {
        FixedPool pool(1);
        std::promise<void> firstTaskStarted;
        std::promise<void> releaseFirstTask;
        auto release = releaseFirstTask.get_future().share();
        std::atomic<int> completed{0};

        require(pool.addTask([&]() {
                    firstTaskStarted.set_value();
                    release.wait();
                    completed.fetch_add(1, std::memory_order_relaxed);
                }),
                "Could not add the first graceful-stop task");
        firstTaskStarted.get_future().wait();

        constexpr int queuedTaskCount = 12;
        for (int i = 0; i < queuedTaskCount; ++i)
        {
            require(pool.addTask([&completed]() {
                        completed.fetch_add(1, std::memory_order_relaxed);
                    }),
                    "Could not add a queued graceful-stop task");
        }

        pool.requestStop();
        require(pool.getState() == FixedPool::State::Draining,
                "requestStop() did not enter Draining state");
        require(!pool.addTask([]() {}), "Draining pool accepted a new task");

        releaseFirstTask.set_value();
        pool.wait();
        pool.wait(); // wait()必须幂等。

        require(completed.load(std::memory_order_relaxed) == queuedTaskCount + 1,
                "Graceful stop discarded an accepted task");
        require(pool.getState() == FixedPool::State::Stopped,
                "wait() did not enter Stopped state");
    }

    void testStopNowCancelsPendingTasks()
    {
        FixedPool pool(1);
        std::promise<void> firstTaskStarted;
        std::promise<void> releaseFirstTask;
        auto release = releaseFirstTask.get_future().share();
        std::atomic<int> completed{0};

        require(pool.addTask([&]() {
                    firstTaskStarted.set_value();
                    release.wait();
                    completed.fetch_add(1, std::memory_order_relaxed);
                }),
                "Could not add the running stopNow task");
        firstTaskStarted.get_future().wait();

        for (int i = 0; i < 12; ++i)
        {
            require(pool.addTask([&completed]() {
                        completed.fetch_add(1, std::memory_order_relaxed);
                    }),
                    "Could not add a pending stopNow task");
        }

        std::thread stopper([&pool]() { pool.stopNow(); });
        const auto deadline = std::chrono::steady_clock::now() + 2s;
        while (pool.getState() == FixedPool::State::Running &&
               std::chrono::steady_clock::now() < deadline)
        {
            std::this_thread::yield();
        }

        const bool enteredCancelling = pool.getState() == FixedPool::State::Cancelling;
        const bool rejectedNewTask = !pool.addTask([]() {});
        releaseFirstTask.set_value();
        stopper.join();

        require(enteredCancelling, "stopNow() did not enter Cancelling state");
        require(rejectedNewTask, "Cancelling pool accepted a new task");
        require(completed.load(std::memory_order_relaxed) == 1,
                "stopNow() executed a pending task");
        require(pool.getTaskNum() == 0, "stopNow() did not clear the pending queue");
        require(pool.getState() == FixedPool::State::Stopped,
                "stopNow() did not finish in Stopped state");
    }

    void testWorkerCanRequestStopButCannotJoinItself()
    {
        {
            FixedPool pool(1);
            std::promise<void> stopRequested;
            require(pool.addTask([&]() {
                        pool.requestStop();
                        stopRequested.set_value();
                    }),
                    "Could not add worker requestStop task");

            require(stopRequested.get_future().wait_for(2s) == std::future_status::ready,
                    "Worker requestStop() did not return");
            pool.wait();
            require(pool.getState() == FixedPool::State::Stopped,
                    "External wait() did not join a worker-requested shutdown");
        }

        {
            FixedPool pool(1);
            std::promise<bool> selfJoinRejected;
            auto selfJoinFuture = selfJoinRejected.get_future();
            require(pool.addTask([&]() {
                        bool rejected = false;
                        try
                        {
                            pool.stop();
                        }
                        catch (const std::logic_error&)
                        {
                            rejected = true;
                        }
                        selfJoinRejected.set_value(rejected);
                    }),
                    "Could not add worker self-join task");

            require(selfJoinFuture.wait_for(2s) == std::future_status::ready,
                    "Worker stop() call did not return");
            require(selfJoinFuture.get(), "Worker stop() did not reject self-join");
            require(pool.getState() == FixedPool::State::Running,
                    "Rejected worker stop() changed pool state");
            pool.stop();
        }
    }

    void testWaitRequiresAStopRequest()
    {
        FixedPool pool(1);
        bool rejected = false;
        try
        {
            pool.wait();
        }
        catch (const std::logic_error&)
        {
            rejected = true;
        }

        require(rejected, "wait() accepted a Running pool without requestStop()");
        pool.stop();
    }

    void testDynamicPoolAlsoDrains()
    {
        ol::ThreadPool<true> pool(2, 4, 0, 1s);
        std::atomic<int> completed{0};
        constexpr int taskCount = 20;
        for (int i = 0; i < taskCount; ++i)
        {
            require(pool.addTask([&completed]() {
                        std::this_thread::sleep_for(1ms);
                        completed.fetch_add(1, std::memory_order_relaxed);
                    }),
                    "Could not add a dynamic-pool task");
        }

        pool.stop();
        require(completed.load(std::memory_order_relaxed) == taskCount,
                "Dynamic pool discarded an accepted task during stop()");
        require(pool.getState() == ol::ThreadPool<true>::State::Stopped,
                "Dynamic pool did not enter Stopped state");
    }
} // namespace

int main()
{
    try
    {
        testGracefulRequestAndWait();
        testStopNowCancelsPendingTasks();
        testWorkerCanRequestStopButCannotJoinItself();
        testWaitRequiresAStopRequest();
        testDynamicPoolAlsoDrains();
        std::cout << "thread pool shutdown regression tests passed\n";
        return EXIT_SUCCESS;
    }
    catch (const std::exception& error)
    {
        std::cerr << "thread pool shutdown regression test failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
