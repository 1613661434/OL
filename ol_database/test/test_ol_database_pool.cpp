#include "ol_database.h"

#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <memory>
#include <thread>
#include <type_traits>

namespace
{
    class FakeConnection : public ol::IDBConn
    {
    public:
        inline static std::atomic_int connectCount{0};
        inline static std::atomic_int disconnectCount{0};
        inline static std::atomic_int resetCount{0};

    private:
        bool m_connected{false};

    public:
        bool connect() override
        {
            ++connectCount;
            m_connected = true;
            return true;
        }

        void disconnect() override
        {
            if (m_connected) ++disconnectCount;
            m_connected = false;
        }

        bool isConnected() const override { return m_connected; }
        void reset() override { ++resetCount; }
    };

    bool require(bool condition, const char* message)
    {
        if (condition) return true;
        std::cerr << message << '\n';
        return false;
    }
}

int main()
{
    using Pool = ol::DBPool<FakeConnection>;
    using Lease = Pool::ConnPtr;

    static_assert(!std::is_copy_constructible_v<Lease>);
    static_assert(!std::is_copy_assignable_v<Lease>);
    static_assert(std::is_nothrow_move_constructible_v<Lease>);
    static_assert(!std::is_constructible_v<Lease, Pool*, std::unique_ptr<FakeConnection>>);

    Pool& pool = Pool::GetInst();
    pool.init(2, [](FakeConnection&) {});

    if (!require(pool.idle() == 2, "pool did not initialize both connections")) return 1;

    {
        Lease first = pool.get();
        if (!require(static_cast<bool>(first), "failed to acquire first lease")) return 1;
        if (!require(pool.idle() == 1, "acquired connection remained in idle queue")) return 1;

        Lease moved = std::move(first);
        if (!require(!first && moved, "moving a lease did not transfer ownership")) return 1;
    }

    if (!require(pool.idle() == 2, "lease destructor did not return its connection")) return 1;

    Lease manual = pool.get();
    pool.release(manual);
    pool.release(manual);
    if (!require(pool.idle() == 2, "repeated lease release duplicated a connection")) return 1;

    Lease heldFirst = pool.get();
    Lease heldSecond = pool.get();
    std::promise<void> waiterStarted;
    auto waiter = std::async(std::launch::async, [&pool, &waiterStarted]()
                             {
        waiterStarted.set_value();
        auto lease = pool.get();
        return static_cast<bool>(lease); });

    waiterStarted.get_future().wait();
    if (!require(waiter.wait_for(std::chrono::milliseconds(50)) == std::future_status::timeout,
                 "waiter did not block while all connections were leased"))
        return 1;

    pool.destroy();
    if (!require(waiter.wait_for(std::chrono::seconds(2)) == std::future_status::ready,
                 "destroy did not wake a blocked waiter"))
        return 1;
    if (!require(!waiter.get(), "destroyed pool returned a connection to a waiter")) return 1;

    heldFirst.reset();
    heldSecond.reset();
    if (!require(!pool.isRunning(), "destroyed pool still reports Running")) return 1;
    if (!require(pool.idle() == 0, "leases returned after destroy were put back into the pool")) return 1;
    if (!require(FakeConnection::disconnectCount.load() == 2,
                 "connections returned after destroy were not disconnected exactly once"))
        return 1;

    auto afterDestroy = pool.getTimeout(std::chrono::milliseconds(1));
    if (!require(!afterDestroy, "destroyed pool still issued a lease")) return 1;

    std::cout << "DBPool RAII lease regression test passed\n";
    return 0;
}
