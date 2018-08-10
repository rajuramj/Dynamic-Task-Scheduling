/**
    src file: staticDecomposition.cpp
    Purpose: 1) To compare the performance of so called static decomposition with dynamic (TP) decomposition

    @author Raju Ram
    @version 1.0 08/08/18
*/

#include "staticDecomposition.hpp"

#include <x86intrin.h> //  __rdtsc()
#include <stdexcept>  // std::runtime_error
#include <string>   // std::stod


/**
    Constructor
 */

StaticDecompostion::StaticDecompostion(size_t niters, size_t nthreads, size_t ntasks)
{
	this->niters = niters;
	this->nthreads = nthreads;
	this->ntasks = ntasks;

	this->task_cycles.resize(ntasks, 0);
	this->taskInds.resize(nthreads);

	std::cout << "ctr called" << std::endl;
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
void StaticDecompostion::sleepCycles (int nCycles )
{

  long cycleStart(__rdtsc());

  long upperCycleLimit ( cycleStart
                       + nCycles );

  long cycle (__rdtsc() );

  while ( cycle < upperCycleLimit )
  {
    cycle = __rdtsc();
  }

  std::cout << "Slept for " << nCycles << " cycles\n";
}


// setting the starting and end task id for each thread
void StaticDecompostion::setThreadTaskMap()
{
	size_t start(0), end(0);
	size_t tasks_per_thread = this->ntasks/ this->nthreads;

	for(size_t tid=0; tid < this->nthreads; ++tid) {

		start = tid*tasks_per_thread;

		if(tid == nthreads -1) {
			end = ntasks-1;
		} else {
			end =  (tid + 1)*tasks_per_thread - 1;
		}

		this->taskInds[tid] = std::make_pair(start,end);
	}

	std::cout << "Printing thread-task mapping ..." << std::endl;

	for (auto pair : taskInds) {
		std::cout <<  pair.first << " ---> " << pair.second << std::endl;
	}
}

void StaticDecompostion::setTaskCycles(size_t avgCycles, double dev)
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

	    for (auto cycle : task_cycles) {
	    	std::cout <<  cycle << "\t";
	    }

	    std::cout << std::endl;
}

void doOneIter(size_t tid, StaticDecompostion* thisptr, Barrier bar)
{
	//std::cout << "Peform one iteration " << std::endl;

	size_t stask, etask;

	stask = thisptr->taskInds[tid].first;
	etask = thisptr->taskInds[tid].second;

	for(size_t task=stask; task<=etask; ++task) {
		thisptr->sleepCycles(thisptr->task_cycles[task]);
	}

	// perform barrier
	//bar.applyBarrier();

	//std::cout << "Thread " << tid << "finished one Iter" << std::endl;
}

void StaticDecompostion::runAllTasks()
{
	// create nthreads
	std::thread threads[this->nthreads];
	Barrier bar(this->nthreads);

	for(size_t iter=0; iter < this->niters; ++iter)
	{
		for(size_t tid =0; tid < nthreads; tid++)
		{
			// each thread performs one iteration based on thread id ()
			threads[tid] = std::thread(doOneIter,tid, this, bar);
			//bar.applyBarrier(tid);
		}

	}

	// join all the threads
	for (size_t i=0; i < nthreads; ++i)
	{
		threads[i].join();
	}

	std::cout << "All threads are joined " << std::endl;
}

int main(int argc, char *argv[])
{
	 if( argc < 4 ) {
	      throw std::runtime_error ("<max_iter>, <num_threads>, <num_tasks>, optional(<cpu_cycles>, <deviation>");
	  }

	    size_t niters = std::stoi(argv[1]);
	    size_t nthreads = std::stoi(argv[2]);
	    size_t ntasks = std::stoi(argv[3]);
	    size_t ncycles(0);
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

	    sd.runAllTasks();

	    std::cout << "End of main" << std::endl;
}


