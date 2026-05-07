/****************************************************************************************/
/*
 * 程序名：test_ol_enum_char_ops.cpp
 * 功能描述：测试 ol_enum_char_ops.h 中为底层 char 强类型枚举提供的运算符重载功能
 *          验证：自增/自减、所有比较运算符、算术/赋值运算符的正确性
 *          - 自增：前置++、后置++
 *          - 自减：前置--、后置--
 *          - 比较：<、>、>=、<=、==、!=
 *          - 算术：+、-（枚举 ± 整数偏移）
 *          - 赋值：+=、-=
 * 作者：ol
 * 适用标准：C++11及以上
 */
/****************************************************************************************/

#include "ol_enum_char_ops.h"
#include <iostream>
#include <cstdint>

// 定义测试用的强类型枚举（底层为char，符合头文件适用条件）
enum class CardPoint : char
{
    Card_Begin,
    Card_3,  // 1
    Card_4,  // 2
    Card_5,  // 3
    Card_6,  // 4
    Card_7,  // 5
    Card_8,  // 6
    Card_9,  // 7
    Card_10, // 8
    Card_J,  // 9
    Card_Q,  // 10
    Card_K,  // 11
    Card_A,  // 12
    Card_2,  // 13
    Card_SJ, // Small Joker (14)
    Card_BJ, // Big Joker (15)
    Card_End // 16
};

/**
 * @brief 辅助函数：输出枚举值及对应的底层char数值（便于验证）
 * @param name 枚举值名称描述
 * @param p 枚举值
 */
template <typename E>
void print_enum(const char* name, const E& p)
{
    std::cout << "  " << name << " = " << static_cast<int>(static_cast<char>(p)) << std::endl;
}

int main()
{
    std::cout << "=====================================" << std::endl;
    std::cout << "开始测试 ol_enum_char_ops.h 功能" << std::endl;
    std::cout << "=====================================" << std::endl;

    // 测试1：前置++ + < 运算符（核心循环场景）
    std::cout << "\n【测试1】前置++ + < 运算符（循环遍历 Card_3 ~ Card_2）" << std::endl;
    int loop_count = 0;
    for (CardPoint p = CardPoint::Card_Begin + 1; p < CardPoint::Card_SJ; ++p)
    {
        loop_count++;
        // 仅输出前3个和最后1个，避免输出过长
        if (loop_count <= 3 || loop_count == 13)
        {
            print_enum(
                loop_count == 1 ? "Card_3" : loop_count == 2 ? "Card_4"
                                         : loop_count == 3   ? "Card_5"
                                                             : "Card_2",
                p);
        }
    }
    std::cout << "  循环次数验证：" << loop_count << "（预期13次，Card_3~Card_2共13个值）" << std::endl;
    if (loop_count == 13)
    {
        std::cout << "  ✅ 前置++ + < 运算符测试通过" << std::endl;
    }
    else
    {
        std::cout << "  ❌ 前置++ + < 运算符测试失败" << std::endl;
    }

    // 测试2：加法运算符 +（枚举 + 整数偏移）
    std::cout << "\n【测试2】加法运算符 +（Card_3 + 2）" << std::endl;
    CardPoint p_add = CardPoint::Card_3 + 2;
    print_enum("Card_3 + 2 (预期Card_5)", p_add);
    if (static_cast<char>(p_add) == static_cast<char>(CardPoint::Card_5))
    {
        std::cout << "  ✅ 加法运算符测试通过" << std::endl;
    }
    else
    {
        std::cout << "  ❌ 加法运算符测试失败" << std::endl;
    }

    // 测试3：赋值加法 += 运算符
    std::cout << "\n【测试3】赋值加法 += 运算符（Card_4 += 3）" << std::endl;
    CardPoint p_add_eq = CardPoint::Card_4;
    p_add_eq += 3;
    print_enum("Card_4 += 3 (预期Card_7)", p_add_eq);
    if (static_cast<char>(p_add_eq) == static_cast<char>(CardPoint::Card_7))
    {
        std::cout << "  ✅ 赋值加法运算符测试通过" << std::endl;
    }
    else
    {
        std::cout << "  ❌ 赋值加法运算符测试失败" << std::endl;
    }

    // 测试4：后置++ 运算符
    std::cout << "\n【测试4】后置++ 运算符（Card_5++）" << std::endl;
    CardPoint p_post_inc = CardPoint::Card_5;
    CardPoint p_post_inc_old = p_post_inc++; // 后置++返回原值
    print_enum("Card_5++ 原值", p_post_inc_old);
    print_enum("Card_5++ 后值", p_post_inc);
    if (static_cast<char>(p_post_inc_old) == static_cast<char>(CardPoint::Card_5) &&
        static_cast<char>(p_post_inc) == static_cast<char>(CardPoint::Card_6))
    {
        std::cout << "  ✅ 后置++运算符测试通过" << std::endl;
    }
    else
    {
        std::cout << "  ❌ 后置++运算符测试失败" << std::endl;
    }

    // 测试5：自减运算符 --（前置--、后置--）
    std::cout << "\n【测试5】自减运算符 --（Card_7--、--Card_6）" << std::endl;
    // 测试后置--
    CardPoint p_post_dec = CardPoint::Card_7;
    CardPoint p_post_dec_old = p_post_dec--;
    print_enum("Card_7-- 原值", p_post_dec_old);
    print_enum("Card_7-- 后值", p_post_dec);
    // 测试前置--
    CardPoint p_pre_dec = CardPoint::Card_6;
    --p_pre_dec;
    print_enum("--Card_6 后值 (预期Card_5)", p_pre_dec);

    bool dec_ok = (static_cast<char>(p_post_dec_old) == static_cast<char>(CardPoint::Card_7) &&
                   static_cast<char>(p_post_dec) == static_cast<char>(CardPoint::Card_6) &&
                   static_cast<char>(p_pre_dec) == static_cast<char>(CardPoint::Card_5));
    if (dec_ok)
    {
        std::cout << "  ✅ 自减运算符测试通过" << std::endl;
    }
    else
    {
        std::cout << "  ❌ 自减运算符测试失败" << std::endl;
    }

    // 测试6：比较运算符（>、>=、<=、==、!=）
    std::cout << "\n【测试6】比较运算符（>、>=、<=、==、!=）" << std::endl;
    CardPoint p5 = CardPoint::Card_5;
    CardPoint p6 = CardPoint::Card_6;
    CardPoint p5_copy = CardPoint::Card_5;

    bool gt_ok = (p6 > p5) == true;
    bool ge_ok = (p6 >= p5) == true && (p5 >= p5_copy) == true;
    bool le_ok = (p5 <= p6) == true && (p5 <= p5_copy) == true;
    bool eq_ok = (p5 == p5_copy) == true;
    bool ne_ok = (p5 != p6) == true;

    std::cout << "  Card_6 > Card_5: " << (p6 > p5 ? "true" : "false") << "（预期true）" << std::endl;
    std::cout << "  Card_5 >= Card_5: " << (p5 >= p5_copy ? "true" : "false") << "（预期true）" << std::endl;
    std::cout << "  Card_5 <= Card_6: " << (p5 <= p6 ? "true" : "false") << "（预期true）" << std::endl;
    std::cout << "  Card_5 == Card_5: " << (p5 == p5_copy ? "true" : "false") << "（预期true）" << std::endl;
    std::cout << "  Card_5 != Card_6: " << (p5 != p6 ? "true" : "false") << "（预期true）" << std::endl;

    bool cmp_all_ok = gt_ok && ge_ok && le_ok && eq_ok && ne_ok;
    if (cmp_all_ok)
    {
        std::cout << "  ✅ 所有比较运算符测试通过" << std::endl;
    }
    else
    {
        std::cout << "  ❌ 部分比较运算符测试失败" << std::endl;
    }

    // 测试7：减法运算符 -（枚举 - 整数偏移）
    std::cout << "\n【测试7】减法运算符 -（Card_7 - 2）" << std::endl;
    CardPoint p_sub = CardPoint::Card_7 - 2;
    print_enum("Card_7 - 2 (预期Card_5)", p_sub);
    if (static_cast<char>(p_sub) == static_cast<char>(CardPoint::Card_5))
    {
        std::cout << "  ✅ 减法运算符测试通过" << std::endl;
    }
    else
    {
        std::cout << "  ❌ 减法运算符测试失败" << std::endl;
    }

    // 测试8：赋值减法 -= 运算符
    std::cout << "\n【测试8】赋值减法 -= 运算符（Card_8 -= 3）" << std::endl;
    CardPoint p_sub_eq = CardPoint::Card_8;
    p_sub_eq -= 3;
    print_enum("Card_8 -= 3 (预期Card_5)", p_sub_eq);
    if (static_cast<char>(p_sub_eq) == static_cast<char>(CardPoint::Card_5))
    {
        std::cout << "  ✅ 赋值减法运算符测试通过" << std::endl;
    }
    else
    {
        std::cout << "  ❌ 赋值减法运算符测试失败" << std::endl;
    }

    std::cout << "\n=====================================" << std::endl;
    std::cout << "所有测试完成" << std::endl;
    std::cout << "=====================================" << std::endl;

    return 0;
}