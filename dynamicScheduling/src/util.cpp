/**
    src file: util.cpp
    Purpose: define global constants, interface to set global constants, generic global functions.

    @author Raju Ram
    @version 1.0 04/07/18
 */


#include "util.hpp"
#include "task.hpp"
#include <cassert>


namespace Utility {

const double PI = 4.0 * atan(1.0);
const double K = 2 * PI;

size_t maxIter;
size_t numThreads;
size_t numTasks;
std::vector<double> compute_time;
std::vector<TP::Timer> threadLocTimer;

double gtime_count(0.0);
TP::Timer gtime;
ace::Timing gtiming;

//normal distribution generator
std::default_random_engine generator;
std::normal_distribution<double> distribution;

//const size_t numCols = 100;

//const size_t numRows = 100;
//const size_t numThreads = 10;

//assert(numRows + numThreads != 0);
//const size_t numTasks = numRows/numThreads;
//const size_t numTasks = 5;

const double tol = pow(10,-6);
const bool debug = false;

// Handles std::cout resource among threads.
std::mutex mu;

// Handles thread safe operation in the task class
std::mutex mu_task;

/**
	  converts rows and cols into coordinates
      @param global_row global rows
      @param global_col global columns

      @return x and y coordinates
 */
std::pair<double, double> getXY(std::size_t global_row, std::size_t global_col)
    				//std::pair<double, double> getXY(int global_row, int global_col)
{
	double x,y;

	x = global_col * Task::getGridPtr()->hx_;
	y =  ((Task::getGridPtr()->numRows_ - 1) - global_row)* (Task::getGridPtr()->hy_);

	return std::make_pair(x,y);
}

/**
	  get the global row index in the grid
      @param task_id total number of iterations
      @param localRow local row index from a task

      @return global row index
 */
size_t getGlobalRow(size_t task_id, size_t localRow)
{
	//Assuming same number of local rows for each task, i.e: uniform partitioning
	size_t globalRow = (Task::getGridPtr()->numlocRows_)*(task_id) + localRow;
	return globalRow;
}

/**
      Sets the global params
      @param iters total number of iterations
      @param threads total number of threads
      @tasks total number of tasks

      @return void
 */
void setParams(const size_t iters, const size_t threads, const size_t tasks)
//				const size_t cycles=26000, const double std_dev=0.0)
{
	maxIter = iters;
	numThreads = threads;
	numTasks = tasks;

	// Initialize the global variable here.
	Task::isAllTasksDone.resize(Utility::numTasks, false);

	compute_time.resize(numThreads, 0.0);
	threadLocTimer.resize(numThreads);

	//std::normal_distribution<double> distribution(cycles, std_dev);
}

/**
        Display some global constants

        @return void
 */
void displayUtilGrid()
{
	std::cout << " Utility::maxIter: "  << maxIter << std::endl;
	std::cout << " Utility::numThreads: "  << numThreads << std::endl;
	std::cout << " Utility::numTasks: "  << numTasks << std::endl;
}




}
