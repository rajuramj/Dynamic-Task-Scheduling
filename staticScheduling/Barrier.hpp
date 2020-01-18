/**
 * Barrier.hpp
 *
 */

#ifndef THREAD_BARRIER_H_
#define THREAD_BARRIER_H_

#include <pthread.h>
#include <iostream>

class Barrier {

private:

	pthread_barrier_t _barrier;

public:

  Barrier (size_t nThreads);

  // delete the copy constructor 	
  Barrier(const Barrier&) = delete;  

  //delete the assignment operator
  Barrier& operator=(const Barrier&) = delete;

  ~Barrier ();

  void applyBarrier(size_t tid);
  void applyBarrier();

};


Barrier::Barrier(size_t nThreads) {

	int ret = pthread_barrier_init(&_barrier, NULL, nThreads);

	if(ret==0) {
		std::cout << "Barrier initialised successfully" << std::endl;
	} else {
		std::cout << "Error in barrier initialisation" << std::endl;
	}
}

Barrier::~Barrier() {

	int ret = pthread_barrier_destroy(&_barrier);

	if(ret==0) {
			std::cout << "Barrier destroyed successfully" << std::endl;
		} else {
			std::cout << "Error in barrier destruction" << std::endl;
		}
}

void Barrier::applyBarrier(size_t tid) {

	// waits until all the threads call this function
	int ret = pthread_barrier_wait(&_barrier);

	if(ret ==0 || ret == PTHREAD_BARRIER_SERIAL_THREAD ) {
		std::cout << "Thread " << tid  << " finishes barrier with ret " << ret << std::endl;
	} else {
		std::cout << "Error in pthread_barrier_wait" << std::endl;
	}
}

void Barrier::applyBarrier() {

	// waits until all the threads call this function
	pthread_barrier_wait(&_barrier);
}




#endif /* THREAD_BARRIER_H_ */
