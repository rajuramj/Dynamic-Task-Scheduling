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

#include "../task.h"  // Taskptr is used in the enqueue function

class ThreadPool {

public:
    ThreadPool(size_t);

    /*template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>;
        //-> std::future<bool>;*/

    // Base case
    template<class F, typename T>
    std::future<T> enqueue(F&& f, T&& t);

    //template<class F>
    //std::future<int> enqueue(F&& f, int&& t);

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
                    //if(not finished)
                    //bool hasTaskNotFinished  =  task();
                     task();

                    //enqueue the task again;
                    /*if(hasTaskNotFinished)
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);

                        // don't allow enqueueing after stopping the pool
                        if(stop)
                            throw std::runtime_error("enqueue on stopped ThreadPool");

                        tasks.emplace([task](){ (task)(); });

                        // wake up one thread to execute the task
                        condition.notify_one();
                    }*/

                    //ques : how to access iter number of the tasks

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

/*template<class F>
std::future<int> ThreadPool::enqueue(F&& f, int&& t)
{
    std::cout << "INT specialised template function called\n";

}*/


template<class F, typename T>
std::future<T> ThreadPool::enqueue(F&& f, T&&  t)
{
    {
        std::lock_guard <std::mutex> locker(Utility::mu);
        std::cout << "Specialised template function called\n";
        std::cout << "task id is: " << t << std::endl;
    }

    auto task = std::make_shared< std::packaged_task<T()> >(
            std::bind(std::forward<F>(f), std::forward<T>(t))
        );

    //auto task = std::make_shared< std::packaged_task<char()> >(
               // std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<T> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if(stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task](){ (*task)(); });
    }

    condition.notify_one();

    //std::cout << "Child std::this_thread::get_id(): " << std::this_thread::get_id() << std::endl;
   // std::cout << "Task enqueued\n";

    return res;

}
/*
// add new work item to the pool
// ... implements variadic template feature: it can accept any number of arguments.
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::result_of<F(Args...)>::type>
      //-> std::future<bool>
{

    std::cout << "Non specialised template function called\n";

    using return_type = typename std::result_of<F(Args...)>::type;

    //std::cout << "return type is: " << return_type << std::endl;

    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
    auto task = std::make_shared< std::packaged_task<char()> >(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    int size =  sizeof...(Args);

    std::cout << "size: " << size << std::endl;

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if(stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task](){ (*task)(); });
    }

    condition.notify_one();


    for (auto it = workers.begin(); it!=workers.end(); it++)
    {
        std::cout << "tid: "  << it->get_id() << std::endl;
    }

    //std::cout << "Child std::this_thread::get_id(): " << std::this_thread::get_id() << std::endl;
   // std::cout << "Task enqueued\n";

    return res;
}*/

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
