#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>

class ThreadPool{
public:
    explicit ThreadPool(size_t threadCount = 8): pool_(std::make_shared<Pool>()){
        assert(threadCount > 0); //断言
        //创建threadCount个子线程
        for(size_t i = 0; i < threadCount; i++) {
            std::thread([pool = pool_] {
                std::unique_lock<std::mutex> locker(pool->mtx);
                while(true){
                    if(!pool->tasks.empty()) {
                        auto task = std::move(pool->tasks.front());
                        pool->tasks.pop();
                        locker.unlock();
                        task();//任务将要执行的代码
                        locker.lock();
                        } 
                    else if(pool->isClosed) break;
                    else pool->cond.wait(locker);
                }
            }).detach();
        }
    }

    ThreadPool() = default;
    ThreadPool(ThreadPool&&) = default;

     ~ThreadPool() {
         if(static_cast<bool>(pool_)) { 
            {
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->isClosed = true;
            }
            pool_->cond.notify_all(); //线程池销毁时唤醒所有线程使其退出。
        }
    }

    template<class F>
    void AddTask(F&& task){
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->tasks.emplace(std::forward<F>(task));
        }
        pool_->cond.notify_one(); 
    }


private:
    struct Pool {
        std::mutex mtx; 
        std::condition_variable cond; 
        bool isClosed;
        std::queue<std::function<void()>> tasks;//请求队列：保存的是任务
    };
    std::shared_ptr<Pool> pool_;
};

#endif //THREADPOOL_H