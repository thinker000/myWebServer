#ifndef LOG_H_
#define LOG_H_
/*
    目的：自己实现一个日志功能
        日志系统结构：
            1.日志事件 LogEvent
                负责封装一次日志事件的具体信息
            2.日志级别 LogLevel
                用于表示不同级别的日志，常见的级别有DEBUG/INFO/WARN/ERROR/FATAL 
            3.日志器 Logger
                负责管理日志的生成和输出，协调日志事件、日志级别和日志输出目标
            4.日志格式化器 LogFormatter
                负责将日志事件转换成特定格式的字符串
            5.日志输出目标 LogAppender
                负责将格式化后的日志输出到不同目标，控制台/文件
    功能：

*/

#include <iostream>
#include <string>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <vector>

// 日志事件
class LogEvent
{
public:
    using ptr = std::shared_ptr<LogEvent>;
    // typedef std::shared_ptr<LogEvent> ptr;
    LogEvent();

private:
    const char *m_file = nullptr; // 文件名
    int32_t m_line;               // 行号
    uint32_t m_threadId;          // 线程id
    uint32_t m_fiberId;           // 协程id  保留，目前只用用到了线程
    uint32_t m_elapse;            // 程序启动开始到当前的毫秒数
    uint64_t m_time;              // 时间戳
    /*
    线程id
    
    */
    std::string m_content;
};

// 日志级别
class LogLevel
{
public:
    enum Level
    {
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };
};

// 日志格式化
class LogFormatter
{
public:
    using ptr = std::shared_ptr<LogFormatter>;
    LogFormatter(const std::string &pattern);
    std::string format(LogEvent::ptr event);
    std::ostream &format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);

public:
    // 日志内容项格式化
    class FormatItem
    {
    public:
        using ptr = std::shared_ptr<FormatItem>;
        virtual ~FormatItem() {}
        virtual void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    };

public:
    void init();                          // 初始化，解析日志模板
    bool isError() const;                 // 是否有错误
    const std::string getPattern() const; // 返回日志模板

private:
    std::string m_pattern;                // 日志格式模板
    std::vector<FormatItem::ptr> m_items; // 日志格式解析后格式
    bool m_error = false;                 // 是否有错误
};

// 日志输出目标
class LogAppender
{
public:
    using ptr = std::shared_ptr<LogAppender>;
    virtual ~LogAppender() {}
    virtual void log(LogLevel::Level level, LogEvent::ptr event) = 0;
    void setFormatter(LogFormatter::ptr val);
    LogFormatter::ptr getFormatt() const;

protected:
    LogLevel::Level m_level;
    LogFormatter::ptr m_formatter;
};

// 日志器
class Logger
{
public:
    using ptr = std::shared_ptr<Logger>;
    Logger(const std::string &name = "root");
    void log(LogLevel::Level level, LogEvent::ptr event);

    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);

    LogLevel::Level getLevel() const;
    void setLevel(LogLevel::Level val);

private:
    std::string m_anme;                     // 日志名称
    LogLevel::Level m_level;                // 日志级别
    std::list<LogAppender::ptr> m_appender; // Appender集合
};

// 输出到控制台的Appender
class StdoutLogAppender : public LogAppender
{
public:
    using ptr = std::shared_ptr<StdoutLogAppender>;
    virtual void log(LogLevel::Level level, LogEvent::ptr event) override;

private:
};

// 输出到文件的Appender
class FileLogAppender : public LogAppender
{
public:
    using ptr = std::shared_ptr<FileLogAppender>;
    FileLogAppender(const std::string &fileName);
    virtual void log(LogLevel::Level level, LogEvent::ptr event) override;
    bool reOpen(); // 重新打开文件
private:
    std::string m_fileName;
    std::ofstream m_fileStream;
};

#endif