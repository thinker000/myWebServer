#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>
#include <queue>
#include <atomic>
#include <unordered_map>

// 任务抽象基类
// class Task
// {
// public:
//     Task();
//     virtual ~Task();
//     virtual void run() = 0;
// };

// 线程池的工作模式:固定线程数量的线程池；可变线程数量的线程池
enum class Mode
{
    FIXED,
    VARIABLE
};

// 线程类
class Thread
{

public:
    // 线程函数对象类型
    using ThreadFunc = std::function<void(int)>;

public:
    // 线程构造
    Thread(ThreadFunc func);
    // 线程析构
    ~Thread();
    // 启动线程
    void start();
    // 获取线程id
    int getThreadId() const;

private:
    size_t threadId_;
    ThreadFunc func_;
    static int generateId_;
};

class Threadpool
{

public:
    Threadpool();
    // Threadpool(int threadCount=std::thread::hardware_concurrency(), Mode mode= Mode::FIXED);  //默认值

    Threadpool(const Threadpool &) = delete;

    Threadpool &operator=(const Threadpool &) = delete;

    ~Threadpool();
    // 设置线程池工作模式
    void setMode(Mode mode);
    // 设置任务队列上限阈值
    void setTaskQueueThreshold(size_t threshold);
    // 设置线程数量的阈值--只针对VARIABLE模式
    void setThreadCountThreshold(size_t threshold);

public:
    // 给线程池提交任务--生产者
    void submitTask(std::function<void()> sp);
    // 线程函数，处理任务--消费者
    void handleTask(int threadId);

    void start(int threadCount = std::thread::hardware_concurrency());

public:
    bool checkRunningState() const;
    void threadFunc(int threadId);

private:
    std::unordered_map<int, std::unique_ptr<Thread>> threads_; // 线程队列
    size_t initThreadCounts_;                                  // 固定线程数量的线程池      // std::thread::hardware_concurrency();         //获取硬件支持的线程数
    Mode threadpoolMode_;                                      // 线程池工作模式
    long unsigned int threadCountThreshold;                    // 线程数量的上限
    std::atomic<unsigned int> curThreadCount_;                 // 线程池中的线程总数量
    std::atomic<unsigned int> idleThreadCount_;                // 空闲线程的数量
    std::queue<std::function<void()>> taskQueue_;              // 任务队列
    size_t taskQueueThreshold;                                 // 任务队列中存放任务数量的上限
    std::atomic<unsigned int> taskCount_;                      // 任务队列中任务的数量
    std::mutex mtx_;                                           // 保护任务队列的线程安全
    std::condition_variable condTaskNotEmpty_;                 // 任务队列中进行线程通信 任务队列非空
    std::condition_variable condTaskNotFull_;                  // 任务队列不满

    std::atomic<bool> isRunning_;
    std::condition_variable condExit_; // 等到线程资源全部回收
};

#endif
