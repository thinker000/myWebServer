#include "log.hpp"

Logger::Logger(const std::string &name = "root") : m_anme(name)
{
}
void Logger::log(LogLevel::Level level, LogEvent::ptr event)
{
    if (level >= m_level)
    {
        for (auto &i : m_appender)
        {
            i->log(level, event);
        }
    }
}

void Logger::debug(LogEvent::ptr event)
{
    log(LogLevel::DEBUG, event);
}
void Logger::info(LogEvent::ptr event)
{
    log(LogLevel::INFO, event);
}
void Logger::warn(LogEvent::ptr event)
{
    log(LogLevel::WARN, event);
}
void Logger::error(LogEvent::ptr event)
{
    log(LogLevel::ERROR, event);
}
void Logger::fatal(LogEvent::ptr event)
{
    log(LogLevel::FATAL, event);
}

void Logger::addAppender(LogAppender::ptr appender)
{
    m_appender.push_back(appender);
}
void Logger::delAppender(LogAppender::ptr appender)
{
    for (auto i = m_appender.begin(); i != m_appender.end(); i++)
    {
        if (*i == appender)
        {
            m_appender.erase(i);
            break;
        }
    }
}

LogLevel::Level Logger::getLevel() const
{
    return m_level;
}
void Logger::setLevel(LogLevel::Level val)
{
    m_level = val;
}

void LogAppender::setFormatter(LogFormatter::ptr val)
{
    m_formatter = val;
}
LogFormatter::ptr LogAppender::getFormatt() const
{
    return m_formatter;
}

void StdoutLogAppender::log(LogLevel::Level level, LogEvent::ptr event)
{
    if (level >= m_level)
    {
        std::cout << m_formatter->format(event);
    }
}

FileLogAppender::FileLogAppender(const std::string &fileName) : m_fileName(fileName)
{
}
void FileLogAppender::log(LogLevel::Level level, LogEvent::ptr event)
{
    if (level >= m_level)
    {
        m_fileStream << m_formatter->format(event);
    }
}
bool FileLogAppender::reOpen()
{
    if (m_fileStream.is_open())
    {
        m_fileStream.close();
    }
    m_fileStream.open(m_fileName);
    if (m_fileStream.is_open())
        return true;
    else
        return false;
}