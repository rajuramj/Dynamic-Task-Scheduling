#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

class ThreadPool {
public:
    ThreadPool(size_t);
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type>;
    ~ThreadPool();

    void printCounter()
    {
        std::cout << "Tasks executed by threads" << std::endl;

        for (auto it= counter.begin(); it!= counter.end(); it++)
        {
            std::cout << *it << std::endl;
        }
    }

private:
    // need to keep track of threads so we can join them
    std::vector< std::thread > workers;
    // the task queue

    // tasks also has a pointer of class Task, create a struct
    std::queue< std::function<void()> > tasks;
    
    // synchronization
    std::mutex queue_mutex, cout_mutex;
    std::condition_variable condition;
    bool stop;

    //task counter for each thread
    std::vector <int> counter;
};
 
// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads)
    :   stop(false)
{
    this-> counter.resize(threads);

    for(size_t i = 0;i<threads;++i)
        workers.emplace_back(
            [this,i]
            {
                for(;;)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock,
                            [this]{ return this->stop || !this->tasks.empty(); });
                        if(this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }


                    //if task's pre conditions have been met
                    task();

                    //ques : how to access iter number of the tasks

                    //optional set task post conditioning

                    //if(task. iter < Utility::num_iter)
                      //  enqueue the task again.

                   /* if(task not finished) {
                        enqueue_again
                    }*/
                    std::lock_guard <std::mutex> locker(this->cout_mutex);
                    this->counter[i]++;
                    std::cout << "Task() executed by thead " << i << std::endl;
                }
            }
        );
}

// add new work item to the pool
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if(stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task](){ (*task)(); });
    }
    condition.notify_one();


    /*for (auto it = workers.begin(); it!=workers.end(); it++)
    {
        std::cout << "tid: "  << it->get_id() << std::endl;
    }*/

    //std::cout << "Child std::this_thread::get_id(): " << std::this_thread::get_id() << std::endl;
   // std::cout << "Task enqueued\n";

    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(std::thread &worker: workers)
        worker.join();
}

#endif