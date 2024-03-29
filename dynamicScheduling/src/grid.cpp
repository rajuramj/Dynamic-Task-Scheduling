/**
    src file: grid.cpp
    Purpose: To give (2D finite difference method) grid specifications.

    @author Raju Ram
    @version 1.0 04/07/18
 */


#include "grid.hpp"
#include <assert.h>
#include <algorithm>   // std::generate()
#include <fstream>     // std::ofstream


/**
    the grid constructor sets the grid properties.
	@param numRows number of rows in 2D grid
	@param numCols number of columns in 2D grid
	@param numTasks number of tasks, numRows should be divisible by numTasks

    @return void
 */

Grid::Grid(size_t numRows, size_t numCols, size_t numTasks)
{

	//assert(numRows%numTasks == 0);

	this->numRows_ = numRows;
	this->numCols_ = numCols;
	this->numlocRows_ = numRows / numTasks;

	// we assume (x_min,y_min) is (0,0), i.e the origin
	this->x_max_ = 2.0;
	this->y_max_ = 1.0;
	this->hx_ = x_max_ /(numCols - 1);
	this->hy_ = y_max_ /(numRows - 1);

	size_t len = numRows*numCols;
	this->u1_.resize(len, 0.0);

	//Qt probably does not identify the C++11 lambda function features
	/* std::generate(u1_.begin(), u1_.end(),
               [n=0, this] () mutable {
                 //size_t ret;
                //BCs
                 if(n%numCols_ ==0  || (n+1)%numCols_ ==0 || n> (numCols_*(numRows_-1)))
                 {
                     ++n;
                     return 0;
                 }
                 else
                 return ++n;
                });*/

	this->u2_.resize(len,0.0);
	this->f_.resize(len,0.0);

	//std::cout << "Grid object constructed" << std::endl;
}

Grid::~Grid()
{
	//std::cout << "Grid() destructer called" << std::endl;
}

/**
    display the grid properties

    @return void
 */

void Grid::displayGrid()
{
	std::lock_guard <std::mutex> locker(Utility::mu);
	std::cout << " \n####################################################\n"  << std::endl;
	std::cout << " numRows_:  " <<  numRows_  << std::endl;
	std::cout << " numlocRows_:  " <<  numlocRows_  << std::endl;
	std::cout << " numCols_:  " <<  numCols_  << std::endl;
	std::cout << " numTasks per iteration:  " << (numRows_/numlocRows_)  << std::endl;

	std::cout << " x_max:  " <<  x_max_  << std::endl;
	std::cout << " y_max:  " <<  y_max_  << std::endl;
	std::cout << " hx:  " <<  hx_  << std::endl;
	std::cout << " hy:  " <<  hy_  << std::endl;

	std::cout << " \n####################################################\n"  << std::endl;
}

/**
    display the grid contents
	@param vec vector such as buffer u1 or u2

    @return void
 */
void Grid::displayGrid(const std::vector<double> &vec)
{
	std::lock_guard <std::mutex> locker(Utility::mu);

	for(size_t i=0; i < vec.size(); i++)
	{
		std::cout <<  vec[i] << "\t";

		if( (i+1)% numCols_ ==0)
			std::cout << std::endl;
	}
}

/**
    get the src vector, used to avoid race conditions
	@param curIter current local iteration of a task

    @return reference to the source vector u1 or u2 based on task's iteration number
 */
const std::vector<double> & Grid::getSrcVec(size_t curIter)
{
	return (curIter % 2 ==0) ? this->u1_ : this->u2_;
}

/**
     get the destination vector, used to avoid race conditions
 	 @param curIter current local iteration of a task

     @return reference to the source vector u1 or u2 based on task's iteration number
 */
std::vector<double> & Grid::getDestVec(size_t curIter)
{
	return (curIter % 2 ==0) ? this->u2_ : this->u1_;
}


/**
    writing the buffers u1 and u2 to separate files

    @return void
 */
void Grid::writeSol2File()
{
	// writing into file
	std::ofstream datafile_u1, datafile_u2, datafile_res_iter;
	datafile_u1.open("u1.txt");
	datafile_u2.open("u2.txt");
	/*std::string s = std::string("ref_res_iter_") +
 			  	  	  std::string("{") + std::to_string(this->numRows_) + std::string("}_") +
 	  	  	  	  	  std::string("{") + std::to_string(this->numCols_) + std::string("}_") +
					  std::string("{") + std::to_string(Utility::numTasks) + std::string("}.dat");*/

	datafile_res_iter.open("res_iter.dat");

	datafile_res_iter <<  "iter_number \t global_res" << std::endl;
	datafile_res_iter << this->grid_iters_ << "\t\t";
	datafile_res_iter << this->grid_gres_ << std::endl;

	size_t ind;
	double diff=0.0;

	for (size_t row = 0; row < numRows_ ; ++row)
	{
		for (size_t col = 0; col < numCols_ ; ++col)
		{

			//x = col*hx_;
			//y = y_max_ - row*hy_;
			ind = row*numCols_ + col;

			if(diff < fabs(u1_[ind] - u2_[ind]))
				diff = fabs(u1_[ind] - u2_[ind]);

			datafile_u1 << u1_[ind]<< std::endl;
			datafile_u2 << u2_[ind]<< std::endl;

			//datafile_u1 << x <<"\t"<< y << "\t" << u1_[ind]<< std::endl;
			//datafile_u2 << x <<"\t"<< y << "\t" << u2_[ind]<< std::endl;
		}
	}

	datafile_u1.close();
	datafile_u2.close();

	std::cout << "The solution has been written into u1.txt and u2.txt ....." << std::endl;
	std::cout << " inf norm of u1 and u2 vector is: " << diff << std::endl;
}





