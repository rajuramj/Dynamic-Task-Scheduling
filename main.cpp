#include <thread>
#include <chrono>
#include <memory>    // smart pointers

#include "task.h"
#include "ThreadPool/ThreadPool.h"
#include "grid.hpp"


void thread_init(int tid)
{
    std::cout << "Thread " << tid << " created\n";
}


template <typename T>
T lval2rval(T input)
{
    return input;
}


int main(int argc, char* argv[])
{
    // gets allocated for the lifetime of the program
    //static size_t num_threads = 6;

	static unsigned const num_threads = 5;
   // static unsigned const num_threads = std::thread::hardware_concurrency();
    std::cout << "The number of threads: " << num_threads << std::endl;
    size_t num_tasks = Utility::numTasks;
    std::mutex mu;

    // Set the global grid_params
    const size_t num_rows = 8;
    const size_t num_cols = 8;

    //Utility::setGridParams(num_rows, num_cols, num_tasks);
    //Utility::displayUtilGrid();

    // Construct a grid object
    std::shared_ptr <Grid> gridPtr (new Grid (num_rows, num_cols, num_tasks));
    Task::setGridPtr(gridPtr);

    std::cout <<  " gridPtr.use_count(): "  <<   gridPtr.use_count() << std::endl;

    //Create a pool of num_threads workers
    ThreadPool pool(num_threads);
    std::vector< std::future< bool > > results;

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
            //tasks[i]->init_u();
            //tasks[i]->displayGrid();
        }
        catch (std::exception &e)
        {
            std::cout << e.what() << std::endl;

        }

        //tasks[i]->displayGrid();
        //tasks[i]->updateGrid();
    }


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

//    results.emplace_back(
//                 pool.enqueue ([&] {
//                 tasks[0]->displayGrid();
//                 //tasks[0]->updateGrid();
//                  //std::this_thread::sleep_for(std::chrono::seconds(1));
//                 return 1;
//                 })
//                 );



    //std::function <void(void)> f = tasks[0]->displayGrid;
    //results.emplace_back(pool.enqueue(f));

    //for(int task_id=0; task_id < 2; task_id++)
    //int currIter=0;
    //{
      //  std::lock_guard <std::mutex> locker(mu);
        //task_id=0;

    //}


  /*  results.emplace_back(pool.enqueue([&tasks](int answer) {

         tasks[0]->updateGrid();

        return answer; }, 42));*/

    //std::cout << my_result.get() << std::endl;

    //for(size_t currIter=0; currIter < Utility::numIter; currIter++)
    //{
        for(int task_id=0; task_id < num_tasks; task_id++)
        {

         /*results.emplace_back (*/

        	pool.enqueue([task_id, &tasks]() -> bool {

             //std::cout << "my tid is: " <<task_id << std::endl;
               // 29 Jan 2018, suggestion by Daniel
               if(tasks[task_id]->isPreCondsMet())
               {
                 tasks[task_id]->updateGrid();
                  tasks[task_id]->setPostConds();
               }
               //bool ret_val(false);

               bool ret_val (false);

               // If this task has not finished all the iterations, return true, we need to enqueue it again.
               if(not(tasks[task_id]->hasFinishedIters()) )
               {
            	 std::lock_guard <std::mutex> locker(Utility::mu);
                 std::cout <<  "main: Hey guys, please reinsert me" << std::endl;
                 ret_val = true;
               }
               else
               {
            	   std::lock_guard <std::mutex> locker(Utility::mu);
            	   std::cout  <<  "main: I am finished, no need to reinsert me "
            			   << " iter number " << tasks[task_id]->iter_number
						   << std::endl;
               }


               //std::lock_guard <std::mutex> locker(mu);
               //std::cout << "task_d: " << task_id << std::endl;
               //size_t iter_num = tasks[task_id]->getIterNum();

            //tasks[task_id]->updateGrid();
             //std::this_thread::sleep_for(std::chrono::seconds(1));

            //return Task::gridPtr->getDestVec(iter_num);
            //return 0;
  		//co

               return ret_val; }/*)*/
                   );

            //std::cout << "task_d: " << task_id << std::endl;

            //{
             // std::lock_guard <std::mutex> locker(mu);
              //task_id ++;
            //}

           //std::this_thread::sleep_for(std::chrono::seconds(1));
       }





    //}


    /*for(auto & result: results)
    {
        gridPtr->displayGrid(result.get());
        std::cout << std::endl;

    } */




    //pool.printCounter();


//    for(auto && result: results)
//        std::cout << result.get() << ' ';
//        std::cout << std::endl;

    return 0;

}


    /* // gets allocated for the lifetime of the program
    static size_t num_threads = 6;
    static size_t num_tasks = 6;


    // Thread creation
    std::thread t[num_threads];

    for(size_t i=0; i<num_threads; i++)
    {
        t[i] = std::thread(&thread_init, i);
    }


    Task* tasks[num_tasks];



    for(size_t i=0; i<num_tasks; i++)
    {
        tasks[i] = new Task(i,3,6);
        //tasks[i]->displayGrid();
        //tasks[i]->updateGrid();
    }

 //Threads created above joins the master thread
    for(size_t i=0; i<num_threads; i++)
    {
        t[i].join();
    }
   */
