#ifndef __OL_BITREE_H
#define __OL_BITREE_H 1

#include <initializer_list>
#include <iostream>
#include <vector>

namespace ol
{

    template <typename T>
    class BITree
    {
    private:
        std::vector<T> b; // ������0��ʼ

    private:
        size_t lowbit(signed long long i) const
        {
            return i & (-i);
        }

    public:
        BITree(const std::vector<T>& arr) : b(arr)
        {
            init();
        }

        BITree(std::initializer_list<T> list) : b(list)
        {
            init();
        }

        // ��ʼ������
        void init()
        {
            for (size_t i = 0, size = b.size(); i < size; ++i)
            {
                size_t j = i + lowbit(i + 1);
                if (j < size) b[j] += b[i];
            }
        }

        // �޸�
        void add(size_t idx, T x)
        {
            ++idx;
            while (idx <= b.size())
            {
                b[idx - 1] += x;
                idx += lowbit(idx);
            }
        }

        // ���
        T sum(size_t idx) const
        {
            if (idx >= b.size()) idx = b.size() - 1;
            T res = T();
            idx++;
            while (idx > 0)
            {
                res += b[idx - 1];
                idx -= lowbit(idx);
            }
            return res;
        }

        // ����ͣ�����[left, right]�ĺͣ��±��0��ʼ��
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

        // ����
        void reset(const std::vector<T>& arr)
        {
            b = arr;
            init();
        }

        void reset(std::initializer_list<T> list)
        {
            b = list;
            init();
        }

        // ��ӡ
        void print() const
        {
            std::cout << "BITree: ";
            for (size_t i = 0, size = b.size(); i < size; ++i)
            {
                std::cout << b[i] << " ";
            }
            std::cout << "\n";
        }

        // ��С
        size_t size() const
        {
            return b.size();
        }

        T operator[](size_t idx) const
        {
            if (idx >= b.size()) return T();
            return b[idx];
        }

        BITree<T>& operator=(std::initializer_list<T> list)
        {
            b = list;
            init();
            return *this;
        }

        BITree<T>& operator=(const std::vector<T>& arr)
        {
            b = arr;
            init();
            return *this;
        }
    };

} // namespace ol

#endif // !__OL_BITREE_H