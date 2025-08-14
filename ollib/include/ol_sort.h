/****************************************************************************************/
/*
 * 程序名：ol_sort.h
 * 功能描述：排序算法工具类，提供高效的排序实现，支持多种容器类型，特性包括：
 *          - 容器特性萃取：适配STL容器和原生数组，统一迭代器操作
 *          - 插入排序：支持双向迭代器和随机访问迭代器，自动适配容器类型
 *          - 快速排序：基于随机访问迭代器，结合三数取中法选择枢纽，小数组自动切换插入排序
 *          - 提供容器打印功能（调试用）
 * 作者：ol
 * 适用标准：C++11及以上（需支持迭代器特性、类型萃取等）
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
    // 容器特性萃取
    // ===========================================================================
    /**
     * @brief 容器特性萃取模板，统一STL容器和原生数组的操作接口
     * @tparam Container 容器类型（STL容器或原生数组）
     */
    template <typename Container>
    struct container_traits
    {
        using iterator = typename Container::iterator;             // 容器迭代器类型
        using const_iterator = typename Container::const_iterator; // 常量迭代器类型
        using value_type = typename Container::value_type;         // 元素类型
        using size_type = typename Container::size_type;           // 大小类型

        /**
         * @brief 获取容器起始迭代器
         * @param c 容器引用
         * @return 起始迭代器
         */
        static iterator begin(Container& c)
        {
            return c.begin();
        }

        /**
         * @brief 获取容器结束迭代器
         * @param c 容器引用
         * @return 结束迭代器
         */
        static iterator end(Container& c)
        {
            return c.end();
        }

        /**
         * @brief 获取容器大小
         * @param c 容器引用
         * @return 容器元素数量
         */
        static size_type size(Container& c)
        {
            return c.size();
        }
    };

    /**
     * @brief 容器特性萃取模板特化（原生数组）
     * @tparam T 数组元素类型
     * @tparam N 数组大小
     */
    template <typename T, size_t N>
    struct container_traits<T[N]>
    {
        using iterator = T*;             // 数组迭代器（指针）
        using const_iterator = const T*; // 常量迭代器（常量指针）
        using value_type = T;            // 元素类型
        using size_type = size_t;        // 大小类型

        /**
         * @brief 获取数组起始指针
         * @param arr 原生数组引用
         * @return 数组首元素指针
         */
        static iterator begin(T (&arr)[N])
        {
            return arr;
        }

        /**
         * @brief 获取数组结束指针
         * @param arr 原生数组引用
         * @return 数组尾后指针
         */
        static iterator end(T (&arr)[N])
        {
            return arr + N;
        }

        /**
         * @brief 获取数组大小
         * @return 数组元素数量（N）
         */
        static size_type size(T (&)[N])
        {
            return N;
        }
    };
    // ===========================================================================

    // 排序算法实现 (内部实现)
    // ===========================================================================
    namespace detail
    {

        /**
         * @brief 插入排序实现（双向迭代器版本）
         * @tparam Iterator 双向迭代器类型
         * @param first 起始迭代器
         * @param last 结束迭代器
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
         * @brief 插入排序实现（随机访问迭代器版本）
         * @tparam RandomIt 随机访问迭代器类型
         * @param first 起始迭代器
         * @param last 结束迭代器
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

                // 移动元素找到插入位置
                while (j > first && *(j - 1) > key)
                {
                    *j = *(j - 1);
                    --j;
                }

                *j = key;
            }
        }

        /**
         * @brief 插入排序统一接口（自动判断迭代器类型）
         * @tparam Iterator 迭代器类型
         * @param first 起始迭代器
         * @param last 结束迭代器
         */
        template <typename Iterator>
        void insertion_sort_impl(Iterator first, Iterator last)
        {
            using category = typename std::iterator_traits<Iterator>::iterator_category;
            insertion_sort_impl(first, last, category());
        }

        /**
         * @brief 三数取中法选择枢纽元素（快速排序辅助函数）
         * @tparam RandomIt 随机访问迭代器类型
         * @param low 起始迭代器
         * @param high 结束迭代器（最后一个元素）
         * @return 选中的枢纽元素值
         */
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

        /**
         * @brief 快速排序实现（随机访问迭代器）
         * @tparam RandomIt 随机访问迭代器类型
         * @param first 起始迭代器
         * @param last 结束迭代器
         * @note 元素数量<=16时自动切换为插入排序
         */
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
    // ===========================================================================

    // 用户接口 - 插入排序
    // ===========================================================================
    /**
     * @brief 插入排序（容器版本）
     * @tparam Container 容器类型（支持迭代器）
     * @param c 待排序的容器
     * @note 自动适配双向迭代器和随机访问迭代器
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
     * @brief 插入排序（迭代器版本）
     * @tparam Iterator 迭代器类型（双向或随机访问）
     * @param first 起始迭代器
     * @param last 结束迭代器
     */
    template <typename Iterator>
    void insertion_sort(Iterator first, Iterator last)
    {
        detail::insertion_sort_impl(first, last);
    }
    // ===========================================================================

    // 用户接口 - 快速排序
    // ===========================================================================
    /**
     * @brief 快速排序（容器版本）
     * @tparam Container 容器类型（需支持随机访问迭代器）
     * @param c 待排序的容器
     * @note 元素数量<=16时自动切换为插入排序，提高效率
     */
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

    /**
     * @brief 快速排序（迭代器版本）
     * @tparam RandomIt 随机访问迭代器类型
     * @param first 起始迭代器
     * @param last 结束迭代器
     */
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
    // ===========================================================================

    // ===========================================================================
    /**
     * @brief 打印容器元素（调试用）
     * @tparam Container 容器类型（支持范围for循环）
     * @param c 待打印的容器
     * @note 元素类型需支持std::cout输出
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