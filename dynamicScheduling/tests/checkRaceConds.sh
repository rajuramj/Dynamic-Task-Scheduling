#!/bin/bash

#=================================================================================================================================
# 
#  	Author: Raju Ram
#	Creation Date: 8 March 2018
#
#
#	This script checks for the race condition in the asynchronous implementation of task parallel jacobi solver. 
#	The reference solution is serial jacobi solver. The buffers (u1, u2), the final residual and final iterations are compared.
#
#====================================================================================================================================

numRows=100
numCols=100
maxIters=20000

# Remove previously generated txt files


#---------------------------------- Run the reference solvers ------------------------------------------------------

./SerialSolver/serialSolver $numRows $numCols $maxIters >> output_serial.txt

if [ 331 -eq 1 ] ; then

for tasks in  5 10 20 50
do
	echo "Reference run with numRows=$numRows, numCols=$numCols, maxIters=$maxIters, thread_id=4, task_id=$tasks ........"
	../ref_task_parallel_jacobi  $numRows $numCols $maxIters 4 $tasks >> output_parallel.txt
done

fi

#------------------------- Varying the number of threads for a given grid configuration -------------------------------------------------

if [ 1 -eq 1 ] ; then

for threads in 1 2 3 4 5 6 7  8  #13 15 16 18 19 23 25 # {1..24}
do
      for tasks in 2 10 20 50
      do

         echo "Starting job with numRows=$numRows, numCols=$numCols, maxIters=$maxIters, thread_id=$threads, task_id=$tasks ........"
         ../hpc_task_parallel_jacobi        $numRows $numCols $maxIters $threads $tasks >> output_parallel.txt
	  
	  # Compare the buffers u1, and u2 in paralel and serial case
	  diff u1.txt serial_u1.txt >> error.dat
          #echo "----------------------------------------------------------" >> error.txt
	  diff u2.txt serial_u2.txt >> error.dat
	  #echo "----------------------------------------------------------" >> error.txt
	  #diff res_iter.dat ref_res_iter_{$numRows}_{$numCols}_{$tasks}.dat >> error.dat 
      done

done
fi

rm *.txt


