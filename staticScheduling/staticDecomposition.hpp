/**
    header file: staticDecompostion.hpp
    Purpose: to declare class variables and member functions of StaticDecompostion class

    @author Raju Ram
    @version 1.0 04/07/18
*/

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <cstdlib> 		// for size_t
#include <utility>  	// std::pair


#ifndef STDEC_H
#define STDEC_H

class StaticDecompostion
{

private:
	size_t niters;
	size_t nthreads;
	size_t ntasks;
	bool toBePinned;
	std::vector<double> task_cycles;
	std::vector<std::pair <size_t, size_t >> taskInds;

	// output synchronisation
	std::mutex cout_mutex;

public:

	StaticDecompostion(size_t niters, size_t nthreads, size_t ntasks);

	void setThreadTaskMap();
	void setTaskCycles(double avgCycles, double dev);

	inline void pinThreadOnSocket(size_t thread_id);
//	/inline void pinMainThread();

	//debug version
	void sleepCycles (double nCycles, size_t taskid);
	void sleepCycles (double nCycles);

	void runAllTasks();



	~StaticDecompostion();
};


#endif // STDEC_H


