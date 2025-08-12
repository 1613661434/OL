/*****************************************************************************************/
/*   ��������ol_public.h���˳����ǿ�����ܹ���ͷ�ļ���������Ӧ�ÿ������õ�ͷ�ļ���             */
/*   ���ߣ�ol                                                                             */
/*****************************************************************************************/

#ifndef __OL_PUBLIC_H
#define __OL_PUBLIC_H 1
#include "ol_chrono.h"  // �Զ���ʱ�䴦����
#include "ol_cqueue.h"  // �Զ���ѭ������ʵ��
#include "ol_fstream.h" // �Զ����ļ�����������
#include "ol_ftp.h"     // �Զ���FTP�����̹���
#include "ol_ipc.h"     // �Զ�����̼�ͨ�Ź���
#include "ol_signal.h"  // �Զ����źŴ�����
#include "ol_string.h"  // �Զ����ַ���������
#include "ol_tcp.h"     // �Զ���TCP�����̹���

#include <algorithm>          // C++�㷨��
#include <atomic>             // C++ԭ�Ӳ�����
#include <condition_variable> // C++��������
#include <ctype.h>            // �ַ����ͼ�麯��
#include <deque>              // C++˫�˶�������
#include <errno.h>            // �����붨��
#include <fcntl.h>            // �ļ�����ѡ�open��fcntl�ȣ�
#include <forward_list>       // C++������������
#include <fstream>            // C++�ļ�������
#include <iostream>           // C++���������
#include <limits.h>           // ��ֵ���ͼ���ֵ����
#include <list>               // C++˫����������
#include <locale.h>           // ���ػ�����
#include <map>                // C++����ӳ������
#include <math.h>             // ��ѧ������
#include <memory>             // C++����ָ���
#include <mutex>              // C++��������
#include <queue>              // C++��������
#include <signal.h>           // �źŴ������
#include <stdarg.h>           // �ɱ��������
#include <stdio.h>            // C��׼���������
#include <stdlib.h>           // C��׼�⣨�ڴ���䡢������ȣ�
#include <string.h>           // C�ַ���������
#include <string>             // C++�ַ�����
#include <sys/stat.h>         // �ļ�״̬��Ϣ
#include <sys/types.h>        // ����ϵͳ��������
#include <thread>             // C++�߳���
#include <time.h>             // Cʱ�䴦����
#include <unordered_map>      // C++����ӳ������
#include <vector>             // C++��̬��������

#ifdef __linux__
#include <arpa/inet.h>    // IP��ַת������
#include <dirent.h>       // Ŀ¼��������
#include <netdb.h>        // �������ݿ����
#include <netinet/in.h>   // �����ַ�ṹ
#include <poll.h>         // I/O��·����
#include <pthread.h>      // POSIX�߳̿�
#include <semaphore.h>    // POSIX�ź���
#include <strings.h>      // �ַ���������չ����
#include <sys/epoll.h>    // ������I/O��·����
#include <sys/ipc.h>      // IPC������
#include <sys/sem.h>      // System V�ź���
#include <sys/shm.h>      // �����ڴ����
#include <sys/signalfd.h> // �ź�ת��Ϊ�ļ�������
#include <sys/socket.h>   // �׽��ֱ�̻���
#include <sys/time.h>     // ʱ���ȡ������
#include <sys/timerfd.h>  // ��ʱ���ļ�������
#include <termios.h>      // �ն�I/O����
#include <unistd.h>       // Unix��׼ϵͳ����
#include <utime.h>        // �ļ�ʱ�������
#endif                    // __linux__

#endif // !__OL_PUBLIC_H