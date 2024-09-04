#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <errno.h>

#include "../buffer/buffer.hpp"

class HttpRequest
{
public:
    HttpRequest() { Init(); }
    ~HttpRequest() = default;

    enum PARSE_STATE
    {
        REQUEST_LINE, // 正在解析请求首行
        HEADERS,      // 头
        BODY,         // 体
        FINISH,       // 完成
    };

    enum HTTP_CODE
    {
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
    // 解析接收缓冲区的数据
    bool parse(Buffer &buff);

    // 获取解析结果的接口
    // 获取请求方法
    std::string method() const;
    // 获取请求URL
    std::string path() const;
    std::string &path();
    // 获取http版本号
    std::string version() const;
    // 是否保持长连接
    bool IsKeepAlive() const;

    std::unordered_map<std::string, int> Post_();

private:
    // 解析请求行
    bool ParseRequestLine_(const std::string &line);
    // 解析请求首部字段
    void ParseHeader_(const std::string &line);
    // 解析请求消息体
    void ParseBody_(const std::string &line);

    void ParsePath_();
    void ParsePost_();
    // 解析表单数据
    void ParseFromUrlencoded_();

    PARSE_STATE state_;
    std::string method_, path_, version_, body_;          // 请求方法、URL、版本号、消息体
    std::unordered_map<std::string, std::string> header_; // 请求头部字段
    std::unordered_map<std::string, int> post_;           // post请求表单数据

    static const std::unordered_set<std::string> DEFAULT_HTML;
};

#endif // HTTP_REQUEST_H
