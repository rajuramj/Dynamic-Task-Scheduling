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

#include "../grid.hpp"
#include "../Timer.hpp"
#include <pthread.h>  // for pinning
#include <unistd.h>   // for sysconf

class ThreadPool {

public:
	//ThreadPool(size_t);
	ThreadPool(size_t, std::shared_ptr<Grid>);


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

	// sybchronisation
	std::mutex queue_mutex;
	std::condition_variable condition;
	bool stop;

private:
	// need to keep track of threads so we can join them
	std::vector< std::thread > workers;
	// the task queue

	// tasks also has a pointer of class Task, create a struct
	std::queue< std::function<bool()> > tasks;

	// synchronization
	// Number of tasks finished.
	std::atomic<size_t> numTasksDone;

	//task counter for each thread
	std::vector <int> counter;

	// pointer to the grid
	std::shared_ptr<Grid> gridPtr;

	// Time measurement
	TP::Timer timer;

};

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t numThreads,  std::shared_ptr<Grid> ptr2Grid)
: stop(false), numTasksDone(0)
{


	this-> counter.resize(numThreads);
	this->gridPtr = ptr2Grid;
	// number of available cpus
	size_t num_cores = sysconf(_SC_NPROCESSORS_ONLN);
	std::cout << "number of available cpus " << num_cores <<  std::endl;

	for(size_t thread_id = 0; thread_id < numThreads; ++thread_id)
		workers.emplace_back(
				[this, thread_id, num_cores, numThreads]
	{

		//timer.reset();

		// Pin current thread to specific CPU core.
		bool toBePinned(true);
		if(toBePinned)
		{
			if (numThreads > num_cores)
			{
				throw std::runtime_error("numThreads exceeded than max number of CPUs");
			}

			cpu_set_t CPUSet;
			CPU_ZERO(&CPUSet);
			CPU_SET(static_cast<int>(2*thread_id), &CPUSet);
			pthread_t curr_thread(pthread_self());

			pthread_setaffinity_np
			(curr_thread,
					sizeof(cpu_set_t),
					&CPUSet
			);
		}

		for(;;)  // Infinite for loop until return is issued.
		{
			std::function<bool()> task;

			{
				std::unique_lock<std::mutex> lock(this->queue_mutex);

				// The current thread blocks until (stop is set to true) or
				// (we have got something in the task queue)
				// until the wait condition is true, the thread unlocks the mutex, put the thread in
				// blocked/waiting state

				this->condition.wait(lock,
						[this]{return this->stop || !this->tasks.empty();} // lambda function for conditionals
				);

				// All the threads finishes when stop is set to true and tasks queue becomes empty.
				if(this->stop && this->tasks.empty())
				{
					break;
				}

				// move the FIFO queue's front task into a local 'task' object.
				task = std::move(this->tasks.front());
				this->tasks.pop();

			}    	// unlock queue_mutex here



			timer.reset();
			bool hasTaskNotFinished = task();
			Utility::compute_time[thread_id] += timer.elapsed();

			//enqueue the task again;
			if(hasTaskNotFinished)
			{
				// enqueue the task again
				// This is evil: code goes into recursion: why?
				 //this->enqueue(task);

				std::unique_lock<std::mutex> lock(queue_mutex);

				// don't allow enqueueing after stopping the pool
				if(stop)
					throw std::runtime_error("enqueue on stopped ThreadPool");


				tasks.emplace(task);
				//Utility::compute_time[thread_id] += timer.elapsed();

				condition.notify_one();

			}
			else
			{

				Utility::compute_time[thread_id] += timer.elapsed();

				{
					//std::unique_lock<std::mutex> locker(queue_mutex);
					// numTasksDone is atomic so thread safe
					this->numTasksDone ++ ; // use atomic
				}

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
						std::cout << "Thread " << thread_id << " sets the stop variable to true" << std::endl;
					}

					// wake up all the threads to exit the infinite loop
					condition.notify_all();

					// end the global time counter
					Utility::gtime_count = Utility::gtime.elapsed();

					{
						std::lock_guard <std::mutex> locker(Utility::mu);
						std::cout <<  "global time taken  "  << Utility::gtime_count << " secs. "
								<< std::endl;
					}

					// write the solution to the file
					// gridPtr->writeSol2File();

					/*// Display the grid
					std::cout << "u1: " << std::endl;
					gridPtr->displayGrid(gridPtr->u1_);
					std::cout << "u2: " << std::endl;
					gridPtr->displayGrid(gridPtr->u2_);*/

				}


			} //end of else


			if(Utility::debug)
			{
				std::lock_guard <std::mutex> locker(Utility::mu);
				std::cout << "Task() executed by thread " << thread_id << std::endl;
			}



		} // for loop infinite loop



		{
			std::lock_guard <std::mutex> locker(Utility::mu);

			std::cout <<  "compute time by thread " << thread_id << "  " <<100*Utility::compute_time[thread_id]/Utility::gtime_count
					<< " % "  << std::endl;
		}


				 } // end of for threads loop
		);    // end of emplace_end


}



template<class F>
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

	/*if(stop == false)
	{
		{
		  std::unique_lock<std::mutex> lock(queue_mutex);
		  stop = true;
		}
		condition.notify_all();
	}*/


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
