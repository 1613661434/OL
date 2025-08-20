/*
 *  程序名：test_shm.cpp，此程序演示Linux共享内存的使用。
 *  作者：ol
 *  功能：通过共享内存实现进程间数据共享，存储并更新一个包含编号和姓名的结构体数据
 */

#if !defined(__linux__)
#error "仅支持Linux平台，不支持当前系统！"
#endif

#include <cstdlib>   // 包含atoi等函数
#include <cstring>   // 包含字符串操作函数（如strcpy）
#include <iostream>  // 包含标准输入输出流库
#include <sys/ipc.h> // 包含IPC（进程间通信）相关函数声明
#include <sys/shm.h> // 包含共享内存相关函数声明

using namespace std; // 使用标准命名空间std

// 定义共享内存中存储的数据结构
// 用于存储女孩的编号和姓名信息
struct Sgirl
{
    int id;        // 编号
    char name[51]; // 姓名，最多50个字符(预留1个字节给字符串结束符)
};

// 主函数
int main(int argc, char* argv[])
{
    // 检查命令行参数是否正确
    // 程序需要接收两个参数：编号(整数)和姓名(字符串)
    if (argc != 3)
    {
        cerr << "Using ./test_shm 3 西施\n"; // 输出正确的使用方法
        return -1;                           // 参数错误，程序退出
    }

    // 1. 创建或获取共享内存
    // shmget()函数用于创建或获取共享内存
    // 参数1：0x5005 是共享内存的键值(key)，用于标识共享内存
    // 参数2：sizeof(Sgirl) 指定共享内存的大小，这里是结构体Sgirl的大小
    // 参数3：0640 | IPC_CREAT 权限设置，0640表示读写权限，IPC_CREAT表示如果不存在则创建
    int shmid = shmget(0x5005, sizeof(Sgirl), 0640 | IPC_CREAT);
    if (shmid == -1) // 判断共享内存创建/获取是否成功
    {
        cerr << "shmget() fail\n"; // 输出错误信息
        return -1;                 // 操作失败，程序退出
    }

    // 2. 将共享内存连接到当前进程的地址空间
    // shmat()函数用于将共享内存附加到进程地址空间
    // 参数1：shmid 是共享内存的ID，即shmget()的返回值
    // 参数2：0 表示让系统自动选择附加地址
    // 参数3：0 表示默认权限
    // 返回值：指向共享内存的指针，需要强制转换为Sgirl*类型
    Sgirl* sgptr = (Sgirl*)shmat(shmid, 0, 0);
    if (sgptr == (void*)-1) // 判断共享内存附加是否成功
    {
        cerr << "shmat() fail\n"; // 输出错误信息
        return -1;                // 操作失败，程序退出
    }

    // 3. 操作共享内存中的数据
    // 先输出共享内存中已有的旧值
    cout << "旧值：编号-" << sgptr->id << ",姓名-" << sgptr->name << "\n";
    // 将命令行参数中的新值写入共享内存
    sgptr->id = atoi(argv[1]);    // 将第一个参数(字符串)转换为整数，存入id
    strcpy(sgptr->name, argv[2]); // 将第二个参数(字符串)复制到name
    // 输出更新后的新值
    cout << "新值：编号-" << sgptr->id << ",姓名-" << sgptr->name << "\n";

    // 4. 将共享内存与当前进程分离
    // shmdt()函数用于将共享内存从进程地址空间中分离
    // 参数：sgptr 是共享内存的指针
    shmdt(sgptr);

    // 5. 删除共享内存（当前注释掉，表示共享内存会保留，供其他进程使用）
    // 注意：共享内存是内核级别的资源，不会随进程结束而自动释放
    // 如果需要删除，取消下面的注释
    // if(shmctl(shmid,IPC_RMID,0)==-1){
    //   cerr << "shmctl(IPC_RMID) fail\n";  // 输出错误信息
    //   return -1;                          // 操作失败，程序退出
    // }

    return 0; // 程序正常结束
}