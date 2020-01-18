#!/bin/bash
# This script is used to automate the run

nthreads=1
maxthreads=8
ntask=25
ncycles1=2600  
ncycles2=10000  
ncycles3=26000  
niter=100000

./../build/examples/tasks.granularity $ncycles1 $ntask $maxthreads $niter > out
while [ $nthreads -le $maxthreads ]; do

    echo "run with $nthreads threads ..."

    echo -n $nthreads >> err
    echo -n $'\t' >> err
    ./../build/examples/tasks.granularity $ncycles1 $ntask $nthreads $niter > out 2>> err
    ./../build/examples/tasks.granularity $ncycles2 $ntask $nthreads $niter > out 2>> err
    ./../build/examples/tasks.granularity $ncycles3 $ntask $nthreads $niter > out 2>> err
    echo "" >> err

    (( nthreads += 1 ))
done
