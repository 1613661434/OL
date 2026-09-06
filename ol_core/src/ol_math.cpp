#include "ol_math.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>    // 用于numeric_limits
#include <stdexcept> // 添加异常处理支持

using namespace std;

namespace ol
{
    namespace
    {
        void validateIterationArguments(double (*func)(double), double initial_value,
                                        double tolerance, size_t max_iterations)
        {
            if (func == nullptr) throw invalid_argument("Function pointer must not be null.");
            if (!isfinite(initial_value)) throw invalid_argument("Initial value must be finite.");
            if (!isfinite(tolerance) || tolerance <= 0)
                throw invalid_argument("Tolerance must be finite and greater than zero.");
            if (max_iterations == 0)
                throw invalid_argument("Maximum iterations must be greater than zero.");
        }

        template <typename Derivative>
        double newtonImpl(double (*func)(double), Derivative derivative, double initial_value,
                          double tolerance, size_t max_iterations)
        {
            constexpr double ZERO_DERIV_THRESHOLD = 1e-12;
            double current = initial_value;

            for (size_t i = 0; i < max_iterations; ++i)
            {
                const double value = func(current);
                if (!isfinite(value))
                    throw runtime_error("Function returned a non-finite value during Newton iteration.");
                if (value == 0) return current;

                const double derivative_value = derivative(current);
                if (!isfinite(derivative_value))
                    throw runtime_error("Derivative returned a non-finite value during Newton iteration.");
                if (fabs(derivative_value) < ZERO_DERIV_THRESHOLD)
                    throw runtime_error("Zero derivative encountered.");

                const double next = current - value / derivative_value;
                if (!isfinite(next))
                    throw runtime_error("Newton iteration produced a non-finite value.");
                if (fabs(next - current) < tolerance) return next;

                current = next;
            }

            throw runtime_error("Exceeded maximum iterations without convergence.");
        }
    } // namespace

    // 数值计算-非线性方程求解法-二分迭代法-收敛阶P=1
    // Bisection_Method(回调函数,左区间,右区间,误差限,最大迭代次数|默认1000)
    double Bisection_Method(double (*func)(double), double low, double high, double tolerance, const size_t max_iterations)
    {
        if (func == nullptr) throw invalid_argument("Function pointer must not be null.");
        if (!isfinite(low) || !isfinite(high) || low >= high)
            throw invalid_argument("Bisection interval must contain two finite values with low < high.");
        if (!isfinite(tolerance) || tolerance <= 0)
            throw invalid_argument("Tolerance must be finite and greater than zero.");
        if (max_iterations == 0)
            throw invalid_argument("Maximum iterations must be greater than zero.");

        double f_low = func(low);
        const double f_high = func(high);
        if (!isfinite(f_low) || !isfinite(f_high))
            throw invalid_argument("Function values at interval endpoints must be finite.");
        if (f_low == 0) return low;
        if (f_high == 0) return high;
        if (signbit(f_low) == signbit(f_high))
            throw invalid_argument("Function has same sign at both endpoints. Bisection requires a sign change in initial interval.");

        for (size_t i = 0; i < max_iterations; ++i)
        {
            // 分别除以2可避免high-low在极端有限值区间发生溢出。
            const double mid = low / 2 + high / 2;
            const double half_width = high / 2 - low / 2;
            const double f_mid = func(mid);

            if (!isfinite(f_mid))
                throw runtime_error("Function returned a non-finite value during bisection.");
            if (f_mid == 0 || half_width <= tolerance || mid == low || mid == high)
                return mid;

            if (signbit(f_mid) == signbit(f_low))
            {
                low = mid;
                f_low = f_mid; // 更新 f_low，避免重复计算
            }
            else
            {
                high = mid;
            }
        }

        throw runtime_error("Exceeded maximum iterations without convergence.");
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
        validateIterationArguments(func, initial_value, tolerance, max_iterations);
        if (der_func == nullptr) throw invalid_argument("Derivative function pointer must not be null.");

        return newtonImpl(func, der_func, initial_value, tolerance, max_iterations);
    }

    // 数值导数版牛顿迭代法：使用中心差分逼近f'(x)
    double Newton_Method(double (*func)(double), double initial_value, double tolerance, const size_t max_iterations)
    {
        validateIterationArguments(func, initial_value, tolerance, max_iterations);

        const auto numerical_derivative = [func](double x)
        {
            // 中心差分的理论合适步长约为机器精度的立方根。
            const double step = cbrt(numeric_limits<double>::epsilon()) * max(1.0, fabs(x));
            const double left = x - step;
            const double right = x + step;
            const double denominator = right - left;

            if (!isfinite(left) || !isfinite(right) || denominator == 0)
                throw runtime_error("Unable to choose a finite-difference step for numerical derivative.");

            const double f_left = func(left);
            const double f_right = func(right);
            if (!isfinite(f_left) || !isfinite(f_right))
                throw runtime_error("Function returned a non-finite value while estimating derivative.");

            return (f_right - f_left) / denominator;
        };

        return newtonImpl(func, numerical_derivative, initial_value, tolerance, max_iterations);
    }

    // 数值计算-非线性方程求解法-弦截迭代法-收敛阶P=1.618
    // Secant_Method(回调函数,初值0,初值1,误差限,最大迭代次数|默认1000,是否为定端点弦截法|固定点0|默认变端点)
    double Secant_Method(double (*func)(double), double initial_value_0, double initial_value_1, double tolerance, const size_t max_iterations, bool isFixedPoint_0)
    {
        constexpr double ZERO_DERIV_THRESHOLD = 1e-12; // 零判断阈值

        auto loop_func = [&func, ZERO_DERIV_THRESHOLD](double x0, double x1) -> double
        {
            double f_x1 = func(x1);
            double res = f_x1 - func(x0);

            if (fabs(res) < ZERO_DERIV_THRESHOLD) throw runtime_error("Zero derivative encountered.");

            return x1 - f_x1 * (x1 - x0) / res;
        };

        double Xi_minus_1 = initial_value_0; // Xi-1
        double Xi;                           // Xi
        double Xi_plus_1 = initial_value_1;  // Xi+1

        for (size_t i = 0; i < max_iterations; ++i)
        {
            Xi = Xi_plus_1;
            Xi_plus_1 = loop_func(Xi_minus_1, Xi);

            if (fabs(Xi_plus_1 - Xi) < tolerance) return Xi_plus_1;
            if (!isFixedPoint_0) Xi_minus_1 = Xi;
        }

        throw runtime_error("Exceeded maximum iterations without convergence.");
    }

} // namespace ol
