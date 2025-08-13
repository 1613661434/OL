/****************************************************************************************/
/*
 * ��������ol_chrono.h
 * ����������ʱ����������༰�������ϣ�֧���������ԣ�
 *          - ʱ���ַ�����ʽ�������ָ�ʽת����
 *          - ʱ������ַ�����ת
 *          - ʱ��ƫ�Ƽ��㣨����������
 *          - �߾��ȼ�ʱ����΢�뼶��
 *          - ��ƽ̨���ߺ�����֧�����롢΢�롢���롢�뼶��
 * ���ߣ�ol
 * ���ñ�׼��C++11�����ϣ���֧��chrono��thread�����ԣ�
 */
/****************************************************************************************/

#ifndef __OL_CHRONO_H
#define __OL_CHRONO_H 1

#include <chrono>
#include <ctime>
#include <string>
#include <thread>

#ifdef _WIN32
#include <windows.h> // Windows ƽ̨ʹ�� windows.h ��� sys/time.h
#elif defined(__linux__)
#include <sys/time.h> // Linus ƽ̨ʹ�� sys/time.h
#endif

namespace ol
{

    // ʱ����������ɺ�����
    // ===========================================================================
    /*
      ȡ����ϵͳ��ʱ�䣨���ַ�����ʾ����
      strtime�����ڴ�Ż�ȡ����ʱ�䡣
      timetvl��ʱ���ƫ��������λ���룬0��ȱʡֵ����ʾ��ǰʱ�䣬30��ʾ��ǰʱ��30��֮���ʱ��㣬-30��ʾ��ǰʱ��30��֮ǰ��ʱ��㡣
      fmt�����ʱ��ĸ�ʽ��fmtÿ���ֵĺ��壺yyyy-��ݣ�mm-�·ݣ�dd-���ڣ�hh24-Сʱ��mi-���ӣ�ss-�룬
      ȱʡ��"yyyy-mm-dd hh24:mi:ss"��Ŀǰ֧�����¸�ʽ��
      "yyyy-mm-dd hh24:mi:ss"
      "yyyymmddhh24miss"
      "yyyy-mm-dd"
      "yyyymmdd"
      "hh24:mi:ss"
      "hh24miss"
      "hh24:mi"
      "hh24mi"
      "hh24"
      "mi"
      ע�⣺
        1��Сʱ�ı�ʾ������hh24������hh����ô����Ŀ����Ϊ�˱��������ݿ��ʱ���ʾ����һ�£�
        2�������г��˳��õ�ʱ���ʽ���������������Ӧ�ÿ������������޸�Դ����timetostr()�������Ӹ���ĸ�ʽ֧�֣�
        3�����ú�����ʱ�����fmt��������ʽ��ƥ�䣬strtime�����ݽ�Ϊ�ա�
        4��ʱ����������λ�������Ŀ�����һλ����λ�����������λ����ǰ�油0��
    */

    /**
     * ��ȡ����ϵͳʱ�䲢��ʽ��Ϊ�ַ���
     * @param strtime ���ڴ�Ž�����ַ�������
     * @param fmt �����ʽ��Ĭ��"yyyy-mm-dd hh24:mi:ss"����֧�ָ�ʽ����ע
     * @param timetvl ʱ��ƫ�������룩������Ϊδ��������Ϊ��ȥ��0Ϊ��ǰʱ��
     * @return ��ʽ�����ʱ���ַ�������
     * @note ֧�ֵĸ�ʽ������"yyyy-mm-dd hh24:mi:ss"��"yyyymmddhh24miss"��"yyyy-mm-dd"��
     */
    std::string& ltime(std::string& strtime, const std::string& fmt = "", const int timetvl = 0);

    /**
     * ��ȡ����ϵͳʱ�䲢��ʽ��ΪC�ַ���
     * @param strtime ���ڴ�Ž�����ַ�����ָ�루��ȷ���㹻�ռ䣩
     * @param fmt �����ʽ��Ĭ��"yyyy-mm-dd hh24:mi:ss"��
     * @param timetvl ʱ��ƫ�������룩
     * @return ��ʽ�����C�ַ���ָ��
     */
    char* ltime(char* strtime, const std::string& fmt = "", const int timetvl = 0);

    /**
     * ��ȡ����ϵͳʱ�䲢��ʽ��Ϊ�ַ������޲������أ��������壩
     * @param fmt �����ʽ
     * @param timetvl ʱ��ƫ�������룩
     * @return ��ʽ�����ʱ���ַ���
     */
    std::string ltime1(const std::string& fmt = "", const int timetvl = 0);
    // ===========================================================================

    // ===========================================================================
    /**
     * ��ʱ���ת��Ϊָ����ʽ���ַ���
     * @param ttime ʱ�����time_t���ͣ�
     * @param strtime ���ڴ�Ž�����ַ�������
     * @param fmt �����ʽ��Ĭ��"yyyy-mm-dd hh24:mi:ss"��
     * @return ��ʽ�����ʱ���ַ�������
     * @note ���fmt�ĸ�ʽ����ȷ��strtime��Ϊ��
     */
    std::string& timetostr(const time_t ttime, std::string& strtime, const std::string& fmt = "");

    /**
     * ��ʱ���ת��Ϊָ����ʽ��C�ַ���
     * @param ttime ʱ�����time_t���ͣ�
     * @param strtime ���ڴ�Ž�����ַ�����ָ��
     * @param fmt �����ʽ
     * @return ��ʽ�����C�ַ���ָ��
     * @note ���fmt�ĸ�ʽ����ȷ��strtime��Ϊ��
     */
    char* timetostr(const time_t ttime, char* strtime, const std::string& fmt = "");

    /**
     * ��ʱ���ת��Ϊָ����ʽ���ַ������޲������أ��������壩
     * @param ttime ʱ�����time_t���ͣ�
     * @param fmt �����ʽ
     * @return ��ʽ�����ʱ���ַ���
     */
    std::string timetostr1(const time_t ttime, const std::string& fmt = "");

    /**
     * ���ַ���ת��Ϊʱ���
     * @param strtime ��������ʱ����Ϣ���ַ����������yyyymmddhh24miss��
     * @return ��Ӧ��ʱ�������ʽ����ʱ����-1
     */
    time_t strtotime(const std::string& strtime);
    // ===========================================================================

    // ===========================================================================
    /**
     * ��ʱ���ַ�������ƫ�Ƽ��㣨C�ַ����汾��
     * @param in_stime ����ʱ���ַ�������ʽ���ޣ���һ��Ҫ����yyyymmddhh24miss��һ���������٣�˳��Ҳ���ܱ䣩
     * @param out_stime ���ƫ�ƺ��ʱ���ַ���
     * @param timetvl ƫ������������Ϊδ��������Ϊ��ȥ��
     * @param fmt �����ʽ
     * @return �ɹ�����true��ʧ�ܷ���false�������ʽ����
     * @note in_stime��out_stime����������ͬһ�������ĵ�ַ���������ʧ�ܣ�out_stime�����ݻ���ա�
     */
    bool addtime(const std::string& in_stime, char* out_stime, const int timetvl, const std::string& fmt = "");

    /**
     * ��ʱ���ַ�������ƫ�Ƽ��㣨std::string�汾��
     * @param in_stime ����ʱ���ַ�������ʽ���ޣ���һ��Ҫ����yyyymmddhh24miss��һ���������٣�˳��Ҳ���ܱ䣩
     * @param out_stime ���ƫ�ƺ��ʱ���ַ�������
     * @param timetvl ƫ������
     * @param fmt �����ʽ
     * @return �ɹ�����true��ʧ�ܷ���false
     * @note in_stime��out_stime����������ͬһ�������ĵ�ַ���������ʧ�ܣ�out_stime�����ݻ���ա�
     */
    bool addtime(const std::string& in_stime, std::string& out_stime, const int timetvl, const std::string& fmt = "");
    // ===========================================================================

    // ===========================================================================
    /**
     * �߾��ȼ�ʱ���ࣨ΢�뼶��
     */
    class ctimer
    {
    private:
        struct timeval m_start; // ��ʱ��ʼ��ʱ��㡣
        struct timeval m_end;   // ��ʱ������ʱ��㡣
    public:
        // ���캯���л����start������
        ctimer();

        // ��ʼ��ʱ��
        void start();

        /**
         * ������ϴ�start()����ǰ����ȥʱ��
         * @return ��ȥʱ�䣨�룩��С�����Ϊ΢��
         * @note ���ú���Զ����¿�ʼ��ʱ
         */
        double elapsed();
    };
    // ===========================================================================

    // ===========================================================================
    /**
     * ͨ�����ߺ�����֧������ʱ�䵥λ��
     * @param duration ����ʱ����std::chrono::duration���ͣ�
     */
    template <typename Rep, typename Period>
    inline void sleep(std::chrono::duration<Rep, Period> duration)
    {
        std::this_thread::sleep_for(duration);
    }

    /**
     * ���뼶����
     * @param nanoseconds ����ʱ�������룩
     */
    inline void sleep_ns(long long nanoseconds)
    {
        sleep(std::chrono::nanoseconds(nanoseconds));
    }

    /**
     * ΢�뼶����
     * @param microseconds ����ʱ����΢�룩
     */
    inline void sleep_us(long long microseconds)
    {
        sleep(std::chrono::microseconds(microseconds));
    }

    /**
     * ���뼶����
     * @param milliseconds ����ʱ�������룩
     */
    inline void sleep_ms(long long milliseconds)
    {
        sleep(std::chrono::milliseconds(milliseconds));
    }

    /**
     * �뼶����
     * @param seconds ����ʱ�����룩
     */
    inline void sleep_sec(long long seconds)
    {
        sleep(std::chrono::seconds(seconds));
    }
    // ===========================================================================

} // namespace ol

#endif // !__OL_CHRONO_H