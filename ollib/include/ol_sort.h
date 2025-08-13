/****************************************************************************************/
/*
 * ��������ol_sort.h
 * ���������������㷨�����࣬�ṩ��Ч������ʵ�֣�֧�ֶ����������ͣ����԰�����
 *          - ����������ȡ������STL������ԭ�����飬ͳһ����������
 *          - ��������֧��˫���������������ʵ��������Զ�������������
 *          - �������򣺻���������ʵ��������������ȡ�з�ѡ����Ŧ��С�����Զ��л���������
 *          - �ṩ������ӡ���ܣ������ã�
 * ���ߣ�ol
 * ���ñ�׼��C++11�����ϣ���֧�ֵ��������ԡ�������ȡ�ȣ�
 */
/****************************************************************************************/

#ifndef __OL_SORT_H
#define __OL_SORT_H 1

#include <algorithm>
#include <array>
#include <deque>
#include <iostream>
#include <iterator>
#include <list>
#include <memory>
#include <type_traits>
#include <vector>

namespace ol
{
    // ����������ȡ
    // ===========================================================================
    /**
     * ����������ȡģ�壬ͳһSTL������ԭ������Ĳ����ӿ�
     * @tparam Container �������ͣ�STL������ԭ�����飩
     */
    template <typename Container>
    struct container_traits
    {
        using iterator = typename Container::iterator;             // ��������������
        using const_iterator = typename Container::const_iterator; // ��������������
        using value_type = typename Container::value_type;         // Ԫ������
        using size_type = typename Container::size_type;           // ��С����

        /**
         * ��ȡ������ʼ������
         * @param c ��������
         * @return ��ʼ������
         */
        static iterator begin(Container& c)
        {
            return c.begin();
        }

        /**
         * ��ȡ��������������
         * @param c ��������
         * @return ����������
         */
        static iterator end(Container& c)
        {
            return c.end();
        }

        /**
         * ��ȡ������С
         * @param c ��������
         * @return ����Ԫ������
         */
        static size_type size(Container& c)
        {
            return c.size();
        }
    };

    /**
     * ����������ȡģ���ػ���ԭ�����飩
     * @tparam T ����Ԫ������
     * @tparam N �����С
     */
    template <typename T, size_t N>
    struct container_traits<T[N]>
    {
        using iterator = T*;             // �����������ָ�룩
        using const_iterator = const T*; // ����������������ָ�룩
        using value_type = T;            // Ԫ������
        using size_type = size_t;        // ��С����

        /**
         * ��ȡ������ʼָ��
         * @param arr ԭ����������
         * @return ������Ԫ��ָ��
         */
        static iterator begin(T (&arr)[N])
        {
            return arr;
        }

        /**
         * ��ȡ�������ָ��
         * @param arr ԭ����������
         * @return ����β��ָ��
         */
        static iterator end(T (&arr)[N])
        {
            return arr + N;
        }

        /**
         * ��ȡ�����С
         * @return ����Ԫ��������N��
         */
        static size_type size(T (&)[N])
        {
            return N;
        }
    };
    // ===========================================================================

    // �����㷨ʵ�� (�ڲ�ʵ��)
    // ===========================================================================
    namespace detail
    {

        /**
         * ��������ʵ�֣�˫��������汾��
         * @tparam Iterator ˫�����������
         * @param first ��ʼ������
         * @param last ����������
         */
        template <typename Iterator>
        void insertion_sort_impl(Iterator first, Iterator last,
                                 std::bidirectional_iterator_tag)
        {
            if (first == last) return;

            for (Iterator i = first; i != last; ++i)
            {
                auto key = *i;
                Iterator j = i;

                if (j != first)
                {
                    Iterator prev = j;
                    --prev;

                    while (j != first && *prev > key)
                    {
                        *j = *prev;
                        --j;

                        if (j != first)
                        {
                            --prev;
                        }
                    }
                }

                *j = key;
            }
        }

        /**
         * ��������ʵ�֣�������ʵ������汾��
         * @tparam RandomIt ������ʵ���������
         * @param first ��ʼ������
         * @param last ����������
         */
        template <typename RandomIt>
        void insertion_sort_impl(RandomIt first, RandomIt last,
                                 std::random_access_iterator_tag)
        {
            if (first == last) return;

            for (RandomIt i = first + 1; i != last; ++i)
            {
                auto key = *i;
                RandomIt j = i;

                // �ƶ�Ԫ���ҵ�����λ��
                while (j > first && *(j - 1) > key)
                {
                    *j = *(j - 1);
                    --j;
                }

                *j = key;
            }
        }

        /**
         * ��������ͳһ�ӿڣ��Զ��жϵ��������ͣ�
         * @tparam Iterator ����������
         * @param first ��ʼ������
         * @param last ����������
         */
        template <typename Iterator>
        void insertion_sort_impl(Iterator first, Iterator last)
        {
            using category = typename std::iterator_traits<Iterator>::iterator_category;
            insertion_sort_impl(first, last, category());
        }

        /**
         * ����ȡ�з�ѡ����ŦԪ�أ�����������������
         * @tparam RandomIt ������ʵ���������
         * @param low ��ʼ������
         * @param high ���������������һ��Ԫ�أ�
         * @return ѡ�е���ŦԪ��ֵ
         */
        template <typename RandomIt>
        auto median_of_three(RandomIt low, RandomIt high)
        {
            RandomIt mid = low + (high - low) / 2;

            // ������Ԫ������
            if (*low > *mid) std::iter_swap(low, mid);
            if (*low > *high) std::iter_swap(low, high);
            if (*mid > *high) std::iter_swap(mid, high);

            // ����ֵ�ŵ�lowλ����Ϊ��Ŧ
            std::iter_swap(low, mid);
            return *low;
        }

        /**
         * ��������ʵ�֣�������ʵ�������
         * @tparam RandomIt ������ʵ���������
         * @param first ��ʼ������
         * @param last ����������
         * @note Ԫ������<=16ʱ�Զ��л�Ϊ��������
         */
        template <typename RandomIt>
        void quick_sort_impl(RandomIt first, RandomIt last)
        {
            // ����Ԫ������
            auto size = last - first;

            // С����ʹ�ò�������
            if (size <= 16)
            {
                insertion_sort_impl(first, last,
                                    typename std::iterator_traits<RandomIt>::iterator_category());
                return;
            }

            // ѡ����ŦԪ��
            auto pivot = median_of_three(first, last - 1);

            RandomIt low = first;
            RandomIt high = last - 1;

            // ��������
            while (low < high)
            {
                while (low < high && *high >= pivot)
                {
                    if (*high == pivot) break; // ������ͬԪ��
                    --high;
                }
                *low = *high;

                while (low < high && *low <= pivot)
                {
                    if (*low == pivot) break; // ������ͬԪ��
                    ++low;
                }
                *high = *low;
            }

            *low = pivot; // ��Ŧ��λ

            // �ݹ�����������
            quick_sort_impl(first, low);
            quick_sort_impl(low + 1, last);
        }

    } // namespace detail
    // ===========================================================================

    // �û��ӿ� - ��������
    // ===========================================================================
    /**
     * �������������汾��
     * @tparam Container �������ͣ�֧�ֵ�������
     * @param c �����������
     * @note �Զ�����˫���������������ʵ�����
     */
    template <typename Container>
    void insertion_sort(Container& c)
    {
        using traits = container_traits<Container>;

        detail::insertion_sort_impl(
            traits::begin(c),
            traits::end(c));
    }

    /**
     * �������򣨵������汾��
     * @tparam Iterator ���������ͣ�˫���������ʣ�
     * @param first ��ʼ������
     * @param last ����������
     */
    template <typename Iterator>
    void insertion_sort(Iterator first, Iterator last)
    {
        detail::insertion_sort_impl(first, last);
    }
    // ===========================================================================

    // �û��ӿ� - ��������
    // ===========================================================================
    /**
     * �������������汾��
     * @tparam Container �������ͣ���֧��������ʵ�������
     * @param c �����������
     * @note Ԫ������<=16ʱ�Զ��л�Ϊ�����������Ч��
     */
    template <typename Container>
    void quick_sort(Container& c)
    {
        using traits = container_traits<Container>;
        using iterator = typename traits::iterator;
        // ȷ����������ʵ�����
        static_assert(
            std::is_same_v<
                typename std::iterator_traits<iterator>::iterator_category,
                std::random_access_iterator_tag>,
            "quick_sort requires random access iterators");
        detail::quick_sort_impl(traits::begin(c), traits::end(c));
    }

    /**
     * �������򣨵������汾��
     * @tparam RandomIt ������ʵ���������
     * @param first ��ʼ������
     * @param last ����������
     */
    template <typename RandomIt>
    void quick_sort(RandomIt first, RandomIt last)
    {
        // ȷ����������ʵ�����
        static_assert(
            std::is_same_v<
                typename std::iterator_traits<RandomIt>::iterator_category,
                std::random_access_iterator_tag>,
            "quick_sort requires random access iterators");
        detail::quick_sort_impl(first, last);
    }
    // ===========================================================================

    // ===========================================================================
    /**
     * ��ӡ����Ԫ�أ������ã�
     * @tparam Container �������ͣ�֧�ַ�Χforѭ����
     * @param c ����ӡ������
     * @note Ԫ��������֧��std::cout���
     */
    template <typename Container>
    void print_container(const Container& c)
    {
        for (const auto& item : c)
        {
            std::cout << item << " ";
        }
        std::cout << "\n";
    }
    // ===========================================================================

} // namespace ol

#endif // !__OL_SORT_H