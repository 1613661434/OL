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

#ifndef OL_SORT_H
#define OL_SORT_H 1

#include "ol_base/ol_sort_base.h"

namespace ol
{
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
        base::insertion_sort_base(first, last, comp);
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

        base::binary_insertion_sort_base(first, last, comp);
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

        base::shell_sort_base(first, last, comp);
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
        base::bubble_sort_base(first, last, comp);
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
        base::selection_sort_base(first, last, comp);
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

        base::heap_sort_base(first, last, comp);
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

        base::merge_sort(first, last, comp);
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

        base::counting_sort_base(first, last, comp);
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
        base::radix_sort_lsd_base(first, last, radix);

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

        base::radix_sort_msd_base(first, last, 0, max_pos, radix);
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
        auto groups = base::radix_group_by_prefix_base(
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

        base::quick_sort_base(first, last, comp);
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

        base::bucket_sort_float_base(first, last, num_buckets, min_val, max_val, comp);
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

        base::bucket_sort_int_base(first, last, num_buckets, comp);
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

#endif // !OL_SORT_H