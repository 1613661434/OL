#include "ol_math.h"
#include <cmath>
#include <cstddef>
#include <iostream>

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

int main()
{
    const double tolerance = 1e-6;
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
    }
    catch (const exception& e)
    {
        cerr << "[Error] " << e.what() << endl;
    }

    // ##Simple_Iteration_Method 弦截迭代法
    cout << "\n----------- 简单迭代法 -----------\n";
    try
    {
        const double root = Simple_Iteration_Method(iter_func, 1.0, tolerance);
        cout << "Approximate root: " << root << endl;
    }
    catch (const exception& e)
    {
        cerr << "[Error] " << e.what() << endl;
    }

    // ##Newton_Method 牛顿迭代法
    cout << "\n----------- 牛顿迭代法 -----------\n";
    try
    {
        const double root = Newton_Method(function, der_function, 1.0, tolerance);
        cout << "Approximate root: " << root << endl;
    }
    catch (const exception& e)
    {
        cerr << "[Error] " << e.what() << endl;
    }

    // ##Secant_Method 弦截迭代法
    cout << "\n----------- 弦截迭代法 -----------\n";
    try
    {
        const double root = Secant_Method(function, 1.0, 2.0, tolerance);
        cout << "Approximate root: " << root << endl;
    }
    catch (const exception& e)
    {
        cerr << "[Error] " << e.what() << endl;
    }

    return 0;
}