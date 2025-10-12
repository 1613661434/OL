/***************************************************************************************/
/*
 * 程序名：ol_mutex.h
 * 功能描述：多线程同步互斥锁实现集合，当前包含：
 *        自旋锁类（spin_mutex）：基于 std::atomic_flag 实现的轻量级无锁互斥机制
 *          - 特性：不可拷贝、不可移动，通过自旋等待获取锁，适合短临界区场景
 *          - 接口：lock()（阻塞直到获取锁）、unlock()（释放锁）
 * 作者：ol
 * 适用标准：C++ 11 及以上（需支持 atomic_flag、delete 关键字及基类继承特性）
 */
/***************************************************************************************/

#ifndef OL_MUTEX_H
#define OL_MUTEX_H 1

#include "ol_type_traits.h"
#include <memory>

namespace ol
{

    // ===========================================================================
    // 自旋锁类，用于多线程同步
    class spin_mutex : public TypeNonCopyableMovable
    {
    private:
        std::atomic_flag flag;

    public:
        // 构造函数，初始化原子标志
        spin_mutex()
        {
            flag.clear();
        }

        // 加锁（自旋等待直到获取锁）
        inline void lock()
        {
            while (flag.test_and_set());
        }

        // 解锁
        inline void unlock()
        {
            flag.clear();
        }
    };
    // ===========================================================================

} // namespace ol

#endif // !OL_MUTEX_H