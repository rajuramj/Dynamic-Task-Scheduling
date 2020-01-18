#!/bin/bash
# This script is used to automate the run

nth8=8
nth1=1
ntask=2
maxtask=64
#maxtask=8*$nth8
ncycles=26000    # 10 micro seconds
niter=100000

./hpc_task_parallel_jacobi_socket0 150 150 $niter $nth1 $ntask $ncycles 0 > out
while [ $ntask -le $maxtask ]; do

    echo "run with $ntask tasks ..."

    echo -n $ntask >> err
    echo -n $'\t' >> err
    ./hpc_task_parallel_jacobi_socket0 150 150 $niter $nth1 $ntask $ncycles 0 > out 2>> err
    ./hpc_task_parallel_jacobi_socket0 150 150 $niter $nth8 $ntask $ncycles 0 > out 2>> err
    echo "" >> err

    (( ntask += 2 ))
done
