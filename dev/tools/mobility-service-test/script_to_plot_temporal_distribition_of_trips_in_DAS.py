# Modes of transportation: To be specified in the list modesToBePlotted below:
# pandas library required; 
# psychopg2 required
# the input section below must be reviewed to specifiy correct connections and table names


import csv
import time
import collections
import matplotlib.pyplot as plt
import numpy as np
import sys
import pandas as pd
import psycopg2


######################################################################################################################
# Input Section: Database connection, Activity schedule tablename and The modes to be plotted should be specified here
modesToBePlotted = ['Car','BusTravel']
DB_HOST = 'localhost' #'172.25.184.48'
DB_PORT = '5432'
DB_USER = 'postgres'
DB_PASSWORD = 'postgres'
DB_NAME = 'simmobcity'
DAS_TABLE_NAME = 'demand.das_ltpopulation'
######################################################################################################################



listOfTimeIntervals = np.arange(3.25,27,0.5)  # time intervals given in float
dictOfTimeIntervalIndices = {} # creating a dictionary to convert these time intervals into indices
for i in range(len(listOfTimeIntervals)):
	dictOfTimeIntervalIndices[listOfTimeIntervals[i]] = i

tripStartCounts = {}
modeList = []

fetch_ct_plus_1 = "SELECT person_id, stop_mode, prev_stop_departure_time FROM " + DAS_TABLE_NAME 

connection_string = "dbname='" + DB_NAME + "' user='" + DB_USER + "' host='" + DB_HOST + "' port='" + DB_PORT + "' password='" + DB_PASSWORD + "'"
conn = psycopg2.connect(connection_string)
cur = conn.cursor()
cur.execute(fetch_ct_plus_1)
rows = cur.fetchall()
personIdList = []
ct_plus1_aba_list = []
for row in rows:
	stop_activity_type = row[0]
	mode_type= row[1]
	start_time = float(row[2])
	if mode_type not in modesToBePlotted:
		continue

	if mode_type in tripStartCounts:
		tripStartCounts[mode_type][dictOfTimeIntervalIndices[start_time]] += 1
	else:

		tripStartCounts[mode_type] = [0] * 48   # 48 is the number of time intervals
		modeList.append(mode_type)
		tripStartCounts[mode_type][dictOfTimeIntervalIndices[start_time]] = 1




#######################################################################################################
# Plotting them together
combinedData = []

for i in range(len(modeList)):
	combinedData.append(tripStartCounts[modeList[i]])




combinedData = np.array(combinedData)
combinedData = np.transpose(combinedData)
df2 = pd.DataFrame(combinedData, columns=modeList)
df = df2.plot.bar(xticks=df2.index)


plt.xlabel('Time of day (30 min time intervals)--->', fontsize=18)
plt.ylabel('Count of trips starting in the 30 min interval ---->', fontsize=16)
namesOfTimeIntervals = ['03:00-03:30 ', '03:30-04:00  ', '04:00-04:30', '04:30-05:00', '05:00-05:30', '05:30 -06:00',
						'06:00-06:30', '06:30-07:00', '07:00 -07:30', '07:30-08:00', '08:00-08:30', '08:30 -09:00',
						'09:00-09:30', '09:30-10:00', '10:00 -10:30', '10:30-11:00', '11:00-11:30', '11:30 -12:00 ',
						'12:00-12:30 ', '12:30-01:00 ', '01:00 -01:30 ', '01:30-02:00 ', '02:00-02:30 ',
						'02:30 -03:00 ', '03:00-03:30 ', '03:30-04:00 ', '04:00 -04:30 ', '04:30-05:00 ',
						'05:00-05:30 ', '05:30 -06:00 ', '06:00-06:30 ', '06:30-07:00 ', '07:00 -07:30 ',
						'07:30-08:00 ', '08:00-08:30 ', '08:30 -09:00 ', '09:00-09:30 ', '09:30-10:00 ',
						'10:00 -10:30 ', '10:30-11:00 ', '11:00-11:30 ', '11:30-12:00', '12:00-12:30', '12:30-01:00',
						'01:00 -01:30', '01:30-02:00', '02:00-02:30', '02:30 -03:00']
xtickNames = plt.setp(df, xticklabels=namesOfTimeIntervals)
plt.setp(xtickNames, rotation=90, fontsize=10)
plt.title('Number of trips starting at different times of day ', fontsize=25)
mng = plt.get_current_fig_manager()
mng.full_screen_toggle()
plt.show(block = False)
plt.savefig("Number of trips starting at different times of day.png.png", bbox_inches='tight')
