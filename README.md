# myWebServer
# WebServer
    基于Linux平台用C++实现的webserver
## 环境要求
* Linux WSL2(Ubuntu 22.04.4 LTS)
* g++ 11.4.0
* C++17

## 目录树
```
.
├── bin
│   └── server
├── buffer
│   ├── buffer.cpp
│   └── buffer.hpp
├── build
│   └── Makefile
├── http
│   ├── httpconn.cpp
│   ├── httpconn.hpp
│   ├── httprequest.cpp
│   ├── httprequest.hpp
│   ├── httpresponse.cpp
│   └── httpresponse.hpp
├── resources
│   ├── CGI
│   │   ├── compute.cgi
│   │   ├── compute.cpp
│   │   └── compute_.html
│   ├── compute.html
│   ├── error.html
│   ├── index.html
│   ├── picture
│   │   ├── home.png
│   │   └── img.jpg
│   ├── picture.html
│   ├── video
│   │   └── mp4.mp4
│   └── video.html
├── main.cpp
├── Makefile
├── readme.md
├── threadpool
│   ├── threadpool.cpp
│   ├── threadpool.hpp
└── server
    ├── epoller.cpp
    ├── epoller.h
    ├── readme.md
    ├── threadpool.h
    ├── webserver.cpp
    └── webserver.h
```

## 项目启动
```bash
make
./bin/server
```

## 功能
* 利用 I/O复用技术 Epoll+线程池 实现多线程的Reactor高并发模型；
* 利用 正则与状态机解析HTTP请求报文，可以解析的文件类型有html、png、mp4等；
* 利用 标准库容器封装char，实现自动增长的缓冲区；
* 实现 GET、POST方法的部分内容的解析，处理POST请求，实现计算功能；

#### Nginx实现反向代理和负载均衡
1. 反向代理
2. 负载均衡  
- 为了本地单机实现Nginx负载均衡和反向代理，修改程序的入口参数来指定程序运行监听的端口号。


#### 线程池的工作原理
1. 线程池的创建
2. 线程池的管理
3. 线程池的销毁
- 线程池的创建：在服务器初始化时，创建线程池，并根据系统的配置来指定线程池中的线程数量
- 线程池的管理：在服务器启动的同时开启线程池，因此需要在线程池开始工作之前的管理操作有线程池的工作模式、线程池的最大线程数量、任务队列的最大容量等
    工作模式：固定线程数量模式、动态数量模式
    固定线程数量工作模式：在线程池开始工作之前，创建固定数量的线程；而动态数量模式，可以根据任务量的大小和系统状态来动态的进行增加线程数量
    线程池在开始工作之前先创建指定数量的线程数量：因为创建和销毁线程也具有一定的开销，包括线程栈的创建和释放，在高并发场景下频繁的创建和释放线程影响系统的整体效率。
- 线程池的销毁：使用unique_ptr智能指针来管理线程池对象，当程序退出时，唤醒所有的线程，释放线程池资源。


#### 自增缓冲区
- 实现输入缓冲和输出缓冲
- 通过读指针和写指针来确定读取的位置和写入的位置，以及以及缓冲区中剩余可用空间的大小，如果剩余空间不够就进行扩容，够用就将剩余未读取的数据移动到缓冲区的最前面，接着填入新的数据。
- 定义了从文件描述符中读取数据并写入到缓冲区的接口，通过分散读的写法，保证将指定文件描述中的数据全部读完，并读取到缓冲区。


#### http连接

##### http请求
1. 从缓冲区中读取数据并进行解析，主要接口：解析请求行、请求头部字段、消息体（POSt方法时）
   - 采用有限状态机模式进行完成解析。

##### http响应

## 致谢
Linux高性能服务器编程，游双著。
参考牛客WebServer服务器项目。