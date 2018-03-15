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

//Constuctor
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

    /*for (size_t i=0; i < u_src_.size(); i++)
    {
        u_src_[i] = task_id_* u_src_.size() + i;
    }*/


    if(taskLoc.compare("Interior") == 0)
        this->boundary_ = InteriorTask;
    else if((taskLoc.compare("Bottom")) == 0 )
        this->boundary_ = BottomBound;
    else if((taskLoc.compare ("Top")) == 0)
        this->boundary_ = TopBound;
    else
       throw std::invalid_argument("Invalid argumentt, Set task location as 'Interior', 'Bottom' or 'Top'.");


     //std::cout << "New task " << this->task_id_ << " created\n";

     /*for(size_t i=0; i<u_.size(); i++)
         {
             std::cout <<  u_[i].target << "\t";

             if( (i+1)% num_cols_ ==0)
                 std::cout << std::endl;
         }*/

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



//Destructor
Task :: ~Task()
{
    std::cout << "Destructor called in class Task" << std::endl;
}

// Static functions
void Task::setGridPtr(const std::shared_ptr<Grid>& ptr)
{
    //Task::gridPtr = std::move(ptr);
	gridPtr = std::move(ptr);
}

const std::shared_ptr<Grid>& Task::getGridPtr()
{
	return gridPtr;
}

void Task::setTPPtr(const std::shared_ptr<ThreadPool>& ptr)
{
    //Task::gridPtr = std::move(ptr);
	TPPtr = std::move(ptr);
}


void Task::setTaskObjs(const std::vector< std::shared_ptr<Task> >&  taskVec)
{
	// Copy the vector containing the pointers to Task objects.
	Task::tasks = taskVec;
}

// Set the neighbours for the current task object
void Task::setNbrs(const std::shared_ptr<Task> up, const std::shared_ptr<Task> down)
{

    this->nbrs_.up = up;
    this->nbrs_.down = down;

    /*std::lock_guard <std::mutex> locker(Utility::mu);
    std::cout << "Task::setNbrs : taskid " << this->task_id_ <<  " up " << this->nbrs_.up  <<  "  down " <<   this->nbrs_.down << std::endl;
	*/
}


// Check with neighboring tasks if pre conditions are met
bool Task::isPreCondsMet()
{
    int diff1(0), diff2(0);

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
        return false;
}

void Task::setPostConds()
{
    this->iter_number ++;

    // GASPI: Here send the iter_no if I am boundary task

    //std::cout << "iter = " << iter_number << std::endl;
}

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

//set the RHS f_ vector
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


// Initiliase the u_ vector
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


// Display the contents of the grid in the current task

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



// This function updates the contents of the grid. It computes global row from a local row.
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

// Every task compute their residual
void Task::computeResidual()
{
	size_t startRow = Utility::getGlobalRow(this->task_id_, 0);
	size_t endRow = startRow + gridPtr->numlocRows_;   // endRow not included
	const size_t numCols = gridPtr->numCols_;
	const size_t endCol = numCols - 1;
	double my_residual = 0.0;
	size_t ind;

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


	/*if(this->iter_number == 0)
	{
		std::lock_guard <std::mutex> locker(Utility::mu);
		std::cout << "hx_coeff: " << hx_coeff << " ,hy_coeff: "  << hy_coeff << ",center_coeff: " << center_coeff << std::endl;
	}*/

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

		    my_residual += (temp*temp);
	    }
	}


	/*{
			std::lock_guard <std::mutex> locker(Utility::mu);
			std::cout << "(" << this->iter_number << ")"  << "tid: "  << this->task_id_
							<< "  my_residual : " << my_residual
							<< std::endl;
	}*/


	//debug
	//my_residual = this->iter_number + 1;

	 /*{
	    std::lock_guard <std::mutex> locker(Utility::mu);
	    std::cout << "(" << this->getIterNum()  << ") task_id: " << this->task_id_ << "   residual_sq:  " << my_residual << std::endl;
	 }*/


	ind = this->iter_number % Utility::numTasks;
	// dummy initiliasation: this iteration is finished in all the tasks
	bool isLocAllTasksDone(false);

	this->loc_res2_[ind] = my_residual;
	this->loc_iters_[ind] =  true;

	size_t numTasksFinished(0);

	for(std::shared_ptr<Task> taskptr : tasks)
	{
		// Check if this iteration (hash value: ind) is finished in all the tasks.
		if (taskptr->loc_iters_[ind])
		{
			numTasksFinished ++;
			//indDoneAllTasks = false;
		}
	}

	// This should be thread safe
	{
		if (numTasksFinished == Utility::numTasks)
	    	{
				std::lock_guard <std::mutex> locker(Utility::mu_task);

				if(isAllTasksDone[ind] == false)
				{
					isAllTasksDone[ind] = true;
					isLocAllTasksDone = true;
				}
	    	}
	}


    // If 'indDoneAllTasks' is true here, it means all tasks has finished their iteration hash 'ind'
	// global reduce of the residual
	//if(indDoneAllTasks)

	// Only one thread should do this per iteration
	if(isLocAllTasksDone)
	//if (isAllTasksDone[ind])
	{
		double global_res(0.0);

		for(std::shared_ptr<Task> taskptr : tasks)
		{
			//volatile double my_volatile_res2(taskptr->loc_res2_[ind]);
			global_res += taskptr->loc_res2_[ind];

			/*{
				std::lock_guard <std::mutex> locker(Utility::mu);
				std::cout << "(" << taskptr->iter_number << ")"  << "tid: "  << taskptr->task_id_
						<< "  my_volatile_res2: " << my_volatile_res2
						<< std::endl;
			}*/


			//global_res += taskptr->loc_res2_[ind];

			// mark the counter to false in all the task at location 'ind', i.e flush (reset) true markers.
			taskptr->loc_iters_[ind] = false;
		}

		global_res = std::sqrt(global_res);

		// debug
		//global_res = this->iter_number + 1;

		// set the variable global_res_ in the Task class.
		this->global_res_ = global_res;

		// Debug: Set the global residual and number of iterations in the grid class.
		gridPtr->grid_gres_  = this->global_res_;
		gridPtr->grid_iters_ = this->iter_number;


		/*{
			std::lock_guard <std::mutex> locker(Utility::mu);
			//#####################################################
			std::cout  <<  " ("  << this->iter_number
					   << ")  task_id: " << task_id_  <<  " , ind: "  << ind <<  " ,global_res: "
					    << global_res_   << std::endl;
		}*/


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

	return;

	// isResTerCriMet() ???

}

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

// This should be called by one task
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

			/*std::lock_guard <std::mutex> locker(Utility::mu);
			std::cout <<  "##################################################Task::calFinalPhase() task id: "
					<< this->task_id_  <<  " ,max_iter " << max_iter << " Utility::maxIter " << Utility::maxIter
					<< std::endl;*/
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



