/*
为什么需要应用层 buffer
1 方便数据处理, 特别是应用层的包组装和拆解
2 方便异步的发送,(发送数据直接塞到发送缓冲区里面,等待epoll异步去发送)
3 提高发送效率,多个包合并一起发送

readIndex                                writeIndex
   (a)   b   c   d   e   f   g   h   i   j   (-)   -   -   -   -

读取 3 个字节,即 a b c

             readIndex                    writeIndex
    a   b   c   (d)   e   f   g   h   i   j   (-)   -   -   -   -

将占用空间的读过的字节回收,进行调整, adjustBuffer

 readIndex                    writeIndex
    (d)   e   f   g   h   i   j   (-)   -   -   -   -   -   -   -

InBuffer 
1 服务端调用 read 成功从 socket 缓冲区读到数据,会写入到 InBuffer 后面
2 服务端从 InBuffer 前面读取数据,进行解码得到请求

OutBuffer
1 服务端向外发送数据,会将数据编码后写入到 OutBuffer 后面
2 服务端在 fd 可写的情况下,调用 write 将 OutBuffer 里面的数据全部发送出去

*/


#ifndef ROCKET_NET_TCP_TCP_BUFFER_H
#define ROCKET_NET_TCP_TCP_BUFFER_H

#include <vector>

namespace rocket {
class TcpBuffer {
   public:
    typedef std::shared_ptr<TcpBuffer> s_ptr;

    TcpBuffer(int size);
    ~TcpBuffer();

    // 返回可读字节数
    int readAble();

    // 返回可写的字节数
    int writeAble();

    int readIndex();

    int writeIndex();

    void writeIndex(const char* buf, int size);

    void writeToBuffer(const char* buf, int size);

    void readFromBuffer(std::vector<char>& re, int size);

    void resizeBuffer(int new_size);

    void adjustBuffer();

    void moveReadIndex(int size);

    void moveWriteIndex(int size);

   public:
    std::vector<char> m_buffer;

   private:
    int m_read_index{0};
    int m_write_index{0};
    int m_size{0};
};
}  // namespace rocket

#endif