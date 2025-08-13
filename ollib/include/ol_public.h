/*****************************************************************************************/
/*
 * ��������ol_public.h
 * ����������������ܹ���ͷ�ļ���������Ӧ�ÿ������õ��Զ��幤�����ϵͳͷ�ļ������԰�����
 *          - ��������������Զ��幤���ࣨʱ�䡢���С��ļ����������硢IPC�ȣ�
 *          - ����C++��׼�����ͷ�ļ����������̡߳�����ָ��ȣ�
 *          - ��ƽ̨��Linux������ϵͳ�ض�ͷ�ļ������硢���̡�I/O�ȣ�
 *          - ��Ӧ�ÿ�����ͷ�ļ����ã�ͳһ��������
 * ���ߣ�ol
 * ���ñ�׼��C++11�����ϣ���֧���̡߳�ԭ�Ӳ��������ԣ�
 */
/*****************************************************************************************/

#ifndef __OL_PUBLIC_H
#define __OL_PUBLIC_H 1

// �Զ��幤����ͷ�ļ�
#include "ol_chrono.h"  // ʱ�䴦���ߣ����ڡ�ʱ�������ʱ���ȣ�
#include "ol_cqueue.h"  // ѭ������ʵ�֣��̶���С��FIFO������
#include "ol_fstream.h" // �ļ����������ߣ�Ŀ¼���ļ���д����־�ȣ�
#include "ol_ftp.h"     // FTP�ͻ��˹��ߣ��ļ��ϴ������ء�Ŀ¼�����ȣ�
#include "ol_ipc.h"     // ���̼�ͨ�Ź��ߣ��ź����������ڴ桢���������ȣ�
#include "ol_signal.h"  // �źŴ����ߣ��źŲ����Զ��崦���߼��ȣ�
#include "ol_string.h"  // �ַ��������ߣ���ʽ�����ָת���ȣ�
#include "ol_tcp.h"     // TCP�����̹��ߣ��ͻ��ˡ�������ͨ�ŵȣ�

// C++��׼��ͷ�ļ�
#include <algorithm>          // �㷨�⣨���򡢲��ҡ��任�ȣ�
#include <atomic>             // ԭ�Ӳ����⣨�̰߳�ȫ�ı���������
#include <condition_variable> // �����������߳�ͬ�����ƣ�
#include <ctype.h>            // �ַ����ͼ�飨��Сд�������жϵȣ�
#include <deque>              // ˫�˶�����������Ч��β������
#include <errno.h>            // �����붨�壨ϵͳ���ô�����Ϣ��
#include <fcntl.h>            // �ļ�����ѡ�open��fcntl����������
#include <forward_list>       // ����������������Ч����ɾ����
#include <fstream>            // �ļ���������C++����ļ���д��
#include <iostream>           // �����������cout��cin�ȣ�
#include <limits.h>           // ��ֵ���ͼ���ֵ��INT_MAX��LLONG_MIN�ȣ�
#include <list>               // ˫����������������λ�ø�Ч������
#include <locale.h>           // ���ػ����ã��ַ����롢������ʽ�ȣ�
#include <map>                // ����ӳ����������ֵ�ԣ��Զ�����
#include <math.h>             // ��ѧ�����⣨���Ǻ�����������ȣ�
#include <memory>             // ����ָ��⣨unique_ptr��shared_ptr�ȣ�
#include <mutex>              // �������ࣨ�߳�ͬ������ֹ��̬������
#include <queue>              // ����������FIFO���ݽṹ��
#include <signal.h>           // �źŴ�����ƣ�ϵͳ�źŲ���
#include <stdarg.h>           // �ɱ��������printf�����ʵ�֣�
#include <stdio.h>            // C��׼���������printf��fopen�ȣ�
#include <stdlib.h>           // C��׼�⣨�ڴ���䡢�������������Ƶȣ�
#include <string.h>           // C�ַ�������strcpy��strlen�ȣ�
#include <string>             // C++�ַ����ࣨstd::string��������
#include <sys/stat.h>         // �ļ�״̬��Ϣ��stat�������ļ�Ȩ�޵ȣ�
#include <sys/types.h>        // ����ϵͳ�������ͣ�pid_t��size_t�ȣ�
#include <thread>             // C++�߳��ࣨ�����������̣߳�
#include <time.h>             // Cʱ�䴦��ʱ���������ʱ��ȣ�
#include <unordered_map>      // ����ӳ����������ϣ��ʵ�֣�O(1)���ң�
#include <vector>             // ��̬������������Ч������ʣ�

// Linuxƽ̨�ض�ͷ�ļ�
#ifdef __linux__
#include <arpa/inet.h>    // IP��ַת����inet_addr��inet_ntoa�ȣ�
#include <dirent.h>       // Ŀ¼������opendir��readdir�ȣ�
#include <netdb.h>        // �������ݿ⣨�����������������ѯ�ȣ�
#include <netinet/in.h>   // �����ַ�ṹ��sockaddr_in�ȣ�
#include <poll.h>         // I/O��·���ã�poll������
#include <pthread.h>      // POSIX�߳̿⣨����C����̲߳�����
#include <semaphore.h>    // POSIX�ź������߳�/����ͬ����
#include <strings.h>      // �ַ���������չ��bzero��strcasecmp�ȣ�
#include <sys/epoll.h>    // ������I/O��·���ã�epollϵ�к�����
#include <sys/ipc.h>      // IPC�����ɣ�ftok������
#include <sys/sem.h>      // System V�ź��������̼�ͬ����
#include <sys/shm.h>      // �����ڴ���������̼����ݹ���
#include <sys/signalfd.h> // �ź�ת��Ϊ�ļ���������ͳһI/O����
#include <sys/socket.h>   // �׽��ֱ�̻�����socket��bind�ȣ�
#include <sys/time.h>     // ʱ���ȡ�����ã�gettimeofday�ȣ�
#include <sys/timerfd.h>  // ��ʱ���ļ����������߾��ȶ�ʱ��
#include <termios.h>      // �ն�I/O���ƣ�����ͨ�š��ն����õȣ�
#include <unistd.h>       // Unix��׼ϵͳ���ã�read��write��close�ȣ�
#include <utime.h>        // �ļ�ʱ����������޸ķ���/�޸�ʱ�䣩
#endif                    // __linux__

#endif // !__OL_PUBLIC_H