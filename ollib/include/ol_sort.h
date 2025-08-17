/****************************************************************************************/
/*
 * 程序名：ol_sort.h
 * 功能描述：排序算法工具类，提供高效的排序实现，支持多种容器类型与自定义排序规则，特性包括：
 *          - 容器特性萃取：适配STL容器（vector、deque等）和原生数组，统一迭代器操作接口
 *          - 多种排序算法：插入排序、快速排序、希尔排序、冒泡排序、选择排序、堆排序、归并排序等
 *          - 自定义比较器：支持传入符合严格弱序（Strict Weak Ordering）的比较函数/对象
 *          - 提供容器打印功能（调试用），支持所有可范围遍历的容器类型
 * 作者：ol
 * 适用标准：C++11及以上（需支持迭代器特性、类型萃取、函数对象等特性）
 * 核心约束：自定义比较器必须满足严格弱序，需遵守4条规则：
 *          1. 非自反性：comp(a, a)必须返回false
 *          2. 非对称性：若comp(a, b)为true，则comp(b, a)必须为false
 *          3. 传递性：若comp(a, b)与comp(b, c)均为true，则comp(a, c)必须为true
 *          4. 不可比传递性：若a与b、b与c均不可比（comp(a,b)与comp(b,a)均为false），则a与c不可比
 *          禁止使用<=、>=等违反严格弱序的逻辑，否则可能导致排序结果错误或运行时未定义行为
 */
/****************************************************************************************/

#ifndef __OL_SORT_H
#define __OL_SORT_H 1

#include <algorithm>
#include <array>
#include <cstring>
#include <deque>
#include <functional>
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
         * @param container 容器引用
         * @return 起始迭代器
         */
        static iterator begin(Container& container)
        {
            return container.begin();
        }

        /**
         * @brief 获取容器结束迭代器
         * @param container 容器引用
         * @return 结束迭代器
         */
        static iterator end(Container& container)
        {
            return container.end();
        }

        /**
         * @brief 获取容器大小
         * @param container 容器引用
         * @return 容器元素数量
         */
        static size_type size(Container& container)
        {
            return container.size();
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
         * @param array 原生数组引用
         * @return 数组首元素指针
         */
        static iterator begin(T (&array)[N])
        {
            return array;
        }

        /**
         * @brief 获取数组结束指针
         * @param array 原生数组引用
         * @return 数组尾后指针
         */
        static iterator end(T (&array)[N])
        {
            return array + N;
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
        // 插入排序相关实现
        // -----------------------------------------------------------------------
        /**
         * @brief 插入排序实现（双向迭代器版本）
         * @tparam Iterator 双向迭代器类型
         * @tparam Compare 比较函数类型，需满足严格弱序（Strict Weak Ordering）
         * @param first 起始迭代器
         * @param last 结束迭代器
         * @param comp 比较函数对象，返回true表示第一个参数应排在前面
         */
        template <typename Iterator, typename Compare>
        void insertion_sort_impl(Iterator first, Iterator last,
                                 std::bidirectional_iterator_tag, const Compare& comp)
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

                    while (j != first && comp(key, *prev))
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
         * @tparam Compare 比较函数类型，需满足严格弱序（Strict Weak Ordering）
         * @param first 起始迭代器
         * @param last 结束迭代器
         * @param comp 比较函数对象，返回true表示第一个参数应排在前面
         */
        template <typename RandomIt, typename Compare>
        void insertion_sort_impl(RandomIt first, RandomIt last,
                                 std::random_access_iterator_tag, const Compare& comp)
        {
            if (first == last) return;

            for (RandomIt i = first + 1; i != last; ++i)
            {
                auto key = *i;
                RandomIt j = i;

                while (j > first && comp(key, *(j - 1)))
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
         * @tparam Compare 比较函数类型，需满足严格弱序（Strict Weak Ordering）
         * @param first 起始迭代器
         * @param last 结束迭代器
         * @param comp 比较函数对象，返回true表示第一个参数应排在前面
         */
        template <typename Iterator, typename Compare>
        void insertion_sort_impl(Iterator first, Iterator last, const Compare& comp)
        {
            using category = typename std::iterator_traits<Iterator>::iterator_category;
            insertion_sort_impl(first, last, category(), comp);
        }

        // 折半插入排序相关实现
        // -----------------------------------------------------------------------
        /**
         * @brief 折半查找（用于折半插入排序）
         * @tparam RandomIt 随机访问迭代器类型
         * @tparam Compare 比较函数类型
         * @param first 起始迭代器
         * @param last 结束迭代器
         * @param value 要查找的值
         * @param comp 比较函数对象
         * @return 插入位置的迭代器
         */
        template <typename RandomIt, typename ValueType, typename Compare>
        RandomIt binary_search_impl(RandomIt first, RandomIt last,
                                    const ValueType& value, const Compare& comp)
        {
            while (first < last)
            {
                RandomIt mid = first + (last - first) / 2;
                if (comp(value, *mid))
                {
                    last = mid;
                }
                else
                {
                    first = mid + 1;
                }
            }
            return first;
        }

        /**
         * @brief 折半插入排序实现
         * @tparam RandomIt 随机访问迭代器类型
         * @tparam Compare 比较函数类型
         * @param first 起始迭代器
         * @param last 结束迭代器
         * @param comp 比较函数对象
         */
        template <typename RandomIt, typename Compare>
        void binary_insertion_sort_impl(RandomIt first, RandomIt last, const Compare& comp)
        {
            if (first == last) return;

            for (RandomIt i = first + 1; i != last; ++i)
            {
                auto key = *i;
                RandomIt pos = binary_search_impl(first, i, key, comp);

                // 移动元素以为插入腾出空间
                for (RandomIt j = i; j > pos; --j)
                {
                    *j = *(j - 1);
                }

                *pos = key;
            }
        }

        // 希尔排序相关实现
        // -----------------------------------------------------------------------
        /**
         * @brief 希尔排序中对单个组进行排序
         * @tparam RandomIt 随机访问迭代器类型
         * @tparam Compare 比较函数类型
         * @param first 起始迭代器
         * @param last 结束迭代器
         * @param start 组的起始位置
         * @param step 步长
         * @param comp 比较函数对象
         */
        template <typename RandomIt, typename Compare>
        void shell_group_sort(RandomIt first, RandomIt last,
                              size_t start, size_t step, const Compare& comp)
        {
            size_t n = last - first;

            for (size_t i = start + step; i < n; i += step)
            {
                auto key = *(first + i);
                size_t j = i - step;

                while (j >= start && comp(key, *(first + j)))
                {
                    *(first + j + step) = *(first + j);
                    j -= step;
                }

                *(first + j + step) = key;
            }
        }

        /**
         * @brief 希尔排序实现
         * @tparam RandomIt 随机访问迭代器类型
         * @tparam Compare 比较函数类型
         * @param first 起始迭代器
         * @param last 结束迭代器
         * @param comp 比较函数对象
         */
        template <typename RandomIt, typename Compare>
        void shell_sort_impl(RandomIt first, RandomIt last, const Compare& comp)
        {
            size_t n = last - first;
            if (n <= 1) return;

            // 使用(3^k - 1)/2的递增序列：1, 4, 13, 40, 121...
            size_t step = 1;
            while (step < n / 3)
            {
                step = 3 * step + 1;
            }

            while (step >= 1)
            {
                // 对每个组执行插入排序
                for (size_t i = 0; i < step; ++i)
                {
                    shell_group_sort(first, last, i, step, comp);
                }
                step /= 3;
            }
        }

        // 冒泡排序相关实现
        // -----------------------------------------------------------------------
        /**
         * @brief 冒泡排序实现
         * @tparam Iterator 双向迭代器类型
         * @tparam Compare 比较函数类型
         * @param first 起始迭代器
         * @param last 结束迭代器
         * @param comp 比较函数对象
         */
        template <typename Iterator, typename Compare>
        void bubble_sort_impl(Iterator first, Iterator last, const Compare& comp)
        {
            if (first == last) return;

            bool swapped;
            Iterator end = last;

            do
            {
                swapped = false;
                Iterator current = first;
                Iterator next = first;
                ++next;

                while (next != end)
                {
                    if (comp(*next, *current))
                    {
                        std::iter_swap(current, next);
                        swapped = true;
                    }
                    ++current;
                    ++next;
                }
                --end; // 每轮结束后，最大元素已"冒泡"到末尾
            } while (swapped);
        }

        // 选择排序相关实现
        // -----------------------------------------------------------------------
        /**
         * @brief 选择排序实现
         * @tparam Iterator 迭代器类型
         * @tparam Compare 比较函数类型
         * @param first 起始迭代器
         * @param last 结束迭代器
         * @param comp 比较函数对象
         */
        template <typename Iterator, typename Compare>
        void selection_sort_impl(Iterator first, Iterator last, const Compare& comp)
        {
            if (first == last) return;

            for (Iterator i = first; i != last; ++i)
            {
                Iterator min_it = i;
                for (Iterator j = i; j != last; ++j)
                {
                    if (comp(*j, *min_it))
                    {
                        min_it = j;
                    }
                }
                std::iter_swap(i, min_it);
            }
        }

        // 堆排序相关实现
        // -----------------------------------------------------------------------
        /**
         * @brief 堆化操作（迭代实现）
         * @tparam RandomIt 随机访问迭代器类型
         * @tparam Compare 比较函数类型
         * @param first 起始迭代器
         * @param size 堆的大小
         * @param index 当前节点索引
         * @param comp 比较函数对象
         */
        template <typename RandomIt, typename Compare>
        void heapify_impl(RandomIt first, size_t size, size_t index, const Compare& comp)
        {
            while (true)
            {
                size_t largest = index;
                size_t left = 2 * index + 1;
                size_t right = 2 * index + 2;

                if (left < size && comp(*(first + largest), *(first + left)))
                {
                    largest = left;
                }
                if (right < size && comp(*(first + largest), *(first + right)))
                {
                    largest = right;
                }

                if (largest == index)
                {
                    break;
                }

                std::iter_swap(first + index, first + largest);
                index = largest;
            }
        }

        /**
         * @brief 堆排序实现
         * @tparam RandomIt 随机访问迭代器类型
         * @tparam Compare 比较函数类型
         * @param first 起始迭代器
         * @param last 结束迭代器
         * @param comp 比较函数对象
         */
        template <typename RandomIt, typename Compare>
        void heap_sort_impl(RandomIt first, RandomIt last, const Compare& comp)
        {
            size_t n = last - first;
            if (n <= 1) return;

            // 构建堆
            for (ssize_t i = static_cast<ssize_t>(n / 2 - 1); i >= 0; --i)
            {
                heapify_impl(first, n, static_cast<size_t>(i), comp);
            }

            // 提取最大元素并重建堆
            for (size_t i = n - 1; i > 0; --i)
            {
                std::iter_swap(first, first + i);
                heapify_impl(first, i, 0, comp);
            }
        }

        // 归并排序相关实现
        // -----------------------------------------------------------------------
        /**
         * @brief 合并两个已排序的子序列
         * @tparam RandomIt 随机访问迭代器类型（原始数据）
         * @tparam TempIt 随机访问迭代器类型（临时缓冲区）
         * @tparam Compare 比较函数类型
         * @param first 起始迭代器
         * @param mid 中间迭代器
         * @param last 结束迭代器
         * @param temp 临时存储区间的起始迭代器
         * @param comp 比较函数对象
         */
        template <typename RandomIt, typename TempIt, typename Compare>
        void merge_impl(RandomIt first, RandomIt mid, RandomIt last,
                        TempIt temp, const Compare& comp)
        {
            RandomIt i = first, j = mid;
            TempIt k = temp;

            while (i < mid && j < last)
            {
                if (comp(*i, *j))
                {
                    *k++ = *i++;
                }
                else
                {
                    *k++ = *j++;
                }
            }

            while (i < mid)
            {
                *k++ = *i++;
            }

            while (j < last)
            {
                *k++ = *j++;
            }

            // 将临时数组复制回原数组
            std::copy(temp, k, first);
        }

        /**
         * @brief 归并排序递归实现
         * @tparam RandomIt 随机访问迭代器类型（原始数据）
         * @tparam TempIt 随机访问迭代器类型（临时缓冲区）
         * @tparam Compare 比较函数类型
         * @param first 起始迭代器
         * @param last 结束迭代器
         * @param temp 临时存储区间
         * @param comp 比较函数对象
         */
        template <typename RandomIt, typename TempIt, typename Compare>
        void merge_sort_impl(RandomIt first, RandomIt last,
                             TempIt temp, const Compare& comp)
        {
            if (last - first > 1)
            {
                RandomIt mid = first + (last - first) / 2;
                auto temp_mid = temp + (mid - first);

                merge_sort_impl(first, mid, temp, comp);
                merge_sort_impl(mid, last, temp_mid, comp);
                merge_impl(first, mid, last, temp, comp);
            }
        }

        /**
         * @brief 归并排序入口
         * @tparam RandomIt 随机访问迭代器类型
         * @tparam Compare 比较函数类型
         * @param first 起始迭代器
         * @param last 结束迭代器
         * @param comp 比较函数对象
         */
        template <typename RandomIt, typename Compare>
        void merge_sort(RandomIt first, RandomIt last, const Compare& comp)
        {
            size_t n = last - first;
            if (n <= 1) return;

            std::vector<typename std::iterator_traits<RandomIt>::value_type> temp(n);
            merge_sort_impl(first, last, temp.begin(), comp);
        }

        // 计数排序相关实现
        // -----------------------------------------------------------------------
        /**
         * @brief 计数排序实现（仅适用于整数类型）
         * @tparam RandomIt 随机访问迭代器类型
         * @tparam Compare 比较函数类型
         * @param first 起始迭代器
         * @param last 结束迭代器
         * @param comp 比较函数对象
         */
        template <typename RandomIt, typename Compare>
        typename std::enable_if<std::is_integral<typename std::iterator_traits<RandomIt>::value_type>::value, void>::type
        counting_sort_impl(RandomIt first, RandomIt last, const Compare& comp)
        {
            using ValueType = typename std::iterator_traits<RandomIt>::value_type;
            size_t n = last - first;
            if (n <= 1) return;

            // 找到最小值和最大值
            ValueType min_val = *first, max_val = *first;
            for (RandomIt it = first; it != last; ++it)
            {
                if (comp(*it, min_val)) min_val = *it;
                if (comp(max_val, *it)) max_val = *it;
            }

            // 创建计数数组
            size_t range = max_val - min_val + 1;
            std::vector<size_t> count(range, 0);

            // 计数
            for (RandomIt it = first; it != last; ++it)
            {
                ++count[*it - min_val];
            }

            // 计算前缀和，得到的是在排序后数组的结束位置
            for (size_t i = 1; i < range; ++i)
            {
                count[i] += count[i - 1];
            }

            // 放置元素，从后往前遍历是为了保证排序的稳定性
            std::vector<ValueType> output(n);
            for (auto it = last - 1; it >= first; --it)
            {
                output[count[*it - min_val] - 1] = *it;
                --count[*it - min_val];
            }

            // 复制回原数组
            std::copy(output.begin(), output.end(), first);
        }

        // 基数排序相关实现
        // -----------------------------------------------------------------------
        /**
         * @brief 获取数字的指定位值（从0开始，0为最低位）
         *
         * 负数处理逻辑：
         * 将负数映射到[radix, 2*radix-1]范围的桶，正数映射到[0, radix-1]范围的桶
         * 确保所有负数整体排在正数之前，同时保持负数内部和正数内部的正确排序
         */
        template <typename T>
        int get_digit(T num, int digit, int radix)
        {
            // 处理负数：通过偏移量将负数映射到更高范围的桶，保证负数排在正数前
            bool is_negative = false;
            if (num < 0)
            {
                is_negative = true;
                num = -num; // 转为正数处理其位值
            }

            for (int i = 0; i < digit; ++i)
            {
                num /= radix;
            }

            int result = num % radix;
            // 负数偏移radix个位置，确保负数桶与正数桶分离
            return is_negative ? radix + result : result;
        }

        /**
         * @brief 计算数字的最大位数
         */
        template <typename T>
        int get_max_digits(const std::vector<T>& nums, int radix)
        {
            if (nums.empty()) return 0;

            T max_num = std::abs(nums[0]);
            for (const auto& num : nums)
            {
                T abs_num = std::abs(num);
                if (abs_num > max_num)
                {
                    max_num = abs_num;
                }
            }

            if (max_num == 0) return 1;

            int digits = 0;
            while (max_num > 0)
            {
                max_num /= radix;
                ++digits;
            }
            return digits;
        }

        /**
         * @brief 基数排序实现（适用于整数类型）
         */
        template <typename RandomIt>
        typename std::enable_if<std::is_integral<typename std::iterator_traits<RandomIt>::value_type>::value, void>::type
        radix_sort_impl(RandomIt first, RandomIt last, int radix = 10)
        {
            using ValueType = typename std::iterator_traits<RandomIt>::value_type;
            size_t n = last - first;
            if (n <= 1) return;

            // 将基数转换为无符号类型，避免比较时的类型不匹配
            const size_t radix_unsigned = static_cast<size_t>(radix);

            // 将数据复制到临时容器
            std::vector<ValueType> nums(first, last);
            int max_digits = get_max_digits(nums, radix);

            // 由于要处理负数，需要2*radix个桶（radix个正数桶，radix个负数桶）
            std::vector<std::vector<ValueType>> buckets(2 * radix_unsigned);

            for (int digit = 0; digit < max_digits; ++digit)
            {
                // 清空桶
                for (auto& bucket : buckets)
                {
                    bucket.clear();
                }

                // 分配：将元素放入对应的桶
                for (const auto& num : nums)
                {
                    int d = get_digit(num, digit, radix);
                    buckets[static_cast<size_t>(d)].push_back(num);
                }

                // 收集：先处理负数桶，再处理正数桶，确保负数排在前面
                size_t index = 0;
                // 先处理负数桶 (radix 到 2*radix-1) - 使用无符号类型比较
                for (size_t i = radix_unsigned; i < 2 * radix_unsigned; ++i)
                {
                    for (const auto& num : buckets[i])
                    {
                        nums[index++] = num;
                    }
                }
                // 再处理正数桶 (0 到 radix-1) - 使用无符号类型比较
                for (size_t i = 0; i < radix_unsigned; ++i)
                {
                    for (const auto& num : buckets[i])
                    {
                        nums[index++] = num;
                    }
                }
            }

            // 将排序结果复制回原容器
            std::copy(nums.begin(), nums.end(), first);
        }

        // 快速排序相关实现
        // -----------------------------------------------------------------------
        /**
         * @brief 三数取中法选择基准元素
         * 从区间的首、中、尾三个位置选择中间值作为基准，减少最坏情况
         */
        template <typename RandomIt, typename Compare>
        auto median_of_three(RandomIt low, RandomIt high, const Compare& comp)
        {
            RandomIt mid = low + (high - low) / 2;

            // 对三个位置的元素进行排序
            if (comp(*mid, *low)) std::iter_swap(low, mid);
            if (comp(*high, *low)) std::iter_swap(low, high);
            if (comp(*high, *mid)) std::iter_swap(mid, high);

            // 返回基准值（此时已位于low位置）
            return *low;
        }

        /**
         * @brief 快速排序内部实现函数（挖坑填数版）
         * 包含小规模数据优化：当区间长度≤16时使用插入排序
         */
        template <typename RandomIt, typename Compare>
        void quick_sort_impl(RandomIt first, RandomIt last, const Compare& comp)
        {
            auto size = last - first;

            // 优化点：小规模数据（≤16个元素）使用插入排序
            if (size <= 16)
            {
                insertion_sort_impl(first, last, comp);
                return;
            }

            // 初始化指针，high指向最后一个元素
            RandomIt low = first;
            RandomIt high = last - 1;

            // 三数取中法选择基准值，同时调整high指针
            auto pivot = median_of_three(low, high--, comp); // high--因为三数取中已处理最后一个元素

            // 挖坑填数核心逻辑
            while (low < high)
            {
                // 从右向左找小于等于基准的元素，填入左边的坑
                while (low < high && !comp(*high, pivot)) // *high >= pivot
                {
                    // 遇到与基准相等的元素，提前退出以平衡分区
                    if (!comp(pivot, *high)) // *high == pivot
                    {
                        --high;
                        break;
                    }
                    --high;
                }
                *low = *high;

                // 从左向右找大于等于基准的元素，填入右边的坑
                while (low < high && comp(*low, pivot)) // *low < pivot
                {
                    // 遇到与基准相等的元素，提前退出以平衡分区
                    if (!comp(pivot, *low)) // *low == pivot
                    {
                        ++low;
                        break;
                    }
                    ++low;
                }
                *high = *low;
            }

            // 将基准值填入最后一个坑
            *low = pivot;

            // 递归排序左右子区间
            quick_sort_impl(first, low, comp);    // 左区间：[first, low)
            quick_sort_impl(low + 1, last, comp); // 右区间：[low+1, last)
        }

    } // namespace detail
    // ===========================================================================

    // 用户接口 - 插入排序
    // ===========================================================================
    /**
     * @brief 插入排序（容器版本，支持默认比较器）
     * @tparam Container 容器类型（支持迭代器）
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param container 待排序的容器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     *
     * 算法特性：
     * - 稳定性：稳定排序（相等元素保持原有顺序）
     * - 时间复杂度：O(n²)，与初始有序度密切相关（接近有序时效率高）
     * - 空间复杂度：O(1)，原地排序
     * - 适用场景：小规模数据或接近有序的数据
     */
    template <typename Container, typename Compare = std::less<typename container_traits<Container>::value_type>>
    void insertion_sort(Container& container, const Compare& comp = Compare())
    {
        using traits = container_traits<Container>;
        detail::insertion_sort_impl(traits::begin(container), traits::end(container), comp);
    }

    /**
     * @brief 插入排序（迭代器版本，支持默认比较器）
     * @tparam Iterator 迭代器类型（双向或随机访问）
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param first 起始迭代器
     * @param last 结束迭代器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     *
     * 算法特性：
     * - 稳定性：稳定排序（相等元素保持原有顺序）
     * - 时间复杂度：O(n²)，与初始有序度密切相关（接近有序时效率高）
     * - 空间复杂度：O(1)，原地排序
     * - 适用场景：小规模数据或接近有序的数据
     */
    template <typename Iterator, typename Compare = std::less<typename std::iterator_traits<Iterator>::value_type>>
    void insertion_sort(Iterator first, Iterator last, const Compare& comp = Compare())
    {
        detail::insertion_sort_impl(first, last, comp);
    }
    // ===========================================================================

    // 用户接口 - 折半插入排序
    // ===========================================================================
    /**
     * @brief 折半插入排序（容器版本，支持默认比较器）
     * @tparam Container 容器类型（需支持随机访问迭代器）
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param container 待排序的容器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     *
     * 算法特性：
     * - 稳定性：稳定排序（相等元素保持原有顺序）
     * - 时间复杂度：O(n²)，与初始有序度有关（减少了比较次数但仍需移动元素）
     * - 空间复杂度：O(1)，原地排序
     * - 适用场景：中等规模数据，相比普通插入排序减少了比较次数
     */
    template <typename Container, typename Compare = std::less<typename container_traits<Container>::value_type>>
    void binary_insertion_sort(Container& container, const Compare& comp = Compare())
    {
        using traits = container_traits<Container>;
        using iterator = typename traits::iterator;

        static_assert(
            std::is_same_v<
                typename std::iterator_traits<iterator>::iterator_category,
                std::random_access_iterator_tag>,
            "binary_insertion_sort requires random access iterators");

        detail::binary_insertion_sort_impl(traits::begin(container), traits::end(container), comp);
    }

    /**
     * @brief 折半插入排序（迭代器版本，支持默认比较器）
     * @tparam RandomIt 随机访问迭代器类型
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param first 起始迭代器
     * @param last 结束迭代器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     *
     * 算法特性：
     * - 稳定性：稳定排序（相等元素保持原有顺序）
     * - 时间复杂度：O(n²)，与初始有序度有关（减少了比较次数但仍需移动元素）
     * - 空间复杂度：O(1)，原地排序
     * - 适用场景：中等规模数据，相比普通插入排序减少了比较次数
     */
    template <typename RandomIt, typename Compare = std::less<typename std::iterator_traits<RandomIt>::value_type>>
    void binary_insertion_sort(RandomIt first, RandomIt last, const Compare& comp = Compare())
    {
        static_assert(
            std::is_same_v<
                typename std::iterator_traits<RandomIt>::iterator_category,
                std::random_access_iterator_tag>,
            "binary_insertion_sort requires random access iterators");

        detail::binary_insertion_sort_impl(first, last, comp);
    }
    // ===========================================================================

    // 用户接口 - 希尔排序
    // ===========================================================================
    /**
     * @brief 希尔排序（容器版本，支持默认比较器）
     * @tparam Container 容器类型（需支持随机访问迭代器）
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param container 待排序的容器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     *
     * 算法特性：
     * - 稳定性：不稳定排序（相等元素可能改变相对顺序）
     * - 时间复杂度：O(n^1.3)~O(n²)，与初始有序度和步长序列有关
     * - 空间复杂度：O(1)，原地排序
     * - 适用场景：中等规模数据，比普通插入排序效率高
     */
    template <typename Container, typename Compare = std::less<typename container_traits<Container>::value_type>>
    void shell_sort(Container& container, const Compare& comp = Compare())
    {
        using traits = container_traits<Container>;
        using iterator = typename traits::iterator;

        static_assert(
            std::is_same_v<
                typename std::iterator_traits<iterator>::iterator_category,
                std::random_access_iterator_tag>,
            "shell_sort requires random access iterators");

        detail::shell_sort_impl(traits::begin(container), traits::end(container), comp);
    }

    /**
     * @brief 希尔排序（迭代器版本，支持默认比较器）
     * @tparam RandomIt 随机访问迭代器类型
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param first 起始迭代器
     * @param last 结束迭代器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     *
     * 算法特性：
     * - 稳定性：不稳定排序（相等元素可能改变相对顺序）
     * - 时间复杂度：O(n^1.3)~O(n²)，与初始有序度和步长序列有关
     * - 空间复杂度：O(1)，原地排序
     * - 适用场景：中等规模数据，比普通插入排序效率高
     */
    template <typename RandomIt, typename Compare = std::less<typename std::iterator_traits<RandomIt>::value_type>>
    void shell_sort(RandomIt first, RandomIt last, const Compare& comp = Compare())
    {
        static_assert(
            std::is_same_v<
                typename std::iterator_traits<RandomIt>::iterator_category,
                std::random_access_iterator_tag>,
            "shell_sort requires random access iterators");

        detail::shell_sort_impl(first, last, comp);
    }
    // ===========================================================================

    // 用户接口 - 冒泡排序
    // ===========================================================================
    /**
     * @brief 冒泡排序（容器版本，支持默认比较器）
     * @tparam Container 容器类型（支持双向迭代器）
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param container 待排序的容器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     *
     * 算法特性：
     * - 稳定性：稳定排序（相等元素保持原有顺序）
     * - 时间复杂度：O(n²)，与初始有序度有关（可通过优化提前终止）
     * - 空间复杂度：O(1)，原地排序
     * - 适用场景：小规模数据或教学演示，实际应用中效率较低
     */
    template <typename Container, typename Compare = std::less<typename container_traits<Container>::value_type>>
    void bubble_sort(Container& container, const Compare& comp = Compare())
    {
        using traits = container_traits<Container>;
        detail::bubble_sort_impl(traits::begin(container), traits::end(container), comp);
    }

    /**
     * @brief 冒泡排序（迭代器版本，支持默认比较器）
     * @tparam Iterator 双向迭代器类型
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param first 起始迭代器
     * @param last 结束迭代器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     *
     * 算法特性：
     * - 稳定性：稳定排序（相等元素保持原有顺序）
     * - 时间复杂度：O(n²)，与初始有序度有关（可通过优化提前终止）
     * - 空间复杂度：O(1)，原地排序
     * - 适用场景：小规模数据或教学演示，实际应用中效率较低
     */
    template <typename Iterator, typename Compare = std::less<typename std::iterator_traits<Iterator>::value_type>>
    void bubble_sort(Iterator first, Iterator last, const Compare& comp = Compare())
    {
        detail::bubble_sort_impl(first, last, comp);
    }
    // ===========================================================================

    // 用户接口 - 选择排序
    // ===========================================================================
    /**
     * @brief 选择排序（容器版本，支持默认比较器）
     * @tparam Container 容器类型（支持迭代器）
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param container 待排序的容器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     *
     * 算法特性：
     * - 稳定性：不稳定排序（相等元素可能改变相对顺序）
     * - 时间复杂度：O(n²)，与初始有序度无关
     * - 空间复杂度：O(1)，原地排序
     * - 适用场景：小规模数据，交换操作成本较高的场景
     */
    template <typename Container, typename Compare = std::less<typename container_traits<Container>::value_type>>
    void selection_sort(Container& container, const Compare& comp = Compare())
    {
        using traits = container_traits<Container>;
        detail::selection_sort_impl(traits::begin(container), traits::end(container), comp);
    }

    /**
     * @brief 选择排序（迭代器版本，支持默认比较器）
     * @tparam Iterator 迭代器类型
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param first 起始迭代器
     * @param last 结束迭代器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     *
     * 算法特性：
     * - 稳定性：不稳定排序（相等元素可能改变相对顺序）
     * - 时间复杂度：O(n²)，与初始有序度无关
     * - 空间复杂度：O(1)，原地排序
     * - 适用场景：小规模数据，交换操作成本较高的场景
     */
    template <typename Iterator, typename Compare = std::less<typename std::iterator_traits<Iterator>::value_type>>
    void selection_sort(Iterator first, Iterator last, const Compare& comp = Compare())
    {
        detail::selection_sort_impl(first, last, comp);
    }
    // ===========================================================================

    // 用户接口 - 堆排序
    // ===========================================================================
    /**
     * @brief 堆排序（容器版本，支持默认比较器）
     * @tparam Container 容器类型（需支持随机访问迭代器）
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param container 待排序的容器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     *
     * 算法特性：
     * - 稳定性：不稳定排序（相等元素可能改变相对顺序）
     * - 时间复杂度：O(n log n)，与初始有序度无关
     * - 空间复杂度：O(1)，原地排序
     * - 适用场景：中等至大规模数据，对空间使用有严格限制的场景
     */
    template <typename Container, typename Compare = std::less<typename container_traits<Container>::value_type>>
    void heap_sort(Container& container, const Compare& comp = Compare())
    {
        using traits = container_traits<Container>;
        using iterator = typename traits::iterator;

        static_assert(
            std::is_same_v<
                typename std::iterator_traits<iterator>::iterator_category,
                std::random_access_iterator_tag>,
            "heap_sort requires random access iterators");

        detail::heap_sort_impl(traits::begin(container), traits::end(container), comp);
    }

    /**
     * @brief 堆排序（迭代器版本，支持默认比较器）
     * @tparam RandomIt 随机访问迭代器类型
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param first 起始迭代器
     * @param last 结束迭代器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     *
     * 算法特性：
     * - 稳定性：不稳定排序（相等元素可能改变相对顺序）
     * - 时间复杂度：O(n log n)，与初始有序度无关
     * - 空间复杂度：O(1)，原地排序
     * - 适用场景：中等至大规模数据，对空间使用有严格限制的场景
     */
    template <typename RandomIt, typename Compare = std::less<typename std::iterator_traits<RandomIt>::value_type>>
    void heap_sort(RandomIt first, RandomIt last, const Compare& comp = Compare())
    {
        static_assert(
            std::is_same_v<
                typename std::iterator_traits<RandomIt>::iterator_category,
                std::random_access_iterator_tag>,
            "heap_sort requires random access iterators");

        detail::heap_sort_impl(first, last, comp);
    }
    // ===========================================================================

    // 用户接口 - 归并排序
    // ===========================================================================
    /**
     * @brief 归并排序（容器版本，支持默认比较器）
     * @tparam Container 容器类型（需支持随机访问迭代器）
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param container 待排序的容器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     *
     * 算法特性：
     * - 稳定性：稳定排序（相等元素保持原有顺序）
     * - 时间复杂度：O(n log n)，与初始有序度无关
     * - 空间复杂度：O(n)，需要额外的存储空间
     * - 适用场景：中等至大规模数据，需要稳定排序的场景
     */
    template <typename Container, typename Compare = std::less<typename container_traits<Container>::value_type>>
    void merge_sort(Container& container, const Compare& comp = Compare())
    {
        using traits = container_traits<Container>;
        using iterator = typename traits::iterator;

        static_assert(
            std::is_same_v<
                typename std::iterator_traits<iterator>::iterator_category,
                std::random_access_iterator_tag>,
            "merge_sort requires random access iterators");

        detail::merge_sort(traits::begin(container), traits::end(container), comp);
    }

    /**
     * @brief 归并排序（迭代器版本，支持默认比较器）
     * @tparam RandomIt 随机访问迭代器类型
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param first 起始迭代器
     * @param last 结束迭代器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     *
     * 算法特性：
     * - 稳定性：稳定排序（相等元素保持原有顺序）
     * - 时间复杂度：O(n log n)，与初始有序度无关
     * - 空间复杂度：O(n)，需要额外的存储空间
     * - 适用场景：中等至大规模数据，需要稳定排序的场景
     */
    template <typename RandomIt, typename Compare = std::less<typename std::iterator_traits<RandomIt>::value_type>>
    void merge_sort(RandomIt first, RandomIt last, const Compare& comp = Compare())
    {
        static_assert(
            std::is_same_v<
                typename std::iterator_traits<RandomIt>::iterator_category,
                std::random_access_iterator_tag>,
            "merge_sort requires random access iterators");

        detail::merge_sort(first, last, comp);
    }
    // ===========================================================================

    // 用户接口 - 计数排序
    // ===========================================================================
    /**
     * @brief 计数排序（容器版本，支持默认比较器）
     * @tparam Container 容器类型（需支持随机访问迭代器，元素类型为整数）
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param container 待排序的容器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     *
     * 算法特性：
     * - 稳定性：稳定排序（相等元素保持原有顺序）
     * - 时间复杂度：O(n + k)，n为元素个数，k为数值范围，与初始有序度无关
     * - 空间复杂度：O(n + k)，需要额外的存储空间
     * - 适用场景：整数类型数据，且数值范围相对较小的场景
     */
    template <typename Container, typename Compare = std::less<typename container_traits<Container>::value_type>>
    typename std::enable_if<std::is_integral<typename container_traits<Container>::value_type>::value, void>::type
    counting_sort(Container& container, const Compare& comp = Compare())
    {
        using traits = container_traits<Container>;
        using iterator = typename traits::iterator;

        static_assert(
            std::is_same_v<
                typename std::iterator_traits<iterator>::iterator_category,
                std::random_access_iterator_tag>,
            "counting_sort requires random access iterators");

        detail::counting_sort_impl(traits::begin(container), traits::end(container), comp);
    }

    /**
     * @brief 计数排序（迭代器版本，支持默认比较器）
     * @tparam RandomIt 随机访问迭代器类型（元素类型为整数）
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param first 起始迭代器
     * @param last 结束迭代器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     *
     * 算法特性：
     * - 稳定性：稳定排序（相等元素保持原有顺序）
     * - 时间复杂度：O(n + k)，n为元素个数，k为数值范围，与初始有序度无关
     * - 空间复杂度：O(n + k)，需要额外的存储空间
     * - 适用场景：整数类型数据，且数值范围相对较小的场景
     */
    template <typename RandomIt, typename Compare = std::less<typename std::iterator_traits<RandomIt>::value_type>>
    typename std::enable_if<std::is_integral<typename std::iterator_traits<RandomIt>::value_type>::value, void>::type
    counting_sort(RandomIt first, RandomIt last, const Compare& comp = Compare())
    {
        static_assert(
            std::is_same_v<
                typename std::iterator_traits<RandomIt>::iterator_category,
                std::random_access_iterator_tag>,
            "counting_sort requires random access iterators");

        detail::counting_sort_impl(first, last, comp);
    }
    // ===========================================================================

    // 用户接口 - 基数排序
    // ===========================================================================
    /**
     * @brief 基数排序（容器版本，适用于整数类型）
     * @tparam Container 容器类型（需支持随机访问迭代器，元素类型为整数）
     * @param container 待排序的容器
     * @param radix 基数，默认为10（十进制）
     *
     * 算法特性：
     * - 稳定性：稳定排序（相等元素保持原有顺序）
     * - 时间复杂度：O(d*(n+r))，d为最大位数，n为元素个数，r为基数
     * - 空间复杂度：O(n+r)，需要额外的存储空间
     * - 适用场景：整数类型数据，支持正数和负数排序
     */
    template <typename Container>
    typename std::enable_if<std::is_integral<typename container_traits<Container>::value_type>::value, void>::type
    radix_sort(Container& container, int radix = 10)
    {
        // 验证基数有效性
        if (radix < 2)
        {
            throw std::invalid_argument("radix must be greater than or equal to 2");
        }

        using traits = container_traits<Container>;
        using iterator = typename traits::iterator;

        static_assert(
            std::is_same_v<
                typename std::iterator_traits<iterator>::iterator_category,
                std::random_access_iterator_tag>,
            "radix_sort requires random access iterators");

        detail::radix_sort_impl(traits::begin(container), traits::end(container), radix);
    }

    /**
     * @brief 基数排序（迭代器版本，适用于整数类型）
     * @tparam RandomIt 随机访问迭代器类型（元素类型为整数）
     * @param first 起始迭代器
     * @param last 结束迭代器
     * @param radix 基数，默认为10（十进制）
     */
    template <typename RandomIt>
    typename std::enable_if<std::is_integral<typename std::iterator_traits<RandomIt>::value_type>::value, void>::type
    radix_sort(RandomIt first, RandomIt last, int radix = 10)
    {
        // 验证基数有效性
        if (radix < 2)
        {
            throw std::invalid_argument("radix must be greater than or equal to 2");
        }

        static_assert(
            std::is_same_v<
                typename std::iterator_traits<RandomIt>::iterator_category,
                std::random_access_iterator_tag>,
            "radix_sort requires random access iterators");

        detail::radix_sort_impl(first, last, radix);
    }
    // ===========================================================================

    // 用户接口 - 快速排序
    // ===========================================================================
    /**
     * @brief 快速排序（容器版本）
     * @tparam Container 容器类型（需支持随机访问迭代器）
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param container 待排序的容器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     *
     * 算法特性：
     * - 稳定性：不稳定排序（相等元素可能改变相对顺序）
     * - 时间复杂度：O(n log n)，与初始有序度有关但影响程度取决于基准选择策略
     * - 空间复杂度：O(log n)，主要用于递归调用栈
     * - 适用场景：大多数通用排序场景，平均性能优异
     *
     * 实现优化：
     * - 采用三数取中法选择基准元素，减少最坏情况出现概率
     * - 引入阈值优化：当待排序区间长度小于等于16时，自动切换为插入排序
     * - 挖坑填数法替代传统交换，减少元素交换次数
     * - 对相等元素进行特殊处理，平衡左右分区
     */
    template <typename Container, typename Compare = std::less<typename container_traits<Container>::value_type>>
    void quick_sort(Container& container, const Compare& comp = Compare())
    {
        using traits = container_traits<Container>;
        using iterator = typename traits::iterator;

        static_assert(
            std::is_same_v<
                typename std::iterator_traits<iterator>::iterator_category,
                std::random_access_iterator_tag>,
            "quick_sort requires random access iterators");

        detail::quick_sort_impl(traits::begin(container), traits::end(container), comp);
    }

    /**
     * @brief 快速排序（迭代器版本）
     * @tparam RandomIt 随机访问迭代器类型
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param first 起始迭代器
     * @param last 结束迭代器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     *
     * 算法特性：
     * - 稳定性：不稳定排序（相等元素可能改变相对顺序）
     * - 时间复杂度：O(n log n)，与初始有序度有关但影响程度取决于基准选择策略
     * - 空间复杂度：O(log n)，主要用于递归调用栈
     * - 适用场景：大多数通用排序场景，平均性能优异
     *
     * 实现优化：
     * - 采用三数取中法选择基准元素，减少最坏情况出现概率
     * - 引入阈值优化：当待排序区间长度小于等于16时，自动切换为插入排序
     * - 挖坑填数法替代传统交换，减少元素交换次数
     * - 对相等元素进行特殊处理，平衡左右分区
     */
    template <typename RandomIt, typename Compare = std::less<typename std::iterator_traits<RandomIt>::value_type>>
    void quick_sort(RandomIt first, RandomIt last, const Compare& comp = Compare())
    {
        static_assert(
            std::is_same_v<
                typename std::iterator_traits<RandomIt>::iterator_category,
                std::random_access_iterator_tag>,
            "quick_sort requires random access iterators");

        detail::quick_sort_impl(first, last, comp);
    }
    // ===========================================================================

    /**
     * @brief 打印容器元素（调试用）
     * @tparam Container 容器类型（支持范围for循环）
     * @param container 待打印的容器
     * @note 元素类型需支持std::cout输出
     */
    template <typename Container>
    void print_container(const Container& container)
    {
        for (const auto& item : container)
        {
            std::cout << item << " ";
        }
        std::cout << '\n';
    }

    /**
     * @brief 打印原生数组元素（调试用）
     * @tparam T 数组元素类型
     * @tparam N 数组大小
     * @param array 待打印的原生数组
     * @note 元素类型需支持std::cout输出
     */
    template <typename T, size_t N>
    void print_container(const T (&array)[N])
    {
        for (size_t i = 0; i < N; ++i)
        {
            std::cout << array[i] << " ";
        }
        std::cout << '\n';
    }
    // ===========================================================================

} // namespace ol

#endif // !__OL_SORT_H