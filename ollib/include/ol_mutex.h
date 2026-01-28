/****************************************************************************************/
/*
 * 程序名：ol_mutex.h
 * 功能描述：轻量级多线程同步互斥锁库，基于 std::atomic_flag 实现，支持跨平台（Linux/Windows），特性包括：
 *          - 不可重入自旋锁（spin_mutex）：轻量级无锁互斥，适合短临界区场景
 *          - 可递归自旋锁（recursive_spin_mutex）：支持同一线程重复加锁，解决递归调用死锁问题
 *          - RAII 守卫类（lock_guard_spin/lock_guard_recursive_spin）：构造加锁、析构解锁，防止遗漏解锁
 *          - 唯一守卫类（unique_lock_spin/unique_lock_recursive_spin）：支持手动控制加锁/解锁，灵活度更高
 *          - 异常安全：非持有线程解锁抛出 std::logic_error 异常，便于调试定位问题
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

    // 锁类
    // ===========================================================================
    /**
     * @brief 不可重入自旋锁（基于 std::atomic_flag 实现）
     * @note 1. 轻量级无锁互斥，适合**短临界区**场景，避免长时间占用导致CPU空耗
     * @note 2. 不支持同一线程重复加锁，递归调用会导致死锁
     * @note 3. 多线程安全，符合 C++11 内存模型规范
     */
    class spin_mutex : public TypeNonCopyableMovable
    {
    private:
        std::atomic_flag flag; ///< 原子标志位，用于控制锁竞争（false=未锁定，true=已锁定）

    public:
        /**
         * @brief 构造函数，初始化原子标志为未锁定状态
         */
        spin_mutex()
        {
            flag.clear(std::memory_order_relaxed);
        }

        /**
         * @brief 加锁操作，自旋等待直到获取锁
         * @note 自旋过程中CPU空转，适合临界区执行时间极短的场景
         */
        inline void lock()
        {
            while (flag.test_and_set(std::memory_order_acquire));
        }

        /**
         * @brief 解锁操作，释放锁允许其他线程竞争
         * @note 必须由持有锁的线程调用，否则行为未定义
         */
        inline void unlock()
        {
            flag.clear(std::memory_order_release);
        }
    };

    /**
     * @brief 可递归自旋锁（支持同一线程重复加锁，解决递归调用死锁问题）
     * @note 1. 基于不可重入自旋锁扩展，增加线程持有者记录和递归计数
     * @note 2. 非持有线程调用解锁会抛出 std::logic_error 异常
     * @note 3. 递归加锁次数无上限，需避免无限递归导致栈溢出
     */
    class recursive_spin_mutex : public TypeNonCopyableMovable
    {
    private:
        std::atomic_flag flag;                     ///< 核心自旋标志，控制多线程锁竞争
        std::atomic<std::thread::id> owner_thread; ///< 记录当前持有锁的线程ID（空ID=无持有者）
        std::atomic<int> recursion_count;          ///< 记录同一线程的递归加锁次数

    public:
        /**
         * @brief 构造函数，初始化成员变量为默认无锁状态
         */
        recursive_spin_mutex()
        {
            flag.clear(std::memory_order_relaxed);
            owner_thread.store(std::thread::id(), std::memory_order_relaxed);
            recursion_count.store(0, std::memory_order_relaxed);
        }

        /**
         * @brief 加锁操作，同一线程递归加锁直接递增计数，不同线程自旋竞争
         * @note 1. 同一线程多次调用不会阻塞，仅递增递归计数
         * @note 2. 不同线程会自旋等待，直到锁被释放
         */
        inline void lock()
        {
            const std::thread::id current_thread = std::this_thread::get_id();
            const std::thread::id current_owner = owner_thread.load(std::memory_order_acquire);

            // 同一线程，递归加锁，无需竞争
            if (current_thread == current_owner)
            {
                recursion_count.fetch_add(1, std::memory_order_relaxed);
                return;
            }

            // 不同线程，自旋等待获取锁
            while (flag.test_and_set(std::memory_order_acquire));

            // 获取锁成功，记录持有者和初始计数
            owner_thread.store(current_thread, std::memory_order_release);
            recursion_count.store(1, std::memory_order_relaxed);
        }

        /**
         * @brief 解锁操作，递归计数减至0时释放锁，非持有线程调用抛异常
         * @throw std::logic_error 非持有线程尝试解锁时抛出该异常，包含详细错误信息
         * @note 1. 仅当递归计数归0时，才会真正释放锁并清空持有者记录
         * @note 2. 异常信息格式为："recursive_spin_mutex::unlock(): called by non-owner thread"
         */
        inline void unlock()
        {
            const std::thread::id current_thread = std::this_thread::get_id();
            const std::thread::id current_owner = owner_thread.load(std::memory_order_acquire);

            // 非持有线程尝试解锁，抛逻辑错误异常
            if (current_thread != current_owner)
            {
                throw std::logic_error("recursive_spin_mutex::unlock(): called by non-owner thread");
            }

            // 递减递归计数，判断是否需要真正释放锁
            const int new_count = recursion_count.fetch_sub(1, std::memory_order_relaxed) - 1;
            if (new_count <= 0)
            {
                flag.clear(std::memory_order_release);
                owner_thread.store(std::thread::id(), std::memory_order_release);
                recursion_count.store(0, std::memory_order_relaxed);
            }
        }
    };
    // ===========================================================================

    // RAII 守卫类
    // ===========================================================================
    /**
     * @brief 不可重入自旋锁 RAII 守卫（构造加锁，析构解锁，防止遗漏解锁）
     * @note 1. 遵循 RAII 设计模式，生命周期与锁绑定
     * @note 2. 不可拷贝、不可移动，仅能在当前作用域使用
     * @note 3. 适合临界区为整个作用域的场景，无需手动调用 lock/unlock
     */
    class lock_guard_spin : public TypeNonCopyableMovable
    {
    private:
        spin_mutex& m_mutex; ///< 待管理的不可重入自旋锁引用

    public:
        /**
         * @brief 构造函数，传入待管理的自旋锁并执行加锁
         * @param mutex 不可重入自旋锁引用（必须有效，且未被当前线程持有）
         */
        explicit lock_guard_spin(spin_mutex& mutex) : m_mutex(mutex)
        {
            m_mutex.lock();
        }

        /**
         * @brief 析构函数，自动执行解锁操作
         * @note 无论作用域正常退出还是异常退出，都会调用解锁，保证锁的释放
         */
        ~lock_guard_spin()
        {
            m_mutex.unlock();
        }
    };

    /**
     * @brief 可递归自旋锁 RAII 守卫（对应 recursive_spin_mutex，支持递归加锁自动解锁）
     * @note 1. 与不可重入守卫类逻辑一致，适配可递归自旋锁
     * @note 2. 递归加锁时，析构函数会对应递减递归计数，直到计数归0释放锁
     * @note 3. 非持有线程构造会阻塞，析构时若非持有者会抛异常
     */
    class lock_guard_recursive_spin : public TypeNonCopyableMovable
    {
    private:
        recursive_spin_mutex& m_mutex; ///< 待管理的可递归自旋锁引用

    public:
        /**
         * @brief 构造函数，传入待管理的可递归自旋锁并执行加锁
         * @param mutex 可递归自旋锁引用（必须有效）
         */
        explicit lock_guard_recursive_spin(recursive_spin_mutex& mutex) : m_mutex(mutex)
        {
            m_mutex.lock();
        }

        /**
         * @brief 析构函数，自动执行解锁操作
         * @throw std::logic_error 若当前线程非锁持有者，会抛出该异常
         */
        ~lock_guard_recursive_spin()
        {
            m_mutex.unlock();
        }
    };
    // ===========================================================================

    // 唯一守卫类
    // ===========================================================================
    /**
     * @brief 不可重入自旋锁唯一守卫（支持手动控制加锁/解锁，灵活度高于 RAII 守卫）
     * @note 1. 与 lock_guard_spin 相比，支持手动锁定/解锁、重复锁定（需判断状态）
     * @note 2. 析构时若持有锁，会自动解锁，防止遗漏
     * @note 3. 适合临界区非连续、需要手动控制锁生命周期的场景
     */
    class unique_lock_spin : public TypeNonCopyableMovable
    {
    private:
        spin_mutex& m_mutex; ///< 待管理的不可重入自旋锁引用
        bool m_owned;        ///< 标记当前是否持有锁（true=持有，false=未持有）

    public:
        /**
         * @brief 构造函数，传入待管理的自旋锁并初始化持有状态
         * @param mutex 不可重入自旋锁引用（必须有效）
         */
        explicit unique_lock_spin(spin_mutex& mutex) : m_mutex(mutex), m_owned(false)
        {
            lock();
        }

        /**
         * @brief 析构函数，若持有锁则自动解锁
         */
        ~unique_lock_spin()
        {
            if (m_owned)
            {
                m_mutex.unlock();
            }
        }

        /**
         * @brief 手动加锁（未持有锁时才执行）
         * @note 已持有锁时调用无效果，不会递增递归计数（不可重入）
         */
        void lock()
        {
            if (!m_owned)
            {
                m_mutex.lock();
                m_owned = true;
            }
        }

        /**
         * @brief 手动解锁（持有锁时才执行）
         * @note 未持有锁时调用无效果，不会导致未定义行为
         */
        void unlock()
        {
            if (m_owned)
            {
                m_mutex.unlock();
                m_owned = false;
            }
        }

        /**
         * @brief 判断当前是否持有锁
         * @return true-已持有锁，false-未持有锁
         */
        bool isLock() const { return m_owned; }
    };

    /**
     * @brief 可递归自旋锁唯一守卫（支持手动控制，兼容递归加锁场景）
     * @note 1. 适配 recursive_spin_mutex，支持手动锁定/解锁和递归加锁
     * @note 2. 析构时若持有锁，会自动解锁（递减递归计数至0释放锁）
     * @note 3. 非持有线程调用 unlock() 会抛 std::logic_error 异常
     */
    class unique_lock_recursive_spin : public TypeNonCopyableMovable
    {
    private:
        recursive_spin_mutex& m_mutex; ///< 待管理的可递归自旋锁引用
        bool m_owned;                  ///< 标记当前是否持有锁（true=持有，false=未持有）

    public:
        /**
         * @brief 构造函数，传入待管理的可递归自旋锁并初始化持有状态
         * @param mutex 可递归自旋锁引用（必须有效）
         */
        explicit unique_lock_recursive_spin(recursive_spin_mutex& mutex) : m_mutex(mutex), m_owned(false)
        {
            lock();
        }

        /**
         * @brief 析构函数，若持有锁则自动解锁
         * @throw std::logic_error 若当前线程非锁持有者，会抛出该异常
         */
        ~unique_lock_recursive_spin()
        {
            if (m_owned)
            {
                m_mutex.unlock();
            }
        }

        /**
         * @brief 手动加锁（未持有锁时才执行，支持递归）
         * @note 已持有锁时调用无效果，递归计数由 recursive_spin_mutex 内部维护
         */
        void lock()
        {
            if (!m_owned)
            {
                m_mutex.lock();
                m_owned = true;
            }
        }

        /**
         * @brief 手动解锁（持有锁时才执行，递归计数归0时释放锁）
         * @throw std::logic_error 若当前线程非锁持有者，会抛出该异常
         * @note 未持有锁时调用无效果，不会导致未定义行为
         */
        void unlock()
        {
            if (m_owned)
            {
                m_mutex.unlock();
                m_owned = false;
            }
        }

        /**
         * @brief 判断当前是否持有锁
         * @return true-已持有锁，false-未持有锁
         */
        bool isLock() const { return m_owned; }
    };
    // ===========================================================================

} // namespace ol

#endif // !OL_MUTEX_H