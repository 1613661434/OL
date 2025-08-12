#include "../include/ol_math.h"
#include <cmath>
#include <iostream>
#include <limits>    // 用于numeric_limits
#include <stdexcept> // 添加异常处理支持

using namespace std;

namespace ol
{
    // 数值计算-非线性方程求解法-二分迭代法-收敛阶P=1
    // Bisection_Method(回调函数,左区间,右区间,误差限,最大迭代次数|默认1000)
    double Bisection_Method(double (*func)(double), double low, double high, double tolerance, const size_t max_iterations)
    {
        double f_low = func(low);
        double f_high = func(high);

        if (f_low * f_high > 0)
        {
            throw invalid_argument("Function has same sign at both endpoints. Bisection requires a sign change in initial interval.");
        }

        size_t loop_count = ceil(log2((high - low) / tolerance) - 1); // 事前估计：二分法好处可以在计算之前知道迭代次数
        if (loop_count > max_iterations)
        {
            throw runtime_error("Exceeded maximum iterations without convergence.");
        }

        double mid;
        do
        {
            mid = low + (high - low) / 2;

            double f_mid = func(mid);

            if (f_mid * f_low > 0)
            {
                low = mid;
                f_low = f_mid; // 更新 f_low，避免重复计算
            }
            else
            {
                high = mid;
            }
        } while (--loop_count > 0);

        return mid;
    }

    // 数值计算-非线性方程求解法-简单迭代法-收敛阶P=1
    // Simple_Iteration_Method(回调迭代函数,初值,误差限,最大迭代次数|默认1000)
    double Simple_Iteration_Method(double (*iter_func)(double), double initial_value, double tolerance, const size_t max_iterations)
    {
        double Xi = initial_value; // Xi
        double Xi_plus_1;          // Xi+1
        for (size_t i = 0; i < max_iterations; ++i)
        {
            Xi_plus_1 = iter_func(Xi);
            if (fabs(Xi_plus_1 - Xi) < tolerance) return Xi_plus_1;
            Xi = Xi_plus_1;
        }

        throw runtime_error("Exceeded maximum iterations without convergence.");
    }

    // 数值计算-非线性方程求解法-牛顿迭代法-收敛阶P=2
    // Newton_Method(回调函数,回调导函数,初值,误差限,最大迭代次数|默认1000)
    double Newton_Method(double (*func)(double), double (*der_func)(double), double initial_value, double tolerance, const size_t max_iterations)
    {
        constexpr double ZERO_DERIV_THRESHOLD = 1e-12; // 零判断阈值

        auto loop_func = [&func, &der_func](double x) -> double
        {
            double df = der_func(x);
            if (fabs(df) < ZERO_DERIV_THRESHOLD) throw runtime_error("Zero derivative encountered.");
            return x - func(x) / df;
        };

        double Xi;                        // Xi
        double Xi_plus_1 = initial_value; // Xi+1
        size_t iter = 0;                  // 迭代次数

        do
        {
            Xi = Xi_plus_1;
            Xi_plus_1 = loop_func(Xi_plus_1);

            if (fabs(Xi_plus_1 - Xi) < tolerance) return Xi_plus_1;
        } while (++iter < max_iterations);

        throw runtime_error("Exceeded maximum iterations without convergence.");
    }

    // 数值计算-非线性方程求解法-弦截迭代法-收敛阶P=1.618
    // Secant_Method(回调函数,初值0,初值1,误差限,最大迭代次数|默认1000,是否为定端点弦截法|固定点0|默认变端点)
    double Secant_Method(double (*func)(double), double initial_value0, double initial_value1, double tolerance, const size_t max_iterations, bool isFixedPoint0)
    {
        constexpr double ZERO_DERIV_THRESHOLD = 1e-12; // 零判断阈值

        auto loop_func = [&func](double x0, double x1) -> double
        {
            double f_x1 = func(x1);
            double res = f_x1 - func(x0);

            if (fabs(res) < ZERO_DERIV_THRESHOLD) throw runtime_error("Zero derivative encountered.");

            return x1 - f_x1 * (x1 - x0) / res;
        };

        double Xi_minus_1 = initial_value0; // Xi-1
        double Xi;                          // Xi
        double Xi_plus_1 = initial_value1;  // Xi+1

        for (size_t i = 0; i < max_iterations; ++i)
        {
            Xi = Xi_plus_1;
            Xi_plus_1 = loop_func(Xi_minus_1, Xi);

            if (fabs(Xi_plus_1 - Xi) < tolerance) return Xi_plus_1;
            if (!isFixedPoint0) Xi_minus_1 = Xi;
        }

        throw runtime_error("Exceeded maximum iterations without convergence.");
    }

} // namespace ol