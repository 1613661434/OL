/*
 *  程序名：test_ol_tcp_ctcpclient_binary.cpp，此程序演示采用开发框架的ctcpclient类传输二进制数据（网络通讯的客户端）
 *  作者：ol
 */

#if !defined(__unix__)
#error "仅支持Linux平台，不支持当前系统！"
#endif

#include "ol_tcp.h"

using namespace ol;
using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("Using:./test_ol_tcp_ctcpclient_binary ip port\n");
        printf("Sample./test_ol_tcp_ctcpclient_binary 192.168.195.5 5005\n");
        return -1;
    }

    ctcpclient tcpclient;
    if (tcpclient.connect(argv[1], atoi(argv[2])) == false) // 向服务端发起连接请求。
    {
        printf("tcpclient.connect(%s,%s) failed.\n", argv[1], argv[2]);
        return -1;
    }

    struct st_girl // 超女结构体。
    {
        int bh;
        char name[31];
    } stgirl;

    string recvbuf;

    for (int ii = 0; ii < 10; ii++)
    {
        memset(&stgirl, 0, sizeof(stgirl));
        stgirl.bh = ii;
        sprintf(stgirl.name, "西施%d", ii);

        if (tcpclient.write(&stgirl, sizeof(stgirl)) == false) // 向服务端发送请求数据。
        {
            printf("tcpclient.write() failed.\n");
            break;
        }
        cout << sformat("发送：bh=%d,name=%s\n", stgirl.bh, stgirl.name);

        sleep(1);

        if (tcpclient.read(recvbuf) == false) // 接收服务端的回应报文。
        {
            printf("tcpclient.read() failed.\n");
            break;
        }
        cout << "接收：" << recvbuf << endl;
    }
}