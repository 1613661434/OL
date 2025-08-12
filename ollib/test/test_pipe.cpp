/*
 *  程序名：test_pipe.cpp，此程序演示Linux管道的使用。
 *  作者：ol
 *  功能：通过管道实现父子进程间的通信，父进程向子进程发送消息
 */
#include <stdio.h>  // 标准输入输出库
#include <string.h> // 字符串操作库
#include <unistd.h> // 包含pipe、fork、read、write等系统调用
#include <wait.h>   // 包含wait函数，用于等待子进程结束

// 主函数
int main()
{
    int fd[2]; // 管道文件描述符数组，fd[0]为读端，fd[1]为写端

    // 创建匿名管道
    // pipe()函数成功返回0，失败返回-1
    int ret = pipe(fd);
    if (ret == -1)
    {
        perror("pipe() failed."); // 输出错误信息
        return -1;
    }

    // 创建子进程
    pid_t pid = fork();

    if (pid < 0) // fork失败的情况
    {
        perror("fork() failed."); // 输出错误信息
        // 失败时关闭管道
        close(fd[0]);
        close(fd[1]);
        return -1;
    }
    else if (pid == 0) // 子进程执行逻辑
    {
        // 子进程只需要读端，先关闭写端（重要！）
        close(fd[1]);

        char buffer[1024] = {0}; // 用于存储读取到的数据

        // 从管道读端读取数据
        if (read(fd[0], buffer, sizeof(buffer) - 1) == -1)
        {
            perror("read() failed."); // 输出错误信息
            close(fd[0]);             // 出错时关闭读端
            return -1;
        }

        // 打印读取到的内容
        printf("The content is :%s\n", buffer);

        // 读取完成后关闭读端
        close(fd[0]);
        return 0;
    }
    else // 父进程执行逻辑
    {
        // 父进程只需要写端，先关闭读端（重要！）
        close(fd[0]);

        char buffer[] = "success!"; // 要发送的字符串

        // 向管道写端写入数据
        if (write(fd[1], buffer, strlen(buffer)) == -1)
        {
            perror("write() failed."); // 输出错误信息
            close(fd[1]);              // 出错时关闭写端
            return -1;
        }

        // 写入完成后关闭写端（触发子进程read返回0）
        close(fd[1]);

        wait(NULL); // 等待子进程结束，避免僵尸进程
        return 0;
    }

    return 0;
}