#!/bin/bash
# usage
#	./compute_cdf.bash <input_file> <time_interval(h) left> <time_interval(h) right>
FILE1=$1 # the subtrip_metrics.csv
METRICS="SMS_Taxi"


for METRIC in $METRICS; do 
	METRIC_FILE=$FILE1.$METRIC.csv
	grep "$METRIC" $FILE1 | cut -f11 -d',' > $METRIC_FILE
	./compute_cdf.m $METRIC_FILE
done
