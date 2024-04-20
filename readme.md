# WebServer
    用C++实现的webserver

## 功能
* 利用 I/O复用技术 Epoll+线程池 实现多线程的Reactor高并发模型；
* 利用 正则与状态机解析HTTP请求报文，可以解析的文件类型有html、png、mp4等；
* 利用 标准库容器封装char，实现自动增长的缓冲区；
* 实现 GET、POST方法的部分内容的解析，处理POST请求，实现计算功能；

## 环境要求
* Linux WSL2(Ubuntu 22.04.4 LTS)
* g++ 11.4.0

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

## 致谢
Linux高性能服务器编程，游双著。
参考牛客WebServer服务器项目。