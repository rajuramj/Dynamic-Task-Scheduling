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
//int main()
{

	assert(argc == 6);
    std::string s1 = argv[1],s2= argv[2],s3 = argv[3], s4 = argv[4], s5 = argv[5];

    const size_t num_rows = std::stoi(s1);
    const size_t num_cols = std::stoi(s2);
    Utility::setParams(std::stoi(s3), std::stoi(s4), std::stoi(s5));

    Utility::displayUtilGrid();


    //static unsigned const num_threads = std::thread::hardware_concurrency();
   	const size_t num_threads = Utility::numThreads;
    const size_t num_tasks = Utility::numTasks;

    // Construct a grid object
    std::shared_ptr <Grid> gridPtr (new Grid (num_rows, num_cols, num_tasks));
    Task::setGridPtr(gridPtr);


    //std::cout <<  " gridPtr.use_count(): "  <<   gridPtr.use_count() << std::endl;

    //Create a pool of num_threads workers
    //ThreadPool pool(num_threads, gridPtr);

    std::shared_ptr<ThreadPool> pool = std::make_shared<ThreadPool>(num_threads, gridPtr);
    Task::setTPPtr(pool);


    //Task* tasks[num_tasks];
    std::vector< std::shared_ptr<Task>> tasks(num_tasks);
    std::string task_bound;
    //Task *tptr;

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
            tasks[i] = std::shared_ptr<Task> (new Task(i, num_tasks, task_bound));
            tasks[i]->set_f();
            tasks[i]->init_u();
            //tasks[i]->displayGrid();
        }
        catch (std::exception &e)
        {
            std::cout << e.what() << std::endl;

        }

    }

    // set task objetcs in task class
    Task::setTaskObjs(tasks);

    /*for (auto taskptr: tasks)
	{
    	std::cout <<  "main tasks" <<taskptr << std::endl;
	}

    for (auto taskptr: Task::tasks)
    {
       std::cout << "Task::tasks  " << taskptr << std::endl;
    }
*/

//    tasks[0]->setNbrs(NULL, tasks[1]);
//    tasks[1]->setNbrs(tasks[0], NULL);


    //Display the initial configration of the grid
    gridPtr->displayGrid();

		// Set the neighbor task pointers.
	for (size_t tid=0; tid < num_tasks; ++tid)
		{
			if(tid==0)
				tasks[tid]->setNbrs(nullptr, tasks[tid+1]);
			else if (tid +1 == num_tasks)
				tasks[tid]->setNbrs(tasks[tid-1], nullptr);
			else
				tasks[tid]->setNbrs(tasks[tid-1], tasks[tid+1]);
		}

         for (auto taskptr: tasks)
        {

            std::lock_guard <std::mutex> locker(Utility::mu);
            std::cout << taskptr << std::endl;
            //taskptr->updateGrid();
            //taskptr->setPostConds();
        }

		


		for(size_t task_id=0; task_id < num_tasks; task_id++)
		{
			//task_id, &tasks
			pool->enqueue([task_id, tasks]() -> bool {


			if(tasks[task_id]->isPreCondsMet())
			   {
				 tasks[task_id]->updateGrid();
				 double residual = tasks[task_id]->computeResidual();

				 {
					 std::lock_guard <std::mutex> locker(Utility::mu);
					 std::cout << "(" << tasks[task_id]->getIterNum()  << ") task_id: " << task_id << "   residual:  " << residual << std::endl;
				 }


				 tasks[task_id]->setPostConds();
			   }

			   bool ret_val (false);

			   // If this task has not finished all the iterations, return true, we need to enqueue it again.
			   if(not(tasks[task_id]->hasFinishedIters()) )
			   {
				 //std::lock_guard <std::mutex> locker(Utility::mu);
				 //std::cout << "Task: " << tasks[task_id]->task_id_  <<   "main: Hey guys, please reinsert me" << std::endl;
				 ret_val = true;
			   }

			   if( !Task::resTermFlagSet   &&  tasks[task_id]->isResTerCriMet()  )
				{
				   tasks[task_id]->calFinalPhase();
				}

			   
			   //bool ret_val (false);
			   return ret_val;

		});  // end of lambda function definition

		   }  // end of for loop


    return 0;

}     // end of main() function

