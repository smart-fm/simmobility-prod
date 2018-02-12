#!/bin/bash

OUT_LOCATION=$1 #the dev/Basic folder after SimMobility run

# The database connections and the table containing the activity schedule should be specified
# in the inputs section of the script_to_plot_temporal_distribition_of_trips_in_DAS.py
python2.7 script_to_plot_temporal_distribition_of_trips_in_DAS.py

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
