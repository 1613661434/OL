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

#ifdef _WIN32
#include <cstdint>        // 包含intptr_t的头文件
typedef intptr_t ssize_t; // MSVC用intptr_t代替ssize_t
#endif

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
         * @brief 插入排序实现
         * @tparam Iterator 迭代器类型（双向迭代器类型）
         * @tparam Compare 比较函数类型，需满足严格弱序（Strict Weak Ordering）
         * @param first 起始迭代器
         * @param last 结束迭代器
         * @param comp 比较函数对象，返回true表示第一个参数应排在前面
         */
        template <typename Iterator, typename Compare>
        void insertion_sort_impl(Iterator first, Iterator last, const Compare& comp)
        {
            if (first == last) return;

            // 从第二个元素开始迭代（第一个元素已"有序"）
            for (Iterator i = std::next(first); i != last; ++i)
            {
                auto key = *i;  // 保存当前待插入的元素
                Iterator j = i; // 从当前位置向前查找插入点

                // 向前查找：只要没到起始位置，且key应排在前一个元素前面
                // std::prev(j)对双向迭代器等价于--j的临时值，对随机访问迭代器等价于j-1
                while (j != first && comp(key, *std::prev(j)))
                {
                    *j = *std::prev(j); // 前一个元素后移
                    --j;                // 指针前移（双向/随机访问迭代器均支持）
                }

                *j = key; // 将key插入到正确位置
            }
        }
        // -----------------------------------------------------------------------

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
        // -----------------------------------------------------------------------

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
                // 将 j 改为有符号类型（ptrdiff_t），避免下溢
                ptrdiff_t j = static_cast<ptrdiff_t>(i - step);

                // 有符号比较，避免 j 下溢后仍满足 j >= start
                while (j >= static_cast<ptrdiff_t>(start) && comp(key, *(first + j)))
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
        // -----------------------------------------------------------------------

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

            bool swapped; // 记录是否进行过交换操作
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
            } while (swapped); // 一轮内，如果一次交换操作都没有进行，说明数组已经有序，可以提前终止算法
        }
        // -----------------------------------------------------------------------

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
        // -----------------------------------------------------------------------

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
        // -----------------------------------------------------------------------

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
        // -----------------------------------------------------------------------

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
        // -----------------------------------------------------------------------

        // 基数排序（LSD）相关实现
        // -----------------------------------------------------------------------
        /**
         * @brief 快速幂算法（二进制 exponentiation）
         * 计算 base^exponent，时间复杂度 O(log exponent)
         */
        template <typename T>
        T fast_pow(T base, int exponent)
        {
            T result = 1;
            while (exponent > 0)
            {
                // 如果当前指数为奇数，乘上当前基数
                if (exponent % 2 == 1)
                {
                    result *= base;
                }
                // 基数平方，指数折半
                base *= base;
                exponent /= 2;
            }
            return result;
        }

        /**
         * @brief 基数排序的计数排序子过程
         * @tparam ValueType 元素类型（整数）
         * @param nums 待排序的临时数组（已通过偏移量转为非负数）
         * @param k 当前排序的位数（0为最低位）
         * @param radix 基数（默认为10）
         */
        template <typename ValueType>
        void radix_count_lsd_sort(std::vector<ValueType>& nums, int k, int radix)
        {
            const size_t n = nums.size();
            if (n <= 1) return;

            // 计数数组：存储每个digit的出现次数
            std::vector<int> count(radix, 0);

            // 使用快速幂计算除数（radix^k）
            ValueType divisor = fast_pow(static_cast<ValueType>(radix), k);

            // 1. 统计当前位的数字出现次数
            for (ValueType num : nums)
            {
                int digit = static_cast<int>((num / divisor) % radix);
                ++count[digit];
            }

            // 2. 计算前缀和，确定每个数字在结果中的位置
            for (int i = 1; i < radix; ++i)
            {
                count[i] += count[i - 1];
            }

            // 3. 从后往前遍历，按当前位排序（保证稳定性）
            std::vector<ValueType> sorted(n);
            for (int i = static_cast<int>(n) - 1; i >= 0; --i)
            {
                ValueType num = nums[i];
                int digit = static_cast<int>((num / divisor) % radix);
                sorted[count[digit] - 1] = num;
                --count[digit];
            }

            // 4. 复制回原数组
            nums.swap(sorted);
        }

        /**
         * @brief 基数排序核心实现（LSD策略）
         * @tparam RandomIt 随机访问迭代器类型
         * @param first 起始迭代器
         * @param last 结束迭代器
         * @param radix 基数
         */
        template <typename RandomIt>
        void radix_sort_lsd_impl(RandomIt first, RandomIt last, int radix)
        {
            using ValueType = typename std::iterator_traits<RandomIt>::value_type;
            const size_t n = std::distance(first, last);
            if (n <= 1) return;

            // 一次循环同时找到最大和最小元素（从第二个元素开始比较）
            ValueType min_val = *first;
            ValueType max_val = *first;
            for (RandomIt it = std::next(first); it != last; ++it)
            {
                if (*it < min_val)
                    min_val = *it;
                if (*it > max_val)
                    max_val = *it;
            }

            // 处理负数：计算偏移量将所有数转为非负数
            const ValueType offset = (min_val < 0) ? -min_val : 0;

            // 复制数据并应用偏移量
            std::vector<ValueType> nums;
            nums.reserve(n);
            for (RandomIt it = first; it != last; ++it)
            {
                nums.push_back(*it + offset);
            }

            // 调整最大值（加上偏移量）
            max_val += offset;

            // 计算最大元素的位数（使用do-while确保0也能正确得到位数1）
            int max_digits = 0;
            ValueType temp = max_val;
            do
            {
                ++max_digits;
                temp /= radix;
            } while (temp > 0);

            // 从低位到高位，依次对每一位进行计数排序
            for (int k = 0; k < max_digits; ++k)
            {
                radix_count_lsd_sort(nums, k, radix);
            }

            // 将所有元素转回原始值（减去偏移量）并写回原容器
            auto dest_it = first;
            for (ValueType num : nums)
            {
                *dest_it++ = num - offset;
            }
        }
        // -----------------------------------------------------------------------

        // 基数排序（MSD）相关实现
        // -----------------------------------------------------------------------
        /**
         * @brief 获取字符串在指定位置的字符（支持越界处理，使用unsigned char确保无符号性）
         * @param str 输入字符串
         * @param pos 字符位置（从0开始）
         * @return 若pos在字符串长度范围内则返回对应unsigned char，否则返回'\0'
         */
        inline unsigned char get_char(const std::string& str, size_t pos)
        {
            return (pos < str.size()) ? static_cast<unsigned char>(str[pos]) : '\0';
        }

        /**
         * @brief 基数排序MSD核心递归实现
         * @tparam RandomIt 随机访问迭代器类型（元素为string）
         * @param first 起始迭代器
         * @param last 结束迭代器
         * @param pos 当前处理的字符位置（从0开始）
         * @param max_pos 最大处理位置（-1表示处理全部字符）
         * @param radix 基数（字符集大小，默认为256包含所有unsigned char值）
         */
        template <typename RandomIt>
        void radix_sort_msd_impl(RandomIt first, RandomIt last,
                                 size_t pos, int max_pos, int radix = 256)
        {
            // 递归终止条件：区间长度小于等于1，或已处理到最大指定位置
            if (last - first <= 1 || (max_pos != -1 && pos >= static_cast<size_t>(max_pos)))
                return;

            // 创建桶：radix个桶用于存放不同字符的元素，1个额外桶用于存放短字符串
            using ValueType = typename std::iterator_traits<RandomIt>::value_type;
            std::vector<std::vector<ValueType>> buckets(radix + 1);

            // 分配元素到对应桶（使用unsigned char避免符号扩展）
            for (auto iter = first; iter != last; ++iter)
            {
                unsigned char c = get_char(*iter, pos);
                size_t bucket_idx = static_cast<size_t>(c) + 1; // +1避开索引0
                buckets[bucket_idx].push_back(*iter);
            }

            // 将桶中元素写回原区间，并对非空桶递归排序下一位
            auto dest = first;
            for (auto& bucket : buckets)
            {
                if (bucket.empty()) continue;

                // 将当前桶元素复制回原区间
                dest = std::copy(bucket.begin(), bucket.end(), dest);

                // 递归处理下一位（空字符桶不需要继续递归）
                if (!bucket.empty() && &bucket != &buckets[0])
                {
                    auto bucket_first = dest - bucket.size();
                    radix_sort_msd_impl(bucket_first, dest, pos + 1, max_pos, radix);
                }
            }
        }

        /**
         * @brief 按前n位分组的辅助函数
         * @tparam RandomIt 随机访问迭代器类型（元素为string）
         * @param first 起始迭代器
         * @param last 结束迭代器
         * @param group_pos 分组的位数
         * @param radix 基数
         * @return 分组结果（vector of vector<string>）
         */
        template <typename RandomIt>
        std::vector<std::vector<typename std::iterator_traits<RandomIt>::value_type>>
        radix_group_by_prefix_impl(RandomIt first, RandomIt last,
                                   size_t group_pos, int radix = 256)
        {
            // 先按指定位数进行MSD排序
            radix_sort_msd_impl(first, last, 0, static_cast<int>(group_pos), radix);

            // 提取分组结果
            using ValueType = typename std::iterator_traits<RandomIt>::value_type;
            std::vector<std::vector<ValueType>> groups;
            if (first == last) return groups;

            // 按前缀相同性分组
            groups.emplace_back();
            groups.back().push_back(*first);
            const size_t prefix_len = std::min<size_t>(group_pos, first->size());
            std::string prev_prefix = first->substr(0, prefix_len);

            for (auto iter = std::next(first); iter != last; ++iter)
            {
                const size_t curr_len = std::min<size_t>(group_pos, iter->size());
                std::string curr_prefix = iter->substr(0, curr_len);

                if (curr_prefix != prev_prefix)
                {
                    groups.emplace_back();
                    prev_prefix = curr_prefix;
                }
                groups.back().push_back(*iter);
            }

            return groups;
        }
        // -----------------------------------------------------------------------

        // 快速排序相关实现
        // -----------------------------------------------------------------------
        /**
         * @brief 三数取中法选择基准元素
         * 从区间的首、中、尾三个位置选择中间值放到low位置作为基准且high位置返回时已经排序好，减少最坏情况
         */
        template <typename RandomIt, typename Compare>
        auto median_of_three(RandomIt low, RandomIt high, const Compare& comp)
        {
            RandomIt mid = low + (high - low) / 2;

            // 对三个位置的元素进行排序
            if (comp(*high, *mid)) std::iter_swap(high, mid);
            if (comp(*high, *low)) std::iter_swap(high, low);
            if (comp(*low, *mid)) std::iter_swap(low, mid);

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
                    if (!comp(pivot, *high)) // *high == pivot（严格弱序下：!comp(pivot, *high) && !comp(*high, pivot)）
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
                    // 遇到与基准等价的元素，提前退出以平衡分区
                    // 保留此判断以增强对不规范比较器的容错性
                    if (comp(pivot, *low)) // *low == pivot（非严格弱序下：comp(pivot, *low) && comp(*low, pivot)）
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
        // -----------------------------------------------------------------------

        // 桶排序相关实现
        // -----------------------------------------------------------------------
        /**
         * @brief 桶排序核心实现（适用于浮点数）
         *
         * 算法逻辑：
         * 1. 根据数值范围划分桶（桶的划分基于数值大小，与比较器无关）
         * 2. 将元素分配到对应桶中
         * 3. 对每个桶内元素使用自定义比较器进行排序
         * 4. 合并所有桶的结果
         */
        template <typename RandomIt, typename Compare>
        void bucket_sort_float_impl(RandomIt first, RandomIt last,
                                    size_t num_buckets,
                                    double min_val, double max_val,
                                    const Compare& comp)
        {
            using ValueType = typename std::iterator_traits<RandomIt>::value_type;
            const size_t n = std::distance(first, last);
            if (n <= 1) return;

            // 创建桶容器
            std::vector<std::vector<ValueType>> buckets(num_buckets);
            const double range = max_val - min_val;

            // 分配元素到对应桶（桶的划分基于数值范围）
            for (RandomIt it = first; it != last; ++it)
            {
                size_t idx = static_cast<size_t>(((*it - min_val) / range) * num_buckets);
                if (idx >= num_buckets) idx = num_buckets - 1;
                buckets[idx].push_back(*it);
            }

            // 桶内排序：使用自定义比较器
            for (auto& bucket : buckets)
                insertion_sort_impl(bucket.begin(), bucket.end(), comp);

            // 合并所有桶的结果
            auto dest_it = first;
            for (const auto& bucket : buckets)
                dest_it = std::copy(bucket.begin(), bucket.end(), dest_it);
        }

        /**
         * @brief 桶排序核心实现（适用于整数）
         *
         * 与浮点数版本的区别：
         * - 自动计算数据范围（无需手动指定min_val和max_val）
         * - 使用比较器确定最值和桶内排序规则
         */
        template <typename RandomIt, typename Compare>
        void bucket_sort_int_impl(RandomIt first, RandomIt last,
                                  size_t num_buckets,
                                  const Compare& comp)
        {
            using ValueType = typename std::iterator_traits<RandomIt>::value_type;
            const size_t n = std::distance(first, last);
            if (n <= 1) return;

            // 使用比较器计算数据范围（找最值）
            ValueType min_val = *first, max_val = *first;
            for (RandomIt it = std::next(first); it != last; ++it)
            {
                if (comp(*it, min_val)) min_val = *it;
                if (comp(max_val, *it)) max_val = *it;
            }

            // 创建桶并分配元素
            std::vector<std::vector<ValueType>> buckets(num_buckets);
            const ValueType range = max_val - min_val + 1;                         // +1 避免除零
            const ValueType bucket_size = (range + num_buckets - 1) / num_buckets; // 向上取整

            for (RandomIt it = first; it != last; ++it)
            {
                size_t idx = static_cast<size_t>((*it - min_val) / bucket_size);
                if (idx >= num_buckets) idx = num_buckets - 1;
                buckets[idx].push_back(*it);
            }

            // 桶内排序：使用自定义比较器
            for (auto& bucket : buckets)
                insertion_sort_impl(bucket.begin(), bucket.end(), comp);

            // 合并所有桶的结果
            auto dest_it = first;
            for (const auto& bucket : buckets)
                dest_it = std::copy(bucket.begin(), bucket.end(), dest_it);
        }
        // -----------------------------------------------------------------------

    } // namespace detail
    // ===========================================================================

    // 用户接口 - 插入排序
    // ===========================================================================
    /**
     * @brief 插入排序（迭代器版本，支持默认比较器）
     * @tparam Iterator 迭代器类型（支持双向迭代器）
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param first 起始迭代器
     * @param last 结束迭代器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     * @note
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

    /**
     * @brief 插入排序（容器版本，支持默认比较器）
     * @tparam Container 容器类型（支持迭代器）
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param container 待排序的容器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     * @note
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
        insertion_sort(traits::begin(container), traits::end(container), comp);
    }
    // ===========================================================================

    // 用户接口 - 折半插入排序
    // ===========================================================================
    /**
     * @brief 折半插入排序（迭代器版本，支持默认比较器）
     * @tparam RandomIt 随机访问迭代器类型
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param first 起始迭代器
     * @param last 结束迭代器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     * @note
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

    /**
     * @brief 折半插入排序（容器版本，支持默认比较器）
     * @tparam Container 容器类型（需支持随机访问迭代器）
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param container 待排序的容器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     * @note
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
        binary_insertion_sort(traits::begin(container), traits::end(container), comp);
    }
    // ===========================================================================

    // 用户接口 - 希尔排序
    // ===========================================================================
    /**
     * @brief 希尔排序（迭代器版本，支持默认比较器）
     * @tparam RandomIt 随机访问迭代器类型
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param first 起始迭代器
     * @param last 结束迭代器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     * @note
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

    /**
     * @brief 希尔排序（容器版本，支持默认比较器）
     * @tparam Container 容器类型（需支持随机访问迭代器）
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param container 待排序的容器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     * @note
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
        shell_sort(traits::begin(container), traits::end(container), comp);
    }
    // ===========================================================================

    // 用户接口 - 冒泡排序
    // ===========================================================================
    /**
     * @brief 冒泡排序（迭代器版本，支持默认比较器）
     * @tparam Iterator 双向迭代器类型
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param first 起始迭代器
     * @param last 结束迭代器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     * @note
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

    /**
     * @brief 冒泡排序（容器版本，支持默认比较器）
     * @tparam Container 容器类型（支持双向迭代器）
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param container 待排序的容器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     * @note
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
        bubble_sort(traits::begin(container), traits::end(container), comp);
    }
    // ===========================================================================

    // 用户接口 - 选择排序
    // ===========================================================================
    /**
     * @brief 选择排序（迭代器版本，支持默认比较器）
     * @tparam Iterator 迭代器类型
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param first 起始迭代器
     * @param last 结束迭代器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     * @note
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

    /**
     * @brief 选择排序（容器版本，支持默认比较器）
     * @tparam Container 容器类型（支持迭代器）
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param container 待排序的容器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     * @note
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
        selection_sort(traits::begin(container), traits::end(container), comp);
    }
    // ===========================================================================

    // 用户接口 - 堆排序
    // ===========================================================================
    /**
     * @brief 堆排序（迭代器版本，支持默认比较器）
     * @tparam RandomIt 随机访问迭代器类型
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param first 起始迭代器
     * @param last 结束迭代器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     * @note
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

    /**
     * @brief 堆排序（容器版本，支持默认比较器）
     * @tparam Container 容器类型（需支持随机访问迭代器）
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param container 待排序的容器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     * @note
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
        heap_sort(traits::begin(container), traits::end(container), comp);
    }
    // ===========================================================================

    // 用户接口 - 归并排序
    // ===========================================================================
    /**
     * @brief 归并排序（迭代器版本，支持默认比较器）
     * @tparam RandomIt 随机访问迭代器类型
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param first 起始迭代器
     * @param last 结束迭代器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     * @note
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

    /**
     * @brief 归并排序（容器版本，支持默认比较器）
     * @tparam Container 容器类型（需支持随机访问迭代器）
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param container 待排序的容器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     * @note
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
        merge_sort(traits::begin(container), traits::end(container), comp);
    }
    // ===========================================================================

    // 用户接口 - 计数排序
    // ===========================================================================
    /**
     * @brief 计数排序（迭代器版本，支持默认比较器）
     * @tparam RandomIt 随机访问迭代器类型（元素类型为整数）
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param first 起始迭代器
     * @param last 结束迭代器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     * @note
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

    /**
     * @brief 计数排序（容器版本，支持默认比较器）
     * @tparam Container 容器类型（需支持随机访问迭代器，元素类型为整数）
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param container 待排序的容器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     * @note
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
        counting_sort(traits::begin(container), traits::end(container), comp);
    }
    // ===========================================================================

    // 用户接口 - 基数排序（LSD）
    // ===========================================================================
    /**
     * @brief 基数排序（迭代器版本，LSD策略，适用于整数类型）
     * @tparam RandomIt 随机访问迭代器类型
     * @tparam Compare 比较器类型，仅支持std::less（升序）和std::greater（降序）
     * @param first 排序区间的起始迭代器（包含）
     * @param last 排序区间的结束迭代器（不包含）
     * @param radix 基数，默认为10（十进制）
     * @note
     * 算法特性：
     * - 非比较型排序，基于数字位数特性实现
     * - 稳定性：稳定排序（相等元素保持原始相对顺序）
     * - 时间复杂度：O(d*(n+r))，d为最大位数，n为元素数，r为基数
     * - 支持升序（默认，使用std::less）和降序（使用std::greater）
     * - 降序通过"先升序排序，再反转结果"实现
     * 适用场景：
     * - 待排序元素为**整数类型**（如int、long、unsigned int等）的场景
     * - 元素取值范围较大，但**位数d较小且固定**的场景（如手机号、身份证号、固定长度编码）
     * - 对排序稳定性有要求，且需避免比较型排序最坏情况（如O(n²)）的场景
     * - 数据量较大（n较大），且基数r选择合理（如2^8=256，适配内存操作）的场景
     * - 不适用场景：非整数类型数据、位数差异极大且最大值位数d远大于log_r n的场景
     */
    template <typename RandomIt, typename Compare = std::less<typename std::iterator_traits<RandomIt>::value_type>>
    typename std::enable_if<std::is_integral<typename std::iterator_traits<RandomIt>::value_type>::value, void>::type
    radix_sort_lsd(RandomIt first, RandomIt last, int radix = 10, const Compare& comp = Compare())
    {
        if (radix < 2)
            throw std::invalid_argument("Radix must be greater than or equal to 2");

        // 基数排序默认按升序执行
        detail::radix_sort_lsd_impl(first, last, radix);

        // 判断是否需要降序（通过比较器类型）
        if constexpr (std::is_same_v<Compare, std::greater<typename std::iterator_traits<RandomIt>::value_type>>)
            std::reverse(first, last); // 降序处理：反转升序结果
    }

    /**
     * @brief 基数排序（容器版本，LSD策略，适用于整数类型）
     * @tparam Container 容器类型（需支持随机访问迭代器，元素为整数）
     * @tparam Compare 比较器类型，仅支持std::less（升序）和std::greater（降序）
     * @param container 待排序的容器
     * @param radix 基数，默认为10（十进制）
     * @param comp 比较器，std::less为升序，std::greater为降序
     * @note
     * 算法特性：
     * - 非比较型排序，基于数字位数特性实现
     * - 稳定性：稳定排序（相等元素保持原始相对顺序）
     * - 时间复杂度：O(d*(n+r))，d为最大位数，n为元素数，r为基数
     * - 支持升序（默认，使用std::less）和降序（使用std::greater）
     * - 降序通过"先升序排序，再反转结果"实现
     * 适用场景：
     * - 待排序元素为**整数类型**（如int、long、unsigned int等）的场景
     * - 元素取值范围较大，但**位数d较小且固定**的场景（如手机号、身份证号、固定长度编码）
     * - 对排序稳定性有要求，且需避免比较型排序最坏情况（如O(n²)）的场景
     * - 数据量较大（n较大），且基数r选择合理（如2^8=256，适配内存操作）的场景
     * - 不适用场景：非整数类型数据、位数差异极大且最大值位数d远大于log_r n的场景
     */
    template <typename Container, typename Compare = std::less<typename container_traits<Container>::value_type>>
    typename std::enable_if<std::is_integral<typename container_traits<Container>::value_type>::value, void>::type
    radix_sort_lsd(Container& container, int radix = 10, const Compare& comp = Compare())
    {
        using traits = container_traits<Container>;
        radix_sort_lsd(traits::begin(container), traits::end(container), radix, comp);
    }
    // ===========================================================================

    // 用户接口 - 基数排序（MSD）
    // ===========================================================================
    /**
     * @brief 基数排序（迭代器版本，MSD策略，字符串专用）
     * @tparam RandomIt 随机访问迭代器类型（元素为string）
     * @param first 起始迭代器
     * @param last 结束迭代器
     * @param max_pos 最大处理位数（-1表示处理全部字符，默认-1）
     * @param radix 基数（字符集大小，默认256支持所有ASCII字符）
     */
    template <typename RandomIt>
    typename std::enable_if<
        std::is_same<typename std::iterator_traits<RandomIt>::value_type, std::string>::value,
        void>::type
    radix_sort_msd(RandomIt first, RandomIt last, int max_pos = -1, int radix = 256)
    {
        static_assert(
            std::is_same_v<
                typename std::iterator_traits<RandomIt>::iterator_category,
                std::random_access_iterator_tag>,
            "radix_sort_msd requires random access iterators");

        if (radix < 2)
            throw std::invalid_argument("Radix must be greater than or equal to 2");
        if (max_pos < -1)
            throw std::invalid_argument("max_pos must be >= -1");

        detail::radix_sort_msd_impl(first, last, 0, max_pos, radix);
    }

    /**
     * @brief 基数排序（容器版本，MSD策略，字符串专用）
     * @tparam Container 容器类型（元素为string，需支持随机访问迭代器）
     * @param container 待排序的容器
     * @param max_pos 最大处理位数（-1表示处理全部字符，默认-1）
     * @param radix 基数（字符集大小，默认256支持所有ASCII字符）
     * @note
     * 算法特性：
     * - 非比较型排序，基于字符高位优先策略
     * - 稳定性：稳定排序（相等元素保持原始相对顺序）
     * - 时间复杂度：O(d*(n + r))，d为最大字符串长度，n为元素数，r为基数
     * - 空间复杂度：O(n + r)，需要额外存储空间
     * 适用场景：
     * - 字符串排序（尤其是变长字符串）
     * - 高位差异明显的数据（如按首字母排序的单词表）
     * - 需要按前缀进行分组的场景
     */
    template <typename Container>
    typename std::enable_if<
        std::is_same<typename container_traits<Container>::value_type, std::string>::value,
        void>::type
    radix_sort_msd(Container& container, int max_pos = -1, int radix = 256)
    {
        using traits = container_traits<Container>;
        radix_sort_msd(traits::begin(container), traits::end(container), max_pos, radix);
    }

    /**
     * @brief 按字符串前缀分组（基于基数排序MSD）
     * @tparam Container 容器类型（元素为string，需支持随机访问迭代器）
     * @param container 待分组的容器
     * @param group_pos 按前几位分组（必须>=1）
     * @param radix 基数（字符集大小，默认256）
     * @return 分组结果，每个子容器包含前缀相同的字符串
     * @note 分组结果内部已按MSD排序
     */
    template <typename Container>
    typename std::enable_if<
        std::is_same<typename container_traits<Container>::value_type, std::string>::value,
        std::vector<Container>>::type
    radix_group_by_prefix(Container& container, size_t group_pos, int radix = 256)
    {
        using traits = container_traits<Container>;

        if (group_pos == 0)
            throw std::invalid_argument("group_pos must be >= 1");
        if (radix < 2)
            throw std::invalid_argument("Radix must be greater than or equal to 2");

        // 调用分组实现
        auto groups = detail::radix_group_by_prefix_impl(
            traits::begin(container),
            traits::end(container),
            group_pos,
            radix);

        // 转换为目标容器类型
        std::vector<Container> result;
        result.reserve(groups.size());
        for (auto& group : groups)
        {
            result.emplace_back(group.begin(), group.end());
        }
        return result;
    }
    // ===========================================================================

    // 用户接口 - 快速排序
    // ===========================================================================
    /**
     * @brief 快速排序（迭代器版本）
     * @tparam RandomIt 随机访问迭代器类型
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param first 起始迭代器
     * @param last 结束迭代器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     * @note
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

    /**
     * @brief 快速排序（容器版本）
     * @tparam Container 容器类型（需支持随机访问迭代器）
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param container 待排序的容器
     * @param comp 比较函数对象，返回true表示第一个参数应排在前面
     * @note
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
        quick_sort(traits::begin(container), traits::end(container), comp);
    }
    // ===========================================================================

    // 用户接口 - 桶排序
    // ===========================================================================
    /**
     * @brief 桶排序（迭代器版本，适用于浮点数）
     *
     * 与容器版本特性一致，支持自定义比较器，适用于浮点数范围排序。
     */
    template <typename RandomIt, typename Compare = std::less<typename std::iterator_traits<RandomIt>::value_type>>
    typename std::enable_if<
        std::is_floating_point<typename std::iterator_traits<RandomIt>::value_type>::value,
        void>::type
    bucket_sort(RandomIt first, RandomIt last,
                size_t num_buckets = 10,
                double min_val = 0.0, double max_val = 1.0,
                const Compare& comp = Compare())
    {
        static_assert(
            std::is_same_v<
                typename std::iterator_traits<RandomIt>::iterator_category,
                std::random_access_iterator_tag>,
            "bucket_sort requires random access iterators");

        if (num_buckets < 1)
            throw std::invalid_argument("Number of buckets must be at least 1");
        if (min_val >= max_val)
            throw std::invalid_argument("min_val must be less than max_val");

        detail::bucket_sort_float_impl(first, last, num_buckets, min_val, max_val, comp);
    }

    /**
     * @brief 桶排序（迭代器版本，适用于整数）
     *
     * 与容器版本特性一致，支持自定义比较器，自动计算数据范围。
     */
    template <typename RandomIt, typename Compare = std::less<typename std::iterator_traits<RandomIt>::value_type>>
    typename std::enable_if<
        std::is_integral<typename std::iterator_traits<RandomIt>::value_type>::value,
        void>::type
    bucket_sort(RandomIt first, RandomIt last,
                size_t num_buckets = 10,
                const Compare& comp = Compare())
    {
        static_assert(
            std::is_same_v<
                typename std::iterator_traits<RandomIt>::iterator_category,
                std::random_access_iterator_tag>,
            "bucket_sort requires random access iterators");

        if (num_buckets < 1)
            throw std::invalid_argument("Number of buckets must be at least 1");

        detail::bucket_sort_int_impl(first, last, num_buckets, comp);
    }

    /**
     * @brief 桶排序（容器版本，适用于浮点数）
     * @tparam Container 容器类型（需支持随机访问迭代器，元素为浮点数）
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param container 待排序的容器
     * @param num_buckets 桶数量（默认10，建议设为接近元素数量的平方根）
     * @param min_val 数据最小值（必须小于max_val）
     * @param max_val 数据最大值（必须大于min_val）
     * @param comp 比较函数对象，决定桶内元素的排序规则
     * @throw std::invalid_argument 当桶数量小于1或min_val >= max_val时抛出
     * @note
     * 算法特性：
     * - 混合排序：桶划分基于数值范围，桶内使用插入排序（支持比较器）
     * - 稳定性：稳定排序（相等元素保持原始相对顺序）
     * - 时间复杂度：平均O(n + k)，最坏O(n²)（k为桶数量）
     *   - 性能与数据分布和初始有序度密切相关：
     *     1. 数据分布均匀时，各桶元素数量均衡，整体效率接近线性
     *     2. 初始有序度高时，桶内元素已有序，插入排序效率显著提升（接近O(m)）
     *     3. 数据分布不均时，元素集中在少数桶中，退化为桶内插入排序的O(m²)
     * - 空间复杂度：O(n + k)，需额外存储k个桶及所有元素
     * - 适用场景：数据分布均匀且范围已知的浮点数排序，尤其适合初始有序度较高的数据集
     */
    template <typename Container, typename Compare = std::less<typename Container::value_type>>
    typename std::enable_if<
        std::is_floating_point<typename Container::value_type>::value,
        void>::type
    bucket_sort(Container& container,
                size_t num_buckets = 10,
                double min_val = 0.0, double max_val = 1.0,
                const Compare& comp = Compare())
    {
        using traits = container_traits<Container>;
        bucket_sort(traits::begin(container), traits::end(container), num_buckets, min_val, max_val, comp);
    }

    /**
     * @brief 桶排序（容器版本，适用于整数）
     * @tparam Container 容器类型（需支持随机访问迭代器，元素为整数）
     * @tparam Compare 比较函数类型，需满足严格弱序，默认使用std::less
     * @param container 待排序的容器
     * @param num_buckets 桶数量（默认10，建议设为接近元素数量的平方根）
     * @param comp 比较函数对象，决定排序规则
     * @throw std::invalid_argument 当桶数量小于1时抛出
     * @note
     * 算法特性：
     * - 混合排序：自动划分桶，桶内使用插入排序（支持比较器）
     * - 稳定性：稳定排序（相等元素保持原始相对顺序）
     * - 时间复杂度：平均O(n + k)，最坏O(n²)（k为桶数量）
     *   - 性能与数据分布和初始有序度均相关：
     *     1. 当数据分布均匀时，各桶元素数量均衡，桶内插入排序效率高
     *     2. 初始有序度高时，桶内元素已有序，插入排序退化到O(m)（m为桶内元素数）
     *     3. 若数据集中在少数桶中，会退化为桶内插入排序的O(m²)，整体接近O(n²)
     * - 空间复杂度：O(n + k)，需要额外空间存储k个桶及n个元素
     * - 适用场景：数据分布均匀且范围已知的整数排序，尤其适合初始有序度较高的数据集
     */
    template <typename Container, typename Compare = std::less<typename Container::value_type>>
    typename std::enable_if<
        std::is_integral<typename Container::value_type>::value,
        void>::type
    bucket_sort(Container& container, size_t num_buckets = 10, const Compare& comp = Compare())
    {
        using traits = container_traits<Container>;
        bucket_sort(traits::begin(container), traits::end(container), num_buckets, comp);
    }
    // ===========================================================================

    // 打印容器（调试用）
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