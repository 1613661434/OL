/*
 *  程序名：test_ol_ipcANDol_cqueue-o-PCM_P.cpp，此程序演示生产者-消费者模型的生产者。
 *  作者：ol
 */
#include "ol_public.h"
#include <iostream>

using namespace ol;
using namespace std;

struct Sgirl
{
    int id;
    char name[51];

    Sgirl(int i, char* n)
    {
        id = i;
        strcpy(name, n);
    }
};

int main()
{
    using shmtype = cqueue<Sgirl, 5>;

    // 1.获取共享内存id
    int shmid = shmget(0x5005, sizeof(shmtype), 0640 | IPC_CREAT);
    if (shmid == -1)
    {
        cerr << "shmget() fail\n";
        return -1;
    }

    // 2.将共享内存连接到当前进程的地址空间
    shmtype* CQptr = (shmtype*)shmat(shmid, 0, 0);
    if (CQptr == (void*)-1)
    {
        cerr << "shmat() fail\n";
        return -1;
    }

    // 3.循环队列初始化
    CQptr->init();

    // 4.设置信号量
    csemp mutex; // 互斥锁
    mutex.init(0x5001);

    csemp hav_ele; // 资源数
    hav_ele.init(0x5002, 0, 0);

    csemp hav_emp; // 空位数
    hav_emp.init(0x5003, 5, 0);

    // 5.空位数-1
    hav_emp.wait();

    // 6.上锁
    mutex.wait();

    // 7.生产数据1
    CQptr->push(Sgirl(1, "西施"));

    // 8.开锁
    mutex.post();

    // 9.资源数+1
    hav_ele.post();

    // 10.脱离共享内存
    shmdt(CQptr);

    // 11.删除共享内存
    //  if(shmctl(shmid,IPC_RMID,0)==-1){
    //  	cerr << "shmctl(IPC_RMID) fail\n";
    //  	return -1;
    //  }

    return 0;
}