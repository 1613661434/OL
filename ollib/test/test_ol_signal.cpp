#if !defined(__linux__)
#error "仅支持Linux平台，不支持当前系统！"
#endif

#include "ol_signal.h"
#include <fcntl.h>
#include <iostream>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace ol;
using namespace std;

// 检查文件描述符是否有效
bool isFdValid(int fd)
{
    return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}

int main()
{
    // 注册信号处理函数用于对比
    signal(SIGINT, [](int s)
           { cout << "意外捕获信号 " << s << "\n"; });
    signal(SIGTERM, [](int s)
           { cout << "意外捕获信号 " << s << "\n"; });

    // 测试1：信号忽略功能
    cout << "\n=== 信号测试 ===" << "\n";
    cout << "当前进程ID: " << getpid() << "\n";
    cout << "请尝试 Ctrl+C 或 kill " << getpid() << " (5秒)" << "\n";
    ignoreSignalsCloseIO(false);
    sleep(5);
    cout << "信号测试结束（未退出即成功）" << "\n";

    // 测试2：I/O关闭功能（子进程中执行）
    cout << "\n=== I/O关闭测试 ===" << "\n";
    int pipefd[2];
    pipe(pipefd); // 创建管道用于子进程状态传递

    pid_t pid = fork();
    if (pid == 0)
    {
        close(pipefd[0]); // 关闭子进程读端
        ignoreSignalsCloseIO(true);

        // 检查三个标准FD的状态并写入管道
        bool fd0 = isFdValid(0); // stdin
        bool fd1 = isFdValid(1); // stdout
        bool fd2 = isFdValid(2); // stderr
        write(pipefd[1], &fd0, sizeof(fd0));
        write(pipefd[1], &fd1, sizeof(fd1));
        write(pipefd[1], &fd2, sizeof(fd2));

        close(pipefd[1]);
        _exit(0);
    }
    else if (pid > 0)
    {
        close(pipefd[1]); // 关闭父进程写端
        waitpid(pid, nullptr, 0);

        // 从管道读取子进程的FD状态
        bool fd0, fd1, fd2;
        read(pipefd[0], &fd0, sizeof(fd0));
        read(pipefd[0], &fd1, sizeof(fd1));
        read(pipefd[0], &fd2, sizeof(fd2));
        close(pipefd[0]);

        // 输出验证结果
        cout << "stdin (0) 是否关闭: " << (fd0 ? "否" : "是") << "\n";
        cout << "stdout (1) 是否关闭: " << (fd1 ? "否" : "是") << "\n";
        cout << "stderr (2) 是否关闭: " << (fd2 ? "否" : "是") << "\n";
    }
    else
    {
        cerr << "fork失败: " << strerror(errno) << "\n";
        return 1;
    }

    cout << "\n所有测试完成" << "\n";
    return 0;
}