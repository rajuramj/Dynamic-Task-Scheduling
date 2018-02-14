#include "grid.hpp"
#include <assert.h>
#include <algorithm>   // std::generate()

Grid::Grid(size_t numRows, size_t numCols, size_t numTasks)
{

    assert(numRows%numTasks == 0);

    this->numRows_ = numRows;
    this->numCols_ = numCols;
    this->numlocRows_ = numRows / numTasks;


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

    std::cout << "Grid object constructed" << std::endl;
}

Grid::~Grid()
{
    std::cout << "Grid() destructer called" << std::endl;
}

void Grid::displayGrid()
{
    std::lock_guard <std::mutex> locker(Utility::mu);
    /*std::cout << " Task::globalGrid_.h_x:  " <<  Utility::globalGrid_.h_x  << std::endl;
    std::cout << " Task::globalGrid_.h_y:  " <<  Utility::globalGrid_.h_y  << std::endl;*/

    std::cout << "u1_: " << std::endl;

    for(size_t i=0; i < u1_.size(); i++)
    {
        std::cout <<  u1_[i] << "\t";

        if( (i+1)% numCols_ ==0)
            std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout << "u2_: " << std::endl;

    for(size_t i=0; i < u2_.size(); i++)
    {
        std::cout <<  u2_[i] << "\t";

        if( (i+1)% numCols_ ==0)
            std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout << "f_: " << std::endl;

    for(size_t i=0; i < f_.size(); i++)
    {
        std::cout <<  f_[i] << "\t";

        if( (i+1)% numCols_ ==0)
            std::cout << std::endl;
    }
}

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

// Returns reference to the source vector
 const std::vector<double> & Grid::getSrcVec(size_t curIter)
 {
     return (curIter % 2 ==0) ? this->u1_ : this->u2_;
 }

 // Returns reference to the destination vector
 std::vector<double> & Grid::getDestVec(size_t curIter)
 {
     return (curIter % 2 ==0) ? this->u2_ : this->u1_;
 }






