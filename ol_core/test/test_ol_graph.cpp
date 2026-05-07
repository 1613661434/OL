#include "ol_graph.h"

using namespace ol;
using namespace std;

// 自定义枚举类型作为节点
enum class CustomNode
{
    A,
    B,
    C,
    D,
    E
};

// 为自定义枚举重载输出运算符
std::ostream& operator<<(std::ostream& os, const CustomNode& node)
{
    switch (node)
    {
    case CustomNode::A:
        return os << "A";
    case CustomNode::B:
        return os << "B";
    case CustomNode::C:
        return os << "C";
    case CustomNode::D:
        return os << "D";
    case CustomNode::E:
        return os << "E";
    default:
        return os << "Unknown";
    }
}

// 完整测试用例 - 覆盖所有成员函数
int main()
{
    // 1. 测试有向加权图（string节点，double权重）
    std::cout << "=== 测试1: 有向加权图（string节点，double权重） ===" << std::endl;
    Graph<true, true, std::string, double> strNodeGraph;

    // 测试addNode和addEdge
    strNodeGraph.addNode("Start"); // 单独添加节点
    strNodeGraph.addEdge("A", "B", 3.14);
    strNodeGraph.addEdge("B", "C", 2.718);
    strNodeGraph.addEdge("C", "A", 1.618);
    strNodeGraph.addEdge("A", "C", 0.577);

    // 测试print
    std::cout << "图结构:" << std::endl;
    strNodeGraph.print();

    // 测试size
    std::cout << "节点总数: " << strNodeGraph.size() << std::endl;

    // 测试hasEdge
    std::cout << "A到B是否有边: " << (strNodeGraph.hasEdge("A", "B") ? "是" : "否") << std::endl;
    std::cout << "B到A是否有边: " << (strNodeGraph.hasEdge("B", "A") ? "是" : "否") << std::endl;

    // 测试weight
    std::cout << "A->B的权重: " << strNodeGraph.weight("A", "B") << std::endl;

    // 测试neighbors
    std::cout << "A的邻居: ";
    for (const auto& edge : strNodeGraph.neighbors("A"))
    {
        std::cout << edge.to << "(" << edge.weight << ") ";
    }
    std::cout << "\n"
              << std::endl;

    // 2. 测试无向加权图（int节点，int权重）
    std::cout << "=== 测试2: 无向加权图（int节点，int权重） ===" << std::endl;
    Graph<false, true> intNodeGraph; // 使用默认类型

    // 测试addEdge（自动添加节点）
    intNodeGraph.addEdge(0, 1, 5);
    intNodeGraph.addEdge(1, 2, 3);
    intNodeGraph.addEdge(2, 0, 2);

    // 测试print
    std::cout << "图结构:" << std::endl;
    intNodeGraph.print();

    // 测试rmEdge
    std::cout << "删除边1-2后:" << std::endl;
    intNodeGraph.rmEdge(1, 2);
    intNodeGraph.print();

    // 测试hasEdge和weight
    std::cout << "1到2是否有边: " << (intNodeGraph.hasEdge(1, 2) ? "是" : "否") << std::endl;
    std::cout << "0到2的权重: " << intNodeGraph.weight(0, 2) << std::endl;

    // 测试size
    std::cout << "节点总数: " << intNodeGraph.size() << "\n"
              << std::endl;

    // 3. 测试无向无权图（自定义枚举节点）
    std::cout << "=== 测试3: 无向无权图（枚举节点） ===" << std::endl;
    Graph<false, false, CustomNode> enumNodeGraph;

    // 添加边
    enumNodeGraph.addEdge(CustomNode::A, CustomNode::B);
    enumNodeGraph.addEdge(CustomNode::B, CustomNode::C);
    enumNodeGraph.addEdge(CustomNode::C, CustomNode::D);
    enumNodeGraph.addEdge(CustomNode::D, CustomNode::A);

    // 测试print
    std::cout << "图结构:" << std::endl;
    enumNodeGraph.print();

    // 测试neighbors
    std::cout << "B的邻居: ";
    for (const auto& edge : enumNodeGraph.neighbors(CustomNode::B))
    {
        std::cout << edge.to << " ";
    }
    std::cout << std::endl;

    // 测试rmEdge
    enumNodeGraph.rmEdge(CustomNode::A, CustomNode::B);
    std::cout << "删除边A-B后B的邻居: ";
    for (const auto& edge : enumNodeGraph.neighbors(CustomNode::B))
    {
        std::cout << edge.to << " ";
    }
    std::cout << std::endl;

    // 测试hasEdge
    std::cout << "A到B是否有边: " << (enumNodeGraph.hasEdge(CustomNode::A, CustomNode::B) ? "是" : "否") << std::endl;

    // 测试size
    std::cout << "节点总数: " << enumNodeGraph.size() << std::endl;

    // 测试异常处理
    std::cout << "\n=== 测试4: 异常处理 ===" << std::endl;
    try
    {
        // 尝试获取不存在的边的权重
        intNodeGraph.weight(7, 8);
    }
    catch (const std::invalid_argument& e)
    {
        std::cout << "捕获异常: " << e.what() << std::endl;
    }

    // 尝试获取不存在的节点的邻居
    auto v = enumNodeGraph.neighbors(CustomNode::E);
    if (v.empty())
    {
        std::cout << "没有邻居\n";
    }

    return 0;
}