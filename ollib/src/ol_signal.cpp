#include "../include/ol_signal.h"

namespace ol
{

#ifdef __linux__
    // ���Թر�ȫ�����źš��ر�ȫ����IO��ȱʡֻ�����źţ�����IO��
    // ��ϣ����̨��������źŴ��ţ���Ҫʲô�źſ����ڳ��������á�
    // ʵ���Ϲرյ�IO��0��1��2��
    void closeioandsignal(bool bcloseio)
    {
        for (int i = 0; i < 64; ++i) signal(i, SIG_IGN);

        if (bcloseio == false) return;

        for (int i = 0; i < 3; ++i) close(i);
    }
#endif // __linux__

} // namespace ol