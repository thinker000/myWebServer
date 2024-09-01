#include "threadpools.hpp"
#include <iostream>

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
void Threadpool::submitTask(std::shared_ptr<Task> sp)
{
    std::unique_lock<std::mutex> lock(mtx_);
    // 将任务放入任务队列中
    taskQueue_.emplace(sp);
    // 任务数量+1
    taskCount_++;
    // 通知其他线程，任务队列中新放了任务，任务队列不为空
    condTaskNotEmpty_.notify_all();
    //如果线程池模式是VARIABLE，可以根据任务数量和空闲线程的数量，来判断是否需要创建新的线程
    if(threadpoolMode_ ==Mode::VARIABLE 
        && taskCount_ > idleThreadCount_
        && curThreadCount_ < threadCountThreshold)
    {
        std::cout<< "create new thread..."<<std::endl;
        //创建新的线程对象
        auto ptr = std::make_unique<Thread>(std::bind(&Threadpool::threadFunc, this, std::placeholders::_1));
		int threadId = ptr->getThreadId();
		threads_.emplace(threadId, std::move(ptr));
        //启动线程
        threads_[threadId]->start();
        curThreadCount_ ++;
        idleThreadCount_ ++;
    }
}

// 线程函数，处理任务--消费者
void Threadpool::handleTask()
{
    
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
        auto ptr = std::make_unique<Thread>(std::bind(&Threadpool::threadFunc,this,std::placeholders::_1));
        int threadId = ptr->getThreadId();
        threads_.emplace(threadId,std::move(ptr));
    }

    //启动所有线程
    for(size_t i=0;i<threadCount;i++)
    {
        threads_[i]->start();   // 需要去执行一个线程函数
        idleThreadCount_ ++;    // 记录初始空闲线程的数量
    }
}

bool Threadpool::checkRunningState() const
{
    return isRunning_;
}
//线程池中的线程只需要处理读和写时间，读取http请求报文，写（发送）http响应报文
void Threadpool::threadFunc(int threadId)
{

}