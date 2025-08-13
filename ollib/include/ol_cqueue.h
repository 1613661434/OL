/****************************************************************************************/
/*
 * ��������ol_cqueue.h
 * �����������̶���С��ѭ�����У�circular queue��ģ���֧࣬���������ԣ�
 *          - ���ھ�̬����ʵ�֣���С��ģ�����ָ��
 *          - ֧��POD���ͺͷ�POD���ͣ��Զ������ڴ����
 *          - �ṩ��ӡ����ӡ��鿴��ͷ���жϿ����Ȼ�������
 *          - ֧���ƶ�������ƶ���ֵ�����ÿ�������͸�ֵ��������Դ��ͻ��
 *          - ֧��ԭ�ع���Ԫ�أ�emplace������������
 * ���ߣ�ol
 * ���ñ�׼��C++11�����ϣ���֧��constexpr��type_traits����ֵ���õ����ԣ�
 */
/****************************************************************************************/

#ifndef __OL_CQUEUE_H
#define __OL_CQUEUE_H 1

#include <iostream>
#include <string.h>    // ����memset
#include <type_traits> // ����std::is_pod
#include <utility>     // ����std::move��std::forward

namespace ol
{

    /**
     * ѭ������ģ����
     * ���ھ�̬����ʵ�֣���С�̶���֧�ָ�Ч��FIFO���Ƚ��ȳ�������
     * @tparam T ������Ԫ�ص���������
     * @tparam MAX_SIZE ���е�����������������0��
     */
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
        // ���캯�����Զ���ʼ������
        cqueue()
        {
            init();
        }

        // �����������ͷ���Դ����ƽ�������������ֶ���������������
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

        /**
         * �ƶ����캯��
         * @param other ���ƶ��Ķ��ж���
         */
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

        /**
         * �ƶ���ֵ�����
         * @param other ���ƶ��Ķ��ж���
         * @return ��ǰ���ж��������
         */
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

        /**
         * ��ʼ�����У���δ��ʼ��ʱ��Ч��
         * ���ڹ����ڴ泡�������Զ����ù��캯��ʱ��
         * @note ������ڹ����ڴ�Ķ��У�������ù��캯����������ô˺�����ʼ����
         */
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

        /**
         * �ж϶����Ƿ�����
         * @return true-����������false-����δ��
         */
        bool full() const
        {
            return m_size == MAX_SIZE;
        }

        /**
         * �ж϶����Ƿ�Ϊ��
         * @return true-����Ϊ�գ�false-���зǿ�
         */
        bool empty() const
        {
            return m_size == 0;
        }

        /**
         * Ԫ����ӣ������汾��
         * @param e ����ӵ�Ԫ�أ��������ã�
         * @return true-��ӳɹ���false-�����������ʧ��
         */
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

        /**
         * Ԫ����ӣ��ƶ��汾��
         * @param e ����ӵ�Ԫ�أ���ֵ���ã�
         * @return true-��ӳɹ���false-�����������ʧ��
         */
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

        /**
         * Ԫ�س���
         * @return true-���ӳɹ���false-����Ϊ�ճ���ʧ��
         */
        bool pop()
        {
            if (empty()) return false;

            m_front = (m_front + 1) % MAX_SIZE; // ����ͷָ����ơ�
            --m_size;

            return true;
        }

        /**
         * ��ȡ���е�ǰԪ������
         * @return ���г��ȣ�>=0��
         */
        size_t size() const
        {
            return m_size;
        }

        /**
         * ��ȡ��ͷԪ�أ���const�汾��
         * @return ��ͷԪ�ص�����
         * @throws std::out_of_range ������Ϊ��ʱ
         */
        T& front()
        {
            if (empty()) throw std::out_of_range("����Ϊ��");
            return m_data[m_front];
        }

        /**
         * ��ȡ��ͷԪ�أ�const�汾��
         * @return ��ͷԪ�صĳ�������
         * @throws std::out_of_range ������Ϊ��ʱ
         */
        const T& front() const
        {
            if (empty()) throw std::out_of_range("����Ϊ��");
            return m_data[m_front];
        }

        /**
         * ԭ�ع���Ԫ����ӣ�ֱ���ڶ����ڴ��й������
         * @tparam Args ���캯�����������б�
         * @param args ���캯��������ת�����ã�
         * @return true-��ӳɹ���false-�����������ʧ��
         */
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

        /**
         * ��ӡ����������Ԫ�أ������ã�
         * Ҫ��Ԫ������֧��std::cout���
         */
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