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
    // 容器特性萃取
    // ======================

    // 主模板 - 用于STL容器
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

    // 原生数组特化
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
    // 排序算法实现 (内部实现)
    // ======================

    namespace detail
    {

        // 插入排序（双向迭代器）
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

        // 插入排序（随机访问迭代器）
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

                // 移动元素找到插入位置
                while (j > first && *(j - 1) > key)
                {
                    *j = *(j - 1);
                    --j;
                }

                *j = key;
            }
        }

        // 插入排序统一接口
        template <typename Iterator>
        void insertion_sort_impl(Iterator first, Iterator last)
        {
            using category = typename std::iterator_traits<Iterator>::iterator_category;
            insertion_sort_impl(first, last, category());
        }

        // 三数取中法选择枢纽元素（快速排序辅助函数）
        template <typename RandomIt>
        auto median_of_three(RandomIt low, RandomIt high)
        {
            RandomIt mid = low + (high - low) / 2;

            // 对三个元素排序
            if (*low > *mid) std::iter_swap(low, mid);
            if (*low > *high) std::iter_swap(low, high);
            if (*mid > *high) std::iter_swap(mid, high);

            // 将中值放到low位置作为枢纽
            std::iter_swap(low, mid);
            return *low;
        }

        // 快速排序主函数
        template <typename RandomIt>
        void quick_sort_impl(RandomIt first, RandomIt last)
        {
            // 计算元素数量
            auto size = last - first;

            // 小数组使用插入排序
            if (size <= 16)
            {
                insertion_sort_impl(first, last,
                                    typename std::iterator_traits<RandomIt>::iterator_category());
                return;
            }

            // 选择枢纽元素
            auto pivot = median_of_three(first, last - 1);

            RandomIt low = first;
            RandomIt high = last - 1;

            // 分区操作
            while (low < high)
            {
                while (low < high && *high >= pivot)
                {
                    if (*high == pivot) break; // 处理相同元素
                    --high;
                }
                *low = *high;

                while (low < high && *low <= pivot)
                {
                    if (*low == pivot) break; // 处理相同元素
                    ++low;
                }
                *high = *low;
            }

            *low = pivot; // 枢纽归位

            // 递归排序子序列
            quick_sort_impl(first, low);
            quick_sort_impl(low + 1, last);
        }

    } // namespace detail
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // ======================
    // 用户接口 - 插入排序
    // ======================

    // 插入排序 - 容器版本
    template <typename Container>
    void insertion_sort(Container& c)
    {
        using traits = container_traits<Container>;

        detail::insertion_sort_impl(
            traits::begin(c),
            traits::end(c));
    }

    // 插入排序 - 迭代器版本
    template <typename Iterator>
    void insertion_sort(Iterator first, Iterator last)
    {
        detail::insertion_sort_impl(first, last);
    }
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // ======================
    // 用户接口 - 快速排序
    // ======================

    // 快速排序 - 容器版本
    template <typename Container>
    void quick_sort(Container& c)
    {
        using traits = container_traits<Container>;
        using iterator = typename traits::iterator;
        // 确保是随机访问迭代器
        static_assert(
            std::is_same_v<
                typename std::iterator_traits<iterator>::iterator_category,
                std::random_access_iterator_tag>,
            "quick_sort requires random access iterators");
        detail::quick_sort_impl(traits::begin(c), traits::end(c));
    }

    // 快速排序 - 迭代器版本
    template <typename RandomIt>
    void quick_sort(RandomIt first, RandomIt last)
    {
        // 确保是随机访问迭代器
        static_assert(
            std::is_same_v<
                typename std::iterator_traits<RandomIt>::iterator_category,
                std::random_access_iterator_tag>,
            "quick_sort requires random access iterators");
        detail::quick_sort_impl(first, last);
    }
    ///////////////////////////////////// /////////////////////////////////////

    ///////////////////////////////////// /////////////////////////////////////
    // 输出
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