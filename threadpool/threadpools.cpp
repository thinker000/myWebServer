#include "threadpools.hpp"
#include <iostream>
#include <chrono>

const int THREAD_MAX_IDLE_TIME = 60;

Threadpool::Threadpool()
    : initThreadCounts_(0), threadpoolMode_(Mode::FIXED), threadCountThreshold(0), curThreadCount_(0), idleThreadCount_(0), taskCount_(0), isRunning_(false)
{
}

Threadpool::~Threadpool()
{
}

// 设置线程池工作模式
void Threadpool::setMode(Mode mode)
{
    if (checkRunningState())
        return;
    threadpoolMode_ = mode;
}
// 设置任务队列上限阈值
void Threadpool::setTaskQueueThreshold(size_t threshold)
{
    if (checkRunningState())
        return;
    taskQueueThreshold = threshold;
}
// 设置线程数量的阈值--只针对VARIABLE模式
void Threadpool::setThreadCountThreshold(size_t threshold)
{
    if (checkRunningState())
        return;
    if (threadpoolMode_ == Mode::VARIABLE)
        threadCountThreshold = threshold;
}
// 给线程池提交任务--生产者
void Threadpool::submitTask(std::function<void()> sp)
{
    std::unique_lock<std::mutex> lock(mtx_);
    // 将任务放入任务队列中
    taskQueue_.emplace(sp);
    // 任务数量+1
    taskCount_++;
    // 通知其他线程，任务队列中新放了任务，任务队列不为空
    condTaskNotEmpty_.notify_all();
    // 如果线程池模式是VARIABLE，可以根据任务数量和空闲线程的数量，来判断是否需要创建新的线程
    if (threadpoolMode_ == Mode::VARIABLE && taskCount_ > idleThreadCount_ && curThreadCount_ < threadCountThreshold)
    {
        std::cout << "create new thread..." << std::endl;
        // 创建新的线程对象
        auto ptr = std::make_unique<Thread>(std::bind(&Threadpool::threadFunc, this, std::placeholders::_1));
        int threadId = ptr->getThreadId();
        threads_.emplace(threadId, std::move(ptr));
        // 启动线程
        threads_[threadId]->start();
        curThreadCount_++;
        idleThreadCount_++;
    }
}

// 线程函数，处理任务--消费者
void Threadpool::handleTask(int threadId)
{
    // 所有线程执行完毕，线程池才回收线程资源
    auto lastTime = std::chrono::high_resolution_clock().now();
    for (;;)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mtx_);
            // 获取任务
            while (taskQueue_.size() == 0)
            {
                if (!isRunning_)
                {
                    threads_.erase(threadId);
                    std::cout << "threadid:" << std::this_thread::get_id() << " exit!" << std::endl;
                    condExit_.notify_all();
                    return;
                }
                if (threadpoolMode_ == Mode::VARIABLE)
                {
                    if (std::cv_status::timeout == condTaskNotEmpty_.wait_for(lock, std::chrono::seconds(1)))
                    {
                        auto now = std::chrono::high_resolution_clock().now();
                        auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
                        if (dur.count() >= THREAD_MAX_IDLE_TIME && curThreadCount_ > initThreadCounts_)
                        {
                            threads_.erase(threadId);
                            curThreadCount_--;
                            idleThreadCount_--;
                            std::cout << "threadid:" << std::this_thread::get_id() << " exit!" << std::endl;
                            return;
                        }
                    }
                }
                else
                {
                    condTaskNotEmpty_.wait(lock);
                }
            }

            idleThreadCount_--;
            std::cout << "tid:" << std::this_thread::get_id() << "获取任务成功..." << std::endl;
            // 获取任务成功
            task = std::move(taskQueue_.front());
            taskQueue_.pop();
            taskCount_--;
            // 如果有任务，通知其他线程继续执行任务
            if (taskQueue_.size() > 0)
                condTaskNotEmpty_.notify_all();
            // 取出一个任务，通知此时任务队列不满
            condTaskNotFull_.notify_all();
        }
        // 执行任务
        task();
        // 任务执行完，空闲线程数量+1
        idleThreadCount_++;
        lastTime = std::chrono::high_resolution_clock().now();
    }
}

// 开启线程池
void Threadpool::start(int threadCount)
{
    // 线程池的运行状态
    isRunning_ = true;
    // 初始化线程个数
    initThreadCounts_ = threadCount;
    curThreadCount_ = threadCount;
    // 创建线程对象
    for (size_t i = 0; i < threadCount; i++)
    {
        auto ptr = std::make_unique<Thread>(std::bind(&Threadpool::threadFunc, this, std::placeholders::_1));
        int threadId = ptr->getThreadId();
        threads_.emplace(threadId, std::move(ptr));
    }

    // 启动所有线程
    for (size_t i = 0; i < threadCount; i++)
    {
        threads_[i]->start(); // 需要去执行一个线程函数
        idleThreadCount_++;   // 记录初始空闲线程的数量
    }
}

bool Threadpool::checkRunningState() const
{
    return isRunning_;
}
// 线程池中的线程只需要处理读和写时间，读取http请求报文，写（发送）http响应报文
void Threadpool::threadFunc(int threadId)
{
    // 所有线程执行完毕，线程池才回收线程资源
    auto lastTime = std::chrono::high_resolution_clock().now();
    for (;;)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mtx_);
            // 获取任务
            while (taskQueue_.size() == 0)
            {
                if (!isRunning_)
                {
                    threads_.erase(threadId);
                    std::cout << "threadid:" << std::this_thread::get_id() << " exit!" << std::endl;
                    condExit_.notify_all();
                    return;
                }
                if (threadpoolMode_ == Mode::VARIABLE)
                {
                    if (std::cv_status::timeout == condTaskNotEmpty_.wait_for(lock, std::chrono::seconds(1)))
                    {
                        auto now = std::chrono::high_resolution_clock().now();
                        auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
                        if (dur.count() >= THREAD_MAX_IDLE_TIME && curThreadCount_ > initThreadCounts_)
                        {
                            threads_.erase(threadId);
                            curThreadCount_--;
                            idleThreadCount_--;
                            std::cout << "threadid:" << std::this_thread::get_id() << " exit!" << std::endl;
                            return;
                        }
                    }
                }
                else
                {
                    condTaskNotEmpty_.wait(lock);
                }
            }

            idleThreadCount_--;
            std::cout << "tid:" << std::this_thread::get_id() << "获取任务成功..." << std::endl;
            // 获取任务成功
            task = std::move(taskQueue_.front());
            taskQueue_.pop();
            taskCount_--;
            // 如果有任务，通知其他线程继续执行任务
            if (taskQueue_.size() > 0)
                condTaskNotEmpty_.notify_all();
            // 取出一个任务，通知此时任务队列不满
            condTaskNotFull_.notify_all();
        }
        // 执行任务
        task();
        // 任务执行完，空闲线程数量+1
        idleThreadCount_++;
        lastTime = std::chrono::high_resolution_clock().now();
    }
}

int Thread::generateId_ = 0;
// 线程构造
Thread::Thread(ThreadFunc func)
    : func_(func), threadId_(generateId_++)
{
}
// 线程析构
Thread::~Thread()
{
}
// 启动线程
void Thread::start()
{
    std::thread t(func_, threadId_);
    t.detach(); // 设置线程分离模式
}
// 获取线程id
int Thread::getThreadId() const
{
    return threadId_;
}