/**
    src file: task.cpp
    Purpose: 1) to define Task: pre conditions, perform task and post conditions
			 2) when perform task is jacobi update: define update grid method, getresidual etc.

    @author Raju Ram
    @version 1.0 04/07/18
*/




#include <stdexcept>
#include "task.hpp"
#include <assert.h>
#include <algorithm>  // std::max
#include <limits>     // std::numeric_limits
#include <chrono>


//Shared pointer to the grid
std::shared_ptr<Grid> Task::gridPtr;

// Array of grid pointers
std::vector< std::shared_ptr<Task> > Task::tasks;

//Shared pointer to the ThreadPool
std::shared_ptr<ThreadPool> Task::TPPtr;

// set initial global residual as very large number
//double Task::global_res_ = std::numeric_limits<double>::max();

// Flag that is set to true when residual termination criteria is met, initially it is set to false.
std::atomic<bool> Task::resTermFlagSet(false);

//global varaible isAllTasksDone initialized to false
std::vector<bool> Task::isAllTasksDone(Utility::numTasks, false);
//isAllTasksDone.resize(Utility::numTasks, false);


//std::vector<double> Task::global_res_(Utility::numTasks, 0.0);

// Stores the global information of the grid
//GlobalGrid Task::globalGrid;
//Task::GlobalGridParams Task::globalGrid_;

/**
    Constructor that sets task properties
	@param tid task id
	@param numTasks total number of tasks
	@param taskLoc it specifies where the task domain lies in 2D grid

    @return void
 */
Task :: Task(size_t tid, size_t numTasks, std::string taskLoc)
{

    this->iter_number = 0;
    this->task_id_ = tid;

    this->row_start_ = gridPtr->numlocRows_ * tid;
    this->row_end_ = row_start_ + gridPtr->numlocRows_; // not included

    this->y_coor_top_ =  gridPtr->y_max_ - (this->task_id_*gridPtr->numlocRows_ * gridPtr->hy_);
    global_res_ = std::numeric_limits<double>::max();

    this->loc_res2_.resize(numTasks, 0.0);
    this->loc_iters_.resize(numTasks,false);


    if(taskLoc.compare("Interior") == 0)
        this->boundary_ = InteriorTask;
    else if((taskLoc.compare("Bottom")) == 0 )
        this->boundary_ = BottomBound;
    else if((taskLoc.compare ("Top")) == 0)
        this->boundary_ = TopBound;
    else
       throw std::invalid_argument("Invalid argumentt, Set task location as 'Interior', 'Bottom' or 'Top'.");

}


////Copy Constructor
//Task :: Task (const Task &task)
//{
//     std::cout << "Copy constructor called" << std::endl;

//    this->task_id_ = task.task_id_;
//    this->row_start_ = task.row_start_;
//    this->row_end_ = task.row_end_;
//    this->num_cols_ = task.num_cols_;
//    int len =  task.num_cols_ * (task.row_end_ - task.row_start_ + 1);
//    this->u_.resize(len);


//    for(size_t i=0; i< task.u_.size(); i++)
//    {
//        this->u_[i] =  task.u_[i];
//    }

//}

//// Copy assignment operator
//Task& Task :: operator= (const Task& task)
//{
//    std::cout << "Copy assignment opeartor called" << std::endl;

//    assert(task.u_.size() == this->u_.size());

//    if(this != &task)
//    {
//        this->task_id_ = task.task_id_;
//        this->row_start_ = task.row_start_;
//        this->row_end_ = task.row_end_;
//        this->num_cols_ = task.num_cols_;

//        for(size_t i=0; i< task.u_.size(); i++)
//        {
//            this->u_[i] =  task.u_[i];
//        }

//    }

//    return *this;
//}



/**
    Destructor that joins the worker thread (in static scheduling)

    @return void
 */
Task :: ~Task()
{
    std::cout << "Destructor called in class Task" << std::endl;

    // only for static scheduling
    for(std::thread &worker: threads_ss)
      {
      	// worker thread can be joined only once.
      	if(worker.joinable())
      	{
      		//std::cout << " Thread is joined now" << std::endl;
      		worker.join();
      	}
      	else
      	{
      		std::cout << " worker thread is not joinable in Task destrucotr" << std::endl;
      	}

      }
}

/**
    Static function: keeps/sets one copy of pointer to the grid in Task class

    @return void
 */
void Task::setGridPtr(const std::shared_ptr<Grid>& ptr)
{
	gridPtr = std::move(ptr);
}

/**
    Static function: getter function

    @return returns one copy of pointer to the grid, in Task class
 */
const std::shared_ptr<Grid>& Task::getGridPtr()
{
	return gridPtr;
}

/**
    Static function: keeps/sets one copy of pointer to threadpool object, in Task class

    @return void
 */
void Task::setTPPtr(const std::shared_ptr<ThreadPool>& ptr)
{
	TPPtr = std::move(ptr);
}

/**
    Static function: keeps/sets one copy of a vector containing the pointers to all Task objects, in Task class

    @return void
 */
void Task::setTaskObjs(const std::vector< std::shared_ptr<Task> >&  taskVec)
{
	Task::tasks = taskVec;
}

/**
    Set top and bottom neighbor for the current task object

    @return void
 */
void Task::setNbrs(const std::shared_ptr<Task> up, const std::shared_ptr<Task> down)
{

    this->nbrs_.up = up;
    this->nbrs_.down = down;

    /*std::lock_guard <std::mutex> locker(Utility::mu);
    std::cout << "Task::setNbrs : taskid " << this->task_id_ <<  " up " << this->nbrs_.up  <<  "  down " <<   this->nbrs_.down << std::endl;
	*/
}


/**
    Static thread scheduling with uniform domain decomposition,
    required ONLY in the **static scheduling**

    @return void
 */
void Task::doStaticScheduling()
{
	//std::cout << "(" << this->task_id_ << ")" << "  doStaticScheduling\n";

	// Define the current task
	auto task = [this]() -> std::pair<std::vector<double>, std::vector<double>> {

		bool isTaskNotFinished(true);

		//do
		{
			//std::cout << "(" << this->task_id_ << ")" << " In the do while loop\n";
			if( this->isPreCondsMet())
			{

				this->performTask();
				//this->sleepCycles(10000);
				this->setPostConds();
			}

			// If this task has finished all the iterations,  task should be finished.
			if(this->hasFinishedIters())
			{
				isTaskNotFinished = false;
			}

		} //while (this->iter_number < 10);
		// isTaskNotFinished &&

		std::pair<std::vector<double>, std::vector<double>> ret_pair(gridPtr->u1_, gridPtr->u2_);
		return ret_pair;
	};

	//std::cout << "Before\n";
	if(this->task_id_ == 0)
	{
		gridPtr->displayGrid(gridPtr->u1_);
		gridPtr->displayGrid(gridPtr->u2_);
	}

	// One thread is assigned/bound to a specific task
	//this->threads_ss.emplace_back(task);
	std::future < std::pair<std::vector<double>, std::vector<double>> >
	fut =  std::async(std::launch::async, task);

	//std::cout << "After\n";
	auto fut_get = fut.get();

	if(this->task_id_ == 0)
	{
		gridPtr->displayGrid(fut_get.first);
		gridPtr->displayGrid(fut_get.second);
	}

}



// Check with neighboring tasks if pre conditions are met

/**
    Checks if a local task has met the pre conditions, based on iteration of top and bottom task

    @return true if pre conditions are met, else false
 */
bool Task::isPreCondsMet()
{
    //debug: pre conds are always met.
    return true;

	//std::cout << "Inside isPreCondsMet() function\n";

    /*int diff1(0), diff2(0);

    if(this->boundary_ != TopBound)
    {
    	assert(this->nbrs_.up != 0);
    	diff1= this->iter_number - this->nbrs_.up->iter_number;
    }

    if(this->boundary_ != BottomBound)
    {

    	assert(this->nbrs_.down != 0);
    	diff2= this->iter_number - this->nbrs_.down->iter_number;
    }

    // Sanity check 
    assert( std::max( abs(diff1), abs(diff2) ) <= 1 );

    // both the differences can be 0 or -1, else we do not update, 
    // +1 is possible but updates are not allowed 
    if ( (diff1 == 0 || diff1 == -1) && (diff2 == 0 || diff2 == -1)  )
        return true;
    else
        return false;*/
}


/**
    the local task contains, this task is performed by a thread

    @return void
 */
void Task::performTask()
{
     // residual of previous iteration

	this->updateResidual();

    // only one task changes the max iterations
    if( this->isResTerCriMet())
    {
        this->changeMaxIter();
    }

    this->updateGrid();

    //return;
}

/**
    let the current task sleep for nCycles
	@param  nCycles numbe of cpu cycles

    @return void
 */
void Task::sleepCycles (int nCycles )
{

  long cycleStart(__rdtsc());

  long upperCycleLimit ( cycleStart
                       + nCycles );

  long cycle (__rdtsc() );

  while ( cycle < upperCycleLimit )
  {
    cycle = __rdtsc();
  }
}


/**
    set the post conditions for each task, here it means iteration increment

    @return void
 */
void Task::setPostConds()
{
    this->iter_number ++;

    /*if(this->task_id_ == 0); // (Utility::numTasks - 1) )
    {
    	std::lock_guard <std::mutex> locker(Utility::mu);

		for (auto taskptr: tasks)
		{
			std::cout << taskptr->iter_number << "\t" << std::flush;
		}

		std::cout << std::endl;

    //std::cout  <<  "("  << this->iter_number
    //		<< ")  task_id: " << task_id_   <<  " In Post Conds Function()"  << std::endl;
    }*/
}

/**
    check if a local task has finished maximum number of iterations or converged

    @return true if a task has finished, false if it has not finished
 */
bool Task::hasFinishedIters()
{
	 //std::cout << "task.cpp-> Utility::maxIter: "  << Utility::maxIter << std::endl;

    // Sanity check: curr itearion number should be smaller than the maximum number of iterations.
    assert(this->iter_number <= Utility::maxIter);

    bool ret_val(false);

    if (this->iter_number == Utility::maxIter)
    {
      ret_val = true;
      /*
      std::lock_guard <std::mutex> locker(Utility::mu);
      std::cout << "Task.cpp: ("  <<  this->task_id_ << "): Iterations are " << this->iter_number << std::endl;*/
    }

    /*{
    	std::lock_guard <std::mutex> locker(Utility::mu);

    	std::cout << "("  <<  this->task_id_ << ")"
    	    		  << "my iter_number " << this->iter_number
    	              << " max iter_number " << Utility::maxIter
    	              << " iters finished?  "
    	              << ( ret_val ? "true" : "false")
    	              << std::endl;
    } */


    return ret_val;//(this->iter_number == Utility::maxIter) ? true : false;

}


/**
    set the RHS f_ vector, used in forming the matrix system in finite difference discretization

    @return void
 */
void Task::set_f()
  {
      /*{
          std::lock_guard <std::mutex> locker(Task::mu);
          std::cout << "Points for taskID" << this->task_id_ << std::endl;
      }*/

      size_t numCols, ind;
      std::pair<double, double> p;

      numCols = this->gridPtr-> numCols_;

      for(size_t iRow= this->row_start_ ; iRow < this->row_end_ ; iRow++)
      {
          for(size_t iCol=0  ; iCol < numCols ; iCol++)
          {
              //std::cout << "("  << iRow << "," << iCol << ")" << "\t";

               p = Utility::getXY(iRow, iCol);
               ind = iRow * numCols + iCol;

               //std::cout << "ind: " << ind << std::endl;

               //std::cout << "("  << p.first << "," << p.second << ")" << "\t";

               gridPtr->f_[ind] = 4.0 * Utility::PI * Utility::PI  * sin(2 * Utility::PI * p.first) *
                        sinh(2* Utility:: PI * p.second);
          }
      }


      // old code
      /*for(size_t ind=0; ind<f_.size(); ind++)
      {
          local_row = (int)  ind/this->num_cols_;
          local_col = ind%this->num_cols_;

          global_col = local_col;
          global_row = Utility::getGlobalRow(this->task_id_, local_row);

           std::pair<double, double> p = Utility::getXY(global_row, global_col);


          this->f_[ind] = 4.0 * Utility::PI * Utility::PI  * sin(2 * Utility::PI * p.first) *
                   sinh(2* Utility:: PI * p.second);
      }*/


  }

/**
    Initiliase the u_ vector

    @return void
 */
void Task::init_u()
{
    if (this->boundary_ == TopBound)
    {
        // Initliase only the top row
        const double mult = sinh(2 * Utility::PI* 1);
        double x;
        //if(task_id_ == 0)
          //  std::cout << "mult is: " << mult << " pi is: " << Utility::PI << std::endl;

        for(size_t i=0; i< gridPtr->numCols_  ; i++)
        {
            x = i * gridPtr->hx_;
            gridPtr->u1_[i] = sin(2 * Utility::PI *x)*mult;
            gridPtr->u2_[i] = gridPtr->u1_[i];
        }
    }

    // All the other entries remains zero. They have been initiliased by zero in the grid constructor.
}


/**
    Display the contents of the grid in the current task

    @return void
 */
void Task::displayGrid()
{
    std::lock_guard <std::mutex> locker(Utility::mu);
    std::cout << "\n ####################################################\n"  << std::endl;
    std::cout << "Task contents....  \n";

    std::cout << "task_id: " << this->task_id_ << std::endl;
    std::cout << "row_start: " << row_start_ << std::endl;
    std::cout << "row_end: " << row_end_ << std::endl;
    std::cout << "y_coor_top: "  << y_coor_top_ << std::endl;
    std::cout << "boundary: " << boundary_ << std::endl;


    /*for(size_t i=0; i < f_.size(); i++)
    {
        std::cout <<  f_[i] << "\t";

        if( (i+1)% num_cols_ ==0)
            std::cout << std::endl;
    }

    if(this->boundary_ == InteriorTask)
        std::cout << "task_id: " << task_id_  << " up nbr: " << this->nbrs_.up->task_id_ << " down nbr: " << this->nbrs_.down->task_id_<< std::endl;
    else if(this->boundary_ == TopBound)
        std::cout << "task_id: " << task_id_  <<  " down nbr: " << this->nbrs_.down->task_id_<< std::endl;
    else if(this->boundary_ == BottomBound)
    std::cout << "task_id: " << task_id_  << " up nbr: " << this->nbrs_.up->task_id_ << std::endl;
     */


}



/**
    This function performs Jacobi update. It computes start and end global row of a task

    @return void
 */
void Task::updateGrid()
{
	if(Utility::debug)
	{
		std::lock_guard <std::mutex> locker(Utility::mu);
		std::cout << "Task: " << this->task_id_ <<  " updateGrid() called "  << std::endl;
	}

    double hxsqinv, hysqinv, mult, up, down, left, right;

    hxsqinv = 1.0/(pow(gridPtr->hx_, 2));
    hysqinv = 1.0/(pow(gridPtr->hy_, 2));
    size_t startRow, endRow;

    mult = 1.0/(pow(Utility::K,2) + 2*(hxsqinv + hysqinv) );
    // local start and end rows
    startRow = Utility::getGlobalRow(this->task_id_, 0);
    endRow = startRow + gridPtr->numlocRows_;   // endRow not included


    if(this->boundary_ == TopBound)
    {
        startRow ++;    // skip the first local row
    }
    else if (this->boundary_ == BottomBound)
    {
       endRow --;    // skip the last local row
    }

    size_t numCols = gridPtr->numCols_;
    size_t endCol = numCols - 1;
    const std::vector <double> &src = gridPtr-> getSrcVec(this->iter_number); 

    std::vector <double> &dest = gridPtr-> getDestVec(this->iter_number);
    size_t ind;


    /*{
        std::lock_guard <std::mutex> locker(Utility::mu);

        if(src == gridPtr->u1_)
        {
            std::cout << "task_id: " << task_id_ << " iter_number: " << iter_number  << " src vector is u1_ "  << std::endl;
        }
        else if (src == gridPtr->u2_)
        {
            std::cout << "task_id: " << task_id_ << " iter_number: " << iter_number  << " src vector is u2_ "  << std::endl;

        }
        //std::cout << "task_id: " << task_id_ << " start: " << startRow << " end: " << endRow << std::endl;
        //std::cout << "task_id: " << task_id_ << " ind-numcol " << start-num_cols_ <<  " u_src_[start - num_cols_]: " << u_src_[start - num_cols_] << std::endl;
    }*/

        for (size_t iRow=startRow; iRow < endRow; iRow++)
        {
             // Skip the left and right boundary
            for(size_t iCol=1; iCol < endCol; iCol++)
            {

                ind = iRow * numCols + iCol;

                up = src[ind - numCols];
                left = src[ind - 1];
                right = src[ind + 1];
                down = src[ind + numCols];

                //debug
                /*std::fill(gridPtr->f_.begin(), gridPtr->f_.end(), 0.0);
                hxsqinv = hysqinv = 1;
                mult = 0.25;*/

               dest[ind] = mult * ( gridPtr->f_[ind] + hxsqinv * ( left + right )
                                            + hysqinv * (up + down) );
            }

        }


       // if(Utility::maxIter == (this->iter_number-2))
       // this->gridPtr->displayGrid(dest);


}

/**
    update the residual

    @return void
 */
void Task::updateResidual()
{
	double local_residual = this->getLocResidual();

	//synchronise the residual among all the threads.
	this->syncResidual(local_residual);
}


/**
    Every task compute their local residual

    @return local residual
 */
double Task::getLocResidual()
{
	size_t startRow = Utility::getGlobalRow(this->task_id_, 0);
	size_t endRow = startRow + gridPtr->numlocRows_;   // endRow not included
	const size_t numCols = gridPtr->numCols_;
	const size_t endCol = numCols - 1;
	double loc_residual = 0.0;
	size_t ind(0);

	if(this->boundary_ == TopBound)
	{
		startRow ++;    // skip the first local row
	}
	else if (this->boundary_ == BottomBound)
	{
		endRow --;    // skip the last local row
	}

	const double hxsqinv = 1.0/(pow(gridPtr->hx_, 2));
	const double hx_coeff =  -hxsqinv;
	const double hysqinv = 1.0/(pow(gridPtr->hy_, 2));
	const double hy_coeff =  -hysqinv;
	const double center_coeff = 2*(hxsqinv + hysqinv) + pow(Utility::K,2);
	double left, right, up, down, center, temp(0.0);

	// isPreCondMet() holds only for the SecVec, DestVec might have race conditiones.
	const std::vector<double>& dest = gridPtr->getSrcVec(this->iter_number);

	for (size_t iRow=startRow; iRow < endRow; iRow++)
	{
		// Skip the left and right boundary
		for(size_t iCol=1; iCol < endCol; iCol++)
	    {
			 ind = iRow * numCols + iCol;

			 up = dest[ind - numCols];
			 left = dest[ind - 1];
			 center = dest[ind];
			 right = dest[ind + 1];
			 down = dest[ind + numCols];

			temp = gridPtr->f_[ind] - (center_coeff * (center) +
					hx_coeff * (left + right) + hy_coeff * (up + down)) ;

		    loc_residual += (temp*temp);
	    }
	}

	return loc_residual;

}


/**
    Every task synchronises their local residual to form global residual
    This is a tricky funtion, good exercise to write race condition free code

    @return void
 */
void Task::syncResidual(double loc_residual)
{
	size_t ind = this->iter_number % Utility::numTasks;
	bool isLocAllTasksDone(false);

	this->loc_res2_[ind] = loc_residual;
	// set the local iter markers for index ind
	this->loc_iters_[ind] =  true;

	size_t numTasksFinished(0);

	for(std::shared_ptr<Task> taskptr : tasks)
	{
		// Check if this iteration (hash value: ind) is finished in all the tasks.
		if (taskptr->loc_iters_[ind])
		{
			numTasksFinished ++;
		}
	}

	// This should be thread safe
	// Setting local isLocAllTasksDone based on local numTasksFinished and  global isAllTasksDone variable.
	if (numTasksFinished == Utility::numTasks)
	{
		std::lock_guard <std::mutex> locker(Utility::mu_task);

		// No other thread has set it to false.
		if(isAllTasksDone[ind] == false)
		{
			isAllTasksDone[ind] = true;
			isLocAllTasksDone = true;
		}
	}


        {
            std::lock_guard <std::mutex> locker(Utility::mu);
            //#####################################################
            std::cout  <<  " ("  << this->iter_number
                           << ")  task_id: " << task_id_ <<  " local_res: "
                           << loc_residual   << std::endl;
        }

    // If 'indDoneAllTasks' is true here, it means all tasks has finished their iteration hash 'ind'
	// global reduce of the residual
	//if(indDoneAllTasks)

	// Only one thread should do this per iteration
	if(isLocAllTasksDone)
	{
		this->global_res_ = 0.0;

		for(std::shared_ptr<Task> taskptr : tasks)
		{
			this->global_res_ += taskptr->loc_res2_[ind];

			// mark the counter to false in all the task at location 'ind', i.e flush (reset) true markers.
			taskptr->loc_iters_[ind] = false;
		}

                {
                    std::lock_guard <std::mutex> locker(Utility::mu);
                    //#####################################################
                    std::cout  <<  " ##################################################### ("  << this->iter_number
                                   << ")  task_id: " << task_id_   <<  " ,global_res: "
                                   << global_res_   << std::endl;
                    /*std::cout  <<  " ##################################################### ("  << this->iter_number
                                   << ")  task_id: " << task_id_  <<  " , ind: "  << ind <<  " ,global_res: "
                                   << global_res_   << std::endl;*/
                }

		this->global_res_ = std::sqrt(this->global_res_);

		// debug
		//global_res = this->iter_number + 1;

		// Debug: Set the global residual and number of iterations in the grid class.
		gridPtr->grid_gres_  = this->global_res_;
		gridPtr->grid_iters_ = this->iter_number;




		// Set stop to true and break the conditional wait for all the threads
		/*if(global_res < Utility::tol)
		{
			//set stop to true;
			if(TPPtr->stop == false)
				{
					{
					  std::unique_lock<std::mutex> lock(TPPtr->queue_mutex);
					  TPPtr->stop = true;
					}

					TPPtr->condition.notify_all();
				}
		}*/

		//Flush the true markers
	   isAllTasksDone[ind] = false;
	}


}


/**
    Every task locally checks if the residual termination criteria has been met
    By construction of this method: only one task can set it to true.

    @return true if the residual termination criteria has been met, else false
 */

bool Task::isResTerCriMet()
{
	bool ret_val(false);

	if(this->global_res_ < Utility::tol )
	{
		{
			// global synchronisation
			std::lock_guard <std::mutex> locker(Utility::mu_task);

			if(resTermFlagSet ==false)
			{
				resTermFlagSet = true;
				ret_val = true;


				{
					std::lock_guard <std::mutex> loc(Utility::mu);
					std::cout << "-----------------------------------------------------------   ("  << this->iter_number
									  << ") global residual termination criteria met by task: " << this->task_id_
									  << std::endl;


				    /*for(std::shared_ptr<Task> taskptr : tasks)
					{
						std::cout << "("  << taskptr->iter_number
															  << ") I am task " << taskptr->task_id_
															  << std::endl;
				    }*/

				}

			}

		}


	}

	return ret_val;

	//return resTermFlagSet;
}



/**
    Update the global max iteration.
    This should be called by one task

    @return true if the residual termination criteria has been met, else false
 */
void Task::changeMaxIter()
{
	assert(Task::resTermFlagSet == true);

	size_t max_iter(0);

	for(std::shared_ptr<Task> taskptr : tasks)
	{
		// This has to be done in a threadsafe manner
		if(taskptr->iter_number > max_iter)
		{
			max_iter = taskptr->iter_number;
		}

	}

	assert(max_iter < Utility::maxIter);

	{
		std::lock_guard <std::mutex> locker(Utility::mu_task);
	  //sleep
		//Utility::maxIter = max_iter + Utility::numTasks;
		//std::this_thread::sleep_for (std::chrono::seconds(1));

		//safe condition
		Utility::maxIter = this->iter_number + Utility::numTasks;
	}

	{
		std::lock_guard <std::mutex> locker(Utility::mu);
		std::cout <<  "--------------------------------------------------------     ("  << this->iter_number << ")  Task::calFinalPhase(): task "
								<< this->task_id_ <<  "  ,Utility::maxIter set to " << Utility::maxIter
								<< std::endl;
	}

}



