#ifndef __OL_SIGNAL_H
#define __OL_SIGNAL_H 1

#include "../include/ol_fstream.h"
#include <signal.h>

#ifdef __linux__
#include <unistd.h>
#endif // __linux__

namespace ol
{

#ifdef __linux__
    // ���Թر�ȫ�����źš��ر�ȫ����IO��ȱʡֻ�����źţ�����IO��
    void closeioandsignal(bool bcloseio = false);
#endif // __linux__

} // namespace ol

#endif // !__OL_SIGNAL_H