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
#include "./ThreadPool/ThreadPool.hpp"


class Task {

  private:

    size_t row_start_;
    size_t row_end_;
    double y_coor_top_;

    //global residual
    static double global_res_;
    //local residual square of num_tasks
    std::vector<double> loc_res2_;


   //local iteration counter of num_tasks
   // If we stores only one iteration number, then there will be race conditions.
    // At a given time at max num_tasks iterations are possible for the tasks
    std::vector<bool> loc_iters_;


    /*struct data_ {
        double src;
        double target;
    };*/

    typedef struct{
        std::shared_ptr<Task> up;
        std::shared_ptr<Task> down;
    } Nbrs;

    Nbrs nbrs_;

    enum TaskBoundary {InteriorTask, BottomBound, TopBound} boundary_;

    // One copy of shared pointer to global grid object
    static std::shared_ptr<Grid> gridPtr;

    // One copy of shared pointer to threadpool object
    static std::shared_ptr <ThreadPool> TPPtr;

    //One copy of vector that keeps the pointers to task objects
     static std::vector< std::shared_ptr<Task> > tasks;

  public:

    // make it private later on
    size_t iter_number;
    size_t task_id_;

    Task(size_t tid, size_t numTasks, std::string taskLoc);
    //Copy constructor
    //Task (const Task &task);
    // Copy assignment operator
    //Task& operator= (const Task &task);
    ~Task();

    // Flag that is set to true when residual termination criteria is met.
    static std::atomic<bool> resTermFlagSet;

    static void setGridPtr(const std::shared_ptr<Grid>& ptr);
    static const std::shared_ptr<Grid>& getGridPtr();

    static void setTPPtr(const std::shared_ptr<ThreadPool>& ptr);

    static void setTaskObjs(const std::vector< std::shared_ptr<Task> >&  taskVec);
    //static const std::shared_ptr<Grid>& getTaskObjs();

    void set_f();
    void init_u();
    void displayGrid();
    void updateGrid();
    double computeResidual();

    size_t getIterNum()
    {
        return iter_number;
    }

     void setNbrs(const std::shared_ptr<Task> up, const std::shared_ptr<Task> down);

     bool isPreCondsMet();
     void setPostConds();

     bool hasFinishedIters();
     bool isResTerCriMet();

     void calFinalPhase();

};

#endif // TASK_H
