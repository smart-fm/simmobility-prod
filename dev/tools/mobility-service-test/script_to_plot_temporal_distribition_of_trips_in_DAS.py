# Modes of transportation: To be specified in the list modesToBePlotted below:
# python3 required; pandas library required
# the activity_schedule file must be present in the same folder; must be in CSV format(just as obtained from the preday output)


import csv
import time
import collections
import matplotlib.pyplot as plt
import numpy as np
import sys
import pandas as pd

# Forcing the usage of python 3 or higher
if sys.version_info[0] < 3:
	raise Exception("Python 3 or a more recent version is required.")


modesToBePlotted = ['Car','BusTravel']


listOfTimeIntervals = np.arange(3.25,27,0.5)  # time intervals given in float
dictOfTimeIntervalIndices = {} # creating a dictionary to convert these time intervals into indices
for i in range(len(listOfTimeIntervals)):
	dictOfTimeIntervalIndices[listOfTimeIntervals[i]] = i

tripStartCounts = {}
modeList = []
with open('activity_schedule') as f:
	next(f)  # this line needs to be commented if there is no header in the file. The sole purpose of this line is to
	# skip the line containing the header

	for row in f:
		listed = row.strip().split(',')
		stop_activity_type = listed[4]
		mode_type = listed[7]
		start_time = float(listed[13])

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
# plt.tight_layout()
# plt.savefig('Number of trips starting at different times of day.png')
plt.show()
