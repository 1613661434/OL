#ifndef OL_SORT_BASE_H
#define OL_SORT_BASE_H 1

#include "ol_type_traits.h"
#include <algorithm>
#include <array>
#include <cstddef>
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

    // 排序算法实现 (内部实现)
    // ===========================================================================
    namespace base
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
        void insertion_sort_base(Iterator first, Iterator last, const Compare& comp)
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
        RandomIt binary_search_base(RandomIt first, RandomIt last,
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
        void binary_insertion_sort_base(RandomIt first, RandomIt last, const Compare& comp)
        {
            if (first == last) return;

            for (RandomIt i = first + 1; i != last; ++i)
            {
                auto key = *i;
                RandomIt pos = binary_search_base(first, i, key, comp);

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
                              ptrdiff_t start, ptrdiff_t step, const Compare& comp)
        {
            const ptrdiff_t n = std::distance(first, last);

            for (ptrdiff_t i = start + step; i < n; i += step)
            {
                auto key = *(first + i);
                ptrdiff_t j = i - step;

                while (j >= start && comp(key, *(first + j)))
                {
                    *(first + j + step) = *(first + j);
                    j -= step;
                }

                // 这里必须要(j + step)否则VC会报错，因为VC是先first+j，导致越界直接报错
                *(first + (j + step)) = key;
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
        void shell_sort_base(RandomIt first, RandomIt last, const Compare& comp)
        {
            const ptrdiff_t n = std::distance(first, last);
            if (n <= 1) return;

            // 使用(3^k - 1)/2的递增序列：1, 4, 13, 40, 121...
            ptrdiff_t step = 1;
            while (step < n / 3)
            {
                step = 3 * step + 1;
            }

            while (step >= 1)
            {
                // 对每个组执行插入排序
                for (ptrdiff_t i = 0; i < step; ++i)
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
        void bubble_sort_base(Iterator first, Iterator last, const Compare& comp)
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
        void selection_sort_base(Iterator first, Iterator last, const Compare& comp)
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
        void heapify_base(RandomIt first, size_t size, size_t index, const Compare& comp)
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
        void heap_sort_base(RandomIt first, RandomIt last, const Compare& comp)
        {
            const ptrdiff_t n = std::distance(first, last);
            if (n <= 1) return;

            // 构建堆
            for (ptrdiff_t i = n / 2 - 1; i >= 0; --i)
            {
                heapify_base(first, n, static_cast<size_t>(i), comp);
            }

            // 提取最大元素并重建堆
            for (ptrdiff_t i = n - 1; i > 0; --i)
            {
                std::iter_swap(first, first + i);
                heapify_base(first, i, 0, comp);
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
        void merge_base(RandomIt first, RandomIt mid, RandomIt last,
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
        void merge_sort_base(RandomIt first, RandomIt last,
                             TempIt temp, const Compare& comp)
        {
            if (last - first > 1)
            {
                RandomIt mid = first + (last - first) / 2;
                auto temp_mid = temp + (mid - first);

                merge_sort_base(first, mid, temp, comp);
                merge_sort_base(mid, last, temp_mid, comp);
                merge_base(first, mid, last, temp, comp);
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
            const ptrdiff_t n = std::distance(first, last);
            if (n <= 1) return;

            std::vector<typename std::iterator_traits<RandomIt>::value_type> temp(n);
            merge_sort_base(first, last, temp.begin(), comp);
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
        counting_sort_base(RandomIt first, RandomIt last, const Compare& comp)
        {
            using ValueType = typename std::iterator_traits<RandomIt>::value_type;
            const ptrdiff_t n = std::distance(first, last);
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
            for (RandomIt it = last - 1; it >= first; --it)
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
        void radix_sort_lsd_base(RandomIt first, RandomIt last, int radix)
        {
            using ValueType = typename std::iterator_traits<RandomIt>::value_type;
            const ptrdiff_t n = std::distance(first, last);
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
        void radix_sort_msd_base(RandomIt first, RandomIt last,
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
                    radix_sort_msd_base(bucket_first, dest, pos + 1, max_pos, radix);
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
        radix_group_by_prefix_base(RandomIt first, RandomIt last,
                                   size_t group_pos, int radix = 256)
        {
            // 先按指定位数进行MSD排序
            radix_sort_msd_base(first, last, 0, static_cast<int>(group_pos), radix);

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
        void quick_sort_base(RandomIt first, RandomIt last, const Compare& comp)
        {
            ptrdiff_t size = std::distance(first, last);

            // 优化点：小规模数据（≤16个元素）使用插入排序
            if (size <= 16)
            {
                insertion_sort_base(first, last, comp);
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
            quick_sort_base(first, low, comp);    // 左区间：[first, low)
            quick_sort_base(low + 1, last, comp); // 右区间：[low+1, last)
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
        void bucket_sort_float_base(RandomIt first, RandomIt last,
                                    size_t num_buckets,
                                    double min_val, double max_val,
                                    const Compare& comp)
        {
            using ValueType = typename std::iterator_traits<RandomIt>::value_type;
            const ptrdiff_t n = std::distance(first, last);
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
                insertion_sort_base(bucket.begin(), bucket.end(), comp);

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
        void bucket_sort_int_base(RandomIt first, RandomIt last,
                                  size_t num_buckets,
                                  const Compare& comp)
        {
            using ValueType = typename std::iterator_traits<RandomIt>::value_type;
            const ptrdiff_t n = std::distance(first, last);
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
                insertion_sort_base(bucket.begin(), bucket.end(), comp);

            // 合并所有桶的结果
            auto dest_it = first;
            for (const auto& bucket : buckets)
                dest_it = std::copy(bucket.begin(), bucket.end(), dest_it);
        }
        // -----------------------------------------------------------------------

    } // namespace base
    // ===========================================================================

} // namespace ol

#endif //! OL_SORT_BASE_H