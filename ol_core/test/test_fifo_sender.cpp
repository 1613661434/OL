/*
 * 程序名：test_fifo_sender.cpp，命名管道的发送端
 * 作者：ol
 * 功能：创建命名管道并向其写入数据（仅支持Linux）
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

#define FIFO_PATH "/tmp/my_fifo" // 命名管道的路径
#define BUFFER_SIZE 1024         // 缓冲区大小

// # 主函数
int main()
{
    // 1. 创建命名管道
    // 0666表示读写权限，S_IFIFO表示创建FIFO文件
    if (mkfifo(FIFO_PATH, 0666) == -1)
    {
        perror("mkfifo() failed");
        exit(EXIT_FAILURE);
    }
    printf("命名管道创建成功: %s\n", FIFO_PATH);

    // 2. 打开命名管道（写方式）
    // 注意：如果没有进程以读方式打开，open会阻塞直到有读进程连接
    int fd = open(FIFO_PATH, O_WRONLY);
    if (fd == -1)
    {
        perror("open() failed");
        exit(EXIT_FAILURE);
    }
    printf("已打开命名管道，准备发送数据...\n");

    // 3. 向管道发送数据
    char buffer[BUFFER_SIZE];
    while (1)
    {
        printf("请输入要发送的消息(输入'quit'退出): ");
        fflush(stdout); // 刷新标准输出，确保提示信息显示

        // 读取用户输入
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL)
        {
            perror("fgets() failed");
            break;
        }

        // 移除输入中的换行符
        buffer[strcspn(buffer, "\n")] = '\0';

        // 检查是否退出
        if (strcmp(buffer, "quit") == 0)
        {
            break;
        }

        // 写入管道
        ssize_t bytes_written = write(fd, buffer, strlen(buffer) + 1);
        if (bytes_written == -1)
        {
            perror("write() failed");
            break;
        }
        printf("已发送 %zd 字节: %s\n", bytes_written, buffer);
    }

    // 4. 关闭并删除命名管道
    close(fd);
    unlink(FIFO_PATH); // 删除FIFO文件
    printf("通信结束，已删除命名管道\n");

    return 0;
}