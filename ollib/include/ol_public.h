/*****************************************************************************************/
/*   程序名：ol_public.h，此程序是开发框架公用头文件，包含了应用开发常用的头文件。             */
/*   作者：ol                                                                             */
/*****************************************************************************************/

#ifndef __OL_PUBLIC_H
#define __OL_PUBLIC_H 1
#include "ol_chrono.h"  // 自定义时间处理工具
#include "ol_cqueue.h"  // 自定义循环队列实现
#include "ol_fstream.h" // 自定义文件流操作工具
#include "ol_ftp.h"     // 自定义FTP网络编程工具
#include "ol_ipc.h"     // 自定义进程间通信工具
#include "ol_signal.h"  // 自定义信号处理工具
#include "ol_string.h"  // 自定义字符串处理工具
#include "ol_tcp.h"     // 自定义TCP网络编程工具

#include <algorithm>          // C++算法库
#include <atomic>             // C++原子操作库
#include <condition_variable> // C++条件变量
#include <ctype.h>            // 字符类型检查函数
#include <deque>              // C++双端队列容器
#include <errno.h>            // 错误码定义
#include <fcntl.h>            // 文件控制选项（open、fcntl等）
#include <forward_list>       // C++单向链表容器
#include <fstream>            // C++文件流操作
#include <iostream>           // C++输入输出流
#include <limits.h>           // 数值类型极限值定义
#include <list>               // C++双向链表容器
#include <locale.h>           // 本地化设置
#include <map>                // C++有序映射容器
#include <math.h>             // 数学函数库
#include <memory>             // C++智能指针库
#include <mutex>              // C++互斥锁类
#include <queue>              // C++队列容器
#include <signal.h>           // 信号处理机制
#include <stdarg.h>           // 可变参数处理
#include <stdio.h>            // C标准输入输出库
#include <stdlib.h>           // C标准库（内存分配、随机数等）
#include <string.h>           // C字符串处理函数
#include <string>             // C++字符串类
#include <sys/stat.h>         // 文件状态信息
#include <sys/types.h>        // 基本系统数据类型
#include <thread>             // C++线程类
#include <time.h>             // C时间处理函数
#include <unordered_map>      // C++无序映射容器
#include <vector>             // C++动态数组容器

#ifdef __linux__
#include <arpa/inet.h>    // IP地址转换函数
#include <dirent.h>       // 目录操作函数
#include <netdb.h>        // 网络数据库操作
#include <netinet/in.h>   // 网络地址结构
#include <poll.h>         // I/O多路复用
#include <pthread.h>      // POSIX线程库
#include <semaphore.h>    // POSIX信号量
#include <strings.h>      // 字符串处理扩展函数
#include <sys/epoll.h>    // 高性能I/O多路复用
#include <sys/ipc.h>      // IPC键生成
#include <sys/sem.h>      // System V信号量
#include <sys/shm.h>      // 共享内存操作
#include <sys/signalfd.h> // 信号转换为文件描述符
#include <sys/socket.h>   // 套接字编程基础
#include <sys/time.h>     // 时间获取与设置
#include <sys/timerfd.h>  // 定时器文件描述符
#include <termios.h>      // 终端I/O控制
#include <unistd.h>       // Unix标准系统调用
#include <utime.h>        // 文件时间戳操作
#endif                    // __linux__

#endif // !__OL_PUBLIC_H