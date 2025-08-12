#ifndef __OL_CQUEUE_H
#define __OL_CQUEUE_H 1

#include <iostream>
#include <string.h>    // ����memset
#include <type_traits> // ����std::is_pod
#include <utility>     // ����std::move��std::forward

namespace ol
{

    // ѭ�����С�
    template <class T, size_t MAX_SIZE>
    class cqueue
    {
    private:
        static_assert(MAX_SIZE > 0, "MAX_SIZE must be greater than 0");
        bool m_inited = false;        // ���б���ʼ����־��true-�ѳ�ʼ����false-δ��ʼ����
        size_t m_size = 0;            // ���е�ʵ�ʳ��ȡ�
        T m_data[MAX_SIZE];           // ������洢ѭ�������е�Ԫ�ء�
        size_t m_front = 0;           // ���е�ͷָ�롣
        size_t m_rear = MAX_SIZE - 1; // ���е�βָ�룬ָ���βԪ�ء�

        cqueue(const cqueue&) = delete;            // ���ÿ������캯����
        cqueue& operator=(const cqueue&) = delete; // ���ø�ֵ������

    public:
        cqueue()
        {
            init();
        }

        ~cqueue()
        {
            if (m_inited == true)
            {
                // �ǿ�ƽ������������Ҫ�ֶ�������������
                if constexpr (!std::is_trivially_destructible_v<T>)
                {
                    for (size_t i = 0; i < m_size; ++i)
                    {
                        size_t index = (m_front + i) % MAX_SIZE;
                        m_data[index].~T(); // ��ʽ������������
                    }
                }
            }
        }

        // �ƶ����캯��
        cqueue(cqueue&& other) noexcept
            : m_inited(other.m_inited),
              m_size(other.m_size),
              m_front(other.m_front),
              m_rear(other.m_rear)
        {
            if (m_inited == true)
            {
                // �ƶ���ƽ���������ͣ�ֱ���ڴ濽��
                if constexpr (std::is_trivially_copyable_v<T>)
                {
                    memcpy(m_data, other.m_data, MAX_SIZE * sizeof(T));
                }
                else
                {
                    // �ǿ�ƽ���������ͣ�����ƶ�
                    for (size_t i = 0; i < other.m_size; ++i)
                    {
                        size_t src_idx = (other.m_front + i) % MAX_SIZE;
                        size_t dst_idx = (m_front + i) % MAX_SIZE;
                        m_data[dst_idx] = std::move(other.m_data[src_idx]);
                    }
                }
                // ���ԭ����
                other.m_inited = false;
                other.m_size = 0;
            }
        }

        // �ƶ���ֵ�����
        cqueue& operator=(cqueue&& other) noexcept
        {
            if (this != &other)
            {
                // ���ͷŵ�ǰ���е���Դ
                if (m_inited == true)
                {
                    if constexpr (!std::is_trivially_destructible_v<T>)
                    {
                        // �ǿ�ƽ���������ͣ��ֶ�������������
                        for (size_t i = 0; i < m_size; ++i)
                        {
                            size_t index = (m_front + i) % MAX_SIZE;
                            m_data[index].~T(); // ��ʽ������������
                        }
                    }
                    // �����Ƿ��ƽ������������Ҫ����״̬
                    m_inited = false;
                    m_size = 0;
                    m_front = 0;
                    m_rear = MAX_SIZE - 1;
                }

                // �ƶ��������е���Դ
                m_inited = other.m_inited;
                m_size = other.m_size;
                m_front = other.m_front;
                m_rear = other.m_rear;

                if (m_inited == true)
                {
                    if constexpr (std::is_trivially_copyable_v<T>)
                    {
                        // ��ƽ���������ͣ�ֱ���ڴ濽��
                        memcpy(m_data, other.m_data, MAX_SIZE * sizeof(T));
                    }
                    else
                    {
                        // �ǿ�ƽ���������ͣ�����ƶ�Ԫ��
                        for (size_t i = 0; i < other.m_size; ++i)
                        {
                            size_t src_idx = (other.m_front + i) % MAX_SIZE;
                            size_t dst_idx = (m_front + i) % MAX_SIZE;
                            m_data[dst_idx] = std::move(other.m_data[src_idx]);
                        }
                    }
                    // ���ԭ����
                    other.m_inited = false;
                    other.m_size = 0;
                }
            }
            return *this;
        }

        // ѭ�����еĳ�ʼ��������
        // ע�⣺������ڹ����ڴ�Ķ��У�������ù��캯����������ô˺�����ʼ����
        void init()
        {
            if (m_inited == true) return; // ѭ�����еĳ�ʼ��ֻ��ִ��һ�Ρ�
            m_inited = true;
            m_front = 0;           // ͷָ�롣
            m_rear = MAX_SIZE - 1; // Ϊ�˷���д���룬��ʼ��ʱ��βָ��ָ����е����һ��λ�á�
            m_size = 0;            // ���е�ʵ�ʳ��ȡ�

            // ����Ԫ�س�ʼ����
            if constexpr (std::is_trivially_copyable_v<T>)
            { // ��ƽ����������ר�ó�ʼ����Ч�����ȣ�
                memset(m_data, 0, sizeof(m_data));
            }
            else
            {
                for (size_t i = 0; i < MAX_SIZE; ++i)
                {                    // �ǿ�ƽ����������ͨ�ó�ʼ��
                    m_data[i] = T(); // ����Ĭ�Ϲ��캯��
                }
            }
        }

        // �ж�ѭ�������Ƿ�����������ֵ��true-������false-δ����
        bool full() const
        {
            return m_size == MAX_SIZE;
        }

        // �ж�ѭ�������Ƿ�Ϊ�գ�����ֵ��true-�գ�false-�ǿա�
        bool empty() const
        {
            return m_size == 0;
        }

        // Ԫ����ӣ�����ֵ��false-ʧ�ܣ�true-�ɹ���
        bool push(const T& e)
        {
            if (full())
            {
                std::cerr << "ѭ���������������ʧ�ܡ�\n";
                return false;
            }

            // ���ƶ���βָ�룬Ȼ���ٿ������ݡ�
            m_rear = (m_rear + 1) % MAX_SIZE; // ��βָ����ơ�
            m_data[m_rear] = e;
            ++m_size;

            return true;
        }

        // ֧����ֵ���õ�push
        bool push(T&& e)
        {
            if (full())
            {
                std::cerr << "ѭ���������������ʧ�ܡ�\n";
                return false;
            }
            m_rear = (m_rear + 1) % MAX_SIZE;
            m_data[m_rear] = std::move(e);
            ++m_size;
            return true;
        }

        // Ԫ�س��ӣ�����ֵ��false-ʧ�ܣ�true-�ɹ���
        bool pop()
        {
            if (empty()) return false;

            m_front = (m_front + 1) % MAX_SIZE; // ����ͷָ����ơ�
            --m_size;

            return true;
        }

        // ��ѭ�����еĳ��ȣ�����ֵ��>=0  ������Ԫ�صĸ�����
        size_t size() const
        {
            return m_size;
        }

        // �鿴��ͷԪ�ص�ֵ��Ԫ�ز����ӡ�
        T& front()
        {
            if (empty()) throw std::out_of_range("����Ϊ��");
            return m_data[m_front];
        }

        const T& front() const
        {
            if (empty()) throw std::out_of_range("����Ϊ��");
            return m_data[m_front];
        }

        // ԭ�ع���Ԫ��
        template <typename... Args>
        bool emplace(Args&&... args)
        {
            if (full())
            {
                std::cerr << "ѭ���������������ʧ�ܡ�\n";
                return false;
            }
            m_rear = (m_rear + 1) % MAX_SIZE;
            new (&m_data[m_rear]) T(std::forward<Args>(args)...);
            ++m_size;
            return true;
        }

        // ��ʾѭ��������ȫ����Ԫ�ء�
        // ����һ����ʱ�����ڵ��Եĺ�����������Ԫ�ص���������֧��cout����ſ��á�
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