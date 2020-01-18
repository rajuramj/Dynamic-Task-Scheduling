#include <iostream>
#include <chrono>
#include <vector>
#include <cmath>
#include <omp.h>
#include <fstream>
#include <cassert>

/*This methods returns the red and black indexes of inner grid points*/
void calIndexes(const int numGridX, const int numGridY, std::vector<int> &redIndex,std::vector<int> &blackIndex)
{
bool toggle = false;

// We have to exclude boundary indexed in the red and black array
  for(int i=1; i<numGridY - 1; ++i)
  {
     toggle = !toggle;
     for(int j=1 ;j<numGridX-1;j+=2)
     {
       if(toggle)
          {
            redIndex.push_back(i*numGridX +j);
            if((numGridX % 2) == 1)
             {
               if (!((j+2) % numGridX == 0)) blackIndex.push_back(1+i*numGridX +j);
             }
             else  blackIndex.push_back(1+i*numGridX +j);
          }
        else
          {
            blackIndex.push_back(i*numGridX +j);
            if((numGridX % 2) == 1)
             {
             if (!((j+2) % numGridX == 0)) redIndex.push_back(1+ i*numGridX +j);
             }
             else redIndex.push_back(1+ i*numGridX +j);
          }

     }
  }

}

/*Red black gauss seidel has been implemented here*/
void Solve_parallel(std::vector<double>&u , const std::vector<double>& f , const int numgridpoints_X,const std::vector<int>& redvec, const std::vector<int>& blackvec , const int numiter){

  const double PI = 4.0 * atan(1.0);
  const size_t numgridpoints = u.size();
  const int numgridpoints_x = numgridpoints_X;
  const int numgridpoints_y = numgridpoints/numgridpoints_x;
  const double hx  = 2.0 / (numgridpoints_x-1);
  const double hy = 1.0 / (numgridpoints_y-1);
  const double hxsq = hx * hx ;
  const double hysq = hy * hy ;
  const double hxsqinv = 1/ hxsq;
  const double hysqinv = 1/ hysq;
  const double K = 2 * PI;
  const double constant = (2*hxsqinv) + (2*hysqinv) + (K*K);
  const double constinv = 1/ constant;

  int numiter_p = 0 ;
  while (numiter_p < numiter){

    // updating red values
    #pragma omp parallel for schedule (static)
    for (size_t i = 0; i < redvec.size(); ++i){
      u[redvec[i]] = constinv  * ( f[redvec[i]] + (hxsqinv)*(u[redvec[i]-1] + u[redvec[i]+1]) + (hysqinv)*( u[redvec[i] - numgridpoints_x] + u[redvec[i] + numgridpoints_x]));
    }
     //updating the black values
    #pragma omp parallel for schedule (static)
    for (size_t i = 0; i < blackvec.size(); ++i) {
        u[blackvec[i]] = constinv * ( f[blackvec[i]] + (hxsqinv)*(u[blackvec[i]-1] + u[blackvec[i]+1]) + (hysqinv)*( u[blackvec[i] - numgridpoints_x] + u[blackvec[i] + numgridpoints_x]));
    }


  ++numiter_p;
  }

}

void StartInit(int const numgridpoints_x,int const numgridpoints_y, int numiter) {
  /* initilization based on the numgridpoints_x and numgridpoints_y */
  // numgridpoints_x = numgrid_interval
  numiter++;
  const double PI = 4.0 * atan(1.0);
  const double hx  = 2.0 / (numgridpoints_x-1);
  const double hy = 1.0 / (numgridpoints_y-1);
  std::cout << "RED BLACK GAUSS SEIDEL PARALLEL SOLVER......." << std::endl;
  std::vector< double> u(numgridpoints_x * numgridpoints_y, 0.0);
  std::vector<double> f(numgridpoints_x * numgridpoints_y, 0.0);
  const double con_sin = 267.744894041;

  // initilization of upper Boundary Values

  std::cout << "Initilizating Boundary Values" << std::endl;
  
  for (int i = (numgridpoints_y-1) * numgridpoints_x; i < (numgridpoints_x * numgridpoints_y) ; ++i ) {
      u[i] = (con_sin * sin(2 * PI * (hx *(i - ((numgridpoints_y - 1) * numgridpoints_x)))));
  }

  // evauating the function
  double x ,y ;
  std::cout << "Initilizating the Domain with Function values" << std::endl;
  #pragma omp parallel for schedule (static)
  for (int  i = 0; i < numgridpoints_x * numgridpoints_y; ++i) {
    x = hx * (i % numgridpoints_x);
    y = hy * (i / numgridpoints_x);
    f[i] = 4 * PI *PI * sin(2 * PI * x) * sinh(2* PI * y );
    //std::cout << "tHE VALUE f_i" <<f[i]<< std::endl;

  }

  // Founding the Red and Black array
  std::vector<int> redIndArr;
  std::vector<int> blackIndArr;
  calIndexes(numgridpoints_x,numgridpoints_y,redIndArr, blackIndArr);
  std::cout << "Starting the solver " << std::endl;

  std::chrono::time_point<std::chrono::system_clock> Start, End;
  Start = std::chrono::system_clock::now();
  Solve_parallel(u,f, numgridpoints_x,redIndArr, blackIndArr, numiter);
  End = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_sec_1 = End - Start;
  std::cout << "The time taken for parallel computation is " <<elapsed_sec_1.count()<< std::endl;

  // updating the Residual;

  long double resi_dual = 0.0;
  std::vector<double> Resi_D(numgridpoints_x * numgridpoints_y ,0.0);
  int total_points = numgridpoints_x * numgridpoints_y;
  const double hxsq = hx * hx ;
  const double hysq = hy * hy ;
  const double K = 2 * PI;


  for (int i = numgridpoints_x +1  ; i < numgridpoints_x * (numgridpoints_y - 1)-1 ; i++){
    if (!( ((i+1)% numgridpoints_x == 0) || (i% numgridpoints_x ==0))){
      Resi_D[i]= f[i] - ((u[i] * ((2/hxsq) + (2/(hysq)) + (K* K))) - (((1/hxsq)*(u[i-1] + u[i+1])) + (1/hysq)*( u[i - numgridpoints_x] + u[i + numgridpoints_x])));
      resi_dual += Resi_D[i] * Resi_D[i];
      
    }
  }
  std::cout << "The Residual for computation is ---   "<<sqrt((resi_dual/total_points))<< std::endl;

  // writing into file
  std::ofstream datafile_p;
  datafile_p.open("solution.txt");

  for (int i = 0; i < numgridpoints_y; ++i) {

    for (int j = 0; j < numgridpoints_x; ++j) {

      datafile_p << hx * (j% numgridpoints_x) <<" "<< hy * (i% numgridpoints_y)<<" "<< u[i*numgridpoints_x + j]<< std::endl ;
    }
  }
  datafile_p.close();
  std::cout << "The solution has been written into solution.txt....." << std::endl;

}

int main(int argc, char const *argv[]) {

  assert(argc == 4);
  int arguments = argc;
  arguments++;
  std::string s1 = argv[1],s2= argv[2],s3 = argv[3];
  int numgridpoints_x = std::stoi(s1) +1;
  int numgridpoints_y = std::stoi(s2) +1;
  int numiter = std::stoi(s3);
  StartInit(numgridpoints_x, numgridpoints_y,numiter);
  return 0 ;

}
