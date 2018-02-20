#include "util.hpp"
#include "task.hpp"
#include <cassert>


namespace Utility {

    const double PI = 4.0 * atan(1.0);
    const double K = 2 * PI;


    const double maxIter = 5000;
    const size_t numCols = 100;

    const size_t numRows = 100;
    const size_t numThreads = 15;

    //assert(numRows + numThreads != 0);
    //const size_t numTasks = numRows/numThreads;
    const size_t numTasks = 10;

    const bool debug = false;

    // Handles std::cout resource among threads.
    std::mutex mu;

    std::pair<double, double> getXY(std::size_t global_row, std::size_t global_col)
    //std::pair<double, double> getXY(int global_row, int global_col)
    {
        double x,y;

        x = global_col * Task::gridPtr->hx_;
        y =  ((Task::gridPtr->numRows_ - 1) - global_row)* (Task::gridPtr->hy_);

        return std::make_pair(x,y);
    }

    size_t getGlobalRow(size_t task_id, size_t localRow)
    {
        //Assuming same number of local rows for each task, i.e: uniform partitioning
        size_t globalRow = (Task::gridPtr->numlocRows_)*(task_id) + localRow;
        return globalRow;
    }



   /*void displayUtilGrid()
   {
       std::cout << "Task::globalGrid_.Cols:  " << Utility::globalGrid_.Cols << std::endl;
       std::cout << "Task::globalGrid_.Rows:  " << Utility::globalGrid_.Rows << std::endl;
       std::cout << "Task::globalGrid_.locRows:  " << Utility::globalGrid_.locRows << std::endl;

       std::cout << "Task::globalGrid_.x_max:  " << Utility::globalGrid_.x_max << std::endl;
       std::cout << "Task::globalGrid_.y_max:  " << Utility::globalGrid_.y_max << std::endl;

       std::cout << " Task::globalGrid_.h_x:  " <<  Utility::globalGrid_.h_x  << std::endl;
       std::cout << " Task::globalGrid_.h_y:  " <<  Utility::globalGrid_.h_y  << std::endl;

   }

    */


}
