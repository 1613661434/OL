/*****************************************************************************************/
/*
 * 程序名：ol_net_public.h
 * 功能描述：网络库公用头文件
 * 作者：ol
 * 适用标准：C++17及以上
 */
/*****************************************************************************************/

#ifndef OL_NET_PUBLIC_H
#define OL_NET_PUBLIC_H 1

#ifdef __linux__
#include "ol_net/ol_net_fwd_decls.h"
#include "ol_net/ol_Buffer.h"
#include "ol_net/ol_InetAddr.h"
#include "ol_net/ol_Channel.h"
#include "ol_net/ol_SocketFd.h"
#include "ol_net/ol_Acceptor.h"
#include "ol_net/ol_Connection.h"
#include "ol_net/ol_EpollChnl.h"
#include "ol_net/ol_EpollFd.h"
#include "ol_net/ol_EventLoop.h"
#include "ol_net/ol_TcpServer.h"
#endif // __linux__

#endif // !OL_NET_PUBLIC_H