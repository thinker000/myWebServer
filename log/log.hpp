#ifndef LOG_H_
#define LOG_H_

#include <iostream>
#include <string>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>


// 前置声明
class Logger;

// 日志事件
class LogEvent
{
public:
    using ptr = std::shared_ptr<LogEvent>;

    LogEvent(const char *file, int32_t line, std::thread::id threadId, const std::string &content);

    const char *getFile() const;
    int32_t getLine() const;
    std::thread::id getThreadId() const;
    uint64_t getTime() const;
    const std::string &getContent() const;

private:
    const char *m_file;    // 文件名
    int32_t m_line;        // 行号
    std::thread::id m_threadId;   // 线程id
    uint64_t m_time;       // 时间戳
    std::string m_content; // 日志内容
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

    static std::string toString(Level level);
};

// 日志格式化
class LogFormatter
{
public:
    using ptr = std::shared_ptr<LogFormatter>;
    LogFormatter(const std::string &pattern);

    std::string format(LogEvent::ptr event);

    class FormatItem
    {
    public:
        using ptr = std::shared_ptr<FormatItem>;
        virtual ~FormatItem() {}
        virtual void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    };
    // 日志格式化项实现
    class MessageFormatItem : public LogFormatter::FormatItem
    {
    public:
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getContent();
        }
    };

    class LevelFormatItem : public LogFormatter::FormatItem
    {
    public:
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << LogLevel::toString(level);
        }
    };

    class ThreadIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getThreadId();
        }
    };

    void init(); // 解析日志模板

private:
    std::string m_pattern;                // 日志格式模板
    std::vector<FormatItem::ptr> m_items; // 日志格式解析后的格式化项
};

// 日志输出目标 基类
class LogAppender
{
public:
    using ptr = std::shared_ptr<LogAppender>;
    virtual ~LogAppender() {}
    virtual void log(LogLevel::Level level, LogEvent::ptr event) = 0;

    void setFormatter(LogFormatter::ptr val);
    LogFormatter::ptr getFormatter() const;

protected:
    LogFormatter::ptr m_formatter;
};

// 输出到控制台的Appender
class StdoutLogAppender : public LogAppender
{
public:
    using ptr = std::shared_ptr<StdoutLogAppender>;
    void log(LogLevel::Level level, LogEvent::ptr event) override;
};

// 输出到文件的Appender
class FileLogAppender : public LogAppender
{
public:
    using ptr = std::shared_ptr<FileLogAppender>;
    FileLogAppender(const std::string &fileName);

    void log(LogLevel::Level level, LogEvent::ptr event) override;
    bool reOpen();

private:
    std::string m_fileName;
    std::ofstream m_fileStream;
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
    std::string m_name;
    LogLevel::Level m_level;
    std::list<LogAppender::ptr> m_appenders;
};

#endif // LOG_H_
