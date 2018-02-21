#!/bin/bash

# This scripts produces results from a run of mobility services.
# You need to specify:
# - OUT_LOCATION and
# - what is required in the PLOTTING DAS section
# You must specify OUT_LOCATION, i.e., the folder of your simmobility run, from command line, e.g.,
#        ./complete_analysis OUT_LOCATION
# OUT_LOCATION is usually something like dev/Basic
# After the script, the output files  are OUT_LOCATION/counter.dat and all the images in OUT_LOCATION, in particular *.eps and *.png files.

#################################
######## INPUT PARAMETERS #######
#################################
OUT_LOCATION=$1 #the dev/Basic folder after SimMobility run


#################################
####### PLOTTING DAS ############
#################################
# The database connections and the table containing the activity schedule should be specified
# in the inputs section of the script_to_plot_temporal_distribition_of_trips_in_DAS.py
python2.7 script_to_plot_temporal_distribition_of_trips_in_DAS.py


# You do not need to worry about these two lines. Since output files may be spread across
# different folders, these two lines just sopy all of them in the same fodler
cp -p  $OUT_LOCATION/data/traveltime.csv $OUT_LOCATION
cp -p $OUT_LOCATION/data/subtrip_metrics.csv $OUT_LOCATION



##################################
########## PLOTTING DENSITY ######
##################################
./compute_seg_density.bash $OUT_LOCATION/out.txt 05 08
./compute_seg_density.bash $OUT_LOCATION/out.txt 08 11
./compute_seg_density.bash $OUT_LOCATION/out.txt 11 14
./compute_seg_density.bash $OUT_LOCATION/out.txt 14 17
./compute_seg_density.bash $OUT_LOCATION/out.txt 17 20
./compute_seg_density.bash $OUT_LOCATION/out.txt 20 24

#################################
####### COMPUTING COUNTS ########
#################################
./counter.bash $OUT_LOCATION > $OUT_LOCATION/counter.dat
echo "Counts are written in $OUT_LOCATION/counter.dat"


###############################################
########## PLOTTING USER-RELATED METRICS ######
###############################################
./process_traveltime.bash $OUT_LOCATION/traveltime.csv

./process_subtrip_metrics.bash $OUT_LOCATION/subtrip_metrics.csv
