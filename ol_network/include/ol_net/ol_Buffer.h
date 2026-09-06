#ifndef OL_BUFFER_H
#define OL_BUFFER_H 1

#include <errno.h> // 用于错误码处理
#include <cstdint>
#include <iostream>
#include <string>

#ifdef __unix__
#include <unistd.h>
#endif // __unix__

namespace ol
{

#ifdef __unix__
    class Buffer
    {
    public:
        static constexpr size_t DEFAULT_MAX_FRAME_SIZE = 64U * 1024U * 1024U;

    private:
        std::string m_buf;          ///< 用于存放数据。
        size_t m_readPos = 0;       ///< 已消费数据的结束位置，避免频繁从字符串头部擦除。
        const uint16_t m_sep;       ///< 报文的分隔符：0-无分隔符；1-四字节网络序报头；2-"\r\n\r\n"。
        const size_t m_maxFrameSize; ///< 单个报文允许的最大字节数。

        void consume(size_t size);
        void compact();

    public:
        explicit Buffer(uint16_t sep = 1, size_t maxFrameSize = DEFAULT_MAX_FRAME_SIZE);
        ~Buffer() = default;

        void append(const char* data, size_t size);        // 把数据追加到m_buf中。
        void appendWithSep(const char* data, size_t size); // 追加数据；sep=1时附加uint32网络字节序长度。
        void erase(size_t pos, size_t n);                  // 从尚未消费的数据中删除指定字节。

        size_t size() const; // 返回尚未消费的数据大小。

        const char* data() const; // 返回尚未消费数据的首地址。

        void clear(); // 清空m_buf。

        inline bool empty() const
        {
            return size() == 0;
        }

        bool pickMessage(std::string& s); // 从m_buf中拆分出一个报文，存放在s中，如果m_buf中没有报文，返回false。

        ssize_t recvFd(int fd); // 从fd读取一个数据块；返回值语义与read()一致。
    };
#endif // __unix__

} // namespace ol

#endif // !OL_BUFFER_H
