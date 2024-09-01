#include "httpresponse.hpp"

using namespace std;

//后缀类型  MIMEType
const unordered_map<string, string> HttpResponse::SUFFIX_TYPE = {
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".mp3",   "audio/mp3" },
    { ".mp4",   "audio/mp4" },
    { ".avi",   "video/x-msvideo" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "}
};

const unordered_map<int, string> HttpResponse::CODE_STATUS = {
    { 200, "OK" },          //成功处理请求
    { 400, "Bad Request" }, //无法理解客户端请求
    { 403, "Forbidden" },   //没有权限
    { 404, "Not Found" }    //为找到请求的资源
};

const unordered_map<int, string> HttpResponse::CODE_PATH = {
    { 400, "/error.html" },
    { 403, "/error.html" },
    { 404, "/error.html" }
};


HttpResponse::HttpResponse() {
    code_ = -1;
    path_ = srcDir_ = "";
    isKeepAlive_ = false;
    mmFile_ = nullptr; 
    mmFileStat_ = { 0 };
}

HttpResponse::~HttpResponse() {
    UnmapFile();
}

void HttpResponse::Init(const string& srcDir, string& path, std::unordered_map<std::string, int> post_, bool isKeepAlive, int code){
    assert(srcDir != "");
    if(mmFile_) { UnmapFile(); }  //解除内存映射

    code_ = code; //响应状态码
    isKeepAlive_ = isKeepAlive;
    path_ = path;
    srcDir_ = srcDir; //当前的工作路径
    mmFile_ = nullptr; 
    mmFileStat_ = { 0 };
    post__ = post_;
}

void HttpResponse::MakeResponse(Buffer& buff) {
    /* 判断请求的资源文件 */
    //拼接得到资源的路径
    //stat(const char* pathname,struct stat* buf):获得文件的元数据
    //pathname:指定文件的路径
    //buf:用来存储文件元数据的空间地址
    //return : 成功返回 0 ，失败返回 -1  error被重置
    //S_ISDIR()宏，判断指定路径是否是目录
    if(stat((srcDir_ + path_).data(), &mmFileStat_) < 0 || S_ISDIR(mmFileStat_.st_mode)) {
        code_ = 404; //访问的是目录
    }
    else if(!(mmFileStat_.st_mode & S_IROTH)) { //判断权限
        code_ = 403; //没有权限
    }
    else if(code_ == -1) { //默认是-1
        code_ = 200; 
    }
    ErrorHtml_();  //封装错误状态显示
    AddStateLine_(buff); 
    AddHeader_(buff); 
    AddContent_(buff); 
    cout<<"封装响应完成！"<<endl;
}

void HttpResponse::UnmapFile() {  
    if(mmFile_) {
        munmap(mmFile_, mmFileStat_.st_size);  //解除响应文件的内存映射
        mmFile_ = nullptr;
    }
}

char* HttpResponse::File() {
    return mmFile_;
}

size_t HttpResponse::FileLen() const {
    return mmFileStat_.st_size;
}

void HttpResponse::ErrorContent(Buffer& buff, string message) 
{
    string body;
    string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if(CODE_STATUS.count(code_) == 1) {
        status = CODE_STATUS.find(code_)->second;
    } else {
        status = "Bad Request";
    }
    body += to_string(code_) + " : " + status  + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>TinyWebServer</em></body></html>";

    buff.Append("Content-length: " + to_string(body.size()) + "\r\n\r\n");
    buff.Append(body);
}

//响应首行
void HttpResponse::AddStateLine_(Buffer& buff) {
    string status;
    if(CODE_STATUS.count(code_) == 1) { //存在
        status = CODE_STATUS.find(code_)->second; //哈希表的值是状态
    }
    else {
        code_ = 400; //错误状态码
        status = CODE_STATUS.find(400)->second;
    }
    buff.Append("HTTP/1.1 " + to_string(code_) + " " + status + "\r\n");
}

//响应头
void HttpResponse::AddHeader_(Buffer& buff) {
    buff.Append("Connection: ");
    if(isKeepAlive_) { 
        buff.Append("keep-alive\r\n");
        buff.Append("keep-alive: max=6, timeout=120\r\n");
    } else{
        buff.Append("close\r\n");
    }
    buff.Append("Content-type: " + GetFileType_() + "\r\n"); //文件类型
    buff.Append("charset: utf-8\r\n");
}

//响应内容
void HttpResponse::AddContent_(Buffer& buff) {
    int srcFd = open((srcDir_ + path_).data(), O_RDONLY); //打开资源   //.data() string的成员函数，返回指向该字符串首字符的指针
    if(srcFd < 0) { 
        ErrorContent(buff, "File NotFound!");
        return; 
    }

    //POST
    if(path_ == "/CGI/compute_.html"){
        AddPostContent_(buff);
        return;
    }

    /* 将文件映射到进程的虚拟地址空间，提高文件的访问速度 
        MAP_PRIVATE 建立一个写入时拷贝的私有映射*/
    //LOG_DEBUG("file path %s", (srcDir_ + path_).data());
    cout<<"file path "<<(srcDir_ + path_).data()<<endl;
    void* mmRet =mmap(nullptr, mmFileStat_.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0); //mmap将一个文件或对象映射到进程的虚拟地址空间
    if(mmRet == MAP_FAILED) {
        ErrorContent(buff, "File NotFound!");
        return; 
    }
    mmFile_ = (char*)mmRet; //指针,映射区域的起始地址

    close(srcFd);
    buff.Append("Content-length: " + to_string(mmFileStat_.st_size) + "\r\n\r\n"); //响应的数据长度大小
}

void HttpResponse::ErrorHtml_() {
    if(CODE_PATH.count(code_) == 1) { 
        path_ = CODE_PATH.find(code_)->second;//哈希表的值是资源名
        stat((srcDir_ + path_).data(), &mmFileStat_);
    }
}

//获取文件类型 获取后缀
string HttpResponse::GetFileType_() {
    /* 判断文件类型 */
    string::size_type idx = path_.find_last_of('.');   //返回最后一个查找到.的位置
    if(idx == string::npos) {
        return "text/plain";
    }
    string suffix = path_.substr(idx);             //返回子串，从第一参数开始，第二个参数指定子串的长度，默认为std::string::npos字符串尾指示器
    if(SUFFIX_TYPE.count(suffix) == 1) {
        return SUFFIX_TYPE.find(suffix)->second;
    }

    return "text/plain";
}

void HttpResponse::AddPostContent_(Buffer& buff){
    int a, b;
    a = post__["a"];
    b = post__["b"];

    int sum = a + b;
    string body;
    body += "<html><head><title>CGI</title></head>";
    body += "<body><p>The result is " + to_string(a) + "+" + to_string(b) + " = " + to_string(sum);
    body += "</p></body></html>";

    buff.Append("Content-length: " + to_string(body.size()) + "\r\n\r\n");
    buff.Append(body);
}