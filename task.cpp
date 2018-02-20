#include <stdexcept>
#include "task.hpp"
#include <assert.h>
#include <algorithm>  // std::max


std::shared_ptr<Grid> Task::gridPtr;

// Stores the global information of the grid
//GlobalGrid Task::globalGrid;
//Task::GlobalGridParams Task::globalGrid_;

//Constuctor
Task :: Task(int tid, std::string taskLoc)
{

    this->iter_number = 0;
    this->task_id_ = tid;

    this->row_start_ = gridPtr->numlocRows_ * tid;
    this->row_end_ = row_start_ + gridPtr->numlocRows_; // not included


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


void  Task::setGridPtr(const std::shared_ptr<Grid>& ptr)
{
    Task::gridPtr = std::move(ptr);
}

// Set the neighbours for the current task object
void Task::setNbrs(Task* up, Task* down)
{
    this->nbrs_.up = up;
    this->nbrs_.down = down;
}


// Check with neighboring tasks if pre conditions are met
bool Task::isPreCondsMet()
{
    int diff1(0), diff2(0);

    if(this->boundary_ != TopBound)
    {
    	diff1= this->iter_number - this->nbrs_.up->iter_number;
    }

    if(this->boundary_ != BottomBound)
    {
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
    // Sanity check: curr itearion number should be smaller than the maximum number of iterations.
    assert(this->iter_number <= Utility::maxIter);

    bool ret_val(false);

    if (this->iter_number == Utility::maxIter) {
      ret_val = true;
      std::cout << "("  <<  this->task_id_ << "): Iterations are " << this->iter_number << std::endl;
    }

   /* {
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
        double mult = sinh(2 * Utility::PI* 1),x;
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
    std::cout << "Task contents....  \n";

    std::cout << "task_id: " << this->task_id_ << std::endl;
    std::cout << "row_start: " << row_start_ << std::endl;
    std::cout << "row_end: " << row_end_ << std::endl;
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
	{
		std::lock_guard <std::mutex> locker(Utility::mu);
		std::cout << "Task: " << this->task_id_ <<  " updateGrid() called "  << std::endl;
	}

    double hxsqinv, hysqinv, mult,  up, down, left, right;
    hxsqinv = 1.0/(pow(gridPtr->hx_, 2));
    hysqinv = 1.0/(pow(gridPtr->hy_, 2));

    mult = 1.0/(pow(Utility::K,2) + 2*(hxsqinv + hysqinv) );

    size_t startRow, endRow;

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

               dest[ind] = mult * ( gridPtr->f_[ind] + hysqinv * (up + down)
                                         +  hxsqinv * ( left + right ) );
            }


        }

        //this->gridPtr->displayGrid(dest);


}

