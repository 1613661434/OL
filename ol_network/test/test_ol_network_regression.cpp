#include "ol_net/ol_Buffer.h"
#include "ol_net/ol_Connection.h"
#include "ol_net/ol_EventLoop.h"
#include "ol_net/ol_SocketFd.h"
#include "ol_tcp.h"

#include <arpa/inet.h>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <future>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <utility>

namespace
{
    using namespace std::chrono_literals;

    void require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }

    class SocketPair
    {
    public:
        int fd[2]{-1, -1};

        SocketPair()
        {
            if (::socketpair(AF_UNIX, SOCK_STREAM, 0, fd) != 0)
                throw std::runtime_error("socketpair failed");
        }

        ~SocketPair()
        {
            if (fd[0] >= 0) ::close(fd[0]);
            if (fd[1] >= 0) ::close(fd[1]);
        }

        SocketPair(const SocketPair&) = delete;
        SocketPair& operator=(const SocketPair&) = delete;
    };

    void testBufferProtocol()
    {
        ol::Buffer buffer(1, 8);
        buffer.appendWithSep("abc", 3);

        const unsigned char expectedHeader[] = {0, 0, 0, 3};
        require(buffer.size() == 7, "Buffer frame size is incorrect");
        require(std::memcmp(buffer.data(), expectedHeader, sizeof(expectedHeader)) == 0,
                "Buffer length header is not uint32 network byte order");

        std::string message;
        require(buffer.pickMessage(message), "Buffer did not produce a complete frame");
        require(message == "abc", "Buffer decoded payload is incorrect");
        require(buffer.empty(), "Buffer did not consume the decoded frame");

        ol::Buffer limited(1, 4);
        const uint32_t oversized = htonl(5);
        limited.append(reinterpret_cast<const char*>(&oversized), sizeof(oversized));

        bool rejected = false;
        try
        {
            limited.pickMessage(message);
        }
        catch (const std::length_error&)
        {
            rejected = true;
        }
        require(rejected, "Buffer accepted an oversized incoming frame");
    }

    void testTcpFrameProtocolAndLimit()
    {
        {
            SocketPair sockets;
            const std::string sent("hello\0world", 11);
            require(ol::tcpwrite(sockets.fd[0], sent, 1024), "tcpwrite failed");

            std::string received;
            require(ol::tcpread(sockets.fd[1], received, 1, 1024), "tcpread failed");
            require(received == sent, "TCP framed payload changed during transfer");
        }

        {
            SocketPair sockets;
            const uint32_t oversized = htonl(4096);
            require(::send(sockets.fd[0], &oversized, sizeof(oversized), 0) == sizeof(oversized),
                    "Could not send oversized frame header");

            std::string received;
            errno = 0;
            require(!ol::tcpread(sockets.fd[1], received, -1, 32),
                    "tcpread accepted an oversized frame");
            require(errno == EMSGSIZE, "tcpread did not report EMSGSIZE for oversized frame");
            require(received.empty(), "tcpread allocated payload storage for oversized frame");
        }
    }

    void testTcpPartialFrameTimeout()
    {
        SocketPair sockets;
        const uint32_t frameSize = htonl(4);
        require(::send(sockets.fd[0], &frameSize, sizeof(frameSize), 0) == sizeof(frameSize),
                "Could not send partial frame header");
        require(::send(sockets.fd[0], "x", 1, 0) == 1, "Could not send partial frame body");

        std::string received;
        const auto begin = std::chrono::steady_clock::now();
        errno = 0;
        require(!ol::tcpread(sockets.fd[1], received, 1, 16),
                "tcpread unexpectedly accepted a partial frame");
        const auto elapsed = std::chrono::steady_clock::now() - begin;

        require(errno == ETIMEDOUT, "Partial frame did not end with ETIMEDOUT");
        require(elapsed < 3s, "Partial frame ignored the total read deadline");
        require(received.empty(), "Failed partial frame left a partially initialized string");
    }

    void testEventLoopTaskReentry()
    {
        ol::EventLoop loop(true, 16, 60, 80);
        std::promise<void> nestedTaskDone;
        auto nestedTaskFuture = nestedTaskDone.get_future();
        std::thread runner([&loop]() { loop.run(-1); });

        loop.pushToQueue([&loop, &nestedTaskDone]() {
            loop.pushToQueue([&nestedTaskDone]() { nestedTaskDone.set_value(); });
        });

        if (nestedTaskFuture.wait_for(2s) != std::future_status::ready)
        {
            std::cerr << "EventLoop deadlocked while a task queued another task\n";
            std::_Exit(EXIT_FAILURE);
        }

        loop.stop();
        runner.join();
    }

    void testAsyncSendKeepsConnectionAlive()
    {
        SocketPair sockets;
        ol::EventLoop loop(true, 16, 60, 80);
        auto socketFd = std::make_unique<ol::SocketFd>(sockets.fd[0]);
        sockets.fd[0] = -1; // 所有权已经交给SocketFd。

        auto conn = std::make_shared<ol::Connection>(&loop, std::move(socketFd), 1024);
        conn->connectEstablished();
        std::weak_ptr<ol::Connection> weakConn = conn;

        conn->send("queued", 6);
        conn->disconnect();
        conn.reset();
        require(!weakConn.expired(), "Queued send did not retain the Connection");

        std::promise<void> queueDrained;
        auto queueDrainedFuture = queueDrained.get_future();
        loop.pushToQueue([&queueDrained]() { queueDrained.set_value(); });
        std::thread runner([&loop]() { loop.run(-1); });

        if (queueDrainedFuture.wait_for(2s) != std::future_status::ready)
        {
            std::cerr << "EventLoop did not drain the queued send\n";
            std::_Exit(EXIT_FAILURE);
        }

        loop.stop();
        runner.join();
        require(weakConn.expired(), "Queued send retained the Connection after execution");
    }

    void testConnectionEtSendDrainsOutput()
    {
        SocketPair sockets;
        const int flags = ::fcntl(sockets.fd[0], F_GETFL, 0);
        require(flags >= 0 && ::fcntl(sockets.fd[0], F_SETFL, flags | O_NONBLOCK) == 0,
                "Could not make Connection socket nonblocking");

        const int sendBufferSize = 4096;
        require(::setsockopt(sockets.fd[0], SOL_SOCKET, SO_SNDBUF,
                             &sendBufferSize, sizeof(sendBufferSize)) == 0,
                "Could not shrink socket send buffer");

        ol::EventLoop loop(true, 32, 60, 80);
        auto socketFd = std::make_unique<ol::SocketFd>(sockets.fd[0]);
        sockets.fd[0] = -1;
        auto conn = std::make_shared<ol::Connection>(&loop, std::move(socketFd), 1024 * 1024);

        std::promise<void> sendComplete;
        auto sendCompleteFuture = sendComplete.get_future();
        conn->setSendCompleteCb([&sendComplete](ol::Connection::Ptr) { sendComplete.set_value(); });
        loop.newConn(conn);
        conn->connectEstablished();

        std::thread runner([&loop]() { loop.run(-1); });
        const std::string sent(512 * 1024, 'E');
        auto receiver = std::async(std::launch::async, [fd = sockets.fd[1]]() {
            std::string received;
            const bool ok = ol::tcpread(fd, received, 5, 1024 * 1024);
            return std::make_pair(ok, std::move(received));
        });
        conn->send(sent.data(), sent.size());

        if (receiver.wait_for(6s) != std::future_status::ready)
        {
            std::cerr << "Connection ET send did not drain its output buffer\n";
            std::_Exit(EXIT_FAILURE);
        }
        auto receiveResult = receiver.get();
        const bool completed = sendCompleteFuture.wait_for(2s) == std::future_status::ready;

        loop.stop();
        runner.join();
        conn->disconnect();
        loop.closeConn(conn);

        require(receiveResult.first, "Peer failed to receive the Connection frame");
        require(receiveResult.second == sent, "Connection ET send lost or changed payload bytes");
        require(completed, "Connection did not report completed output");
    }

    void testTimerFdIsDrained()
    {
        ol::EventLoop loop(true, 16, 1, 80);
        std::atomic<unsigned int> epollTimeouts{0};
        loop.setEpollTimeoutCb([&epollTimeouts](ol::EventLoop*) {
            epollTimeouts.fetch_add(1, std::memory_order_relaxed);
        });

        std::thread runner([&loop]() { loop.run(50); });
        std::this_thread::sleep_for(1300ms);
        const unsigned int before = epollTimeouts.load(std::memory_order_relaxed);
        std::this_thread::sleep_for(400ms);
        const unsigned int after = epollTimeouts.load(std::memory_order_relaxed);

        loop.stop();
        runner.join();
        require(after > before, "timerfd remained readable and forced EventLoop into a busy loop");
    }
} // namespace

int main()
{
    try
    {
        testBufferProtocol();
        testTcpFrameProtocolAndLimit();
        testTcpPartialFrameTimeout();
        testEventLoopTaskReentry();
        testAsyncSendKeepsConnectionAlive();
        testConnectionEtSendDrainsOutput();
        testTimerFdIsDrained();
        std::cout << "network regression tests passed\n";
        return EXIT_SUCCESS;
    }
    catch (const std::exception& error)
    {
        std::cerr << "network regression test failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
