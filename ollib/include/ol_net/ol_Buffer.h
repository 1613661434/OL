#ifndef OL_BUFFER_H
#define OL_BUFFER_H 1

#include <errno.h> // 用于错误码处理
#include <iostream>
#include <string.h>
#include <string>

#ifdef __linux__
#include <unistd.h>
#endif // __linux__

namespace ol
{

#ifdef __linux__
    class Buffer
    {
    private:
        std::string m_buf;    ///< 用于存放数据。
        const uint16_t m_sep; ///< 报文的分隔符：0-无分隔符(固定长度、视频会议)；1-四字节的报头；2-"\r\n\r\n"分隔符（http协议）。

    public:
        Buffer(uint16_t sep = 1);
        ~Buffer();

        void append(const char* data, size_t size);        // 把数据追加到m_buf中。
        void appendWithSep(const char* data, size_t size); // 把数据追加到m_buf中，附加报文头部4字节（报文长度）。
        inline void erase(size_t pos, size_t n)            // 从m_buf的pos开始，删除n个字节，pos从0开始。
        {
            m_buf.erase(pos, n);
        }

        size_t size(); // 返回m_buf的大小。

        const char* data(); // 返回m_buf的首地址。

        void clear(); // 清空m_buf。

        inline bool empty() const
        {
            return m_buf.empty();
        }

        bool pickMessage(std::string& s); // 从m_buf中拆分出一个报文，存放在s中，如果m_buf中没有报文，返回false。

        ssize_t recvFd(int fd); // 从fd读取数据到缓冲区（非阻塞模式）
    };
#endif // __linux__

} // namespace ol

#endif // !OL_BUFFER_H