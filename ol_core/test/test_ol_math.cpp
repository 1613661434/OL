#include "ol_math.h"
#include <cmath>
#include <cstddef>
#include <iostream>
#include <stdexcept>

using namespace ol;
using namespace std;

// 原函数 f(x) = x^3 + 10x - 20
double function(double x)
{
    return pow(x, 3) + 10 * x - 20;
}

// 导函数 f'(x) = 3 * x^2 + 10
double der_function(double x)
{
    return 3 * pow(x, 2) + 10;
}

// 迭代函数 g(x)=(x^3 - 20) / (-10)
double iter_func(double x)
{
    return (pow(x, 3) - 20) / (-10);
}

double identity_function(double x)
{
    return x;
}

int main()
{
    const double tolerance = 1e-6;
    int failures = 0;

    const auto check_root = [&](const char* method, double root)
    {
        if (!isfinite(root) || fabs(function(root)) > tolerance * 10)
        {
            cerr << "[Error] " << method << " returned an invalid root: " << root << endl;
            ++failures;
        }
    };

    const auto expect_invalid_argument = [&](const char* test_name, const auto& operation)
    {
        try
        {
            operation();
            cerr << "[Error] " << test_name << " did not throw invalid_argument." << endl;
            ++failures;
        }
        catch (const invalid_argument&)
        {
        }
        catch (const exception& e)
        {
            cerr << "[Error] " << test_name << " threw an unexpected exception: "
                 << e.what() << endl;
            ++failures;
        }
    };

    cout << "求解方程 x^3 + 10x - 20 = 0 的根\n";
    cout << "原函数 f(x) = x^3 + 10x - 20\n";
    cout << "导函数 f'(x) = 3 * x^2 + 10\n";
    cout << "精确解 x* = 1.59456\n";
    cout << "误差限 e* = " << tolerance << "\n";

    // ##Bisection_Method 二分迭代法
    cout << "\n----------- 二分迭代法 -----------\n";
    try
    {
        const double root = Bisection_Method(function, 1.0, 2.0, tolerance);
        cout << "Approximate root: " << root << endl;
        check_root("Bisection_Method", root);
    }
    catch (const exception& e)
    {
        cerr << "[Error] " << e.what() << endl;
        ++failures;
    }

    const double endpoint_root = Bisection_Method(identity_function, 0.0, 1.0, tolerance);
    if (endpoint_root != 0)
    {
        cerr << "[Error] Bisection endpoint-root test returned: " << endpoint_root << endl;
        ++failures;
    }

    expect_invalid_argument("Bisection zero tolerance", [&]
                            { Bisection_Method(identity_function, -1.0, 1.0, 0.0); });
    expect_invalid_argument("Bisection reversed interval", [&]
                            { Bisection_Method(identity_function, 1.0, -1.0, tolerance); });

    bool iteration_limit_detected = false;
    try
    {
        Bisection_Method(function, 1.0, 2.0, 1e-15, 1);
    }
    catch (const runtime_error&)
    {
        iteration_limit_detected = true;
    }
    catch (const exception& e)
    {
        cerr << "[Error] Bisection iteration-limit test threw: " << e.what() << endl;
        ++failures;
    }
    if (!iteration_limit_detected)
    {
        cerr << "[Error] Bisection iteration-limit test did not report non-convergence." << endl;
        ++failures;
    }

    // 回归测试：旧实现会让size_t循环次数从0下溢并持续迭代。
    try
    {
        const double root = Bisection_Method(identity_function, -1.0, 1.0, 1.0, 1);
        if (root != 0)
        {
            cerr << "[Error] Bisection underflow regression returned: " << root << endl;
            ++failures;
        }
    }
    catch (const exception& e)
    {
        cerr << "[Error] Bisection underflow regression failed: " << e.what() << endl;
        ++failures;
    }

    expect_invalid_argument("Newton null derivative", [&]
                            { Newton_Method(function, nullptr, 1.0, tolerance); });

    // ##Simple_Iteration_Method 弦截迭代法
    cout << "\n----------- 简单迭代法 -----------\n";
    try
    {
        const double root = Simple_Iteration_Method(iter_func, 1.0, tolerance);
        cout << "Approximate root: " << root << endl;
        check_root("Simple_Iteration_Method", root);
    }
    catch (const exception& e)
    {
        cerr << "[Error] " << e.what() << endl;
        ++failures;
    }

    // ##Newton_Method 牛顿迭代法
    cout << "\n----------- 牛顿迭代法 -----------\n";
    try
    {
        const double root = Newton_Method(function, der_function, 1.0, tolerance);
        cout << "Approximate root: " << root << endl;
        check_root("Newton_Method with analytic derivative", root);
    }
    catch (const exception& e)
    {
        cerr << "[Error] " << e.what() << endl;
        ++failures;
    }

    // ##Newton_Method 数值导数版牛顿迭代法
    cout << "\n----------- 牛顿迭代法（数值导数） -----------\n";
    try
    {
        const double root = Newton_Method(function, 1.0, tolerance);
        cout << "Approximate root: " << root << endl;
        check_root("Newton_Method with numerical derivative", root);
    }
    catch (const exception& e)
    {
        cerr << "[Error] " << e.what() << endl;
        ++failures;
    }

    // ##Secant_Method 弦截迭代法
    cout << "\n----------- 弦截迭代法 -----------\n";
    try
    {
        const double root = Secant_Method(function, 1.0, 2.0, tolerance);
        cout << "Approximate root: " << root << endl;
        check_root("Secant_Method", root);
    }
    catch (const exception& e)
    {
        cerr << "[Error] " << e.what() << endl;
        ++failures;
    }

    if (failures != 0)
    {
        cerr << "\n共有 " << failures << " 项测试失败。\n";
        return 1;
    }

    cout << "\n所有数学方法测试通过。\n";
    return 0;
}
