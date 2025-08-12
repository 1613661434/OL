#ifndef __OL_MATH_H
#define __OL_MATH_H 1

#include <cstddef>

namespace ol
{

    // 数值计算-非线性方程求解法-二分迭代法-收敛阶P=1
    // Bisection_Method(回调函数,左区间,右区间,误差限,最大迭代次数|默认1000)
    double Bisection_Method(double (*func)(double), double low, double high, double tolerance, const size_t max_iterations = 1000);

    // 数值计算-非线性方程求解法-简单迭代法-收敛阶P=1
    // Simple_Iteration_Method(回调迭代函数,初值,误差限,最大迭代次数|默认1000)
    double Simple_Iteration_Method(double (*iter_func)(double), double initial_value, double tolerance, const size_t max_iterations = 1000);

    // 数值计算-非线性方程求解法-牛顿迭代法-收敛阶P=2
    // Newton_Method(回调函数,回调导函数,初值,误差限,最大迭代次数|默认1000)
    double Newton_Method(double (*func)(double), double (*der_func)(double), double initial_value, double tolerance, const size_t max_iterations = 1000);

    // 数值计算-非线性方程求解法-弦截迭代法-收敛阶P=1.618
    // Secant_Method(回调函数,初值0,初值1,误差限,最大迭代次数|默认1000,是否为定端点弦截法|固定点0|默认变端点)
    double Secant_Method(double (*func)(double), double initial_value0, double initial_value1, double tolerance, const size_t max_iterations = 1000, bool isFixedPoint0 = false);

} // namespace ol

#endif // !__OL_MATH_H