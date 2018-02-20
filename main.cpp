#include <thread>
#include <chrono>
#include <memory>    // smart pointers

#include "task.hpp"
#include "ThreadPool/ThreadPool.hpp"
#include "grid.hpp"

/*void thread_init(int tid)
{
    std::cout << "Thread " << tid << " created\n";
}

template <typename T>
T lval2rval(T input)
{
    return input;
}*/

int main(int argc, char* argv[])
{
	  //static unsigned const num_threads = std::thread::hardware_concurrency();
	const size_t num_threads = Utility::numThreads;
    const size_t num_tasks = Utility::numTasks;

    // Set the global grid_params
    const size_t num_rows = Utility::numRows;
    const size_t num_cols = Utility::numCols;

    // Construct a grid object
    std::shared_ptr <Grid> gridPtr (new Grid (num_rows, num_cols, num_tasks));
    Task::setGridPtr(gridPtr);


    //std::cout <<  " gridPtr.use_count(): "  <<   gridPtr.use_count() << std::endl;

    //Create a pool of num_threads workers
    ThreadPool pool(num_threads, gridPtr);

    Task* tasks[num_tasks];
    std::string task_bound;

    for(size_t i=0; i<num_tasks; i++)
    {

        if(i==0)
            task_bound = "Top";
        else if (i+1 == num_tasks)
            task_bound = "Bottom";
        else
            task_bound = "Interior";

        try
        {
            tasks[i] = new Task(i, task_bound);
            tasks[i]->set_f();
            tasks[i]->init_u();
            tasks[i]->displayGrid();
        }
        catch (std::exception &e)
        {
            std::cout << e.what() << std::endl;

        }

    }

    //Display the initial configration of the grid
    //gridPtr->displayGrid();

    // Set the neighbor task pointers.
    for (size_t tid=0; tid < num_tasks; ++tid)
    {
        if(tid==0)
            tasks[tid]->setNbrs(NULL, tasks[tid+1]);
        else if (tid +1 == num_tasks)
            tasks[tid]->setNbrs(tasks[tid-1], NULL);
        else
            tasks[tid]->setNbrs(tasks[tid-1], tasks[tid+1]);
    }
 
    for(size_t task_id=0; task_id < num_tasks; task_id++)
    {

    	pool.enqueue([task_id, &tasks,&pool]() -> bool {

        if(tasks[task_id]->isPreCondsMet())
		   {
			 tasks[task_id]->updateGrid();
			  tasks[task_id]->setPostConds();

			  // numTasksDone should be increment here
              // check for race conds
			  //pool.numTasksDone ++;
		   }

		   bool ret_val (false);

		   // If this task has not finished all the iterations, return true, we need to enqueue it again.
		   if(not(tasks[task_id]->hasFinishedIters()) )
		   {
			 //std::lock_guard <std::mutex> locker(Utility::mu);
			 //std::cout << "Task: " << tasks[task_id]->task_id_  <<   "main: Hey guys, please reinsert me" << std::endl;
			 ret_val = true;
		   }

		   /*else
		   {
			   std::lock_guard <std::mutex> locker(Utility::mu);
			   std::cout  <<  "main: I am finished, no need to reinsert me "
					   << " iter number " << tasks[task_id]->iter_number
					   << std::endl;
		   }*/

		   return ret_val; }
			   );

       }





    return 0;

}


