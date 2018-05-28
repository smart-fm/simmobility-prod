############################################
### This script processes the subtrip_metrics.csv, travel_time.csv and waiting_time.csv output files from SimMobility MidTerm
### Outputs are total average travel/trip times (minutes) and total/average trip distances (km).
### _summary.csv files are saved accordingly. Output is also printed to console.
### Run directly from the Basic folder or modify paths accordingly
### Originally written by: Eytan Gross (2018)
### Modified by: Jimi Oke, May 2018
### Usage: Run "python analyze_within_day_trip_metrics.py" from within the dev/Basic folder
###############################################

from psycopg2 import connect
from socket import gethostname
from pandas import DataFrame, read_csv
from sys import exit
from os import remove, chdir, path
from numpy import array

h = path.expanduser('~')
chdir('./')

################################### subtrip_metrics ###################################################################
subtrip_metrics_columns = ['person_id', 'trip_id', 'subtrip_id', 'origin_node', 'origin_taz', 'destination_node', 'destination_taz', 'mode', 'start_time', 'end_time', 'travel_time', 'total_distance', 'cbd_entry_node', 'cbd_exit_node', 'cbd_entry_time', 'cdb_exit_time', 'cbd_travel_time', 'no_cbd_travel_time', 'cbd_distance', 'non_cbd_distance']
subtrip_metrics_summary = DataFrame()
subtrip_metrics = read_csv('subtrip_metrics.csv', names=subtrip_metrics_columns)
subtrip_metrics = subtrip_metrics.set_index('trip_id')
x = subtrip_metrics[subtrip_metrics.travel_time > 0]
y = subtrip_metrics[subtrip_metrics.total_distance > 0]

subtrip_metrics_summary['appearances'] = subtrip_metrics.groupby('mode').travel_time.count()
subtrip_metrics_summary['positive_travel_time'] = x.groupby('mode').travel_time.count()
subtrip_metrics_summary['zero_travel_time'] = subtrip_metrics_summary['appearances'] - subtrip_metrics_summary['positive_travel_time']
subtrip_metrics_summary['total_travel_time'] = 60 * subtrip_metrics.groupby('mode').travel_time.sum()
subtrip_metrics_summary['average_positive_travel_time'] = subtrip_metrics_summary['total_travel_time'] / subtrip_metrics_summary['positive_travel_time']
subtrip_metrics_summary['positive_distance'] = y.groupby('mode').total_distance.count()
subtrip_metrics_summary['positive_distance'] = subtrip_metrics_summary['positive_distance'].fillna(0).astype(int)
subtrip_metrics_summary['zero_distance'] = subtrip_metrics_summary['appearances'] - subtrip_metrics_summary['positive_distance']
subtrip_metrics_summary['travel_distance'] = subtrip_metrics.groupby('mode').total_distance.sum() / 1000
subtrip_metrics_summary['average_distance'] = subtrip_metrics_summary['travel_distance'] / subtrip_metrics_summary['positive_distance']

print "\nsubtrip_metrics summary:\n"
print subtrip_metrics_summary

################################### travel_time ###################################################################
#travel_time_modes = ['ON_BUS', 'ON_MRT', 'ON_CAR', 'ON_MOTORCYCLE', 'ON_TAXI', 'ON_SHARINGCAR', 'ON_PBUS', 'WALK', 'WAIT_BUS', 'ENTRY_PT', 'EXIT_PT', 'ACTIVITY']
travel_time_columns = ['person_id', 'trip_origin_id', 'trip_dest_id', 'subtrip_origin_id', 'subtrip_dest_id', 'subtrip_origin_type', 'subtrip_dest_type', 'travel_mode', 'arrival_time', 'travel_time','pt_line']

travel_time_file = read_csv('traveltime.csv', names=travel_time_columns)
travel_time_summary = DataFrame()
x = travel_time_file[travel_time_file['travel_time'] > 0]

#travel_time_file.travel_time = travel_time_file.travel_time.astype(float)
travel_time_summary['appearances'] = travel_time_file.groupby('travel_mode').travel_time.count()
travel_time_summary['positive_travel_time'] = x.groupby('travel_mode').travel_time.count()
travel_time_summary['positive_travel_time'] = travel_time_summary['positive_travel_time'].fillna(0).astype(int)
travel_time_summary['zero_travel_time'] = travel_time_summary['appearances'] - travel_time_summary['positive_travel_time']
travel_time_summary['total_travel_time'] = travel_time_file.groupby('travel_mode').travel_time.sum() / 60
travel_time_summary['average_positive_travel_time'] = travel_time_summary['total_travel_time'] / travel_time_summary['positive_travel_time']

print "\ntravel time summary:\n"
print travel_time_summary

################################### sub_trips ###################################################################
#sub_trips_modes = ['BusTravel', 'MRT', 'Car', 'Motorcycle', 'Taxi', 'Car Sharing 2', 'PrivateBus', 'Walk', 'Car Sharing 3', 'SMS']
sub_trips_columns = ['trip_id', 'sequence_num', 'travel_mode', 'pt_line_id', 'cbd_traverse_type', 'origin_type', 'destination_type', 'origin_id', 'destination_id']

sub_trips = read_csv('sub_trips.csv', names=sub_trips_columns, index_col='trip_id')
sub_trips_summary = sub_trips.groupby('travel_mode').travel_mode.count()

print "\nsub-trips summary:\n"
print sub_trips_summary

################################### waiting_time ###################################################################
waiting_time_columns = ['person_id', 'origin_node_id', 'dest_node_id', 'start_stop_id', 'end_stop_id', 'pt_line_id_journey', 'pt_line_id', 'board_time', 'wait_time', 'denied_boarding_count']
waiting_time = read_csv('waitingtime.csv', names=waiting_time_columns, index_col='person_id')

waiting_time_summary = DataFrame()
waiting_time_summary['waiting'] = [(len(waiting_time))]
waiting_time_summary['positive_waiting_time'] = len(waiting_time[waiting_time.wait_time > 0])
waiting_time_summary['zero_waiting_time'] = waiting_time_summary.waiting - waiting_time_summary.positive_waiting_time
waiting_time_summary['total_waiting_time'] = sum(waiting_time.wait_time) / 60
waiting_time_summary['average_positive_waiting_time'] = waiting_time_summary.total_waiting_time / waiting_time_summary.positive_waiting_time
waiting_time_summary['average_waiting_time'] = waiting_time_summary.total_waiting_time / waiting_time_summary.waiting

print "\nwaitng time summary:\n"
print waiting_time_summary

#### Write csv files
subtrip_metrics_summary.to_csv('subtrip_metrics_summary.csv',index=True)
travel_time_summary.to_csv('travel_time_summary.csv',index=True)
waiting_time_summary.to_csv('waiting_time_summary.csv',index=True)

