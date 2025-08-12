#include "../include/ol_math.h"
#include <cmath>
#include <iostream>
#include <limits>    // ����numeric_limits
#include <stdexcept> // ����쳣����֧��

using namespace std;

namespace ol
{
    // ��ֵ����-�����Է�����ⷨ-���ֵ�����-������P=1
    // Bisection_Method(�ص�����,������,������,�����,����������|Ĭ��1000)
    double Bisection_Method(double (*func)(double), double low, double high, double tolerance, const size_t max_iterations)
    {
        double f_low = func(low);
        double f_high = func(high);

        if (f_low * f_high > 0)
        {
            throw invalid_argument("Function has same sign at both endpoints. Bisection requires a sign change in initial interval.");
        }

        size_t loop_count = ceil(log2((high - low) / tolerance) - 1); // ��ǰ���ƣ����ַ��ô������ڼ���֮ǰ֪����������
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
                f_low = f_mid; // ���� f_low�������ظ�����
            }
            else
            {
                high = mid;
            }
        } while (--loop_count > 0);

        return mid;
    }

    // ��ֵ����-�����Է�����ⷨ-�򵥵�����-������P=1
    // Simple_Iteration_Method(�ص���������,��ֵ,�����,����������|Ĭ��1000)
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

    // ��ֵ����-�����Է�����ⷨ-ţ�ٵ�����-������P=2
    // Newton_Method(�ص�����,�ص�������,��ֵ,�����,����������|Ĭ��1000)
    double Newton_Method(double (*func)(double), double (*der_func)(double), double initial_value, double tolerance, const size_t max_iterations)
    {
        constexpr double ZERO_DERIV_THRESHOLD = 1e-12; // ���ж���ֵ

        auto loop_func = [&func, &der_func](double x) -> double
        {
            double df = der_func(x);
            if (fabs(df) < ZERO_DERIV_THRESHOLD) throw runtime_error("Zero derivative encountered.");
            return x - func(x) / df;
        };

        double Xi;                        // Xi
        double Xi_plus_1 = initial_value; // Xi+1
        size_t iter = 0;                  // ��������

        do
        {
            Xi = Xi_plus_1;
            Xi_plus_1 = loop_func(Xi_plus_1);

            if (fabs(Xi_plus_1 - Xi) < tolerance) return Xi_plus_1;
        } while (++iter < max_iterations);

        throw runtime_error("Exceeded maximum iterations without convergence.");
    }

    // ��ֵ����-�����Է�����ⷨ-�ҽص�����-������P=1.618
    // Secant_Method(�ص�����,��ֵ0,��ֵ1,�����,����������|Ĭ��1000,�Ƿ�Ϊ���˵��ҽط�|�̶���0|Ĭ�ϱ�˵�)
    double Secant_Method(double (*func)(double), double initial_value0, double initial_value1, double tolerance, const size_t max_iterations, bool isFixedPoint0)
    {
        constexpr double ZERO_DERIV_THRESHOLD = 1e-12; // ���ж���ֵ

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