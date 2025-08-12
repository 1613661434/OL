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
    ///////////////////////////////////// /////////////////////////////////////
    // ======================
    // ����������ȡ
    // ======================

    // ��ģ�� - ����STL����
    template <typename Container>
    struct container_traits
    {
        using iterator = typename Container::iterator;
        using const_iterator = typename Container::const_iterator;
        using value_type = typename Container::value_type;
        using size_type = typename Container::size_type;

        static iterator begin(Container& c)
        {
            return c.begin();
        }
        static iterator end(Container& c)
        {
            return c.end();
        }
        static size_type size(Container& c)
        {
            return c.size();
        }
    };

    // ԭ�������ػ�
    template <typename T, size_t N>
    struct container_traits<T[N]>
    {
        using iterator = T*;
        using const_iterator = const T*;
        using value_type = T;
        using size_type = size_t;

        static iterator begin(T (&arr)[N])
        {
            return arr;
        }
        static iterator end(T (&arr)[N])
        {
            return arr + N;
        }
        static size_type size(T (&)[N])
        {
            return N;
        }
    };
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // ======================
    // �����㷨ʵ�� (�ڲ�ʵ��)
    // ======================

    namespace detail
    {

        // ��������˫���������
        template <typename Iterator>
        void insertion_sort_impl(Iterator first, Iterator last,
                                 std::bidirectional_iterator_tag)
        {
#ifdef DEBUG
            std::cout << "Bidirectional Iterator\n";
#endif // DEBUG

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

        // ��������������ʵ�������
        template <typename RandomIt>
        void insertion_sort_impl(RandomIt first, RandomIt last,
                                 std::random_access_iterator_tag)
        {
#ifdef DEBUG
            std::cout << "Random Access Iterator\n";
#endif // DEBUG

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

        // ��������ͳһ�ӿ�
        template <typename Iterator>
        void insertion_sort_impl(Iterator first, Iterator last)
        {
            using category = typename std::iterator_traits<Iterator>::iterator_category;
            insertion_sort_impl(first, last, category());
        }

        // ����ȡ�з�ѡ����ŦԪ�أ�����������������
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

        // ��������������
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
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // ======================
    // �û��ӿ� - ��������
    // ======================

    // �������� - �����汾
    template <typename Container>
    void insertion_sort(Container& c)
    {
        using traits = container_traits<Container>;

        detail::insertion_sort_impl(
            traits::begin(c),
            traits::end(c));
    }

    // �������� - �������汾
    template <typename Iterator>
    void insertion_sort(Iterator first, Iterator last)
    {
        detail::insertion_sort_impl(first, last);
    }
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // ======================
    // �û��ӿ� - ��������
    // ======================

    // �������� - �����汾
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

    // �������� - �������汾
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
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // ���
    template <typename Container>
    void print_container(const Container& c)
    {
        for (const auto& item : c)
        {
            std::cout << item << " ";
        }
        std::cout << "\n";
    }
    ///////////////////////////////////// /////////////////////////////////////

} // namespace ol

#endif // !__OL_SORT_H