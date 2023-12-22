#include <string.h>
#include <vector>

#include "tcp_buffer.h"
#include "../../common/log.h"

namespace rocket {
TcpBuffer::TcpBuffer(int size)
    : m_size(size) {
    m_buffer.resize(size);
}
TcpBuffer::~TcpBuffer() {
}

// 返回可读字节数
int TcpBuffer::readAble() {
    return m_write_index - m_read_index;
}

// 返回可写的字节数
int TcpBuffer::writeAble() {
    return m_buffer.size() - m_write_index;
}

int TcpBuffer::readIndex() {
    return m_read_index;
}

int TcpBuffer::writeIndex() {
    return m_write_index;
}

void TcpBuffer::writeIndex(const char* buf, int size) {
    if (size > writeAble()) {
        // 要写入的尺寸比可写入的尺寸大,无法写入,需要调整扩容
        int new_size = (int)(1.5 * (m_write_index + size));
        resizeBuffer(new_size);
    }
    // void *memcpy(void*dest, const void *src, size_t n);
    // 由src指向地址为起始地址的连续n个字节的数据复制到以destin指向地址为起始地址的空间内
    // 与strcpy相比，memcpy并不是遇到'\0'就结束，而是一定会拷贝完n个字节
    memcpy(&m_buffer[m_write_index], buf, size);
}

void TcpBuffer::resizeBuffer(int new_size) {
    std::vector<char> tmp(new_size);
    int count = std::min(new_size, readAble());

    memcpy(&tmp[0], &m_buffer[m_read_index], count);
    m_buffer.swap(tmp);

    m_read_index = 0;
    m_write_index = m_read_index + count;
}

void TcpBuffer::readFromBuffer(std::vector<char>& re, int size) {
    if (readAble() == 0) {
        return;
    }
    int read_size = readAble() > size ? size : readAble();

    std::vector<char> tmp(read_size);
    memcpy(&tmp[0], &m_buffer[m_read_index], read_size);

    re.swap(tmp);
    m_read_index += read_size;

    adjustBuffer();
}

void TcpBuffer::adjustBuffer() {
    if (m_read_index < int(m_buffer.size() / 3)) {
        // 前面已读的占用1/3,就平移调整
        return;
    }
    std::vector<char> buffer(m_buffer.size());
    int count = readAble();

    memcpy(&buffer[0], &buffer[m_read_index], count);
    m_buffer.swap(buffer);
    m_read_index = 0;
    m_write_index = m_read_index + count;
    buffer.clear();
}

void TcpBuffer::moveReadIndex(int size) {
    size_t j = m_read_index + size;
    if (j >= m_buffer.size()) {
        ERRORLOG("moveReadIndex error, invalid size %d, old_read_index %d, buffer size %d", size, m_read_index, m_buffer.size());
        return;
    }
    m_read_index = j;
    adjustBuffer();
}

void TcpBuffer::moveWriteIndex(int size) {
    size_t j = m_write_index + size;
    if (j >= m_buffer.size()) {
        ERRORLOG("moveWriteIndex error, invalid size %d, old_read_index %d, buffer size %d", size, m_write_index, m_buffer.size());
        return;
    }
    m_write_index = j;
    adjustBuffer();
}
}  // namespace rocket