#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <iostream>

namespace TP {

class Timer
{
private:
   
   typedef std::chrono::high_resolution_clock Clock_T;
   
public:
   
   Timer() : start_( Clock_T::now() ) { std::cout << "Timer ctor callled" << std::endl;}
    
   void reset() { start_ = Clock_T::now(); }
    
   /// Returns the elapsed time in seconds since either the construction of the object or the last call to reset()
   double elapsed() const
   {
      using namespace std::chrono;
      return duration_cast< duration< double, seconds::period > >( Clock_T::now() - start_ ).count();
   }

private:

   Clock_T::time_point start_;
};

} // namespace TP


#endif // TIMER_H
