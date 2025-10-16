/****************************************************************************************/
/*
 * 程序名：ol_graph.h
 * 功能描述：通用图数据结构模板类的实现，支持以下特性：
 *          - 可配置有向/无向图（通过模板参数控制）
 *          - 可配置有权/无权边（通过模板参数控制）
 *          - 支持自定义节点类型和权重类型（默认均为int）
 *          - 提供基础图操作：添加/删除节点、添加/删除边、查询邻居、获取权重等
 * 作者：ol
 * 适用标准：C++17及以上（需支持constexpr if等特性）
 */
/****************************************************************************************/

#ifndef OL_GRAPH_H
#define OL_GRAPH_H 1

#include "ol_type_traits.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace ol
{
    /**
     * @brief 边结构，根据图的配置动态调整是否包含权重
     * 模板参数：
     * @param NodeType 节点数据类型
     * @param IsWeighted 是否包含权重（由外部图类控制）
     * @param WeightType 权重数据类型（默认 int，仅 IsWeighted=true 时有效）
     */
    template <typename NodeType, bool IsWeighted, typename WeightType = int>
    struct Edge
    {
        NodeType to; // 目标节点（类型由NodeType控制）

        // 仅当有权重时才定义weight成员（类型由WeightType控制）
        typename std::conditional_t<IsWeighted, WeightType, TypeEmpty> weight;

        // 无权图构造函数（仅需目标节点）
        template <bool W = IsWeighted, typename = std::enable_if_t<!W>>
        Edge(NodeType to_node) : to(to_node)
        {
        }

        // 有权图构造函数（需目标节点和权重）
        template <bool W = IsWeighted, typename = std::enable_if_t<W>>
        Edge(NodeType to_node, WeightType w) : to(to_node), weight(w)
        {
        }
    };

    /**
     * @brief 通用图模板类，支持有向/无向、有权/无权配置
     * 模板参数：
     * @param IsDirected 是否为有向图（默认 false）
     * @param IsWeighted 是否为有权图（默认 false）
     * @param NodeType 节点数据类型（默认 int）
     * @param WeightType 权重数据类型（默认 int，仅 IsWeighted=true 时有效）
     */
    template <bool IsDirected = false,
              bool IsWeighted = false,
              typename NodeType = int,
              typename WeightType = int>
    class Graph
    {
    private:
        std::unordered_map<NodeType, std::vector<Edge<NodeType, IsWeighted, WeightType>>> adjList; // adjacency list 的缩写，中文译为 “邻接表”
        size_t nodeCount;                                                                          // 节点总数

    public:
        // 构造函数
        Graph() : nodeCount(0)
        {
        }

        /**
         * @brief 添加节点（若节点已存在则不操作）
         * @param node 要添加的节点
         */
        void addNode(NodeType node)
        {
            if (adjList.find(node) == adjList.end())
            {
                adjList[node] = std::vector<Edge<NodeType, IsWeighted, WeightType>>();
                ++nodeCount;
            }
        }

        /**
         * @brief 添加一条边（支持有权/无权、有向/无向图的适配）
         * @param from 边的起点节点
         * @param to 边的终点节点
         * @param args 可变参数，仅有权图（IsWeighted=true）时需要传入权重值（类型为WeightType）
         * @note 1. 若起点/终点不存在，会自动添加节点
         *       2. 无向图（IsDirected=false）会自动添加反向边（to -> from）
         *       3. 有权图必须传入权重参数，无权图不可传入额外参数，否则会编译报错
         */
        template <typename... Args>
        void addEdge(NodeType from, NodeType to, Args&&... args)
        {
            addNode(from); // 确保起点存在
            addNode(to);   // 确保终点存在

            // 有权图需要传入权重参数，无权图不需要
            if constexpr (IsWeighted)
            {
                adjList[from].emplace_back(to, std::forward<Args>(args)...);
            }
            else
            {
                adjList[from].emplace_back(to);
            }

            // 无向图自动添加反向边
            if constexpr (!IsDirected)
            {
                if constexpr (IsWeighted)
                {
                    adjList[to].emplace_back(from, std::forward<Args>(args)...);
                }
                else
                {
                    adjList[to].emplace_back(from);
                }
            }
        }

        /**
         * @brief 删除指定边（无向图会同时删除反向边）
         * @param from 起点节点
         * @param to 终点节点
         * @note 若边不存在则不操作
         */
        void rmEdge(NodeType from, NodeType to)
        {
            // 先处理正向边
            if (adjList.find(from) != adjList.end())
            {
                auto& edges = adjList[from];
                for (auto it = edges.begin(); it != edges.end(); ++it)
                {
                    if (it->to == to)
                    {
                        edges.erase(it);
                        break;
                    }
                }
            }

            // 无向图需要删除反向边
            if constexpr (!IsDirected)
            {
                if (adjList.find(to) != adjList.end())
                {
                    auto& reverseEdges = adjList[to];
                    for (auto it = reverseEdges.begin(); it != reverseEdges.end(); ++it)
                    {
                        if (it->to == from)
                        {
                            reverseEdges.erase(it);
                            break;
                        }
                    }
                }
            }
        }

        /**
         * @brief 判断边是否存在
         * @param from 起点节点
         * @param to 终点节点
         * @return 存在返回 true，否则返回 false（节点不存在时也返回 false）
         */
        bool hasEdge(NodeType from, NodeType to) const
        {
            if (adjList.find(from) == adjList.end()) return false;

            const auto& edges = adjList.at(from);
            for (const auto& edge : edges)
            {
                if (edge.to == to) return true;
            }
            return false;
        }

        /**
         * @brief 获取边的权重（仅适用于有权图）
         * @param from 起点节点
         * @param to 终点节点
         * @return 边的权重值
         * @throws std::invalid_argument 若节点不存在或边不存在
         */
        template <bool W = IsWeighted, typename = std::enable_if_t<W>>
        WeightType weight(NodeType from, NodeType to) const
        {
            if (adjList.find(from) == adjList.end())
            {
                throw std::invalid_argument("Node does not exist: from");
            }

            const auto& edges = adjList.at(from);
            for (const auto& edge : edges)
            {
                if (edge.to == to) return edge.weight;
            }
            throw std::invalid_argument("Edge does not exist");
        }

        /**
         * @brief 获取节点的所有邻居边（包含目标节点及权重信息）
         * @param node 要查询的节点
         * @return 邻居边的常量引用（若节点不存在，返回空vector）
         * @note 每条边包含目标节点（to），有权图还包含权重（weight）
         */
        const std::vector<Edge<NodeType, IsWeighted, WeightType>>& neighbors(NodeType node) const
        {
            static const std::vector<Edge<NodeType, IsWeighted, WeightType>> emptyEdges;
            auto it = adjList.find(node);
            if (it == adjList.end())
            {
                return emptyEdges; // 返回静态空vector
            }
            return it->second;
        }

        /**
         * @brief 获取图中节点的总数
         * @return 节点数量（size_t类型，>=0）
         */
        size_t size() const
        {
            return nodeCount;
        }

        /**
         * @brief 打印图的邻接表结构（调试用）
         * @note 输出格式为 "节点 -> 邻居1(权重) 邻居2(权重) ..."，无权图不显示权重
         */
        void print() const
        {
            for (const auto& [node, edges] : adjList)
            {
                std::cout << node << " -> ";
                for (const auto& edge : edges)
                {
                    std::cout << edge.to;
                    if constexpr (IsWeighted)
                    {
                        std::cout << "(" << edge.weight << ")";
                    }
                    std::cout << " ";
                }
                std::cout << "\n";
            }
        }
    };

} // namespace ol

#endif // !OL_GRAPH_H