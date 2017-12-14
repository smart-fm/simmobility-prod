#!/bin/bash

OUT_LOCATION=$1 #the dev/Basic folder after SimMobility run


cp -p  $OUT_LOCATION/data/traveltime.csv $OUT_LOCATION
cp -p $OUT_LOCATION/data/subtrip_metrics.csv $OUT_LOCATION

./compute_seg_density.bash $OUT_LOCATION/out.txt 05 08
./compute_seg_density.bash $OUT_LOCATION/out.txt 08 11
./compute_seg_density.bash $OUT_LOCATION/out.txt 11 14
./compute_seg_density.bash $OUT_LOCATION/out.txt 14 17
./compute_seg_density.bash $OUT_LOCATION/out.txt 17 20
./compute_seg_density.bash $OUT_LOCATION/out.txt 20 24

./counter.bash $OUT_LOCATION > $OUT_LOCATION/counter.dat
echo "Counts are written in $OUT_LOCATION/counter.dat"

./process_traveltime.bash $OUT_LOCATION/traveltime.csv

./process_subtrip_metrics.bash $OUT_LOCATION/subtrip_metrics.csv
