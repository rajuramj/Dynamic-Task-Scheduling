#ifndef UTIL_H
#define UTIL_H

#include <utility>
#include <cstddef>  // std::size_t
#include <cmath>
#include <mutex>

namespace Utility{

   // Handles std::cout resource among threads.
    extern std::mutex mu;

    extern const double PI;

    extern const double K;

    extern const double maxIter;

    extern const size_t numTasks;

    extern const size_t numRows;

    extern const size_t numCols;

    extern const size_t numThreads;

    void setGridParams(std::size_t totRows, std::size_t totCols, std::size_t numTasks);

    void displayUtilGrid();

    size_t getGlobalRow(size_t task_id, size_t localRow);

    std::pair<double, double> getXY(std::size_t global_row, std::size_t global_col);
}


#endif // UTIL_H
