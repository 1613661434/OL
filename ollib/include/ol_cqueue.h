#ifndef __OL_CQUEUE_H
#define __OL_CQUEUE_H 1

#include <iostream>
#include <string.h>    // 用于memset
#include <type_traits> // 用于std::is_pod
#include <utility>     // 用于std::move、std::forward

namespace ol
{

    // 循环队列。
    template <class T, size_t MAX_SIZE>
    class cqueue
    {
    private:
        static_assert(MAX_SIZE > 0, "MAX_SIZE must be greater than 0");
        bool m_inited = false;        // 队列被初始化标志，true-已初始化；false-未初始化。
        size_t m_size = 0;            // 队列的实际长度。
        T m_data[MAX_SIZE];           // 用数组存储循环队列中的元素。
        size_t m_front = 0;           // 队列的头指针。
        size_t m_rear = MAX_SIZE - 1; // 队列的尾指针，指向队尾元素。

        cqueue(const cqueue&) = delete;            // 禁用拷贝构造函数。
        cqueue& operator=(const cqueue&) = delete; // 禁用赋值函数。

    public:
        cqueue()
        {
            init();
        }

        ~cqueue()
        {
            if (m_inited == true)
            {
                // 非可平凡复制类型需要手动调用析构函数
                if constexpr (!std::is_trivially_destructible_v<T>)
                {
                    for (size_t i = 0; i < m_size; ++i)
                    {
                        size_t index = (m_front + i) % MAX_SIZE;
                        m_data[index].~T(); // 显式调用析构函数
                    }
                }
            }
        }

        // 移动构造函数
        cqueue(cqueue&& other) noexcept
            : m_inited(other.m_inited),
              m_size(other.m_size),
              m_front(other.m_front),
              m_rear(other.m_rear)
        {
            if (m_inited == true)
            {
                // 移动可平凡复制类型：直接内存拷贝
                if constexpr (std::is_trivially_copyable_v<T>)
                {
                    memcpy(m_data, other.m_data, MAX_SIZE * sizeof(T));
                }
                else
                {
                    // 非可平凡复制类型：逐个移动
                    for (size_t i = 0; i < other.m_size; ++i)
                    {
                        size_t src_idx = (other.m_front + i) % MAX_SIZE;
                        size_t dst_idx = (m_front + i) % MAX_SIZE;
                        m_data[dst_idx] = std::move(other.m_data[src_idx]);
                    }
                }
                // 清空原队列
                other.m_inited = false;
                other.m_size = 0;
            }
        }

        // 移动赋值运算符
        cqueue& operator=(cqueue&& other) noexcept
        {
            if (this != &other)
            {
                // 先释放当前队列的资源
                if (m_inited == true)
                {
                    if constexpr (!std::is_trivially_destructible_v<T>)
                    {
                        // 非可平凡析构类型：手动调用析构函数
                        for (size_t i = 0; i < m_size; ++i)
                        {
                            size_t index = (m_front + i) % MAX_SIZE;
                            m_data[index].~T(); // 显式调用析构函数
                        }
                    }
                    // 无论是否可平凡析构，都需要重置状态
                    m_inited = false;
                    m_size = 0;
                    m_front = 0;
                    m_rear = MAX_SIZE - 1;
                }

                // 移动其他队列的资源
                m_inited = other.m_inited;
                m_size = other.m_size;
                m_front = other.m_front;
                m_rear = other.m_rear;

                if (m_inited == true)
                {
                    if constexpr (std::is_trivially_copyable_v<T>)
                    {
                        // 可平凡复制类型：直接内存拷贝
                        memcpy(m_data, other.m_data, MAX_SIZE * sizeof(T));
                    }
                    else
                    {
                        // 非可平凡复制类型：逐个移动元素
                        for (size_t i = 0; i < other.m_size; ++i)
                        {
                            size_t src_idx = (other.m_front + i) % MAX_SIZE;
                            size_t dst_idx = (m_front + i) % MAX_SIZE;
                            m_data[dst_idx] = std::move(other.m_data[src_idx]);
                        }
                    }
                    // 清空原队列
                    other.m_inited = false;
                    other.m_size = 0;
                }
            }
            return *this;
        }

        // 循环队列的初始化操作。
        // 注意：如果用于共享内存的队列，不会调用构造函数，必须调用此函数初始化。
        void init()
        {
            if (m_inited == true) return; // 循环队列的初始化只能执行一次。
            m_inited = true;
            m_front = 0;           // 头指针。
            m_rear = MAX_SIZE - 1; // 为了方便写代码，初始化时，尾指针指向队列的最后一个位置。
            m_size = 0;            // 队列的实际长度。

            // 数组元素初始化。
            if constexpr (std::is_trivially_copyable_v<T>)
            { // 可平凡复制类型专用初始化（效率优先）
                memset(m_data, 0, sizeof(m_data));
            }
            else
            {
                for (size_t i = 0; i < MAX_SIZE; ++i)
                {                    // 非可平凡复制类型通用初始化
                    m_data[i] = T(); // 调用默认构造函数
                }
            }
        }

        // 判断循环队列是否已满，返回值：true-已满，false-未满。
        bool full() const
        {
            return m_size == MAX_SIZE;
        }

        // 判断循环队列是否为空，返回值：true-空，false-非空。
        bool empty() const
        {
            return m_size == 0;
        }

        // 元素入队，返回值：false-失败；true-成功。
        bool push(const T& e)
        {
            if (full())
            {
                std::cerr << "循环队列已满，入队失败。\n";
                return false;
            }

            // 先移动队尾指针，然后再拷贝数据。
            m_rear = (m_rear + 1) % MAX_SIZE; // 队尾指针后移。
            m_data[m_rear] = e;
            ++m_size;

            return true;
        }

        // 支持右值引用的push
        bool push(T&& e)
        {
            if (full())
            {
                std::cerr << "循环队列已满，入队失败。\n";
                return false;
            }
            m_rear = (m_rear + 1) % MAX_SIZE;
            m_data[m_rear] = std::move(e);
            ++m_size;
            return true;
        }

        // 元素出队，返回值：false-失败；true-成功。
        bool pop()
        {
            if (empty()) return false;

            m_front = (m_front + 1) % MAX_SIZE; // 队列头指针后移。
            --m_size;

            return true;
        }

        // 求循环队列的长度，返回值：>=0  队列中元素的个数。
        size_t size() const
        {
            return m_size;
        }

        // 查看队头元素的值，元素不出队。
        T& front()
        {
            if (empty()) throw std::out_of_range("队列为空");
            return m_data[m_front];
        }

        const T& front() const
        {
            if (empty()) throw std::out_of_range("队列为空");
            return m_data[m_front];
        }

        // 原地构造元素
        template <typename... Args>
        bool emplace(Args&&... args)
        {
            if (full())
            {
                std::cerr << "循环队列已满，入队失败。\n";
                return false;
            }
            m_rear = (m_rear + 1) % MAX_SIZE;
            new (&m_data[m_rear]) T(std::forward<Args>(args)...);
            ++m_size;
            return true;
        }

        // 显示循环队列中全部的元素。
        // 这是一个临时的用于调试的函数，队列中元素的数据类型支持cout输出才可用。
        void printqueue() const
        {
            for (size_t i = 0; i < size(); ++i)
            {
                std::cout << "m_data[" << (m_front + i) % MAX_SIZE << "],value="
                          << m_data[(m_front + i) % MAX_SIZE] << std::endl;
            }
        }
    };

} // namespace ol

#endif // !__OL_CQUEUE_H