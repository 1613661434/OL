/****************************************************************************************/
/*
 * 程序名：test_ol_InetAddr.cpp
 * 功能描述：测试ol::InetAddr类的功能完整性
 */
/****************************************************************************************/

#if !defined(__linux__)
#error "仅支持Linux平台，不支持当前系统！"
#endif

#include "ol_InetAddr.h"
#include <arpa/inet.h>
#include <cassert>
#include <cstring>
#include <iostream>

using namespace ol;
using namespace std;

// 测试基础构造与解析功能（IPv4/IPv6）
void test_basic_construction()
{
    cout << "=== 测试基础构造与解析 ===" << "\n";

    // IPv4普通地址测试
    InetAddr ipv4("192.168.1.1", 8080);
    assert(ipv4.isIpv4() == true);
    assert(ipv4.isIpv6() == false);
    assert(strcmp(ipv4.getIp(), "192.168.1.1") == 0);
    assert(ipv4.getPort() == 8080);
    assert(ipv4.getAddrStr() == "192.168.1.1:8080");
    assert(ipv4.getFamily() == AF_INET);
    cout << "IPv4普通地址: " << ipv4.getAddrStr() << " [OK]" << "\n";

    // IPv4任意地址（INADDR_ANY）测试
    InetAddr ipv4_any(80, false);
    assert(strcmp(ipv4_any.getIp(), "0.0.0.0") == 0);
    assert(ipv4_any.getPort() == 80);
    cout << "IPv4任意地址: " << ipv4_any.getAddrStr() << " [OK]" << "\n";

    // IPv6普通地址测试
    InetAddr ipv6("2001:db8::1", 9090);
    assert(ipv6.isIpv6() == true);
    assert(strcmp(ipv6.getIp(), "2001:db8::1") == 0);
    assert(ipv6.getPort() == 9090);
    assert(ipv6.getAddrStr() == "[2001:db8::1]:9090");
    assert(ipv6.getFamily() == AF_INET6);
    cout << "IPv6普通地址: " << ipv6.getAddrStr() << " [OK]" << "\n";

    // IPv6任意地址（in6addr_any）测试
    InetAddr ipv6_any(443, true);
    assert(strcmp(ipv6_any.getIp(), "::") == 0);
    assert(ipv6_any.getPort() == 443);
    cout << "IPv6任意地址: " << ipv6_any.getAddrStr() << " [OK]" << "\n";

    cout << "基础构造与解析测试通过" << "\n"
         << "\n";
}

// 测试原生sockaddr转换功能
void test_native_address_conversion()
{
    cout << "=== 测试原生地址转换 ===" << "\n";

    // 从sockaddr_in构造IPv4地址
    sockaddr_in native_ipv4;
    memset(&native_ipv4, 0, sizeof(native_ipv4));
    native_ipv4.sin_family = AF_INET;
    native_ipv4.sin_port = htons(12345);
    inet_pton(AF_INET, "10.0.0.1", &native_ipv4.sin_addr);

    InetAddr from_native4(reinterpret_cast<sockaddr*>(&native_ipv4), sizeof(native_ipv4));
    assert(strcmp(from_native4.getIp(), "10.0.0.1") == 0);
    assert(from_native4.getPort() == 12345);
    assert(from_native4.getAddrLen() == sizeof(native_ipv4)); // 验证IPv4地址长度
    cout << "从sockaddr_in转换: " << from_native4.getAddrStr() << " [OK]" << "\n";

    // 从sockaddr_in6构造IPv6地址
    sockaddr_in6 native_ipv6;
    memset(&native_ipv6, 0, sizeof(native_ipv6));
    native_ipv6.sin6_family = AF_INET6;
    native_ipv6.sin6_port = htons(54321);
    inet_pton(AF_INET6, "fe80::1", &native_ipv6.sin6_addr);

    InetAddr from_native6(reinterpret_cast<sockaddr*>(&native_ipv6), sizeof(native_ipv6));
    assert(strcmp(from_native6.getIp(), "fe80::1") == 0);
    assert(from_native6.getPort() == 54321);
    assert(from_native6.getAddrLen() == sizeof(native_ipv6)); // 验证IPv6地址长度
    cout << "从sockaddr_in6转换: " << from_native6.getAddrStr() << " [OK]" << "\n";

    // 测试getAddr()返回的原生地址有效性
    const sockaddr* addr_ptr = from_native4.getAddr();
    assert(addr_ptr->sa_family == AF_INET);
    assert(reinterpret_cast<const sockaddr_in*>(addr_ptr)->sin_port == htons(12345));
    cout << "原生地址指针有效性: 验证通过 [OK]" << "\n";

    cout << "原生地址转换测试通过" << "\n"
         << "\n";
}

// 测试缓存机制（核心优化点）
void test_ip_cache_mechanism()
{
    cout << "=== 测试IP缓存机制 ===" << "\n";

    InetAddr addr("172.16.0.1", 5000);

    // 首次调用getIp()：缓存未初始化，应触发转换
    const char* first_call = addr.getIp();
    assert(strcmp(first_call, "172.16.0.1") == 0);

    // 记录缓存内容（用于后续比较）
    string original_cache = first_call;

    // 二次调用getIp()：应直接返回缓存（内容不变）
    const char* second_call = addr.getIp();
    assert(strcmp(second_call, original_cache.c_str()) == 0); // 内容相同

    // 修改IP后：缓存应失效并重新计算
    addr.setIp("172.16.0.2");
    const char* after_modify = addr.getIp();

    // 验证1：新内容与旧内容不同（缓存已更新）
    assert(strcmp(after_modify, original_cache.c_str()) != 0);
    // 验证2：新内容正确（确实是修改后的IP）
    assert(strcmp(after_modify, "172.16.0.2") == 0);

    // 测试复制操作对缓存的影响（复制后缓存内容一致，但缓冲区独立）
    InetAddr copy = addr;
    assert(strcmp(copy.getIp(), after_modify) == 0); // 内容相同

    cout << "IP缓存机制测试通过" << "\n"
         << "\n";
}

// 测试复制与修改功能
void test_copy_and_modification()
{
    cout << "=== 测试复制与修改功能 ===" << "\n";

    // 测试复制构造
    InetAddr original("192.168.2.1", 8888);
    InetAddr copy_ctor(original);
    assert(strcmp(copy_ctor.getIp(), "192.168.2.1") == 0);
    assert(copy_ctor.getPort() == 8888);
    assert(copy_ctor.getFamily() == original.getFamily());

    // 测试赋值运算符
    InetAddr copy_assign;
    copy_assign = original;
    assert(strcmp(copy_assign.getIp(), "192.168.2.1") == 0);
    assert(copy_assign.getPort() == 8888);

    // 测试独立修改（修改副本不影响原对象）
    copy_assign.setIp("192.168.2.2");
    copy_assign.setPort(9999);
    assert(strcmp(original.getIp(), "192.168.2.1") == 0); // 原对象不变
    assert(original.getPort() == 8888);

    // 测试setAddr()批量修改
    InetAddr modify_test("10.1.1.1", 1000);
    modify_test.setAddr("10.1.1.2", 2000);
    assert(strcmp(modify_test.getIp(), "10.1.1.2") == 0);
    assert(modify_test.getPort() == 2000);

    // 测试通过原生sockaddr修改
    sockaddr_in new_addr;
    memset(&new_addr, 0, sizeof(new_addr));
    new_addr.sin_family = AF_INET;
    new_addr.sin_port = htons(3000);
    inet_pton(AF_INET, "10.1.1.3", &new_addr.sin_addr);
    modify_test.setAddr(reinterpret_cast<sockaddr*>(&new_addr), sizeof(new_addr));
    assert(strcmp(modify_test.getIp(), "10.1.1.3") == 0);
    assert(modify_test.getPort() == 3000);

    cout << "复制与修改功能测试通过" << "\n"
         << "\n";
}

// 测试异常场景处理
void test_exception_handling()
{
    cout << "=== 测试异常场景处理 ===" << "\n";

    // 无效IPv4地址（超出0-255范围）
    bool invalid_ipv4 = false;
    try
    {
        InetAddr("256.0.0.1", 80);
    }
    catch (const invalid_argument& e)
    {
        invalid_ipv4 = true;
        cout << "捕获预期异常: " << e.what() << " [OK]" << "\n";
    }
    assert(invalid_ipv4);

    // 无效IPv6地址（格式错误）
    bool invalid_ipv6 = false;
    try
    {
        InetAddr("2001:db8:gibberish::1", 80); // 'g'不是十六进制字符
    }
    catch (const invalid_argument& e)
    {
        invalid_ipv6 = true;
        cout << "捕获预期异常: " << e.what() << " [OK]" << "\n";
    }
    assert(invalid_ipv6);

    // 地址族不匹配（IPv4对象设置IPv6地址）
    bool family_mismatch = false;
    try
    {
        InetAddr ipv4("192.168.1.1", 80);
        ipv4.setIp("::1"); // 类型不匹配
    }
    catch (const invalid_argument& e)
    {
        family_mismatch = true;
        cout << "捕获预期异常: " << e.what() << " [OK]" << "\n";
    }
    assert(family_mismatch);

    // 原生地址长度超限（基于联合体最大成员sockaddr_in6的大小）
    bool addr_len_exceed = false;
    sockaddr_in dummy;
    memset(&dummy, 0, sizeof(dummy));
    try
    {
        // 传入的长度 > 最大地址结构体（sockaddr_in6）的大小
        socklen_t excessive_len = sizeof(sockaddr_in6) + 1;
        InetAddr(reinterpret_cast<sockaddr*>(&dummy), excessive_len);
    }
    catch (const invalid_argument& e)
    {
        addr_len_exceed = true;
        cout << "捕获预期异常: " << e.what() << " [OK]" << "\n";
    }
    assert(addr_len_exceed);

    cout << "异常场景处理测试通过" << "\n"
         << "\n";
}

int main()
{
    try
    {
        test_basic_construction();
        test_native_address_conversion();
        test_ip_cache_mechanism();
        test_copy_and_modification();
        test_exception_handling();

        cout << "=== 所有ol::InetAddr测试通过！ ===" << "\n";
        return 0;
    }
    catch (const exception& e)
    {
        cerr << "=== 测试失败: " << e.what() << " ===" << "\n";
        return 1;
    }
}