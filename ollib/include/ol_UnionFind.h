/****************************************************************************************/
/*
 * 程序名：ol_UnionFind.h
 * 功能描述：并查集（Union-Find）数据结构模板类，支持以下特性：
 *          - 两种实现策略：整数类型用数组存储，非整数类型用哈希表存储
 *          - 路径压缩优化（Path Compression）
 *          - 按秩合并优化（Union by Rank）
 *          - 支持动态添加元素（非整数类型）
 *          - 跨平台兼容（依赖C++11标准库）
 * 作者：ol
 * 适用标准：C++11及以上（需支持模板特化、STL容器等特性）
 */
/****************************************************************************************/
#ifndef OL_UNIONFIND_H
#define OL_UNIONFIND_H 1

#include <iostream>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace ol
{

    /**
     * @brief 并查集（Union-Find）模板类，用于管理不相交集合的合并与查询
     *
     * 实现了两种存储策略：
     * - 整数类型（如int、size_t）使用连续数组存储，效率更高
     * - 非整数类型（如std::string、自定义哈希类型）使用哈希表存储，支持动态元素
     * 核心优化策略：路径压缩（加速查询）和按秩合并（控制树高）
     * @tparam T 元素类型，需支持比较操作（整数类型或可哈希类型）
     */
    template <typename T, typename Enable = void>
    class UnionFind;

    /**
     * @brief 并查集针对整数类型的特化实现
     *
     * 使用vector存储父节点和秩信息，适合处理连续范围的整数索引场景
     * @tparam T 整数类型（如int、size_t等，需满足std::is_integral_v<T>为true）
     */
    template <typename T>
    class UnionFind<T, std::enable_if_t<std::is_integral_v<T>>>
    {
    private:
        std::vector<T> parent;    ///< 存储每个节点的父节点索引
        std::vector<size_t> rank; ///< 存储每个节点的秩（树的高度）

    public:
        /**
         * @brief 构造函数，初始化并查集大小
         * @param size 初始元素数量（默认为0）
         */
        explicit UnionFind(size_t size = 0)
        {
            init(size);
        }

        /**
         * @brief 重置并查集状态，重新初始化指定数量的节点
         * @param size 新的元素数量，每个节点初始父节点为自身，秩为0
         */
        void init(size_t size)
        {
            parent.resize(size);
            rank.resize(size, 0);
            for (size_t i = 0; i < size; ++i)
            {
                parent[i] = static_cast<T>(i);
            }
        }

        /**
         * @brief 查找元素x所在集合的根节点（带路径压缩）
         *
         * 路径压缩：将查询路径上的所有节点直接指向根节点，加速后续查询
         * @param x 待查找的元素
         * @return 根节点索引
         */
        T find(T x)
        {
            if (parent[x] != x)
            {
                parent[x] = find(parent[x]); // 递归压缩路径
            }
            return parent[x];
        }

        /**
         * @brief 合并元素x和y所在的集合（按秩合并）
         *
         * 按秩合并：将秩较小的树合并到秩较大的树上，避免树高度增长过快
         * @param x 第一个元素
         * @param y 第二个元素
         */
        void unite(T x, T y)
        {
            T rootX = find(x);
            T rootY = find(y);

            if (rootX == rootY) return; // 已在同一集合

            if (rank[rootX] < rank[rootY])
            {
                parent[rootX] = rootY;
            }
            else if (rank[rootX] > rank[rootY])
            {
                parent[rootY] = rootX;
            }
            else
            {
                parent[rootY] = rootX;
                ++rank[rootX]; // 秩相同，合并后根节点秩+1
            }
        }

        /**
         * @brief 判断两个元素是否属于同一集合
         * @param x 第一个元素
         * @param y 第二个元素
         * @return 若在同一集合返回true，否则返回false
         */
        bool connected(T x, T y)
        {
            return find(x) == find(y);
        }

        /**
         * @brief 计算并查集中不相交集合的数量
         * @return 集合数量（根节点的数量）
         */
        size_t countSets() const
        {
            size_t count = 0;
            for (size_t i = 0; i < parent.size(); ++i)
            {
                if (parent[i] == static_cast<T>(i))
                { // 根节点的父节点是自身
                    ++count;
                }
            }
            return count;
        }

        /**
         * @brief 获取并查集中的元素总数
         * @return 元素数量
         */
        size_t size() const
        {
            return parent.size();
        }
    };

    /**
     * @brief 并查集针对非整数类型的通用实现
     *
     * 使用哈希表存储节点信息，支持任意可哈希类型，元素可动态添加
     * @tparam T 非整数类型（需满足std::is_integral_v<T>为false且支持哈希）
     */
    template <typename T>
    class UnionFind<T, std::enable_if_t<!std::is_integral_v<T>>>
    {
    private:
        /**
         * @brief 节点结构，存储父节点和秩信息
         */
        struct Node
        {
            T parent;    ///< 父节点引用
            size_t rank; ///< 节点的秩（树的高度）
            /**
             * @brief 节点构造函数
             * @param p 父节点（默认自身）
             * @param r 初始秩（默认0）
             */
            Node(const T& p = T(), size_t r = 0) : parent(p), rank(r)
            {
            }
        };

        std::unordered_map<T, Node> nodes; ///< 哈希表存储所有节点

    public:
        /**
         * @brief 插入新元素（若不存在则初始化）
         * @param x 待插入的元素
         */
        void insert(const T& x)
        {
            if (nodes.find(x) == nodes.end())
            {
                nodes[x] = Node(x); // 新元素初始父节点为自身
            }
        }

        /**
         * @brief 查找元素x所在集合的根节点（带路径压缩）
         *
         * 路径压缩：将查询路径上的所有节点直接指向根节点，加速后续查询
         * 若元素不存在则自动插入
         * @param x 待查找的元素
         * @return 根节点
         */
        T find(const T& x)
        {
            auto it = nodes.find(x);
            if (it == nodes.end())
            {
                insert(x); // 自动插入不存在的元素
                return x;
            }

            if (it->second.parent != x)
            {
                it->second.parent = find(it->second.parent); // 递归压缩路径
            }
            return it->second.parent;
        }

        /**
         * @brief 合并元素x和y所在的集合（按秩合并）
         *
         * 自动插入不存在的元素，按秩合并策略避免树高度增长过快
         * @param x 第一个元素
         * @param y 第二个元素
         */
        void unite(const T& x, const T& y)
        {
            insert(x);
            insert(y);

            T rootX = find(x);
            T rootY = find(y);

            if (rootX == rootY) return; // 已在同一集合

            auto& nodeX = nodes[rootX];
            auto& nodeY = nodes[rootY];

            if (nodeX.rank < nodeY.rank)
            {
                nodeX.parent = rootY;
            }
            else if (nodeX.rank > nodeY.rank)
            {
                nodeY.parent = rootX;
            }
            else
            {
                nodeY.parent = rootX;
                ++nodeX.rank; // 秩相同，合并后根节点秩+1
            }
        }

        /**
         * @brief 判断两个元素是否属于同一集合
         * @param x 第一个元素
         * @param y 第二个元素
         * @return 若在同一集合返回true，否则返回false
         */
        bool connected(const T& x, const T& y)
        {
            return find(x) == find(y);
        }

        /**
         * @brief 计算并查集中不相交集合的数量
         * @return 集合数量（根节点的数量）
         */
        size_t countSets() const
        {
            size_t count = 0;
            for (const auto& pair : nodes)
            {
                if (pair.first == pair.second.parent)
                { // 根节点的父节点是自身
                    ++count;
                }
            }
            return count;
        }

        /**
         * @brief 获取并查集中的元素总数
         * @return 元素数量
         */
        size_t size() const
        {
            return nodes.size();
        }
    };

} // namespace ol

#endif // !OL_UNIONFIND_H