# This script updates 4 variables in total: avg_wait_time, avg_dwell_time, num_bus_arrivals and num_persons_boarding 

# inputs passed as arguments to the script:
# argument 1 : path to the generated pt_stop_stats file 
# argument 2 : name of the ptstopstats table in the database, along with the schema name
# argument 3: alpha value (updated variables are calculated as :  (1-alpha)* <old_Value> + (alpha)*<new_Value> )

# Example of usage:
### python2.7 upsert_PT_stop_stats.py ptstopstats.csv supply.pt_bus_stop_stats 0.4

import argparse
import datetime
import csv
import psycopg2
import sys
import os
import time
import numpy as np

# inserting into the correct database, the connection details must be entered here
DB_HOST = 'localhost'
DB_PORT = '5432'
DB_USER = 'postgres'
DB_PASSWORD = 'postgres'
DB_NAME = 'simmobcity'

if len(sys.argv) != 4:
    print ('Invalid number of arguments passed ')
    sys.exit(0)

generated_link_TT_file = sys.argv[1]
table_name_in_DB = sys.argv[2] 
alpha = float(sys.argv[3])


# create the connection
connection_string = "dbname='" + DB_NAME + "' user='" + DB_USER + "' host='" + DB_HOST + "' port='" + DB_PORT + "' password='" + DB_PASSWORD + "'"
conn = psycopg2.connect(connection_string)
cur = conn.cursor()

generated_ptstop_stats_TT_file = sys.argv[1]

# change delimiter to commas
res = os.system('sed -i \'s/;/,/g\' ' + generated_link_TT_file)
if res != 0 :
    print 'Cannot find the link travel time file named :', generated_ptstop_stats_TT_file
    sys.exit(0)

# read old values from the DB
retrieve_Old_Values = " \
    select * \
    from " + sys.argv[2]
outputquery = "COPY ({0}) TO STDOUT WITH CSV HEADER".format(retrieve_Old_Values)
with open('old_pt_stop_stat_values', 'w') as f:
    cur.copy_expert(outputquery, f)


# Read the values from the simulated link travel time file and put them in a dictionary
newVals = {}
with open(generated_ptstop_stats_TT_file) as f:
    for row in f:
        listed = row.split(',')
        newVals[(listed[0] , listed[1] , listed[2] )] = np.array ( [ float(listed[3]), float(listed[4]), float(listed[5]), float(listed[6]) ])

# Read the values from the historical link travel time from the DB and put them in a dictionary
oldVals = {}
with open('old_pt_stop_stat_values') as f:
    f.next()
    for row in f:
        listed = row.split(',')
        oldVals[(listed[0] , listed[1] , listed[2] )] = np.array([ float(listed[3]), float(listed[4]), float(listed[5]), float(listed[6]) ])

# update the values using the update rule-
# If values are present in both old and new dictionary use the update rule:  (1-alpha)* <old_TT> + (alpha)*<new_TT>
# If the values of link travel time are not present in the simulated file- old values are retained
# # If the values of link travel time are not present in the simulated file- new values are retained

differenceOfValues = []
updatedVals= {}
oldValsForMean = []
for key in oldVals:
    if key in newVals:
        updatedVals[key] = (1 - alpha) * oldVals[key] + alpha * newVals[key]
        differenceOfValues.append(oldVals[key] - newVals[key])
        oldValsForMean.append(oldVals[key])
    else:
        updatedVals[key] = oldVals[key]

rmsnForPTStopStatsUpdate = np.sqrt(np.mean(np.square(differenceOfValues),1)) / (np.mean(oldValsForMean,1))
print 'RMSN value for link travel time update: ', rmsnForPTStopStatsUpdate
with open('RMSN_records_pt_stop_stats.txt','a') as f:
    f.write("RMSN value for differences in PT stop stats :"+ str( rmsnForPTStopStatsUpdate ) + "\n")

for key in newVals:
    if key not in oldVals:
        updatedVals[key] = newVals[key]


# Write the updated dictionary to a file and push it  to the DB to replace the old TT
with open('updated_table_temp.csv','w') as f:
    csvwriter = csv.writer(f)
    #csvwriter.writerow(['link_id','downstream_link_id','start_time','end_time','travel_time'])
    for key in updatedVals:
        csvwriter.writerow(list(key) + list(updatedVals[key]))
f.close()

drop_table_query = 'DROP TABLE '+ table_name_in_DB
table_name_in_DB_without_schema = table_name_in_DB.split('.')[1]

# create the table
create_table_query =  \
' CREATE TABLE '+ table_name_in_DB	+ \
    '(       interval_id integer NOT NULL,                     \
        bus_stop_code character varying(25) NOT NULL,           \
    bus_line character varying(50) NOT NULL,                 \
        avg_wait_time double precision,                           \
    avg_dwell_time double precision,                           \
    num_bus_arrivals double precision,                          \
    num_persons_boarding double precision,                       \
    CONSTRAINT pt_bus_stop_stats_pkey PRIMARY KEY (interval_id, bus_stop_code, bus_line)    \
    )							         \
    WITH (							  \
        OIDS=FALSE						   \
    );							            \
    ALTER TABLE '+table_name_in_DB+'			             \
        OWNER TO postgres;'

print ('Dropping the table')
cur.execute(drop_table_query)
print ('Creating the table')
cur.execute(create_table_query)



with open('updated_table_temp.csv') as f:
    cur.copy_from(f,table_name_in_DB,sep = ',')

print ('PT_Stop_Stats_calib_table in DB created.. Updated values pushed')

conn.commit()

