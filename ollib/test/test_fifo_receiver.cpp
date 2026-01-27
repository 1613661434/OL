/*
 * 程序名：test_fifo_receiver.cpp，命名管道的接收端
 * 作者：ol
 * 功能：从命名管道读取数据并显示（仅支持Linux）
 */

#if !defined(__unix__)
#error "仅支持Linux平台，不支持当前系统！"
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define FIFO_PATH "/tmp/my_fifo" // 必须与发送端使用相同的路径
#define BUFFER_SIZE 1024         // 缓冲区大小

// # 主函数
int main()
{
    // 1. 检查命名管道是否存在，不存在则创建
    struct stat st;
    if (stat(FIFO_PATH, &st) == -1)
    {
        // 如果不存在则创建
        if (mkfifo(FIFO_PATH, 0666) == -1)
        {
            perror("mkfifo() failed");
            exit(EXIT_FAILURE);
        }
        printf("创建命名管道: %s\n", FIFO_PATH);
    }
    else
    {
        printf("发现已存在的命名管道: %s\n", FIFO_PATH);
    }

    // 2. 打开命名管道（读方式）
    // 注意：如果没有进程以写方式打开，open会阻塞直到有写进程连接
    int fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1)
    {
        perror("open() failed");
        exit(EXIT_FAILURE);
    }
    printf("已打开命名管道，等待接收数据...\n");

    // 3. 从管道接收数据
    char buffer[BUFFER_SIZE];
    while (1)
    {
        // 读取数据
        ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE);
        if (bytes_read == -1)
        {
            perror("read() failed");
            break;
        }
        else if (bytes_read == 0)
        {
            // 写端关闭时会返回0
            printf("发送端已关闭连接\n");
            break;
        }

        // 检查是否是退出命令
        if (strcmp(buffer, "quit") == 0)
        {
            printf("收到退出命令\n");
            break;
        }

        printf("收到 %zd 字节: %s\n", bytes_read, buffer);
    }

    // 4. 关闭并删除命名管道
    close(fd);
    unlink(FIFO_PATH); // 删除FIFO文件
    printf("通信结束，已删除命名管道\n");

    return 0;
}