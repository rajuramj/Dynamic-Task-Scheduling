#!/bin/bash
# This script is used to automate the run

nth1=1
nth8=8
ntask=2
maxtask=64
ncycles=26000    # 10 micro seconds
niter=100000

./../build/examples/tasks.granularity $ncycles $ntask $nth1 $niter > out
while [ $ntask -le $maxtask ]; do

    echo "run with $ntask tasks ..."

    echo -n $ntask >> err
    echo -n $'\t' >> err
    ./../build/examples/tasks.granularity $ncycles $ntask $nth1 $niter > out 2>> err
    ./../build/examples/tasks.granularity $ncycles $ntask $nth8 $niter > out 2>> err
    echo "" >> err

    (( ntask += 2 ))
done
