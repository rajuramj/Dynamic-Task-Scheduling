/**
    header file: staticDecompostion.hpp
    Purpose: to declare class variables and member functions of StaticDecompostion class

    @author Raju Ram
    @version 1.0 04/07/18
*/

#include <iostream>
#include <thread>
#include <vector>
#include <cstdlib> 		// for size_t
#include <utility>  	// std::pair
#include "../include/Barrier.hpp"  // C++ wrapper for pthread barrier

#ifndef STDEC_H
#define STDEC_H

class StaticDecompostion
{

private:
	size_t niters;
	size_t nthreads;
	size_t ntasks;

public:
	std::vector<size_t> task_cycles;
	std::vector<std::pair <size_t, size_t >> taskInds;

	StaticDecompostion(size_t niters, size_t nthreads, size_t ntasks);
	void runAllTasks();
	//void doOneIter(size_t tid);

	void sleepCycles (int nCycles);
	void setThreadTaskMap();
	void setTaskCycles(size_t avgCycles, double dev);

	~StaticDecompostion();
};


#endif // STDEC_H


