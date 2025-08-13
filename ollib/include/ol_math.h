/****************************************************************************************/
/*
 * ��������ol_math.h
 * ������������ֵ���㹤���࣬�ṩ�����Է������ĳ��õ����㷨��֧���������ԣ�
 *          - ���ֵ�������Bisection Method����������P=1
 *          - �򵥵�������Simple Iteration Method����������P=1
 *          - ţ�ٵ�������Newton Method����������P=2
 *          - �ҽص�������Secant Method����������P=1.618��֧�̶ֹ��˵�ͱ�˵�ģʽ
 * ���ߣ�ol
 * ���ñ�׼��C++11������
 */
/****************************************************************************************/

#ifndef __OL_MATH_H
#define __OL_MATH_H 1

#include <cstddef>

namespace ol
{

    /**
     * ���ֵ������������Է��̣�������P=1��
     * @param func Ŀ�꺯����f(x)=0��
     * @param low ������˵�
     * @param high �����Ҷ˵㣨������f(low)��f(high)��ţ�
     * @param tolerance ����ޣ�������ֹ������
     * @param max_iterations ������������Ĭ��1000����ֹ����ѭ����
     * @return ���̵Ľ��ƽ�
     * @note Ҫ������[low, high]��������f(low)*f(high) < 0
     */
    double Bisection_Method(double (*func)(double), double low, double high, double tolerance, const size_t max_iterations = 1000);

    /**
     * �򵥵������������Է��̣�������P=1��
     * @param iter_func ����������x_{n+1} = iter_func(x_n)��
     * @param initial_value ��ʼ����ֵ
     * @param tolerance ����ޣ�|x_{n+1}-x_n| < toleranceʱ��ֹ��
     * @param max_iterations ������������Ĭ��1000��
     * @return ���̵Ľ��ƽ�
     * @note Ҫ����������ڵ�������������������������������ֵС��1��
     */
    double Simple_Iteration_Method(double (*iter_func)(double), double initial_value, double tolerance, const size_t max_iterations = 1000);

    /**
     * ţ�ٵ������������Է��̣�������P=2��
     * @param func Ŀ�꺯����f(x)=0��
     * @param der_func Ŀ�꺯���ĵ�������f��(x)��
     * @param initial_value ��ʼ����ֵ
     * @param tolerance ����ޣ�|x_{n+1}-x_n| < toleranceʱ��ֹ��
     * @param max_iterations ������������Ĭ��1000��
     * @return ���̵Ľ��ƽ�
     * @note Ҫ���ʼֵ����f��(x)��0�Һ����㹻�⻬
     */
    double Newton_Method(double (*func)(double), double (*der_func)(double), double initial_value, double tolerance, const size_t max_iterations = 1000);

    // ��ֵ����-�����Է�����ⷨ-�ҽص�����-������P=1.618
    // Secant_Method(�ص�����,��ֵ0,��ֵ1,�����,����������|Ĭ��1000,�Ƿ�Ϊ���˵��ҽط�|�̶���0|Ĭ�ϱ�˵�)

    /**
     * �ҽص������������Է��̣�������P=1.618��
     * @param func Ŀ�꺯����f(x)=0��
     * @param initial_value_0 ��ʼ����ֵ0
     * @param initial_value_1 ��ʼ����ֵ1
     * @param tolerance ����ޣ�|x_{n+1}-x_n| < toleranceʱ��ֹ��
     * @param max_iterations ������������Ĭ��1000��
     * @param isFixedPoint_0 �Ƿ�ʹ�ù̶��˵�ģʽ���̶�initial_value_0��Ĭ��falseΪ��˵�ģʽ��
     * @return ���̵Ľ��ƽ�
     * @note ������㵼���������ٶȿ��ڶ��ַ�������ţ�ٷ�
     */
    double Secant_Method(double (*func)(double), double initial_value_0, double initial_value_1, double tolerance, const size_t max_iterations = 1000, bool isFixedPoint_0 = false);

} // namespace ol

#endif // !__OL_MATH_H