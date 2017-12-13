#!/bin/bash
# usage
#	./compute_cdf.bash <input_file> <time_interval(h) left> <time_interval(h) right>
FILE1=$1 # the traveltime.csv
METRICS="WAIT_SMS ON_SMS_TAXI"


for METRIC in $METRICS; do 
	METRIC_FILE=$FILE1.$METRIC.csv
	grep "$METRIC," $FILE1 | cut -f10 -d',' > $METRIC_FILE
	./compute_cdf.m $METRIC_FILE
done
