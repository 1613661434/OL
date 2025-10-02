/*****************************************************************************************/
/*
 * 程序名：ol_public.h
 * 功能描述：开发框架公用头文件，整合了应用开发常用的自定义工具类和系统头文件，特性包括：
 *          - 包含框架内所有自定义工具类（时间、队列、文件操作、网络、IPC等）
 *          - 整合C++标准库核心头文件（容器、线程、智能指针等）
 *          - 按平台（Linux）包含系统特定头文件（网络、进程、I/O等）
 *          - 简化应用开发的头文件引用，统一依赖管理
 * 作者：ol
 * 适用标准：C++11及以上（需支持线程、原子操作等特性）
 */
/*****************************************************************************************/

#ifndef OL_PUBLIC_H
#define OL_PUBLIC_H 1

// 自定义工具类头文件
#include "ol_chrono.h"  // 时间处理工具（日期、时间戳、计时器等）
#include "ol_cqueue.h"  // 循环队列实现（固定大小、FIFO操作）
#include "ol_fstream.h" // 文件流操作工具（目录、文件读写、日志等）
#include "ol_ftp.h"     // FTP客户端工具（文件上传、下载、目录操作等）
#include "ol_ipc.h"     // 进程间通信工具（信号量、共享内存、进程心跳等）
#include "ol_signal.h"  // 信号处理工具（信号捕获、自定义处理逻辑等）
#include "ol_string.h"  // 字符串处理工具（格式化、分割、转换等）
#include "ol_tcp.h"     // TCP网络编程工具（客户端、服务器通信等）

// C++标准库头文件
#include <algorithm>          // 算法库（排序、查找、变换等）
#include <atomic>             // 原子操作库（线程安全的变量操作）
#include <condition_variable> // 条件变量（线程同步机制）
#include <ctype.h>            // 字符类型检查（大小写、数字判断等）
#include <deque>              // 双端队列容器（高效首尾操作）
#include <errno.h>            // 错误码定义（系统调用错误信息）
#include <fcntl.h>            // 文件控制选项（open、fcntl函数参数）
#include <forward_list>       // 单向链表容器（高效插入删除）
#include <fstream>            // 文件流操作（C++风格文件读写）
#include <iostream>           // 输入输出流（cout、cin等）
#include <limits.h>           // 数值类型极限值（INT_MAX、LLONG_MIN等）
#include <list>               // 双向链表容器（任意位置高效操作）
#include <locale.h>           // 本地化设置（字符编码、地区格式等）
#include <map>                // 有序映射容器（键值对，自动排序）
#include <math.h>             // 数学函数库（三角函数、幂运算等）
#include <memory>             // 智能指针库（unique_ptr、shared_ptr等）
#include <mutex>              // 互斥锁类（线程同步，防止竞态条件）
#include <queue>              // 队列容器（FIFO数据结构）
#include <signal.h>           // 信号处理机制（系统信号捕获）
#include <stdarg.h>           // 可变参数处理（printf风格函数实现）
#include <stdio.h>            // C标准输入输出（printf、fopen等）
#include <stdlib.h>           // C标准库（内存分配、随机数、程序控制等）
#include <string.h>           // C字符串处理（strcpy、strlen等）
#include <string>             // C++字符串类（std::string及操作）
#include <sys/stat.h>         // 文件状态信息（stat函数、文件权限等）
#include <sys/types.h>        // 基本系统数据类型（pid_t、size_t等）
#include <thread>             // C++线程类（创建、管理线程）
#include <time.h>             // C时间处理（时间戳、本地时间等）
#include <unordered_map>      // 无序映射容器（哈希表实现，O(1)查找）
#include <vector>             // 动态数组容器（高效随机访问）

// Linux平台特定头文件
#ifdef __linux__
#include <arpa/inet.h>    // IP地址转换（inet_addr、inet_ntoa等）
#include <dirent.h>       // 目录操作（opendir、readdir等）
#include <netdb.h>        // 网络数据库（主机名解析、服务查询等）
#include <netinet/in.h>   // 网络地址结构（sockaddr_in等）
#include <poll.h>         // I/O多路复用（poll函数）
#include <pthread.h>      // POSIX线程库（兼容C风格线程操作）
#include <semaphore.h>    // POSIX信号量（线程/进程同步）
#include <strings.h>      // 字符串处理扩展（bzero、strcasecmp等）
#include <sys/epoll.h>    // 高性能I/O多路复用（epoll系列函数）
#include <sys/ipc.h>      // IPC键生成（ftok函数）
#include <sys/sem.h>      // System V信号量（进程间同步）
#include <sys/shm.h>      // 共享内存操作（进程间数据共享）
#include <sys/signalfd.h> // 信号转换为文件描述符（统一I/O处理）
#include <sys/socket.h>   // 套接字编程基础（socket、bind等）
#include <sys/time.h>     // 时间获取与设置（gettimeofday等）
#include <sys/timerfd.h>  // 定时器文件描述符（高精度定时）
#include <termios.h>      // 终端I/O控制（串口通信、终端设置等）
#include <unistd.h>       // Unix标准系统调用（read、write、close等）
#include <utime.h>        // 文件时间戳操作（修改访问/修改时间）
#endif                    // __linux__

#endif // !OL_PUBLIC_H