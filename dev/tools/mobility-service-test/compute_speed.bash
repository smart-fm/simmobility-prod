#!/bin/bash
# usage
#	./compute_cdf.bash <input_file> <time_interval(h) left> <time_interval(h) right>
FILE1=$1 # the out.txt
LEFTH=$2
RIGHTH=$3


TEMP_FILE=$FILE1.csv

grep "seg," $FILE1 | cut -f2,5 -d',' > $TEMP_FILE
./compute_speed.m $TEMP_FILE $LEFTH $RIGHTH

