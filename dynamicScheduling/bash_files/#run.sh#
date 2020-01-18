#!/bin/bash
# This script is used to automate the run

ntasks=25
maxthreads=8
nthreads=1

while [ $nthreads -le $maxthreads ]; do

    echo "run with $nthreads threads and $ntasks tasks ..."

      for i in {1..3}; do				
    ../hpc_task_parallel_jacobi_socket0 150 150 1000000 $nthreads $ntasks 0 0
      done	
      

    (( nthreads *= 2 ))
done
