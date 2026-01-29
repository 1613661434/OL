/****************************************************************************************/
/*
 * 程序名：ol_mutex.h
 * 功能描述：轻量级多线程同步互斥锁库，基于 std::atomic_flag 实现，支持跨平台（Linux/Windows）
 *          - 不可重入自旋锁（spin_mutex）：对齐 std::mutex 接口
 *          - 可递归自旋锁（recursive_spin_mutex）：对齐 std::recursive_mutex 接口
 *          - 均兼容 std::lock_guard/unique_lock
 * 作者：ol
 * 适用标准：C++11及以上（需支持 atomic、thread、stdexcept 等特性）
 */
/****************************************************************************************/

#ifndef OL_MUTEX_H
#define OL_MUTEX_H 1

#include "ol_type_traits.h"
#include <atomic>
#include <thread>
#include <stdexcept>

namespace ol
{
    /**
     * @brief 不可重入自旋锁
     */
    class spin_mutex : public TypeNonCopyableMovable
    {
    private:
        std::atomic_flag flag = ATOMIC_FLAG_INIT; ///< 原子标志位（默认未锁定，C++11 兼容）

    public:
        /**
         * @brief 构造函数，初始化原子标志为未锁定状态
         */
        spin_mutex() noexcept = default;

        /**
         * @brief 析构函数，确保锁未被持有（用户需保证解锁后销毁）
         */
        ~spin_mutex() noexcept = default;

        /**
         * @brief 加锁操作，自旋等待直到获取锁
         */
        void lock() noexcept
        {
            while (flag.test_and_set(std::memory_order_acquire));
        }

        /**
         * @brief 尝试加锁操作，无阻塞，立即返回结果
         * @return true-加锁成功，false-加锁失败（锁已被其他线程持有）
         */
        bool try_lock() noexcept
        {
            return !flag.test_and_set(std::memory_order_acquire);
        }

        /**
         * @brief 解锁操作，释放锁允许其他线程竞争
         */
        void unlock() noexcept
        {
            flag.clear(std::memory_order_release);
        }
    };

    /**
     * @brief 可递归自旋锁
     */
    class recursive_spin_mutex : public TypeNonCopyableMovable
    {
    private:
        std::atomic_flag flag = ATOMIC_FLAG_INIT;                     ///< 核心自旋标志
        std::atomic<std::thread::id> owner_thread{std::thread::id()}; ///< 当前持有锁的线程ID
        std::atomic<int> recursion_count{0};                          ///< 递归加锁计数

    public:
        /**
         * @brief 构造函数，初始化成员变量为无锁状态
         */
        recursive_spin_mutex() noexcept = default;

        /**
         * @brief 析构函数，确保锁未被持有（用户需保证解锁后销毁）
         */
        ~recursive_spin_mutex() noexcept = default;

        /**
         * @brief 加锁操作，同一线程递归加锁递增计数，其他线程自旋等待
         */
        void lock() noexcept
        {
            const std::thread::id current_thread = std::this_thread::get_id();
            const std::thread::id current_owner = owner_thread.load(std::memory_order_acquire);

            // 同一线程：递归加锁，递增计数
            if (current_thread == current_owner)
            {
                recursion_count.fetch_add(1, std::memory_order_relaxed);
                return;
            }

            // 不同线程：自旋等待获取锁
            while (flag.test_and_set(std::memory_order_acquire));

            // 记录持有者和初始计数
            owner_thread.store(current_thread, std::memory_order_release);
            recursion_count.store(1, std::memory_order_relaxed);
        }

        /**
         * @brief 尝试加锁操作，无阻塞，立即返回结果
         * @return true-加锁成功，false-加锁失败（锁已被其他线程持有）
         */
        bool try_lock() noexcept
        {
            const std::thread::id current_thread = std::this_thread::get_id();
            const std::thread::id current_owner = owner_thread.load(std::memory_order_acquire);

            // 同一线程：直接递增计数，返回成功
            if (current_thread == current_owner)
            {
                recursion_count.fetch_add(1, std::memory_order_relaxed);
                return true;
            }

            // 不同线程：尝试获取锁，无阻塞
            if (!flag.test_and_set(std::memory_order_acquire))
            {
                owner_thread.store(current_thread, std::memory_order_release);
                recursion_count.store(1, std::memory_order_relaxed);
                return true;
            }

            // 锁已被其他线程持有，返回失败
            return false;
        }

        /**
         * @brief 解锁操作，递归计数减至0时释放锁，非持有者调用抛异常
         */
        void unlock()
        {
            const std::thread::id current_thread = std::this_thread::get_id();
            const std::thread::id current_owner = owner_thread.load(std::memory_order_acquire);

            // 非持有线程解锁：抛逻辑错误异常
            if (current_thread != current_owner)
            {
                throw std::logic_error("recursive_spin_mutex::unlock(): called by non-owner thread");
            }

            // 递减计数，判断是否需要释放锁
            const int new_count = recursion_count.fetch_sub(1, std::memory_order_relaxed) - 1;
            if (new_count <= 0)
            {
                flag.clear(std::memory_order_release);
                owner_thread.store(std::thread::id(), std::memory_order_release);
                recursion_count.store(0, std::memory_order_relaxed);
            }
        }
    };

} // namespace ol

#endif // !OL_MUTEX_H