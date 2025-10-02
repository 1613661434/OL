/****************************************************************************************/
/*
 * 程序名：ol_math.h
 * 功能描述：数值计算工具类，提供非线性方程求解的常用迭代算法，支持以下特性：
 *          - 二分迭代法（Bisection Method）：收敛阶P=1
 *          - 简单迭代法（Simple Iteration Method）：收敛阶P=1
 *          - 牛顿迭代法（Newton Method）：收敛阶P=2
 *          - 弦截迭代法（Secant Method）：收敛阶P=1.618，支持固定端点和变端点模式
 * 作者：ol
 * 适用标准：C++11及以上
 */
/****************************************************************************************/

#ifndef OL_MATH_H
#define OL_MATH_H 1

#include <cstddef>

namespace ol
{

    /**
     * @brief 二分迭代法求解非线性方程（收敛阶P=1）
     * @param func 目标函数（f(x)=0）
     * @param low 区间左端点
     * @param high 区间右端点（需满足f(low)与f(high)异号）
     * @param tolerance 误差限（迭代终止条件）
     * @param max_iterations 最大迭代次数（默认1000，防止无限循环）
     * @return 方程的近似解
     * @note 要求函数在[low, high]上连续且f(low)*f(high) < 0
     */
    double Bisection_Method(double (*func)(double), double low, double high, double tolerance, const size_t max_iterations = 1000);

    /**
     * @brief 简单迭代法求解非线性方程（收敛阶P=1）
     * @param iter_func 迭代函数（x_{n+1} = iter_func(x_n)）
     * @param initial_value 初始迭代值
     * @param tolerance 误差限（|x_{n+1}-x_n| < tolerance时终止）
     * @param max_iterations 最大迭代次数（默认1000）
     * @return 方程的近似解
     * @note 要求迭代函数在迭代区间内满足收敛条件（导数绝对值小于1）
     */
    double Simple_Iteration_Method(double (*iter_func)(double), double initial_value, double tolerance, const size_t max_iterations = 1000);

    /**
     * @brief 牛顿迭代法求解非线性方程（收敛阶P=2）
     * @param func 目标函数（f(x)=0）
     * @param der_func 目标函数的导函数（f’(x)）
     * @param initial_value 初始迭代值
     * @param tolerance 误差限（|x_{n+1}-x_n| < tolerance时终止）
     * @param max_iterations 最大迭代次数（默认1000）
     * @return 方程的近似解
     * @note 要求初始值附近f’(x)≠0且函数足够光滑
     */
    double Newton_Method(double (*func)(double), double (*der_func)(double), double initial_value, double tolerance, const size_t max_iterations = 1000);

    /**
     * @brief 弦截迭代法求解非线性方程（收敛阶P=1.618）
     * @param func 目标函数（f(x)=0）
     * @param initial_value_0 初始迭代值0
     * @param initial_value_1 初始迭代值1
     * @param tolerance 误差限（|x_{n+1}-x_n| < tolerance时终止）
     * @param max_iterations 最大迭代次数（默认1000）
     * @param isFixedPoint_0 是否使用固定端点模式（固定initial_value_0，默认false为变端点模式）
     * @return 方程的近似解
     * @note 无需计算导数，收敛速度快于二分法，慢于牛顿法
     */
    double Secant_Method(double (*func)(double), double initial_value_0, double initial_value_1, double tolerance, const size_t max_iterations = 1000, bool isFixedPoint_0 = false);

} // namespace ol

#endif // !OL_MATH_H