/****************************************************************************************/
/*
 * 程序名：test_ol_InetAddr.cpp
 * 功能描述：测试ol::InetAddr类的功能完整性，包括：
 *          - IPv4/IPv6地址的构造与解析
 *          - IP地址和端口的获取
 *          - 地址类型判断（IPv4/IPv6）
 *          - 格式化输出（IP:端口）
 *          - 异常处理（无效地址等场景）
 * 作者：ol
 * 适用标准：C++11及以上
 */
/****************************************************************************************/

#if !defined(__linux__)
#error "仅支持Linux平台，不支持当前系统！"
#endif

#include "ol_InetAddr.h"
#include <cassert>
#include <iostream>

using namespace ol;
using namespace std;

// 测试IPv4地址构造与解析
void test_ipv4()
{
    cout << "=== 测试IPv4地址 ===" << "\n";

    // 测试普通构造
    InetAddr addr("192.168.1.1", 8080);
    assert(addr.isIpv4() == true);
    assert(addr.isIpv6() == false);
    assert(strcmp(addr.ip(), "192.168.1.1") == 0);
    assert(addr.port() == 8080);
    assert(addr.toString() == "192.168.1.1:8080");
    cout << "普通IPv4构造: " << addr.toString() << " [OK]" << "\n";

    // 测试"所有接口"构造
    InetAddr any4(80, false);
    assert(any4.isIpv4() == true);
    assert(strcmp(any4.ip(), "0.0.0.0") == 0); // INADDR_ANY对应0.0.0.0
    assert(any4.port() == 80);
    cout << "IPv4任意地址: " << any4.toString() << " [OK]" << "\n";

    // 测试原生sockaddr构造
    sockaddr_in native4;
    memset(&native4, 0, sizeof(native4));
    native4.sin_family = AF_INET;
    native4.sin_addr.s_addr = inet_addr("10.0.0.1");
    native4.sin_port = htons(9000);
    InetAddr fromNative4(reinterpret_cast<sockaddr*>(&native4), sizeof(native4));
    assert(strcmp(fromNative4.ip(), "10.0.0.1") == 0);
    assert(fromNative4.port() == 9000);
    cout << "原生IPv4转换: " << fromNative4.toString() << " [OK]" << "\n";

    cout << "IPv4测试通过" << "\n"
         << "\n";
}

// 测试IPv6地址构造与解析
void test_ipv6()
{
    cout << "=== 测试IPv6地址 ===" << "\n";

    // 测试普通构造
    InetAddr addr("2001:db8::1", 8080);
    assert(addr.isIpv6() == true);
    assert(addr.isIpv4() == false);
    assert(strcmp(addr.ip(), "2001:db8::1") == 0);
    assert(addr.port() == 8080);
    assert(addr.toString() == "[2001:db8::1]:8080");
    cout << "普通IPv6构造: " << addr.toString() << " [OK]" << "\n";

    // 测试"所有接口"构造
    InetAddr any6(80, true);
    assert(any6.isIpv6() == true);
    assert(strcmp(any6.ip(), "::") == 0); // in6addr_any对应::
    assert(any6.port() == 80);
    cout << "IPv6任意地址: " << any6.toString() << " [OK]" << "\n";

    // 测试原生sockaddr构造
    sockaddr_in6 native6;
    memset(&native6, 0, sizeof(native6));
    native6.sin6_family = AF_INET6;
    inet_pton(AF_INET6, "fe80::1", &native6.sin6_addr);
    native6.sin6_port = htons(9000);
    InetAddr fromNative6(reinterpret_cast<sockaddr*>(&native6), sizeof(native6));
    assert(strcmp(fromNative6.ip(), "fe80::1") == 0);
    assert(fromNative6.port() == 9000);
    cout << "原生IPv6转换: " << fromNative6.toString() << " [OK]" << "\n";

    cout << "IPv6测试通过" << "\n"
         << "\n";
}

// 测试异常场景
void test_exceptions()
{
    cout << "=== 测试异常场景 ===" << "\n";

    // 测试无效IP地址
    bool invalidIpCaught = false;
    try
    {
        InetAddr invalid("256.256.256.256", 80); // 无效IPv4
    }
    catch (const invalid_argument& e)
    {
        invalidIpCaught = true;
        cout << "捕获预期异常: " << e.what() << " [OK]" << "\n";
    }
    assert(invalidIpCaught == true);

    // 测试过大的地址长度
    bool largeAddrCaught = false;
    sockaddr_in native;
    memset(&native, 0, sizeof(native));
    try
    {
        InetAddr tooLarge(reinterpret_cast<sockaddr*>(&native), sizeof(sockaddr_storage) + 1); // 长度超限
    }
    catch (const invalid_argument& e)
    {
        largeAddrCaught = true;
        cout << "捕获预期异常: " << e.what() << " [OK]" << "\n";
    }
    assert(largeAddrCaught == true);

    cout << "异常测试通过" << "\n"
         << "\n";
}

// 测试复制构造和赋值
void test_copy()
{
    cout << "=== 测试复制功能 ===" << "\n";

    InetAddr original("192.168.1.100", 12345);

    // 测试复制构造
    InetAddr copyCtor(original);
    assert(strcmp(copyCtor.ip(), "192.168.1.100") == 0);
    assert(copyCtor.port() == 12345);

    // 测试赋值运算符
    InetAddr copyAssign;
    copyAssign = original;
    assert(strcmp(copyAssign.ip(), "192.168.1.100") == 0);
    assert(copyAssign.port() == 12345);

    cout << "复制功能测试通过" << "\n"
         << "\n";
}

// 测试修改IP和端口的功能
void test_modify()
{
    cout << "=== 测试修改IP和端口功能 ===" << "\n";

    // 测试IPv4修改
    InetAddr addr4("192.168.1.1", 8080);
    addr4.setIp("10.0.0.1"); // 修改IP
    assert(strcmp(addr4.ip(), "10.0.0.1") == 0);
    assert(addr4.port() == 8080); // 端口应保持不变

    addr4.setPort(9090); // 修改端口
    assert(addr4.port() == 9090);
    assert(strcmp(addr4.ip(), "10.0.0.1") == 0); // IP应保持不变

    addr4.setAddr("172.16.0.1", 7070); // 同时修改IP和端口
    assert(strcmp(addr4.ip(), "172.16.0.1") == 0);
    assert(addr4.port() == 7070);
    cout << "IPv4修改测试: " << addr4.toString() << " [OK]" << "\n";

    // 测试IPv6修改
    InetAddr addr6("2001:db8::1", 8080);
    addr6.setIp("fe80::1"); // 修改IP
    assert(strcmp(addr6.ip(), "fe80::1") == 0);
    assert(addr6.port() == 8080);

    addr6.setPort(9090); // 修改端口
    assert(addr6.port() == 9090);
    assert(strcmp(addr6.ip(), "fe80::1") == 0);

    addr6.setAddr("::1", 7070); // 同时修改IP和端口
    assert(strcmp(addr6.ip(), "::1") == 0);
    assert(addr6.port() == 7070);
    cout << "IPv6修改测试: " << addr6.toString() << " [OK]" << "\n";

    // 测试修改异常（IPv4地址赋给IPv6对象）
    bool typeMismatchCaught = false;
    try
    {
        addr6.setIp("192.168.1.1"); // IPv6对象不能设置IPv4地址
    }
    catch (const invalid_argument& e)
    {
        typeMismatchCaught = true;
        cout << "捕获预期异常: " << e.what() << " [OK]" << "\n";
    }
    assert(typeMismatchCaught == true);

    cout << "修改功能测试通过" << "\n"
         << "\n";
}

int main()
{
    try
    {
        test_ipv4();
        test_ipv6();
        test_exceptions();
        test_copy();
        test_modify();
        cout << "所有测试通过！" << "\n";
    }
    catch (const exception& e)
    {
        cerr << "测试失败: " << e.what() << "\n";
        return 1;
    }
    return 0;
}