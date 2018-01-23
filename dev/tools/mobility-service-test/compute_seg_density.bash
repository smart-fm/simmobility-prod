#!/bin/bash
# usage
#	./compute_cdf.bash <input_file> <time_interval(h) left> <time_interval(h) right>
FILE1=$1 # the out.txt
LEFTH=$2 # from-hour
RIGHTH=$3 # to-hour


TEMP_FILE=$FILE1.csv

grep "seg," $FILE1 | cut -f2,7 -d',' > $TEMP_FILE
./compute_seg_density.m $TEMP_FILE $LEFTH $RIGHTH

