/***************************************************************************************/
/*
 * 程序名：ol_bitree.h
 * 功能描述：树状数组|二叉索引树（Binary Indexed Tree，BIT）模板类的实现，支持以下特性：
 *          - 基于向量存储，索引从 0 开始
 *          - 提供点更新、前缀和查询、区间和查询功能
 *          - 支持初始化列表和向量构造，支持重置操作
 *          - 重载运算符便于访问和赋值
 * 作者：ol
 * 适用标准：C++11 及以上（需支持初始化列表、auto 等特性）
 */
/***************************************************************************************/

#ifndef OL_BITREE_H
#define OL_BITREE_H 1

#include <initializer_list>
#include <iostream>
#include <vector>

namespace ol
{

    /**
     * @brief 树状数组|二叉索引树（Binary Indexed Tree，BIT）模板类
     *        适用于高效的点更新和前缀和查询操作，时间复杂度均为O(log n)
     * @tparam T 存储的数据类型（需支持加法运算）
     */
    template <typename T>
    class BITree
    {
    private:
        std::vector<T> b; // 索引从0开始

    private:
        /**
         * @brief 计算lowbit值（二进制表示中最低位的1所对应的值）
         * @param i 输入整数（需为非负数）
         * @return 最低位1对应的数值
         */
        size_t lowbit(signed long long i) const
        {
            return i & (-i);
        }

    public:
        /**
         * @brief 从向量构造BITree并初始化
         * @param arr 初始数据向量
         */
        BITree(const std::vector<T>& arr) : b(arr)
        {
            init();
        }

        /**
         * @brief 从初始化列表构造BITree并初始化
         * @param list 初始化列表
         */
        BITree(std::initializer_list<T> list) : b(list)
        {
            init();
        }

        /**
         * @brief 初始化树结构（构建BIT）
         *        对原始数组进行预处理，生成符合BIT规则的存储结构
         */
        void init()
        {
            for (size_t i = 0, size = b.size(); i < size; ++i)
            {
                size_t j = i + lowbit(i + 1);
                if (j < size) b[j] += b[i];
            }
        }

        /**
         * @brief 点更新：为指定索引的元素增加一个值
         * @param idx 要更新的元素索引（从0开始）
         * @param x 要增加的值（可正可负）
         */
        void add(size_t idx, T x)
        {
            ++idx;
            while (idx <= b.size())
            {
                b[idx - 1] += x;
                idx += lowbit(idx);
            }
        }

        /**
         * @brief 前缀和查询：计算[0, idx]的元素和
         * @param idx 前缀的结束索引（从0开始）
         * @return 前缀和结果（若idx超出范围，自动截断到有效范围）
         */
        T sum(size_t idx) const
        {
            if (idx >= b.size()) idx = b.size() - 1;
            T res = T();
            ++idx;
            while (idx > 0)
            {
                res += b[idx - 1];
                idx -= lowbit(idx);
            }
            return res;
        }

        /**
         * @brief 区间和查询：计算[left, right]的元素和
         * @param left 区间起始索引（从0开始）
         * @param right 区间结束索引（从0开始）
         * @return 区间和结果（若区间无效，返回0）
         * @note 无效情况包括：left >= 数组大小、left > right、right >= 数组大小（自动截断）
         */
        T rangeSum(size_t left, size_t right) const
        {
            if (left >= b.size() || left > right)
            {
                return 0;
            }
            if (right >= b.size())
            {
                right = b.size() - 1;
            }
            T leftSum = (left == 0) ? 0 : sum(left - 1);
            return sum(right) - leftSum;
        }

        /**
         * @brief 重置BITree为新的向量数据
         * @param arr 新的数据源向量
         */
        void reset(const std::vector<T>& arr)
        {
            b = arr;
            init();
        }

        /**
         * @brief 重置BITree为初始化列表数据
         * @param list 新的数据源初始化列表
         */
        void reset(std::initializer_list<T> list)
        {
            b = list;
            init();
        }

        /**
         * @brief 打印当前BITree的内部存储结构
         *        用于调试，输出向量b中的所有元素
         */
        void print() const
        {
            std::cout << "BITree: ";
            for (size_t i = 0, size = b.size(); i < size; ++i)
            {
                std::cout << b[i] << " ";
            }
            std::cout << "\n";
        }

        /**
         * @brief 获取BITree的大小（元素个数）
         * @return 向量b的大小
         */
        size_t size() const
        {
            return b.size();
        }

        /**
         * @brief 重载[]运算符，访问内部存储的元素
         * @param idx 元素索引（从0开始）
         * @return 索引对应的元素值（若超出范围，返回T的默认值）
         */
        T operator[](size_t idx) const
        {
            if (idx >= b.size()) return T();
            return b[idx];
        }

        /**
         * @brief 重载=运算符，从初始化列表赋值并重新初始化
         * @param list 赋值的初始化列表
         * @return 当前BITree对象的引用
         */
        BITree<T>& operator=(std::initializer_list<T> list)
        {
            b = list;
            init();
            return *this;
        }

        /**
         * @brief 重载=运算符，从向量赋值并重新初始化
         * @param arr 赋值的向量
         * @return 当前BITree对象的引用
         */
        BITree<T>& operator=(const std::vector<T>& arr)
        {
            b = arr;
            init();
            return *this;
        }
    };

} // namespace ol

#endif // !OL_BITREE_H