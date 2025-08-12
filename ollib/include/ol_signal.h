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
    // 忽略关闭全部的信号、关闭全部的IO，缺省只忽略信号，不关IO。
    void closeioandsignal(bool bcloseio = false);
#endif // __linux__

} // namespace ol

#endif // !__OL_SIGNAL_H