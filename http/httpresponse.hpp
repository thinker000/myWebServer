#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <unordered_map>
#include <fcntl.h>       // open
#include <unistd.h>      // close
#include <sys/stat.h>    // stat
#include <sys/mman.h>    // mmap, munmap

#include "../buffer/buffer.hpp"

class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();

    void Init(const std::string& srcDir, std::string& path, std::unordered_map<std::string, int> post_, bool isKeepAlive = false, int code = -1);
    void MakeResponse(Buffer& buff);
    void UnmapFile(); //解除内存映射
    char* File();    //获得文件映射指针(指向起始位置)
    size_t FileLen() const;
    void ErrorContent(Buffer& buff, std::string message);
    int Code() const { return code_; }

    void AddPostContent_(Buffer& buff);

private:
    void AddStateLine_(Buffer &buff);
    void AddHeader_(Buffer &buff);
    void AddContent_(Buffer &buff);

    void ErrorHtml_();
    std::string GetFileType_();

    int code_; //响应状态码
    bool isKeepAlive_; //是否保持连接 （长连接）

    std::string path_; 
    std::string srcDir_; 
    
    char* mmFile_; //文件内存映射的指针，文件映射到进程地址空间的起始地址
    struct stat mmFileStat_; //文件的元数据(文件的状态信息)

    std::unordered_map<std::string, int> post__; //post请求表单数据

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE; 
    static const std::unordered_map<int, std::string> CODE_STATUS; 
    static const std::unordered_map<int, std::string> CODE_PATH; 
};


#endif //HTTP_RESPONSE_H