/**
    src file: main.cpp
    Purpose: To run different tasks with the threadpool framework

    @author Raju Ram
    @version 1.0 04/07/18
*/


#include <thread>
#include <chrono>
#include <memory>    // smart pointers

#include "task.hpp"
#include "ThreadPool/ThreadPool.hpp"
#include "grid.hpp"

#include "cycletimer.hpp"
#include "Timing.hpp"  // Code taken from ACE project

/*void thread_init(int tid)
{
    std::cout << "Thread " << tid << " created\n";
}*/

int main(int argc, char* argv[])
{

    //assert(argc >=6  && "num_rows, num_cols, max_iter, num_threads, num_tasks, optional(cpu_cycles, deviation) ");

    if( argc < 6 ) {
        throw std::runtime_error ("use as <num_rows>, <num_cols>, <max_iter>, <num_threads>, <num_tasks>, optional(<cpu_cycles>, <deviation>");
      }


    std::string s1 = argv[1],s2= argv[2],s3 = argv[3], s4 = argv[4], s5 = argv[5];
    size_t arg6(0);
    double arg7(0.0);

    if(argc > 6)
        arg6= std::stoi(argv[6]);
    /*else
        std::cout << "Please provide CPU cycles\n";*/

    if(argc==8)
    	arg7= std::stod(argv[7]);

    std::cout << "starting with "
                  << s3 << " total iterations, "
                  << s4 << " threads, "
                  << s5 << " tasks, "
				  << arg6 << " CPU cycles per task, "
				  << arg7 << " hetrogeneity quotient "
				  <<  " .... \n \n "  <<  std::endl;



    //assert(argc == 7 || argc==8);

    const size_t num_rows = std::stoi(s1);
    const size_t num_cols = std::stoi(s2);

    // void setParams(const size_t iters, const size_t threads, const size_t tasks);
    Utility::setParams(std::stoi(s3), std::stoi(s4), std::stoi(s5));

    //Utility::displayUtilGrid();

    //memory leak
    //char *s = (char*) (malloc(10000* sizeof(char)));


    //static unsigned const num_threads = std::thread::hardware_concurrency();
   	const size_t num_threads = Utility::numThreads;
    const size_t num_tasks = Utility::numTasks;

    // Construct a grid object
    std::shared_ptr <Grid> gridPtr (new Grid (num_rows, num_cols, num_tasks));
    Task::setGridPtr(gridPtr);


    //std::cout <<  " gridPtr.use_count(): "  <<   gridPtr.use_count() << std::endl;



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
        	//possibility of some memory leak here.
            tasks[i] = std::shared_ptr<Task> ( new Task(i, num_tasks, task_bound));
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



    //Display the initial configration of the grid
    //gridPtr->displayGrid();

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

	std::vector<double> cycles(num_tasks,0.0);
	double min_cycle =  arg6*(1 - arg7);
	double max_cycle =  arg6*(1 + arg7);
    double step_size =   (max_cycle - min_cycle)/(num_tasks - 1);

    cycles[0] = min_cycle;
    for(int i=1;i < (num_tasks-1); i++)
    {
    	cycles[i] = min_cycle + i*step_size;
    }
    cycles[num_tasks-1] = max_cycle;

    /*std::cout << "CPU Cycles with monotone uniform partioning\n";
    for (double &cycle : cycles)
    {
    	std::cout << cycle << "\n";
    }*/

	// normal distribution
	//std::default_random_engine gen;
	//std::normal_distribution<double> dist(arg6,arg6*arg7);

	// Uniform distribution
	/*std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	//std::uniform_real_distribution<> dist(arg6*(1.0- arg7/2.0 ), arg6*(1.0 + arg7/2.0 ));
	std::uniform_real_distribution<> dist1(arg6*(1.0- arg7), arg6);
	std::uniform_real_distribution<> dist2(arg6, arg6*(1.0 + arg7 ));

	std::cout << "Randnumbers:\n";
	double rand;
	const size_t half = int(num_tasks)/2.0;

	for( size_t i=0; i < num_tasks; i++)
	{
		//double rand = Utility::distribution(Utility::generator);

		if( i < half )
			cycles[i] = (dist1(gen) > 0.0) ? dist1(gen) : 0.0;
			//cycles[i] = arg6*0.01 ;
		else
			cycles[i] = (dist2(gen) > 0.0) ? dist2(gen) : 0.0;
			//cycles[i] = arg6*100 ;

		std::cout << cycles[i] << std::endl;
	}*/

	//Create a pool of num_threads workers -> Dynamic scheduling
	std::shared_ptr<ThreadPool> pool = std::make_shared<ThreadPool>(num_threads, gridPtr);
	Task::setTPPtr(pool);

		// global timer start, timer in sec
		Utility::gtime.reset();

		// start the global time, timer in cycles
		Utility::gtiming.start();

		// Static scheduling
		/*for (auto taskptr: tasks)
		{
			taskptr->doStaticScheduling();
		}*/

		for(size_t task_id=0; task_id < num_tasks; task_id++)
		{

			//task_id, &tasks
			// ,cycles
            pool->enqueue([task_id, tasks, cycles]() -> bool {

			bool ret_val (false);

			if(tasks[task_id]->isPreCondsMet())
			   {
				// Syntheic tasks
                tasks[task_id]->sleepCycles(cycles[task_id]);

				// Peform Jacobi update
                //tasks[task_id]->performTask();

				tasks[task_id]->setPostConds();
			   }

			   // If this task has not finished all the iterations, return true, we need to enqueue it again.
			   if(not(tasks[task_id]->hasFinishedIters()) )
			   {
				 ret_val = true;
			   }

			   return ret_val;

			});  // end of lambda function definition

		 }  // end of for loop


    return 0;

}     // end of main() function

