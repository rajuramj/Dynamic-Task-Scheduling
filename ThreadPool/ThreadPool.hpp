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
#include <assert.h>

class ThreadPool {

public:
    ThreadPool(size_t);

    // Number of tasks finished.
        size_t numTasksDone;

//    template<class F, class... Args>
//    std::future<bool> enqueue(F&& f, Args&&... args);
//      //  -> std::future<typename std::result_of<F(Args...)>::type>;
//        //-> std::future<bool>;

    // Base case
    template<class F>
//    std::future<bool> enqueue(F&& f);
    void enqueue(F&& f);


    ~ThreadPool();

    void printCounter()
    {
        //std::cout << "Tasks executed by threads" << std::endl;

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
    std::queue< std::function<bool()> > tasks;
    
    // synchronization
    std::mutex queue_mutex;
	std::mutex cout_mutex;
    std::condition_variable condition;
    bool stop;


    //task counter for each thread
    std::vector <int> counter;

};
 
// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads)
    :   stop(false), numTasksDone(0)
{
    this-> counter.resize(threads);

    for(size_t i = 0;i<threads;++i)
        workers.emplace_back(
            [this,i]
            {
                for(;;)
                {
                    std::function<bool()> task;
                    {
                       // std::lock_guard <std::mutex> locker(Utility::mu);
                       // std::cout << "Thread " << i << " waiting in the for loop" << std::endl;
                    }

                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);

                        // The current thread blocks until (stop is set to true) or
                        // (we have got something in the task queue)
                        // until the wait condition is true, the thread unlocks the mutex, put the thread in
                        // blocked/waiting state
                        this->condition.wait(lock,
                            [this,i]{
                        			  {
                        	 	 		//std::lock_guard <std::mutex> locker(Utility::mu);
                        	 	 		//std::cout << "Thread " << i << " inside condition wait" << std::endl;
                        	 	 	  }

                        			  if(this->stop || !this->tasks.empty())
                        			  {
                        				  {
                        				    //std::lock_guard <std::mutex> locker(Utility::mu);
                        				    //std::cout << "Thread " << i << " now leaving the condition wait" << std::endl;
                        				  }
                        				  return true;
                        			  }
                        			  else
                        			  {
                        				return false;
                        			  }

                        			//return this->stop || !this->tasks.empty();
                        		 });

                        // All the threads finishes when stop is set to true and tasks queue becomes empty.
                        if(this->stop && this->tasks.empty())
                        {
                        	 {
                        	   // std::lock_guard <std::mutex> locker(Utility::mu);
                        	    //std::cout << "Thread " << i << " now returns" << std::endl;
                        	 }
                            return;
                        }
                        // move the FIFO queue's front task into a local 'task' object.
                        task = std::move(this->tasks.front());
                        this->tasks.pop();

                        {
                         // std::lock_guard <std::mutex> locker(Utility::mu);
                          //std::cout << "Thread " << i << " has popped up the task() " << std::endl;
                        }


                        // lock.unlock() if we do not want to use the curly braces
                    } // unlock queue_mutex here

                    //if task's pre conditions have been met
                    //if(not finished)
                    bool hasTaskNotFinished  =  task();

                    {
                       //std::lock_guard <std::mutex> locker(Utility::mu);
                       //std::cout << " task finished ?? "
                    		//   << (hasTaskNotFinished ? "false" : "true")
						//	   << std::endl;
                    }


                    //enqueue the task again;
                    if(hasTaskNotFinished)
                    {
                    	/*{
                    	  std::lock_guard <std::mutex> locker(Utility::mu);
                          std::cout << " Thread "  << i << " 's task has not finished yet" << std::endl;
                    	}*/

                      // enqueue the task again
                      this->enqueue(task);



                    }
                    else
                    {
                      /*{
                      std::lock_guard <std::mutex> locker(Utility::mu);
                      std::cout << " Thread "  << i << " 's task been finished" << std::endl;
                      }*/

                      {
                              //
                              // do this -> make senses
                              {
                                std::unique_lock<std::mutex> locker(queue_mutex);
                                this->numTasksDone ++ ; // use atomic 
                              }    
                              /*{
                              std::lock_guard <std::mutex> locker(Utility::mu);
                              std::cout << "Number of finished now tasks are "
                            		  << numTasksDone
                            		  << std::endl;
                              }*/

                              //lock.lock();
                              //assert(numTasksDone  <=  Utility::numTasks * Utility::maxIter);

                              // set stop to true when all the threads have finished the task
                              if(numTasksDone == Utility::numTasks) // can be skipped-> can be used as sanity check
                              {
                                assert(stop == false);
                            	  {
                            	  std::unique_lock<std::mutex> lock(queue_mutex);
                            	  stop = true;
                            	  }

                            	  {
                            	  std::lock_guard <std::mutex> locker(Utility::mu);
                            	  std::cout << "Thread " << i << " sets the stop variable to true" << std::endl;
                            	  }

                            	  // wake up all the threads to exit the infinite loop
                            	  condition.notify_all();

                            	  // This thread can also return here, if not that it returns from condition
                            	  // variable since the wait is over after setting the stop to true.

                            	  //return;
                              }

                      }


                    }


                    {
						std::lock_guard <std::mutex> locker(Utility::mu);
						//this->counter[i]++;
						std::cout << "Task() executed by thread " << i << std::endl;
                    }
                }
            }
        );
}



template<class F>
//std::future<bool>
void ThreadPool::enqueue(F&& f)
{
    {
      std::unique_lock<std::mutex> lock(queue_mutex);

      // don't allow enqueueing after stopping the pool
       if(stop)
           throw std::runtime_error("enqueue on stopped ThreadPool");

      tasks.emplace([f]()->bool
        {
          bool ret_val(false);
          ret_val = f();

          /*{
        	  std::lock_guard <std::mutex> locker(Utility::mu);
        	  std::cout<< "In enqueue, task not finished? " << (ret_val ? "true" : "false") << std::endl;
        	  //f();
          }*/

          return ret_val;
        });

      //tasks.emplace([f]() ->bool { (f)(); });




    }



    condition.notify_one();

    return;
}




/*template<class F>
std::future<bool> ThreadPool::enqueue(F&& f)
{
    {
        std::lock_guard <std::mutex> locker(Utility::mu);
        std::cout << "Specialised template function called\n";
        std::cout << "task id is: " << t << std::endl;
    }

    auto task = std::make_shared< std::packaged_task<bool()> >(
            std::bind(std::forward<F>(f), std::forward<T>(t))
        );

    //auto task = std::make_shared< std::packaged_task<char()> >(
               // std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<bool> res = task->get_future();
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

}*/

//// add new work item to the pool
//// ... implements variadic template feature: it can accept any number of arguments.
//template<class F, class... Args>
//std::future<bool> ThreadPool::enqueue(F&& f, Args&&... args)
//    //-> std::future<typename std::result_of<F(Args...)>::type>
//      //->
//{
//
//    std::cout << "Non specialised template function called\n";
//
//    using return_type = typename std::result_of<F(Args...)>::type;
//
//    //std::cout << "return type is: " << return_type << std::endl;
//
//    auto task = std::make_shared< std::packaged_task<bool()> >(
//            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
//        );
//
//    //auto task = std::make_shared< std::packaged_task<char()> >(
//            //    std::bind(std::forward<F>(f), std::forward<Args>(args)...));
//
//    int size =  sizeof...(Args);
//
//    std::cout << "size: " << size << std::endl;
//
//    std::future<bool> res = task->get_future();
//    {
//        std::unique_lock<std::mutex> lock(queue_mutex);
//
//        // don't allow enqueueing after stopping the pool
//        if(stop)
//            throw std::runtime_error("enqueue on stopped ThreadPool");
//
//        tasks.emplace([task](){ (*task)(); });
//    }
//
//    condition.notify_one();
//
//
//    for (auto it = workers.begin(); it!=workers.end(); it++)
//    {
//        std::cout << "tid: "  << it->get_id() << std::endl;
//    }
//
//    //std::cout << "Child std::this_thread::get_id(): " << std::this_thread::get_id() << std::endl;
//   // std::cout << "Task enqueued\n";
//
//    return res;
//}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
//    {
//        std::unique_lock<std::mutex> lock(queue_mutex);
//        stop = true;
//    }
//    condition.notify_all();

    for(std::thread &worker: workers)
    {
    	// worker thread can be joined only once.
    	if(worker.joinable())
    	{
    		//std::cout << " Thread is joined now" << std::endl;
    		worker.join();
    	}
    	else
    	{
    		std::cout << " Thread is not joinable" << std::endl;
    	}

    }

}

#endif
