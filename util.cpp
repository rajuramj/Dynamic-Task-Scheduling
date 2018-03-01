#include "util.hpp"
#include "task.hpp"
#include <cassert>


namespace Utility {

    const double PI = 4.0 * atan(1.0);
    const double K = 2 * PI;

    size_t maxIter;
    size_t numThreads;
    size_t numTasks;
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

    std::pair<double, double> getXY(std::size_t global_row, std::size_t global_col)
    //std::pair<double, double> getXY(int global_row, int global_col)
    {
        double x,y;

        x = global_col * Task::getGridPtr()->hx_;
        y =  ((Task::getGridPtr()->numRows_ - 1) - global_row)* (Task::getGridPtr()->hy_);

        return std::make_pair(x,y);
    }

    size_t getGlobalRow(size_t task_id, size_t localRow)
    {
        //Assuming same number of local rows for each task, i.e: uniform partitioning
        size_t globalRow = (Task::getGridPtr()->numlocRows_)*(task_id) + localRow;
        return globalRow;
    }

    void setParams(const size_t iters, const size_t threads, const size_t tasks)
    {
    	maxIter = iters;
    	numThreads = threads;
		numTasks = tasks;
    }



   void displayUtilGrid()
   {
	   std::cout << " Utility::maxIter: "  << maxIter << std::endl;
	   std::cout << " Utility::numThreads: "  << numThreads << std::endl;
	   std::cout << " Utility::numTasks: "  << numTasks << std::endl;
   }




}
