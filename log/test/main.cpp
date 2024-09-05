#include "../log.hpp"
#include <thread>
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>

pid_t gettid() {
    return static_cast<pid_t>(syscall(SYS_gettid));
}

int main() {
    Logger::ptr logger = std::make_shared<Logger>("example_logger");

    // 设置格式化器
    LogFormatter::ptr formatter = std::make_shared<LogFormatter>("%m %p %t");

    // 控制台输出
    StdoutLogAppender::ptr consoleAppender = std::make_shared<StdoutLogAppender>();
    consoleAppender->setFormatter(formatter);

    // 文件输出
    FileLogAppender::ptr fileAppender = std::make_shared<FileLogAppender>("log.txt");
    fileAppender->setFormatter(formatter);

    // 添加输出目标
    logger->addAppender(consoleAppender);
    logger->addAppender(fileAppender);

    // 创建日志事件并输出
    logger->debug(std::make_shared<LogEvent>(__FILE__, __LINE__, std::this_thread::get_id(), "Debug Message "));
    logger->info(std::make_shared<LogEvent>(__FILE__, __LINE__, std::this_thread::get_id(), "Info Message "));
    logger->warn(std::make_shared<LogEvent>(__FILE__, __LINE__, std::this_thread::get_id(), "Warn Message "));
    getchar();
    return 0;
}
