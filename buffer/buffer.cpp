#include "buffer.hpp"

Buffer::Buffer(int initBuffSize) : buffer_(initBuffSize), readPos_(0), writePos_(0) {}

// 返回可以写入缓冲区的字节数
size_t Buffer::WritableBytes() const
{
    return buffer_.size() - writePos_; 
}

// 返回可以从缓冲区读取的字节数
size_t Buffer::ReadableBytes() const
{
    return writePos_ - readPos_; 
}

// 读取位置之前可用的空间
size_t Buffer::PrependableBytes() const
{ 
    return readPos_;
}

// 返回当前读取位置的数据指针(地址)
const char *Buffer::Peek() const
{ 
    return BeginPtr_() + readPos_;
}

// 确保缓冲区中有足够的空间来写入给定数量的字节数
void Buffer::EnsureWriteable(size_t len)
{ 
    if (WritableBytes() < len)
    {
        MakeSpace_(len); // 对容器的操作（扩容 或者 拷贝移动数据）
    }
    assert(WritableBytes() >= len);
}

// 向缓冲区写入数据后，更新写入位置
void Buffer::HasWritten(size_t len)
{ 
    writePos_ += len;
}

// 读取给定字节数，并更新读取位置
void Buffer::Retrieve(size_t len)
{ 
    assert(len <= ReadableBytes());
    readPos_ += len;
}

// 将读取位置移动到特定位置
void Buffer::RetrieveUntil(const char *end)
{ 
    assert(Peek() <= end);
    Retrieve(end - Peek()); // 移动读指针
}

// 重置缓冲区，初始化读写位置
void Buffer::RetrieveAll()
{ 
    bzero(&buffer_[0], buffer_.size());
    readPos_ = 0;
    writePos_ = 0;
}

// 返回缓冲区可写区域开头的指针const版
const char *Buffer::BeginWriteConst() const
{ 
    return BeginPtr_() + writePos_;
}

char *Buffer::BeginWrite()
{
    return BeginPtr_() + writePos_;
}

// 将数据追加到缓冲区
void Buffer::Append(const std::string &str)
{ 
    Append(str.data(), str.length());
}

void Buffer::Append(const char *str, size_t len)
{
    assert(str);
    EnsureWriteable(len); // 这里的len是临时数组中的数据个数
    std::copy(str, str + len, BeginWrite());
    HasWritten(len);
}

// 从文件描述符中读取数据，并存放到缓冲区中
ssize_t Buffer::ReadFd(int fd, int *saveErrno)
{
    char buff[65536];    // 临时数组
    struct iovec iov[2]; // 定义了一个向量元素  分散的内存
    const size_t writable = WritableBytes();
    /* 分散读， 保证数据全部读完 */
    iov[0].iov_base = BeginPtr_() + writePos_;
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);
    const ssize_t len = readv(fd, iov, 2);
    if (len < 0)
    {
        *saveErrno = errno;
    }
    else if (static_cast<size_t>(len) <= writable)
    {
        writePos_ += len;
    }
    else
    {
        writePos_ = buffer_.size();
        Append(buff, len - writable); // 把剩下的数据做处理（继续放在当前容器里或者是扩容）
    }

    return len;
}

// ssize_t Buffer::WriteFd(int fd, int* saveErrno){
// }
// 获取缓冲区开始的地址
char *Buffer::BeginPtr_()
{
    return &*buffer_.begin();
}

const char *Buffer::BeginPtr_() const
{
    return &*buffer_.begin();
}

// 保证缓冲区空间够用
void Buffer::MakeSpace_(size_t len)
{
    if (WritableBytes() + PrependableBytes() < len)
    { // 剩余可写的大小 加 前面可用的空间(已经读取过的缓存) 小于 临时数组中的长度
        buffer_.resize(writePos_ + len + 1);
    }
    else
    { // 可以装len长度的数据 就直接将后面的数据拷贝到前面
        size_t readable = ReadableBytes();
        std::copy(BeginPtr_() + readPos_, BeginPtr_() + writePos_, BeginPtr_());
        readPos_ = 0;
        writePos_ = readPos_ + readable;
        assert(readable == ReadableBytes());
    }
}

/*
    功能：将数据从文件描述符读到分散的内存块中，即分散读
    ssize_t readv(int fd, const struct iovec* iov,int iovcnt);
    参数：   fd     文件描述符
            iov     指向iovec结构体数组的指针
            iovcnt  指定iovec的个数
    返回值： 成功时返回读写的总字节数，失败时返回-1，并设置相应的errno
*/
/*
     功能: 将多块分散的内存的内存数据一并写入到文件描述符中，即集中写
     ssize_t writev(int fd,const struct iovec* iov,int iovcnt);
     参数：   fd     文件描述符
             iov     指向iovec结构体数组的指针
             iovcnt  指定iovec的个数
     返回值： 成功时返回写入的总字节数，失败时返回-1，并设置相应的errno
*/
/*
     #include<sys/uio.h>
     struct iovec{
         void* iov_base;         //缓冲区首地址
         size_t iov_len;         //缓冲区长度
     };
*/