#ifndef UTIL_H
#define UTIL_H

#include <utility>
#include <cstddef>  // std::size_t
#include <cmath>
#include <mutex>

namespace Utility{

    //Defining struct that captures grid information.
    /*typedef struct {
        double x_max;
        double y_max;
        std::size_t Rows;
        std::size_t locRows;
        std::size_t Cols;
        double h_x;
        double h_y;
    } GlobalGridParams;*/

    //Declaring one instance of GlobalGridParmas
    //extern GlobalGridParams globalGrid_;

    // Handles std::cout resource among threads.
    extern std::mutex mu;

    extern const double PI;

    extern const double K;

    extern const double maxIter;

    void setGridParams(std::size_t totRows, std::size_t totCols, std::size_t numTasks);

    void displayUtilGrid();

    size_t getGlobalRow(size_t task_id, size_t localRow);

     std::pair<double, double> getXY(std::size_t global_row, std::size_t global_col);

    //std::pair<double, double> getXY(int global_row, int global_col);

}


#endif // UTIL_H
