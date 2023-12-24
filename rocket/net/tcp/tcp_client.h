/*
TcpClient

connetc => write => read => connect
非阻塞 Connect
Connect: 链接对端(peer)机器
Write: 将 RPC 响应发送给客户端
Read: 读取客户端发来的数据没,组装为 RPC 请求

返回 0, 表示连接成功
返回 -1,但 errno = EINPROGRESS, 表示连接正在建立,此时可以添加到 epoll 中去监听其可写事件,
等待可写事件就绪后,调用 getsockopt 获取 fd 上的错误,错误为 0 代表连接建立成功.
其他 errno 直接报错

*/