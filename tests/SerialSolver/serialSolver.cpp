#include <iostream>
#include <chrono>
#include <vector>
#include <cmath>
#include <assert.h>
#include <fstream>     // std::ofstream

/*void Solve_GaussSeiedel(std::vector<double>& u , const std::vector<double>& f, const int numgridpoints_X){

  const double PI = 4.0 * atan(1.0);
  std::cout << "Starting Gauss Seidel Solver " << std::endl;
  const size_t numgridpoints = u.size();
  const int numgridpoints_x = numgridpoints_X;
  const int numgridpoints_y = numgridpoints/numgridpoints_x;
  const double hx  = 2.0 / (numgridpoints_x-1);
  const double hy = 1.0 / (numgridpoints_y-1);
  const double hxsq = hx * hx ;
  const double hysq = hy * hy ;
  const double K = 2 * PI;
  long double resi_dual;
  std::vector<double> Resi_D(numgridpoints_x * numgridpoints_y ,0.0);
  //int size = Resi_D.size();
  long double resi_temp =1;

  int numiter = 0 ;
  double tol = pow(10,-5);
  while (resi_temp > tol){
    for (int i = numgridpoints_x +1  ; i < numgridpoints_x * (numgridpoints_y - 1)-1 ; i++) {
      if (!( ((i+1)% numgridpoints_x == 0) || (i% numgridpoints_x ==0))){
        u[i] = (1 /((2/hxsq) + (2/(hysq)) + (K* K))) * ( f[i] + (1/hxsq)*(u[i-1] + u[i+1]) + (1/hysq)*( u[i - numgridpoints_x] + u[i + numgridpoints_x]));
      }
    }
    resi_dual = 0.0;
    for (int i = numgridpoints_x +1  ; i < numgridpoints_x * (numgridpoints_y - 1)-1 ; i++){
      if (!( ((i+1)% numgridpoints_x == 0) || (i% numgridpoints_x ==0))){
        Resi_D[i]= f[i] - ((u[i] * ((2/hxsq) + (2/(hysq)) + (K* K))) - (((1/hxsq)*(u[i-1] + u[i+1])) + (1/hysq)*( u[i - numgridpoints_x] + u[i + numgridpoints_x])));
        resi_dual += Resi_D[i] * Resi_D[i];
        }
    }
    // Residual Function
    std::cout << "The Residual for Iteration number ---   "<<numiter+1<< "   is   "<<sqrt(resi_dual)<< std::endl;
    resi_temp = resi_dual;
    if (resi_temp < pow(10,-11))
      std::cout << "The Solution is converged" << std::endl;
  ++numiter;
  }

}*/


// Returns reference to the source vector
 const std::vector<double> & getSrcVec(size_t curIter, const std::vector<double>& u1, const std::vector<double>& u2)
 {
     return (curIter % 2 ==0) ? u1 : u2;
 }

 // Returns reference to the destination vector
 std::vector<double> & getDestVec(size_t curIter, std::vector<double>& u1, std::vector<double>& u2 )
 {
     return (curIter % 2 ==0) ? u2 : u1;
 }


 void displayGrid(const std::vector<double> &vec, const size_t numCols)
 {
      for(size_t i=0; i < vec.size(); i++)
      {
          std::cout <<  vec[i] << "\t";

          if( (i+1)% numCols ==0)
              std::cout << std::endl;
      }
 }

void Solve_Jacobi(std::vector<double>& u1, std::vector<double>& u2, const std::vector<double>& f, const int numgridpoints_X, const int numiters)
{

    assert(u1.size() == u2.size());
    const double PI = 4.0 * atan(1.0);
    std::cout << "Starting Jacobi Solver " << std::endl;
    const size_t numgridpoints = u1.size();
    const int numgridpoints_x = numgridpoints_X;
    const int numgridpoints_y = numgridpoints/numgridpoints_x;
    const double hx  = 2.0 / (numgridpoints_x-1);
    const double hy = 1.0 / (numgridpoints_y-1);
    const double hxsqinv = 1.0/(hx * hx) ;
    const double hysqinv = 1.0/(hy * hy) ;
    const double K = 2 * PI;
    const double mult = 1.0/(pow(K,2) + 2*(hxsqinv + hysqinv) );


    /*long double resi_dual;
    std::vector<double> Resi_D(numgridpoints_x * numgridpoints_y ,0.0);
    int size = Resi_D.size();
    long double resi_temp =1;
    double tol = pow(10,-5);
    */

    int currIter = 0, ind;
    double up, left, right, down;
    int numCols = numgridpoints_x;

    while (currIter < numiters)
    {

        const std::vector <double> &src = getSrcVec(currIter, u1, u2);

        std::vector <double> &dest =  getDestVec(currIter, u1, u2);


        for (int iRow=1; iRow < numgridpoints_y-1; iRow++)
        {
                     // Skip the left and right boundary
                    for(int iCol=1; iCol < numgridpoints_x-1; iCol++)
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

                       dest[ind] = mult * ( f[ind] + hxsqinv * ( left + right )
                                                    + hysqinv * (up + down) );
                    }


        }


          /*resi_dual = 0.0;
          for (size_t i = numgridpoints_x +1  ; i < numgridpoints_x * (numgridpoints_y - 1)-1 ; i++)
          {
            if (!( ((i+1)% numgridpoints_x == 0) || (i% numgridpoints_x ==0))){
              Resi_D[i]= f[i] - ((u[i] * ((2/hxsq) + (2/(hysq)) + (K* K))) - (((1/hxsq)*(u[i-1] + u[i+1])) + (1/hysq)*( u[i - numgridpoints_x] + u[i + numgridpoints_x])));
              resi_dual += Resi_D[i] * Resi_D[i];
              }
          }


          // Residual Function
          std::cout << "The Residual for Iteration number ---   "<<numiter+1<< "   is   "<<sqrt(resi_dual)<< std::endl;
          resi_temp = resi_dual;
          if (resi_temp < pow(10,-11))
            std::cout << "The Solution is converged" << std::endl;
        */

          ++currIter;
    }



}
void StartSolver(int const numgridpoints_x,int const numgridpoints_y, const int numiters)
{
  /* initilization based on the numgridpoints_x and numgridpoints_y */
  // numgridpoints_x = numgrid_interval
    //xmax is 2 and ymax is 1

  const double PI = 4.0 * atan(1.0);
  const double hx  = 2.0 / (numgridpoints_x-1);
  const double hy = 1.0 / (numgridpoints_y-1);
  const double mult = sinh(2 * PI* 1);
  double x,y;


  std::cout << "Initilization Starts......." << std::endl;
  std::vector<double> u1(numgridpoints_x * numgridpoints_y, 0.0);
  std::vector<double> u2(numgridpoints_x * numgridpoints_y, 0.0);
  std::vector<double> f(numgridpoints_x * numgridpoints_y);

  // initilization of upper Boundary Values
  std::cout << "Initilizating Upper Boundary Values" << std::endl;
  //#pragma omp parallel for schedule (static)
  for (int i=0; i< numgridpoints_x ; i++)
  {
      x = i * hx;
      u1[i] = sin(2 * PI *x)*mult;
      u2[i] = u1[i];
      //std::cout << "the values  u_i " <<u[i]<<std::endl;
  }


  std::cout << "Initilizating the Domain with Function values" << std::endl;
  //#pragma omp parallel for schedule (static)



  //set the RHS f_ vector
        /*{
            std::lock_guard <std::mutex> locker(Task::mu);
            std::cout << "Points for taskID" << this->task_id_ << std::endl;
        }*/

        int numCols, ind, numRows;
        std::pair<double, double> p;

        numCols = numgridpoints_x;
        numRows = numgridpoints_y;

        for(int iRow= 0 ; iRow < numgridpoints_y ; iRow++)
        {
            for(int iCol=0  ; iCol < numCols ; iCol++)
            {
                x = iCol * hx;
                y =  ((numRows - 1) - iRow)* (hy);

                ind = iRow * numCols + iCol;

                f[ind] = 4.0 * PI * PI  * sin(2 * PI * x) *
                          sinh(2* PI * y);
            }
        }



        /*std::cout << "Before starting the solver" << std::endl;
        std::cout << "u1: " << std::endl;
        displayGrid(u1, numgridpoints_x);
        std::cout << "u2: " << std::endl;
        displayGrid(u2, numgridpoints_x);
        std::cout << "f: " << std::endl;
        displayGrid(f, numgridpoints_x);*/


  std::cout << "Now initilization ends......Starting the solver " << std::endl;
  std::chrono::time_point<std::chrono::system_clock> Start, End;

  Start = std::chrono::system_clock::now();
  Solve_Jacobi(u1,u2,f, numgridpoints_x, numiters);
  //Solve_GaussSeiedel(u1,f, numgridpoints_x, numiters);
  End = std::chrono::system_clock::now();

  std::chrono::duration<double> elapsed_sec = End - Start;
  std::cout << "The time taken for computation is " <<elapsed_sec.count()<< std::endl;

  /*std::cout << "After starting the solver" << std::endl;
  std::cout << "u1: " << std::endl;
  displayGrid(u1, numgridpoints_x);
  std::cout << "u2: " << std::endl;
  displayGrid(u2, numgridpoints_x);
  std::cout << "f: " << std::endl;
  displayGrid(f, numgridpoints_x);*/

  // writing into file
  std::ofstream datafile_u1, datafile_u2;

  datafile_u1.open("serial_u1.txt");
  datafile_u2.open("serial_u2.txt");
  double diff=0.0;

  for (int row = 0; row < numgridpoints_y; ++row)
  {
    for (int col = 0; col < numgridpoints_x; ++col)
    {

        x = col*hx;
        //y = y_max_ - row*hy_;
        y = 1.0 - row*hy;
        ind = row*numgridpoints_x + col;

        if(diff < fabs(u1[ind] - u2[ind]))
           diff = fabs(u1[ind] - u2[ind]);

        datafile_u1 << u1[ind]<< std::endl;
        datafile_u2 <<  u2[ind]<< std::endl;

        //datafile_u1 << x <<"\t"<< y << "\t" << u1[ind]<< std::endl;
        //datafile_u2 << x <<"\t"<< y << "\t" << u2[ind]<< std::endl;
    }
  }

  datafile_u1.close();
  datafile_u2.close();

  std::cout << "The solution has been written into serial_u1.txt and serial_u2.txt ....." << std::endl;
  std::cout << " inf norm of u1 and u2 vector is: " << diff << std::endl;

}

int main(int argc, char const *argv[])
{
  assert(argc == 4);
  std::string s1 = argv[1],s2= argv[2],s3 = argv[3];

  int numgridpoints_y = std::stoi(s1);
  int numgridpoints_x = std::stoi(s2);
  int numiters = std::stoi(s3);

  /*std::cout << "Enter the Division you want in X and Y direction as follows " << std::endl;
  std::cout << "Enter the division in X direction" << std::endl;
  std::cin >> numgridpoints_x;
  std::cout << "Enter the division in Y direction" << std::endl;
  std::cin >> numgridpoints_y;
  std::cout << "Enter total number of iterations" << std::endl;
  std::cin >> numiters;*/

  StartSolver(numgridpoints_x, numgridpoints_y, numiters);
  //std::cout << "Input Data has been taken...........Initilization in progess " << std::endl;

  return 0 ;


  // include time
}
