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

#ifndef __OL_GRAPH_H
#define __OL_GRAPH_H 1

#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace ol
{
    // 边结构定义（根据节点类型、权重开关和权重类型动态调整）
    // 模板参数说明：
    //- NodeType：节点数据类型（默认 int）
    //- IsWeighted：是否有权重（默认 false）
    //- WeightType：权重数据类型（默认 int，仅 IsWeighted=true 时有效）
    template <typename NodeType, bool IsWeighted, typename WeightType>
    struct Edge
    {
        NodeType to; // 目标节点（类型由NodeType控制）

        // 仅当有权重时才定义weight成员（类型由WeightType控制）
        typename std::conditional<IsWeighted, WeightType, std::nullptr_t>::type weight;

        // 无权图构造函数（仅需目标节点）
        template <bool W = IsWeighted, typename = std::enable_if_t<!W>>
        Edge(NodeType to_node) : to(to_node), weight(nullptr)
        {
        }

        // 有权图构造函数（需目标节点和权重）
        template <bool W = IsWeighted, typename = std::enable_if_t<W>>
        Edge(NodeType to_node, WeightType w) : to(to_node), weight(w)
        {
        }
    };

    // 通用图模板类
    // 模板参数说明：
    //- IsDirected：是否有向（默认 false）
    //- IsWeighted：是否有权重（默认 false）
    //- NodeType：节点数据类型（默认 int）
    //- WeightType：权重数据类型（默认 int，仅 IsWeighted=true 时有效）
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

        // 添加节点
        void addNode(NodeType node)
        {
            if (adjList.find(node) == adjList.end())
            {
                adjList[node] = std::vector<Edge<NodeType, IsWeighted, WeightType>>();
                ++nodeCount;
            }
        }

        // 添加边
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

        // 删除边
        void removeEdge(NodeType from, NodeType to)
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

        // 判断边是否存在
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

        // 获取边的权重（仅有权图可用，注意：找不到会抛异常！！！）
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

        // 获取节点的所有邻居（不存在则返回空vector）
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

        // 获取节点总数
        size_t size() const
        {
            return nodeCount;
        }

        // 打印图结构
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

#endif // !__OL_GRAPH_H