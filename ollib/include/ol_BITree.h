/***************************************************************************************/
/*
 * ��������ol_bitree.h
 * ������������״����|������������Binary Indexed Tree��BIT��ģ�����ʵ�֣�֧���������ԣ�
 *          - ���������洢�������� 0 ��ʼ
 *          - �ṩ����¡�ǰ׺�Ͳ�ѯ������Ͳ�ѯ����
 *          - ֧�ֳ�ʼ���б���������죬֧�����ò���
 *          - ������������ڷ��ʺ͸�ֵ
 * ���ߣ�ol
 * ���ñ�׼��C++11 �����ϣ���֧�ֳ�ʼ���б�auto �����ԣ�
 */
/***************************************************************************************/

#ifndef __OL_BITREE_H
#define __OL_BITREE_H 1

#include <initializer_list>
#include <iostream>
#include <vector>

namespace ol
{

    /**
     * ��״����|������������Binary Indexed Tree��BIT��ģ����
     * �����ڸ�Ч�ĵ���º�ǰ׺�Ͳ�ѯ������ʱ�临�ӶȾ�ΪO(log n)
     * @tparam T �洢���������ͣ���֧�ּӷ����㣩
     */
    template <typename T>
    class BITree
    {
    private:
        std::vector<T> b; // ������0��ʼ

    private:
        /**
         * ����lowbitֵ�������Ʊ�ʾ�����λ��1����Ӧ��ֵ��
         * @param i ������������Ϊ�Ǹ�����
         * @return ���λ1��Ӧ����ֵ
         */
        size_t lowbit(signed long long i) const
        {
            return i & (-i);
        }

    public:
        /**
         * ����������BITree����ʼ��
         * @param arr ��ʼ��������
         */
        BITree(const std::vector<T>& arr) : b(arr)
        {
            init();
        }

        /**
         * �ӳ�ʼ���б���BITree����ʼ��
         * @param list ��ʼ���б�
         */
        BITree(std::initializer_list<T> list) : b(list)
        {
            init();
        }

        /**
         * ��ʼ�����ṹ������BIT��
         * ��ԭʼ�������Ԥ�������ɷ���BIT����Ĵ洢�ṹ
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
         * ����£�Ϊָ��������Ԫ������һ��ֵ
         * @param idx Ҫ���µ�Ԫ����������0��ʼ��
         * @param x Ҫ���ӵ�ֵ�������ɸ���
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
         * ǰ׺�Ͳ�ѯ������[0, idx]��Ԫ�غ�
         * @param idx ǰ׺�Ľ�����������0��ʼ��
         * @return ǰ׺�ͽ������idx������Χ���Զ��ضϵ���Ч��Χ��
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
         * ����Ͳ�ѯ������[left, right]��Ԫ�غ�
         * @param left ������ʼ��������0��ʼ��
         * @param right ���������������0��ʼ��
         * @return ����ͽ������������Ч������0��
         * @note ��Ч���������left >= �����С��left > right��right >= �����С���Զ��ضϣ�
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
         * ����BITreeΪ�µ���������
         * @param arr �µ�����Դ����
         */
        void reset(const std::vector<T>& arr)
        {
            b = arr;
            init();
        }

        /**
         * ����BITreeΪ��ʼ���б�����
         * @param list �µ�����Դ��ʼ���б�
         */
        void reset(std::initializer_list<T> list)
        {
            b = list;
            init();
        }

        /**
         * ��ӡ��ǰBITree���ڲ��洢�ṹ
         * ���ڵ��ԣ��������b�е�����Ԫ��
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
         * ��ȡBITree�Ĵ�С��Ԫ�ظ�����
         * @return ����b�Ĵ�С
         */
        size_t size() const
        {
            return b.size();
        }

        /**
         * ����[]������������ڲ��洢��Ԫ��
         * @param idx Ԫ����������0��ʼ��
         * @return ������Ӧ��Ԫ��ֵ����������Χ������T��Ĭ��ֵ��
         */
        T operator[](size_t idx) const
        {
            if (idx >= b.size()) return T();
            return b[idx];
        }

        /**
         * ����=��������ӳ�ʼ���б�ֵ�����³�ʼ��
         * @param list ��ֵ�ĳ�ʼ���б�
         * @return ��ǰBITree���������
         */
        BITree<T>& operator=(std::initializer_list<T> list)
        {
            b = list;
            init();
            return *this;
        }

        /**
         * ����=���������������ֵ�����³�ʼ��
         * @param arr ��ֵ������
         * @return ��ǰBITree���������
         */
        BITree<T>& operator=(const std::vector<T>& arr)
        {
            b = arr;
            init();
            return *this;
        }
    };

} // namespace ol

#endif // !__OL_BITREE_H