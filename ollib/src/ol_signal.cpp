#include "ol_signal.h"

namespace ol
{

#ifdef __linux__
    // 忽略关闭全部的信号、关闭全部的IO，缺省只忽略信号，不关IO。
    // 不希望后台服务程序被信号打扰，需要什么信号可以在程序中设置。
    // 实际上关闭的IO是0、1、2。
    void closeioandsignal(bool bcloseio)
    {
        for (int i = 0; i < 64; ++i) signal(i, SIG_IGN);

        if (bcloseio == false) return;

        for (int i = 0; i < 3; ++i) close(i);
    }
#endif // __linux__

} // namespace ol