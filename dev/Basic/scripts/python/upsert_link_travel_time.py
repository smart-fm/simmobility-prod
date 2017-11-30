# inputs passed as arguments to the script:

# argument 1 : path to the generated_link_TT_file to be pushed
# argument 2 : name of the table in the database
# argument 3: alpha value (updated link travel time is calculated as :  (1-alpha)* <old_TT> + (alpha)*<new_TT> )

# Example of usage:
### python upsert_link_travel_time.py link_travel_time.csv link_travel_time 0.4

import argparse
import datetime
import csv
import psycopg2
import sys
import os
import time

# inserting into the .48 server database
DB_HOST = '172.25.184.48'
DB_PORT = '5432'
DB_USER = 'postgres'
DB_PASSWORD = '5M_S1mM0bility'
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

generated_link_TT_file = sys.argv[1]

# change delimiter to commas
os.system('sed -i \'s/;/,/g\' ' + generated_link_TT_file)

# read old values from the DB
retrieve_Old_Values = " \
    select * \
    from " + sys.argv[2]
outputquery = "COPY ({0}) TO STDOUT WITH CSV HEADER".format(retrieve_Old_Values)
with open('old_link_TTs', 'w') as f:
    cur.copy_expert(outputquery, f)


# Read the values from the simulated link travel time file and put them in a dictionary
newVals = {}
with open(generated_link_TT_file) as f:
    for row in f:
        listed = row.split(',')
        newVals[(listed[0] , listed[1] , listed[2] , listed[3])] = float(listed[4])

# Read the values from the historical link travel time from the DB and put them in a dictionary
oldVals = {}
with open('old_link_TTs') as f:
    f.next()
    for row in f:
        listed = row.split(',')
        oldVals[(listed[0] , listed[1] , listed[2] , listed[3])] = float(listed[4])

# update the values using the update rule-
# If values are present in both old and new dictionary use the update rule:  (1-alpha)* <old_TT> + (alpha)*<new_TT>
# If the values of link travel time are not present in the simulated file- old values are retained
# # If the values of link travel time are not present in the simulated file- new values are retained
updatedVals= {}
for key in oldVals:
    if key in newVals:
        updatedVals[key] = (1 - alpha) * oldVals[key] + alpha * newVals[key]
    else:
        updatedVals[key] = oldVals[key]

for key in newVals:
    if key not in oldVals:
        updatedVals[key] = newVals[key]


# Write the updated dictionary to a file and push it  to the DB to replace the old TT
with open('updated_table_temp.csv','w') as f:
    csvwriter = csv.writer(f)
    #csvwriter.writerow(['link_id','downstream_link_id','start_time','end_time','travel_time'])
    for key in updatedVals:
        csvwriter.writerow(list(key) + [updatedVals[key]])
f.close()

drop_table_query = 'DROP TABLE '+ table_name_in_DB
table_name_in_DB_without_schema = table_name_in_DB.split('.')[1]

# create the table
create_table_query = \
    ' CREATE TABLE '+table_name_in_DB	+		\
    '( 							\
      link_id bigint NOT NULL,				\
      downstream_link_id bigint NOT NULL,			\
      start_time time without time zone NOT NULL,		\
      end_time time without time zone NOT NULL,		\
      travel_time double precision NOT NULL,		\
      CONSTRAINT '+ table_name_in_DB_without_schema +'_pkey PRIMARY KEY (link_id, downstream_link_id, start_time, end_time),		\
      CONSTRAINT '+ table_name_in_DB_without_schema +'_downstream_link_id_fkey FOREIGN KEY (downstream_link_id)	\
          REFERENCES supply.link (id) MATCH SIMPLE		\
          ON UPDATE NO ACTION ON DELETE NO ACTION,		\
      CONSTRAINT '+ table_name_in_DB_without_schema +'_id_fkey FOREIGN KEY (link_id)		\
          REFERENCES supply.link (id) MATCH SIMPLE		\
          ON UPDATE NO ACTION ON DELETE NO ACTION		\
    )							\
    WITH (							\
      OIDS=FALSE						\
    );							\
    ALTER TABLE '+table_name_in_DB+'			\
      OWNER TO postgres;'

print ('Dropping the table')
cur.execute(drop_table_query)
print ('Creating the table')
cur.execute(create_table_query)



with open('updated_table_temp.csv') as f:
    cur.copy_from(f,table_name_in_DB,sep = ',')

print ('Link travel-time table in DB created.. Updated values pushed')

conn.commit()



