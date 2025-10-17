#include "ol_net/ol_Buffer.h"

// #define DEBUG

namespace ol
{

#ifdef __linux__
    Buffer::Buffer(uint16_t sep) : m_sep(sep)
    {
    }

    Buffer::~Buffer()
    {
    }

    // 把数据追加到m_buf中。
    void Buffer::append(const char* data, size_t size)
    {
        m_buf.append(data, size);
    }

    // 把数据追加到m_buf中，附加报文头部4字节（报文长度）。
    void Buffer::appendWithSep(const char* data, size_t size)
    {
        if (m_sep == 0) // 没有分隔符。
        {
            m_buf.append(data, size); // 处理报文内容。
        }
        else if (m_sep == 1) // 四字节的报头。
        {
            m_buf.append((char*)&size, 4); // 处理报文长度（头部）。
            m_buf.append(data, size);      // 处理报文内容。
        }
        else if (m_sep == 2) // "\r\n\r\n"分隔符：数据后追加分隔符
        {
            m_buf.append(data, size);
            m_buf.append("\r\n\r\n", 4); // 添加分隔符
        }
    }

    // 返回m_buf的大小。
    size_t Buffer::size()
    {
        return m_buf.size();
    }

    // 返回m_buf的首地址。
    const char* Buffer::data()
    {
        return m_buf.data();
    }

    // 清空m_buf。
    void Buffer::clear()
    {
        m_buf.clear();
    }

    // 从m_buf中拆分出一个报文，存放在s中，如果m_buf中没有报文，返回false。
    bool Buffer::pickMessage(std::string& s)
    {
        if (m_buf.empty()) return false;

        if (m_sep == 0) // 无分隔符：整个缓冲区视为一个报文
        {
            s = std::move(m_buf); // 移动数据，避免拷贝
            m_buf.clear();
            return true;
        }
        else if (m_sep == 1) // 四字节报头：先读长度，再读数据
        {
            if (m_buf.size() < 4) return false; // 不足4字节，无法获取长度

            // 读取长度（主机字节序）
            size_t len;
            memcpy(&len, m_buf.data(), 4);

            // 检查总长度是否足够（4字节头部 + 数据长度）
            if (m_buf.size() < 4 + len) return false;

            // 提取数据
            s = m_buf.substr(4, len);
            // 移除已提取的部分（头部+数据）
            m_buf.erase(0, 4 + len);
            return true;
        }
        else if (m_sep == 2) // "\r\n\r\n"分隔符：查找分隔符位置
        {
            const std::string sep = "\r\n\r\n";
            size_t sep_pos = m_buf.find(sep);
            if (sep_pos == std::string::npos) return false; // 未找到分隔符

            // 提取分隔符前的数据（包含分隔符本身）
            s = m_buf.substr(0, sep_pos + sep.size());
            // 移除已提取的部分
            m_buf.erase(0, sep_pos + sep.size());
            return true;
        }

        return false;
    }

    // 从fd读取数据直接写入m_buf（无临时缓冲区）
    ssize_t Buffer::recvFd(int fd)
    {
        constexpr size_t READ_CHUNK = 4096;
        ssize_t nread_total = 0;

        while (true)
        {
            size_t current_size = m_buf.size();
            // 1. 预留空间并扩展可写长度
            m_buf.reserve(current_size + READ_CHUNK);
            m_buf.resize(current_size + READ_CHUNK); // 确保后续写入的内存可写

            // 2. 获取可写地址
            char* write_ptr = &m_buf[current_size];

            // 3. 读取数据
            ssize_t nread = ::read(fd, write_ptr, READ_CHUNK);

#ifdef DEBUG
            printf("recvFd(%d): current_size=%zu, nread=%ld\n", fd, current_size, nread);
#endif

            if (nread > 0)
            {
                // 只保留实际读取的长度（截断多余的预留空间）
                m_buf.resize(current_size + nread);
                nread_total += nread;
            }
            else if (nread == 0)
            {
                m_buf.resize(current_size); // 恢复到实际长度
                return nread_total;
            }
            else // nread == -1
            {
                m_buf.resize(current_size); // 恢复到实际长度
                if (errno == EINTR)
                    continue;
                else if (errno == EAGAIN || errno == EWOULDBLOCK)
                    return nread_total;
                else
                    return -1;
            }
        }
    }
#endif // __linux__

} // namespace ol