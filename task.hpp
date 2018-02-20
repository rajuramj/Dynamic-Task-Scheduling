#ifndef TASK_H
#define TASK_H

#include <iostream>
#include <vector>
#include <string>
#include <cstddef>  // std::size_t
#include <memory>  // std::shared_ptr
#include "grid.hpp"
#include "task.hpp"
#include  "util.hpp"

class Task {

  private:

    size_t row_start_;
    size_t row_end_;
    double y_coor_top_;

    /*struct data_ {
        double src;
        double target;
    };*/

    typedef struct{
        Task* up;
        Task* down;
    } Nbrs;

    Nbrs nbrs_;

    enum TaskBoundary {InteriorTask, BottomBound, TopBound} boundary_;


  public:

    // make it private later on
    size_t iter_number;
    size_t task_id_;

    Task(int tid,  std::string taskLoc);
    //Copy constructor
    //Task (const Task &task);
    // Copy assignment operator
    //Task& operator= (const Task &task);
    ~Task();

    // Shared pointer to global grid object
    static std::shared_ptr<Grid> gridPtr;

    static void setGridPtr(const std::shared_ptr<Grid>& ptr);

    void set_f();
    void init_u();
    void displayGrid();
    void updateGrid();

    size_t getIterNum()
    {
        return iter_number;
    }

     void setNbrs(Task* up, Task* down);

     bool isPreCondsMet();
     void setPostConds();

     bool hasFinishedIters();

};

#endif // TASK_H
