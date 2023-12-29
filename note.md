# 1 日志系统
使用日志时,采用 C 语言风格的字符串打印日志

DEBUGLOG("create addr %s", addr->toString().c_str()); 结果为
```txt
[日志类型]  时间   线程   打印该日志的位置   输出信息
[DEBUG] [23-12-25 20:34:51.439552000]   [341207:341207] [/data/ai/Cplusplus/simple_rpc/testcases/test_tcp.cc: 11]  create addr 127.0.0.1:12347
```
其宏定义为如下代码:
```c++
#define DEBUGLOG(str, ...)                                                                                   \
    rocket::Logger::GetGlobalLogger()->pushLog((new rocket::LogEvent(rocket::LogLevel::Debug))->toString() + \
    "[" + std::string(__FILE__) + ": " + std::to_string(__LINE__) + "]\t" +                                  \
    rocket::formatString(str, ##__VA_ARGS__) + '\n');                                                        \
    rocket::Logger::GetGlobalLogger()->log(); 
```
# 1.1 Config

保存各配置信息,如日志级别 m_log_level
```c++
rocket::Config::SetGlobalConfig("../conf/rocket.xml");
```



# 1.1 日志类型

初始化全局的日志, g_logger
```c++
rocket::Logger::InitGlobalLogger();
```

GetGlobalLogger() 定义为 static 类型,所以全局都可以获得日志对象,并打印日志














# 2 TcpServer过程
建立连接：
```c++
rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 12347);
rocket::TcpServer tcp_server(addr);
tcp_server.start();
```
## 2.1 创建服务器地址
```c++
rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 12347);
```
IPNetAddr 在 net_addr.h 中,该类继承 NetAddr  
该类有三个构造函数,目的是以多种方式,得到ip地址和端口  

获取 地址信息后,即可通过以下代码连接
```c++
m_addr.sin_family = AF_INET; // 地址族，一般为 AF_INET,ipv4
m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
m_addr.sin_port = htons(m_port);  // 主机字节序(小端字节序) 转换为 网络字节序(大端字节序)
```

1. Least/Most Significant Byte
2. 主机字节序(小端字节序): 最低有效字节存储在最低地址处,最高有效字节存储在最高地址处
3. 网络字节序(大端字节序): 最高有效字节存储在最低地址处,最低有效字节存储在最高地址处
4. 使用网络编程时，比如设置套接字的端口号或者在数据包中指定某些数值时，可以确保数据以网络字节序的格式传输，以避免不同平台之间的字节序不一致导致的问题
5. ntohs 与 htons 作用相反
6. htons(host to network short), short 代表数据类型, 16位的整数

得到的 addr 是指向类对象的智能指针,这个对象含有 网络地址信息 和 一些相关的函数.

# 2.2 初始化服务器
```c++
rocket::TcpServer tcp_server(addr);
```









# 2 TcpClint 　过程






# 5
## 5.1 异步网络通信

异步网络通信是指在进行网络通信时，不会阻塞当前线程或进程，而是允许程序继续执行其他任务，同时在后台进行数据传输和处理。这种通信方式使得程序能够更高效地利用资源，特别是在需要处理多个连接或者大量数据时。

异步网络通信流程通常涉及以下关键点：

1. 非阻塞调用： 异步通信通常使用非阻塞调用，即发起一个网络请求后，不会立即等待响应返回。相反，它会立即返回并允许程序执行其他任务。

2. 回调机制： 异步通信通常使用回调函数或者异步回调的方式来处理操作完成后的响应或结果。当异步操作完成时，系统会调用预先注册的回调函数，以便程序继续处理返回的数据或执行后续操作。

3. 事件驱动： 异步通信常常基于事件驱动的编程模型。程序会监听网络事件或者特定的信号，当有事件发生时，会触发相应的回调函数来处理事件。

4. 多线程或事件循环： 在异步通信中，有时会使用多线程或事件循环来处理多个并发的请求。每个请求都可能在单独的线程中执行，或者通过事件循环来处理多个事件。

5. 异步 IO： 异步通信通常会使用异步 IO 操作，这允许程序在进行文件或网络读写操作时，不会阻塞主线程，而是在后台进行数据传输。

通过异步通信，程序能够更有效地利用系统资源，提高响应性能，特别是在需要同时处理多个连接或大量数据时。这种通信方式在现代网络应用中广泛使用，比如网络服务、实时数据传输等场景。

## 5.2 IO 复用

IO 复用是一种技术，允许一个进程能够监视多个文件描述符（通常是套接字）的 I/O 可读写状态。这样，一个进程就可以在等待多个 I/O 操作完成时进行阻塞，而不需要为每个文件描述符都创建一个独立的线程来进行阻塞式的 I/O 操作。

IO 复用的主要目的是提高程序的性能和效率，特别是在需要处理多个并发连接或者大量 I/O 操作时。

常见的 IO 复用技术包括：

1. select： select() 是 Unix 和类 Unix 系统中的一个系统调用，允许程序在一个或多个文件描述符上进行等待，直到其中一个或多个文件描述符就绪为止。

2. poll： poll() 是一种与 select() 类似的系统调用，也允许程序监视多个文件描述符的状态变化。

3. epoll： epoll() 是 Linux 特有的高效 I/O 复用机制，使用红黑树来存储文件描述符，可以监听大量的文件描述符并高效地处理事件。

4. kqueue： kqueue() 是 BSD 系统中的一种事件通知机制，类似于 epoll，可以监视大量文件描述符的状态变化。

这些 IO 复用技术允许一个进程同时监视多个文件描述符，并在这些描述符中有就绪事件发生时通知程序，从而避免了阻塞式的等待。通过 IO 复用，程序可以更高效地处理多个连接或者大量的 I/O 操作，提高系统的性能和并发处理能力。





