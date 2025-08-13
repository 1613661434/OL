/****************************************************************************************/
/*
 * ��������ol_signal.h
 * �����������źŴ������࣬�ṩ�źź��Ժ�I/O�رչ��ܣ�֧���������ԣ�
 *          - ͳһ�������źţ����Ի򲶻�
 *          - ��ѡ�رձ�׼�������������ֹ�����쳣�����
 *          - ��֧��Linuxƽ̨�������ض�ϵͳ���ã�
 * ���ߣ�ol
 * ���ñ�׼��C++11�����ϣ���֧��Linux�źŻ��ƣ�
 */
/****************************************************************************************/

#ifndef __OL_SIGNAL_H
#define __OL_SIGNAL_H 1

#include "ol_fstream.h"
#include <signal.h>

#ifdef __linux__
#include <unistd.h>
#endif // __linux__

namespace ol
{

#ifdef __linux__
    /**
     * ���Գ����źŲ���ѡ�رձ�׼I/O��
     * @param bcloseio �Ƿ�رձ�׼���������������Ĭ��false-���رգ�
     * @note 1. ���Ե��źŰ���SIGINT��SIGTERM��SIGHUP�ȳ�����ֹ�ź�
     *       2. ��bcloseioΪtrue�����ر�stdin��stdout��stderr���ļ�������0��1��2��
     *       3. �����ں�̨������򣬷�ֹ������ֹ���������
     */
    void closeioandsignal(bool bcloseio = false);
#endif // __linux__

} // namespace ol

#endif // !__OL_SIGNAL_H