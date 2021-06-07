/**
    src file: staticDecomposition.cpp
    Purpose: 1) To compare the performance of so called static decomposition
    			with dynamic (TP) decomposition

    @author Raju Ram
    @version 1.0 08/08/18
*/

#include "staticDecomposition.hpp"

#include <x86intrin.h> //  __rdtsc()
#include <stdexcept>  // std::runtime_error
#include <string>   // std::stod
#include "Barrier.hpp"  // C++ wrapper for pthread barrier
#include "Timing.hpp"   // measure CPU cycles for hotspot
#include <unistd.h>   // for sysconf

/**
    Constructor
 */

StaticDecompostion::StaticDecompostion
(size_t nitersIn
, size_t nthreadsIn
, size_t ntasksIn)
: niters(nitersIn)
, nthreads(nthreadsIn)
, ntasks(ntasksIn)
, toBePinned(true)
{
	this->task_cycles.resize(ntasks, 0);
	this->taskInds.resize(nthreads);
}

/**
    Destructor
 */
StaticDecompostion::~StaticDecompostion()
{
	//std::cout << "Destrctor called" << std::endl;
}

/**
    let the current task sleep for nCycles
	@param  nCycles numbe of cpu cycles

    @return void
 */
void StaticDecompostion::sleepCycles (double nCycles, size_t taskid)
{

  long cycleStart(__rdtsc());

  long upperCycleLimit ( cycleStart
                       + nCycles );

  long cycle (__rdtsc() );

  while ( cycle < upperCycleLimit )
  {
    cycle = __rdtsc();
  }

  std::cout << "Task " << taskid << " slept for " << nCycles << " cycles" << std::endl;
}

void StaticDecompostion::sleepCycles (double nCycles)
{

  long cycleStart(__rdtsc());

  long upperCycleLimit ( cycleStart
                       + nCycles );

  long cycle (__rdtsc() );

  while ( cycle < upperCycleLimit )
  {
    cycle = __rdtsc();
  }

}


// setting the starting and end task id for each thread
void StaticDecompostion::setThreadTaskMap()
{
	size_t start(0), end(0);
	size_t tasks_per_thread = this->ntasks/ this->nthreads;

	for(size_t tid=0; tid < this->nthreads; ++tid) {

		start = tid*tasks_per_thread;

		if(tid == nthreads -1) {
			end = ntasks;
		} else {
			end =  (tid + 1)*tasks_per_thread ;
		}

		std::cout << "Thread " << tid
				  << " works on task ids "
				  << start << " ---> " << end
				  << std::endl;

		this->taskInds[tid] = std::make_pair(start,end);
	}

//	std::cout << "Printing thread-task mapping ..." << std::endl;

//	for (auto pair : taskInds) {
//		std::cout <<  pair.first << " ---> " << pair.second << std::endl;
//	}
}

//setting cpu cycles for each task
void StaticDecompostion::setTaskCycles(double avgCycles, double dev)
{
		double min_cycle =  avgCycles*(1 - dev);
		double max_cycle =  avgCycles*(1 + dev);
	    double step_size =   (max_cycle - min_cycle)/(this->ntasks - 1);

	    this->task_cycles[0] = min_cycle;

	    for(size_t i=1; i < (ntasks-1); i++)
	    {
	    	this->task_cycles[i] = min_cycle + i*step_size;
	    }

	    this->task_cycles[ntasks-1] = max_cycle;

	    std::cout << "Printing task cycles ..." << std::endl;

	    for (auto& cycle : task_cycles) {
	    	std::cout <<  cycle << "\t";
	    }

	    std::cout << std::endl;
}

// Pin the thread to all CPUs of socket 0.
void StaticDecompostion::pinThreadOnSocket(size_t thread_id) {

	size_t num_cores = sysconf(_SC_NPROCESSORS_ONLN);

	if (this->nthreads > num_cores)
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

	/*{
	std::lock_guard<std::mutex> iolock(this->cout_mutex);
	std::cout << "Thread " << thread_id  << " No. of CPUs in the set are " << CPU_COUNT(&CPUSet) << std::endl;
	}*/
}

// Pins the main thread to core 1 (WLOG)
void pinMainThread() {

		cpu_set_t CPUSet;
		CPU_ZERO(&CPUSet);
		CPU_SET(1, &CPUSet);
		pthread_t curr_thread(pthread_self());

		pthread_setaffinity_np
		(curr_thread,
				sizeof(cpu_set_t),
				&CPUSet
		);
}


void StaticDecompostion::runAllTasks()
{
	// create nthreads
	std::thread threads[this->nthreads];
	Barrier bar(this->nthreads);

	ace::Timing timer;
	//std::cout << "timer start: main Thread running on CPU # " << sched_getcpu() << "\n";
	timer.start();

	// all threads
	for(size_t tid =0; tid < nthreads; tid++)
	{

		// lambda function for each thread
		threads[tid] = std::thread ([tid, this, &bar]{

			// Pinning on NUMA Domain 0
			if(this->toBePinned) {
				pinThreadOnSocket(tid);
			}

			/*{
				std::lock_guard<std::mutex> iolock(this->cout_mutex);
				std::cout << "Thread #  " << tid << " running on CPU # " << sched_getcpu() << "\n";
			}*/


			size_t stask,etask;

			// all iters
			for(size_t iter=0; iter < this->niters; ++iter) {

				stask = this->taskInds[tid].first;
				etask = this->taskInds[tid].second;

				// all tasks per thread
				for(size_t task=stask; task<etask; ++task) {
					this->sleepCycles(this->task_cycles[task]);

				}

				// perform barrier
				bar.applyBarrier();

				//std::cout << "Thread " << tid << "finished one Iter" << std::endl;
			}

		});

	}

	// join all the threads
	for (auto& t : threads) {
		t.join();
	}

	//std::cout << "timer stop: main Thread running on CPU # " << sched_getcpu() << "\n";
	timer.stop();


	std::cout << "Total elapsed cycles: " << timer.elapsedCycles() << std::endl;
	std::cout << "All threads are joined " << std::endl;
}

int main(int argc, char *argv[])
{
	 pinMainThread();

	 if( argc < 4 ) {
	      throw std::runtime_error ("<max_iter>, <num_threads>, <num_tasks>, optional(<cpu_cycles>, <deviation>");
	  }

	 	//std::cout << "Begin: main Thread running on CPU # " << sched_getcpu() << "\n";

	    size_t niters = std::stoi(argv[1]);
	    size_t nthreads = std::stoi(argv[2]);
	    size_t ntasks = std::stoi(argv[3]);
	    double ncycles(0.0);
	    double dev(0.0);

	    if(argc >=5) {
	    	ncycles = std::stoi(argv[4]);

	    	if(argc == 6) {
	    		dev = std::stod(argv[5]);
	    	}
	    }

	    std::cout << "starting with "
	              << niters << " total iterations, "
	              << nthreads << " threads, "
	              << ntasks << " tasks, "
				  << ncycles << " CPU cycles per task, "
				  << dev << " hetrogeneity quotient "
				  <<  " .... \n \n "  <<  std::endl;


	    StaticDecompostion sd(niters, nthreads, ntasks);

	    sd.setThreadTaskMap();
		sd.setTaskCycles(ncycles, dev);

		// This functions spawns threads and these threads execute the tasks
	    sd.runAllTasks();

	    //std::cout << "Ends: main Thread running on CPU # " << sched_getcpu() << "\n";
}


