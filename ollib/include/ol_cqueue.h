/****************************************************************************************/
/*
 * 程序名：ol_cqueue.h
 * 功能描述：固定大小的循环队列（circular queue）模板类，支持以下特性：
 *          - 基于静态数组实现，大小由模板参数指定
 *          - 支持POD类型和非POD类型，自动适配内存管理
 *          - 提供入队、出队、查看队头、判断空满等基础操作
 *          - 支持移动构造和移动赋值，禁用拷贝构造和赋值（避免资源冲突）
 *          - 支持原地构造元素（emplace）以提升性能
 * 作者：ol
 * 适用标准：C++11及以上（需支持constexpr、type_traits、右值引用等特性）
 */
/****************************************************************************************/

#ifndef OL_CQUEUE_H
#define OL_CQUEUE_H 1

#include <iostream>
#include <stdexcept>   // 用于std::out_of_range
#include <string.h>    // 用于memset
#include <type_traits> // 用于std::is_pod
#include <utility>     // 用于std::move、std::forward

namespace ol
{

    /**
     * @brief 循环队列模板类
     *        基于静态数组实现，大小固定，支持高效的FIFO（先进先出）操作
     * @tparam T 队列中元素的数据类型
     * @tparam MAX_SIZE 队列的最大容量（必须大于0）
     */
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
        // 构造函数，自动初始化队列
        cqueue()
        {
            init();
        }

        // 析构函数，释放资源（非平凡析构类型需手动调用析构函数）
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

        /**
         * @brief 移动构造函数
         * @param other 待移动的队列对象
         */
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

        /**
         * @brief 移动赋值运算符
         * @param other 待移动的队列对象
         * @return 当前队列对象的引用
         */
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

        /**
         * @brief 初始化队列（仅未初始化时有效）
         *        用于共享内存场景（不自动调用构造函数时）
         * @note 如果用于共享内存的队列，不会调用构造函数，必须调用此函数初始化。
         */
        void init()
        {
            if (m_inited == true) return; // 循环队列的初始化只能执行一次。
            m_inited = true;
            m_front = 0;           // 头指针。
            m_rear = MAX_SIZE - 1; // 为了方便写代码，初始化时，尾指针指向队列的最后一个位置。
            m_size = 0;            // 队列的实际长度。

            // 数组元素初始化。
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                // 可平凡复制类型专用初始化（效率优先）
                memset(m_data, 0, sizeof(m_data));
            }
            else
            {
                for (size_t i = 0; i < MAX_SIZE; ++i)
                {
                    // 非可平凡复制类型通用初始化
                    m_data[i] = T(); // 调用默认构造函数
                }
            }
        }

        /**
         * @brief 判断队列是否已满
         * @return true-队列已满，false-队列未满
         */
        bool full() const
        {
            return m_size == MAX_SIZE;
        }

        /**
         * @brief 判断队列是否为空
         * @return true-队列为空，false-队列非空
         */
        bool empty() const
        {
            return m_size == 0;
        }

        /**
         * @brief 元素入队（拷贝版本）
         * @param e 待入队的元素（常量引用）
         * @return true-入队成功，false-队列已满入队失败
         */
        bool push(const T& e)
        {
            if (full())
            {
                std::cerr << "Circular queue is full, enqueue failed.\n";
                return false;
            }

            // 先移动队尾指针，然后再拷贝数据。
            m_rear = (m_rear + 1) % MAX_SIZE; // 队尾指针后移。
            m_data[m_rear] = e;
            ++m_size;

            return true;
        }

        /**
         * @brief 元素入队（移动版本）
         * @param e 待入队的元素（右值引用）
         * @return true-入队成功，false-队列已满入队失败
         */
        bool push(T&& e)
        {
            if (full())
            {
                std::cerr << "Circular queue is full, enqueue failed.\n";
                return false;
            }
            m_rear = (m_rear + 1) % MAX_SIZE;
            m_data[m_rear] = std::move(e);
            ++m_size;
            return true;
        }

        /**
         * @brief 元素出队
         * @return true-出队成功，false-队列为空出队失败
         */
        bool pop()
        {
            if (empty()) return false;

            m_front = (m_front + 1) % MAX_SIZE; // 队列头指针后移。
            --m_size;

            return true;
        }

        /**
         * @brief 清空队列所有元素（重置队列状态）
         *        对于非平凡析构类型，会显式调用元素的析构函数
         */
        void clear()
        {
            if (!m_inited || empty()) return; // 未初始化或已空则直接返回

            // 处理非平凡析构类型：需要显式调用每个元素的析构函数
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                for (size_t i = 0; i < m_size; ++i)
                {
                    size_t index = (m_front + i) % MAX_SIZE;
                    m_data[index].~T(); // 显式析构元素
                }
            }

            // 重置队列状态（无需清空数组内存，后续操作会覆盖）
            m_front = 0;
            m_rear = MAX_SIZE - 1;
            m_size = 0;
        }

        /**
         * @brief 获取队列当前元素数量
         * @return 队列长度（>=0）
         */
        size_t size() const
        {
            return m_size;
        }

        /**
         * @brief 获取队头元素（非const版本）
         * @return 队头元素的引用
         * @throws std::out_of_range 当队列为空时
         */
        T& front()
        {
            if (empty()) throw std::out_of_range("Circular queue is empty");
            return m_data[m_front];
        }

        /**
         * @brief 获取队头元素（const版本）
         * @return 队头元素的常量引用
         * @throws std::out_of_range 当队列为空时
         */
        const T& front() const
        {
            if (empty()) throw std::out_of_range("Circular queue is empty");
            return m_data[m_front];
        }

        /**
         * @brief 原地构造元素入队（直接在队列内存中构造对象）
         * @tparam Args 构造函数参数类型列表
         * @param args 构造函数参数（转发引用）
         * @return true-入队成功，false-队列已满入队失败
         */
        template <typename... Args>
        bool emplace(Args&&... args)
        {
            if (full())
            {
                std::cerr << "Circular queue is full, enqueue failed.\n";
                return false;
            }
            m_rear = (m_rear + 1) % MAX_SIZE;
            new (&m_data[m_rear]) T(std::forward<Args>(args)...);
            ++m_size;
            return true;
        }

        /**
         * @brief 打印队列中所有元素（调试用）
         *        要求元素类型支持std::cout输出
         */
        void print() const
        {
            for (size_t i = 0; i < size(); ++i)
            {
                std::cout << "m_data[" << (m_front + i) % MAX_SIZE << "],value="
                          << m_data[(m_front + i) % MAX_SIZE] << '\n';
            }
        }
    };

} // namespace ol

#endif // !OL_CQUEUE_H