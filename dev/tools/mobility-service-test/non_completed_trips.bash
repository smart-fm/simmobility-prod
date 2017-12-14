#!/bin/bash
echo $1 
FILE1=$1 

gnuplot -e "data='$FILE1'" non_completed_trips.gp
