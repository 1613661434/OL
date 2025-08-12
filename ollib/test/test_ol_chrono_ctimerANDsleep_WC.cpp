/*
 *  ��������test_ol_chrono_ctimer&sleep_%.cpp���˳�����ʾ��������е�ctimer�ࣨ��ʱ�������÷���
 *  ���ߣ�ol
 */
#include "ol_chrono.h"
#include <iostream>

using namespace ol;
using namespace std;

int main()
{
    ctimer timer;

    printf("elapsed=%lf\n", timer.elapsed());
    sleep_sec(1);
    printf("elapsed=%lf\n", timer.elapsed());
    sleep_sec(1);
    printf("elapsed=%lf\n", timer.elapsed());
    sleep_us(1000);
    printf("elapsed=%lf\n", timer.elapsed());
    sleep_us(100);
    printf("elapsed=%lf\n", timer.elapsed());
    sleep_sec(10);
    printf("elapsed=%lf\n", timer.elapsed());

    // ʹ��ͨ��ģ�壨��Ҫ��ʽָ��ʱ�䵥λ��
    sleep(std::chrono::seconds(2));        // ����2��
    sleep(std::chrono::milliseconds(100)); // ����100����

    // ʹ�����غ�����ֱ�Ӵ���ֵ��
    sleep_sec(1);      // ����1��
    sleep_ms(250);     // ����250����
    sleep_us(500000);  // ����500000΢�루0.5�룩
    sleep_ns(1000000); // ����1000000���루1���룩

    printf("elapsed=%lf\n", timer.elapsed());
}