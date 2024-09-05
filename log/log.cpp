#include "log.hpp"

// 日志事件实现
LogEvent::LogEvent(const char* file, int32_t line, std::thread::id threadId, const std::string& content)
    : m_file(file), m_line(line), m_threadId(threadId), m_content(content) {
    m_time = std::chrono::system_clock::now().time_since_epoch().count();
}

const char* LogEvent::getFile() const { return m_file; }
int32_t LogEvent::getLine() const { return m_line; }
std::thread::id LogEvent::getThreadId() const { return m_threadId; }
uint64_t LogEvent::getTime() const { return m_time; }
const std::string& LogEvent::getContent() const { return m_content; }

// 日志级别实现
std::string LogLevel::toString(Level level) {
    switch(level) {
        case DEBUG: return "DEBUG";
        case INFO: return "INFO";
        case WARN: return "WARN";
        case ERROR: return "ERROR";
        case FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

// 日志格式化实现
LogFormatter::LogFormatter(const std::string &pattern)
    : m_pattern(pattern) {
    init();
}

void LogFormatter::init() {
    // 此处可以实现解析 pattern 的逻辑
    m_items.push_back(std::make_shared<MessageFormatItem>());
    m_items.push_back(std::make_shared<LevelFormatItem>());
    m_items.push_back(std::make_shared<ThreadIdFormatItem>());
}

std::string LogFormatter::format(LogEvent::ptr event) {
    std::stringstream ss;
    for (auto &item : m_items) {
        item->format(ss, nullptr, LogLevel::DEBUG, event);
    }
    return ss.str();
}



// LogAppender 基类实现
void LogAppender::setFormatter(LogFormatter::ptr val) { m_formatter = val; }
LogFormatter::ptr LogAppender::getFormatter() const { return m_formatter; }

// 控制台Appender实现
void StdoutLogAppender::log(LogLevel::Level level, LogEvent::ptr event) {
    if (m_formatter) {
        std::cout << m_formatter->format(event) << std::endl;
    }
}

// 文件Appender实现
FileLogAppender::FileLogAppender(const std::string &fileName) : m_fileName(fileName) {
    reOpen();
}

void FileLogAppender::log(LogLevel::Level level, LogEvent::ptr event) {
    if (m_formatter) {
        m_fileStream << m_formatter->format(event) << std::endl;
    }
}

bool FileLogAppender::reOpen() {
    if (m_fileStream.is_open()) {
        m_fileStream.close();
    }
    m_fileStream.open(m_fileName);
    return m_fileStream.is_open();
}

// Logger 实现
Logger::Logger(const std::string &name) : m_name(name), m_level(LogLevel::DEBUG) {}

void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        for (auto &appender : m_appenders) {
            appender->log(level, event);
        }
    }
}

void Logger::debug(LogEvent::ptr event) { log(LogLevel::DEBUG, event); }
void Logger::info(LogEvent::ptr event) { log(LogLevel::INFO, event); }
void Logger::warn(LogEvent::ptr event) { log(LogLevel::WARN, event); }
void Logger::error(LogEvent::ptr event) { log(LogLevel::ERROR, event); }
void Logger::fatal(LogEvent::ptr event) { log(LogLevel::FATAL, event); }

void Logger::addAppender(LogAppender::ptr appender) { m_appenders.push_back(appender); }
void Logger::delAppender(LogAppender::ptr appender) { m_appenders.remove(appender); }

LogLevel::Level Logger::getLevel() const { return m_level; }
void Logger::setLevel(LogLevel::Level val) { m_level = val; }

