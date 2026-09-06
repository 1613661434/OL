#include "ol_mutex.h"

#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

int main()
{
    ol::recursive_spin_mutex mutex;
    constexpr int threadCount = 4;
    constexpr int iterations = 50000;

    std::atomic<int> ready{0};
    std::atomic_bool start{false};
    int protectedCounter = 0;

    std::vector<std::thread> threads;
    threads.reserve(threadCount);
    for (int i = 0; i < threadCount; ++i)
    {
        threads.emplace_back([&]()
                             {
            ready.fetch_add(1, std::memory_order_release);
            while (!start.load(std::memory_order_acquire)) std::this_thread::yield();

            for (int n = 0; n < iterations; ++n)
            {
                mutex.lock();
                mutex.lock();
                ++protectedCounter;
                mutex.unlock();
                mutex.unlock();
            } });
    }

    while (ready.load(std::memory_order_acquire) != threadCount) std::this_thread::yield();
    start.store(true, std::memory_order_release);

    for (auto& thread : threads) thread.join();

    const int expected = threadCount * iterations;
    if (protectedCounter != expected)
    {
        std::cerr << "recursive_spin_mutex handoff failed: expected=" << expected
                  << " actual=" << protectedCounter << '\n';
        return 1;
    }

    std::cout << "recursive_spin_mutex handoff regression test passed\n";
    return 0;
}
