#ifndef __OL_MATH_H
#define __OL_MATH_H 1

#include <cstddef>

namespace ol
{

    // ��ֵ����-�����Է�����ⷨ-���ֵ�����-������P=1
    // Bisection_Method(�ص�����,������,������,�����,����������|Ĭ��1000)
    double Bisection_Method(double (*func)(double), double low, double high, double tolerance, const size_t max_iterations = 1000);

    // ��ֵ����-�����Է�����ⷨ-�򵥵�����-������P=1
    // Simple_Iteration_Method(�ص���������,��ֵ,�����,����������|Ĭ��1000)
    double Simple_Iteration_Method(double (*iter_func)(double), double initial_value, double tolerance, const size_t max_iterations = 1000);

    // ��ֵ����-�����Է�����ⷨ-ţ�ٵ�����-������P=2
    // Newton_Method(�ص�����,�ص�������,��ֵ,�����,����������|Ĭ��1000)
    double Newton_Method(double (*func)(double), double (*der_func)(double), double initial_value, double tolerance, const size_t max_iterations = 1000);

    // ��ֵ����-�����Է�����ⷨ-�ҽص�����-������P=1.618
    // Secant_Method(�ص�����,��ֵ0,��ֵ1,�����,����������|Ĭ��1000,�Ƿ�Ϊ���˵��ҽط�|�̶���0|Ĭ�ϱ�˵�)
    double Secant_Method(double (*func)(double), double initial_value0, double initial_value1, double tolerance, const size_t max_iterations = 1000, bool isFixedPoint0 = false);

} // namespace ol

#endif // !__OL_MATH_H