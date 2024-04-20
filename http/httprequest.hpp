#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <errno.h> 

#include "../buffer/buffer.hpp"

class HttpRequest {
public:
    HttpRequest() { Init(); }
    ~HttpRequest() = default;

    enum PARSE_STATE {
        REQUEST_LINE,   //正在解析请求首行
        HEADERS,        //头
        BODY,           //体
        FINISH,         //完成     
    };

    enum HTTP_CODE {
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };

    void Init();

    bool parse(Buffer& buff);

    //解析结果的部分封装
    std::string method() const;
    std::string path() const;
    std::string& path();   
    std::string version() const;

    bool IsKeepAlive() const; //是否保持 长连接

    std::unordered_map<std::string, int> Post_();

private:
    bool ParseRequestLine_(const std::string& line); 
    void ParseHeader_(const std::string& line); 
    void ParseBody_(const std::string& line); 

    void ParsePath_(); 
    void ParsePost_(); 
    void ParseFromUrlencoded_(); //解析表单数据
       
    PARSE_STATE state_; 
    std::string method_, path_, version_, body_; 
    std::unordered_map<std::string, std::string> header_; 
    std::unordered_map<std::string, int> post_; //post请求表单数据

    static const std::unordered_set<std::string> DEFAULT_HTML; 
};


#endif //HTTP_REQUEST_H
