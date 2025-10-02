/****************************************************************************************/
/*
 * 程序名：ol_signal.h
 * 功能描述：信号处理工具类，提供信号忽略和I/O关闭功能，支持以下特性：
 *          - 统一处理常见信号（忽略或捕获）
 *          - 可选关闭标准输入输出流（防止程序异常输出）
 *          - 仅支持Linux平台（依赖特定系统调用）
 * 作者：ol
 * 适用标准：C++11及以上（需支持Linux信号机制）
 */
/****************************************************************************************/

#ifndef OL_SIGNAL_H
#define OL_SIGNAL_H 1

#include "ol_fstream.h"
#include <signal.h>

#ifdef __linux__
#include <unistd.h>
#endif // __linux__

namespace ol
{

#ifdef __linux__
    /**
     * @brief 忽略常见信号并可选关闭标准I/O流
     * @param bcloseio 是否关闭标准输入输出错误流（默认false-不关闭）
     * @note 1. 忽略的信号包括SIGINT、SIGTERM、SIGHUP等常见终止信号
     *       2. 若bcloseio为true，将关闭stdin、stdout、stderr（文件描述符0、1、2）
     *       3. 适用于后台服务程序，防止意外终止或输出干扰
     */
    void ignoreSignalsCloseIO(bool bcloseio = false);
#endif // __linux__

} // namespace ol

#endif // !OL_SIGNAL_H