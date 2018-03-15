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

numRows=10
numCols=10
maxIters=20000

# Remove previously generated txt files
rm *.txt

#---------------------------------- Run the reference solvers ------------------------------------------------------

./SerialSolver/serialSolver $numRows $numCols $maxIters >> output_serial.txt

for tasks in 2 5 10
do
	echo "Reference run with numRows=$numRows, numCols=$numCols, maxIters=$maxIters, thread_id=4, task_id=$tasks ........"
	../ref_task_parallel_jacobi  $numRows $numCols $maxIters 4 $tasks >> output_parallel.txt
done


#------------------------- Varying the number of threads for a given grid configuration -------------------------------------------------

if [ 1 -eq 1 ] ; then

for threads in 1 3 5 8  9 12 13 15 16 18 19 23 25 # {1..24}
do
      for tasks in 2 5 10
      do

         echo "Starting job with numRows=$numRows, numCols=$numCols, maxIters=$maxIters, thread_id=$threads, task_id=$tasks ........"
         ../task_parallel_jacobi        $numRows $numCols $maxIters $threads $tasks >> output_parallel.txt
	  
	  # Compare the buffers u1, and u2 in paralel and serial case
	  diff u1.txt serial_u1.txt >> error.txt
          #echo "----------------------------------------------------------" >> error.txt
	  diff u2.txt serial_u2.txt >> error.txt
	  #echo "----------------------------------------------------------" >> error.txt
	  diff res_iter.dat ref_res_iter_{$numRows}_{$numCols}_{$tasks}.dat >> error.txt	

 
      done

done
fi

if [ 1 -eq 2 ] ; then
# reading for 96
 for j in 1
   do
   echo "Starting job with N=$N, n=$n, #defvec=$j, cellsize=$cellsize ..."
   ./helpscript 5 100 00:30:00 sca $j $cellsize
   #./helpscript 4 96 00:30:00 sca $j $cellsize
 done
fi

