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






