#ifndef BUFFER_H
#define BUFFER_H

#include <cstring>  
#include <iostream>
#include <unistd.h>  
#include <sys/uio.h> 
#include <vector> 
#include <atomic>
#include <assert.h>

class Buffer {
public:
    // 缓冲区初始大小
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default; 
    // 可写的字节数
    size_t WritableBytes() const;       
    // 剩余可读的字节数    
    size_t ReadableBytes() const ;      
    // 前面可以用的字节数，即前面的内容已经处理
    size_t PrependableBytes() const;    
    // 获取当前读取的位置指针
    const char* Peek() const;
    // 确保缓冲区空间够用，不够时调用扩容接口进行扩容
    void EnsureWriteable(size_t len);
    // 更新写入的位置
    void HasWritten(size_t len);
    // 读取指定字节数
    void Retrieve(size_t len);
    // 读取到指定位置
    void RetrieveUntil(const char* end);
    // 重置缓冲区，并初始化读写位置
    void RetrieveAll() ; 
    // 获取可写的区域首地址
    const char* BeginWriteConst() const;
    char* BeginWrite();
    // 向缓冲区中追加内容
    void Append(const std::string& str);
    void Append(const char* str, size_t len);
    // 从文件描述符中读取数据，并存放到缓冲区中
    ssize_t ReadFd(int fd, int* Errno);
    //ssize_t WriteFd(int fd, int* Errno);

private:
    char* BeginPtr_(); 
    const char* BeginPtr_() const;
    void MakeSpace_(size_t len); 

    std::vector<char> buffer_; //具体装数据的vector
    std::atomic<std::size_t> readPos_; 
    std::atomic<std::size_t> writePos_; 
};

#endif //BUFFER_H