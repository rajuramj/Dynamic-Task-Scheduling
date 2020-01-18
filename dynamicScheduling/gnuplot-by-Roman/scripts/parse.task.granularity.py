#!/usr/bin/env python
import math
import sys
import os

def parseData(inputf, outputf): 
    orig = []
    

    fin = open(inputf, 'r')
    fout = open(outputf, 'w');
    for l in fin:
        l = l.split()
        nth = int(l[0])
	if nth == 1:
	    orig1 = float(l[1])
	    orig3 = float(l[2])
	    orig10 = float(l[3])
	fout.write(l[0] + ' ' + str((orig1 / float(l[1])) / nth) + ' ' + str((orig3 / float(l[2])) / nth) + ' ' + str((orig10 / float(l[3])) / nth) + '\n')
    fout.close()
    fin.close()

    return

parseData(sys.argv[1], sys.argv[2])
