#include "ol_net/ol_Buffer.h"

#include <algorithm>
#include <arpa/inet.h>
#include <array>
#include <cerrno>
#include <cstring>
#include <limits>
#include <stdexcept>

// #define OL_DEBUG

namespace ol
{

#ifdef __unix__
    Buffer::Buffer(uint16_t sep, size_t maxFrameSize)
        : m_sep(sep), m_maxFrameSize(maxFrameSize)
    {
        if (m_sep > 2) throw std::invalid_argument("Unsupported message separator type");
        if (m_maxFrameSize == 0) throw std::invalid_argument("Maximum frame size must be greater than zero");
    }

    // 把数据追加到m_buf中。
    void Buffer::append(const char* data, size_t size)
    {
        if (data == nullptr && size != 0) throw std::invalid_argument("Buffer data must not be null");
        if (size == 0) return;
        compact();
        m_buf.append(data, size);
    }

    // 把数据追加到m_buf中，附加报文头部4字节（报文长度）。
    void Buffer::appendWithSep(const char* data, size_t size)
    {
        if (data == nullptr && size != 0) throw std::invalid_argument("Buffer data must not be null");
        if (size > m_maxFrameSize || size > std::numeric_limits<uint32_t>::max())
            throw std::length_error("Message exceeds the maximum frame size");

        compact();
        if (m_sep == 0) // 没有分隔符。
        {
            if (size != 0) m_buf.append(data, size); // 处理报文内容。
        }
        else if (m_sep == 1) // 四字节的报头。
        {
            const uint32_t frameSize = htonl(static_cast<uint32_t>(size));
            m_buf.append(reinterpret_cast<const char*>(&frameSize), sizeof(frameSize));
            if (size != 0) m_buf.append(data, size);
        }
        else if (m_sep == 2) // "\r\n\r\n"分隔符：数据后追加分隔符
        {
            if (size != 0) m_buf.append(data, size);
            m_buf.append("\r\n\r\n", 4); // 添加分隔符
        }
    }

    // 返回m_buf的大小。
    size_t Buffer::size() const
    {
        return m_buf.size() - m_readPos;
    }

    // 返回m_buf的首地址。
    const char* Buffer::data() const
    {
        return m_buf.data() + m_readPos;
    }

    // 清空m_buf。
    void Buffer::clear()
    {
        m_buf.clear();
        m_readPos = 0;
    }

    void Buffer::consume(size_t size)
    {
        if (size > this->size()) throw std::out_of_range("Cannot consume beyond buffer size");
        m_readPos += size;
        compact();
    }

    void Buffer::compact()
    {
        if (m_readPos == 0) return;
        if (m_readPos == m_buf.size())
        {
            clear();
            return;
        }

        const size_t unread = m_buf.size() - m_readPos;
        if (m_readPos >= 4096 && m_readPos >= unread)
        {
            m_buf.erase(0, m_readPos);
            m_readPos = 0;
        }
    }

    void Buffer::erase(size_t pos, size_t n)
    {
        if (pos > size()) throw std::out_of_range("Buffer erase position is out of range");
        if (pos == 0)
        {
            consume(std::min(n, size()));
            return;
        }

        m_buf.erase(m_readPos + pos, n);
    }

    // 从m_buf中拆分出一个报文，存放在s中，如果m_buf中没有报文，返回false。
    bool Buffer::pickMessage(std::string& s)
    {
        if (empty()) return false;

        if (m_sep == 0) // 无分隔符：整个缓冲区视为一个报文
        {
            if (size() > m_maxFrameSize) throw std::length_error("Message exceeds the maximum frame size");
            s.assign(data(), size());
            clear();
            return true;
        }
        else if (m_sep == 1) // 四字节报头：先读长度，再读数据
        {
            if (size() < sizeof(uint32_t)) return false;

            uint32_t encodedLength = 0;
            std::memcpy(&encodedLength, data(), sizeof(encodedLength));
            const size_t len = ntohl(encodedLength);
            if (len > m_maxFrameSize) throw std::length_error("Incoming frame exceeds the maximum frame size");

            if (size() - sizeof(uint32_t) < len) return false;

            s.assign(data() + sizeof(uint32_t), len);
            consume(sizeof(uint32_t) + len);
            return true;
        }
        else if (m_sep == 2) // "\r\n\r\n"分隔符：查找分隔符位置
        {
            const std::string sep = "\r\n\r\n";
            const size_t sep_pos = m_buf.find(sep, m_readPos);
            if (sep_pos == std::string::npos)
            {
                if (size() > m_maxFrameSize && size() - m_maxFrameSize >= sep.size())
                    throw std::length_error("Incoming frame exceeds the maximum frame size");
                return false;
            }

            const size_t payloadSize = sep_pos - m_readPos;
            if (payloadSize > m_maxFrameSize)
                throw std::length_error("Incoming frame exceeds the maximum frame size");

            const size_t messageSize = payloadSize + sep.size();
            s.assign(data(), messageSize);
            consume(messageSize);
            return true;
        }

        return false;
    }

    // 从fd读取一个数据块。ET模式由调用方循环到EAGAIN，并在每次读取后及时拆包。
    ssize_t Buffer::recvFd(int fd)
    {
        std::array<char, 16 * 1024> chunk;
        while (true)
        {
            const ssize_t nread = ::read(fd, chunk.data(), chunk.size());
            if (nread > 0)
            {
                append(chunk.data(), static_cast<size_t>(nread));
                return nread;
            }
            if (nread < 0 && errno == EINTR) continue;
            return nread;
        }
    }
#endif // __unix__

} // namespace ol
