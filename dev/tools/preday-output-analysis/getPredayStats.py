"""
Combined script for various statistics from activity_schedule
The script must be run in the same folder where the activity_schedule file was generated 
This script must be run in python2.7;  python2.7 has been chosed because it is the default version available currently on HPC!

Usage : python2.7 getStats.py activity_schedule_fileName > outputFile.csv

Author: Nishant Kumar
Date: 06.08.2018

Das file headers:
  0) person_id character varying,
  1) tour_no integer,
  2) tourType character varying,
  3) stop_no integer,
  4) stop_type character varying,
  5) stop_location integer,
  6) stopZone integer,
  7) stop_mode character varying,
  8) primary_stop boolean,
  9) arrival_time numeric,
  10) departure_time numeric,
  11) prev_stop_location integer,
  12) prev_stopZone integer,
  13) prev_stop_departure_time numeric,
  14) pid bigserial
"""

import sys
import matplotlib.pyplot as plt 

das = []
# Reading the das as a list of lists
with open(sys.argv[1]) as f:
    for row in f:
        das.append(row.strip().split(','))

print '\n\nTotal number of trips ',len(das)


# Mode shares - Overall        
modeShare = {}
for i in range(len(das)):
    if das[i][7] in modeShare :
        modeShare[das[i][7]] += 1 
    else:
        modeShare[das[i][7]] = 1 

print '\n\nMode shares Overall:'
for key in modeShare :
    print key, ',',  round(modeShare[key]*100.0/(len(das)),2)
print '\n\n\n\n '


# Mode shares - for different tour type
def getModeShareForDifferentTourType(tourType):
    modeShare = {}
    c = 0 
    for i in range(len(das)):
        if das[i][2] == tourType:
            if das[i][7] in modeShare :
                modeShare[das[i][7]] += 1 
            else:
                modeShare[das[i][7]] = 1 
            c += 1 
    print '\n\nMode shares for Tour type ',tourType
    for key in modeShare :
        print key, ',',  round(modeShare[key]*100.0/c,2)
    

for tourName in ['Work','Education','Shop', 'Other']:
    getModeShareForDifferentTourType(tourName)
print '\n\n\n\n '



# Mode shares - for different stop type
def getModeShareForDifferentStopType(stopType):
    modeShare = {}
    c = 0 
    for i in range(len(das)):
        if das[i][4] == stopType:
            if das[i][7] in modeShare :
                modeShare[das[i][7]] += 1 
            else:
                modeShare[das[i][7]] = 1 
            c += 1 
    print '\n\nMode shares for Stop type ',stopType
    for key in modeShare :
        print key, ',',  round(modeShare[key]*100.0/c,2)
    

for stopType in ['Work','Education','Shop', 'Other', 'Home']:
    getModeShareForDifferentStopType(stopType)
print '\n\n\n\n '




# count of trips with origin from different zones in descending order 
def getDestinationZoneCounts (stopType):
    tripsToZone = {}
    c = 0 
    for i in range(len(das)):
        if das[i][2] == stopType :
            if int(das[i][12]) in tripsToZone:
                tripsToZone[int(das[i][12])] += 1
            else:
                tripsToZone[int(das[i][12])] = 1 
            c += 1 
    print '\n\nPercentage of trips in destination zones for Tour type ',stopType
    for key, value in sorted(tripsToZone.iteritems(), key=lambda (k,v): (-v,k)):  # -ve sign put to arrange in descending order
        print key,",",round(value * 100.0 /c,2), '  Count : ', value


for stopType in ['Work','Education','Shop', 'Other','Home']:
    getDestinationZoneCounts(stopType)
print '\n\n\n\n '


# count of trips with destination to different zones in descending order 
def getDestinationZoneCounts (stopType):
    tripsToZone = {}
    c = 0 
    for i in range(len(das)):
        if das[i][2] == stopType :
            if int(das[i][6]) in tripsToZone:
                tripsToZone[int(das[i][6])] += 1
            else:
                tripsToZone[int(das[i][6])] = 1 
            c += 1 
    print '\n\nPercentage of trips in destination zones for Tour type ',stopType
    for key, value in sorted(tripsToZone.iteritems(), key=lambda (k,v): (-v,k)):  # -ve sign put to arrange in descending order
        print key,",",round(value * 100.0 /c,2), '  Count : ', value


for stopType in ['Work','Education','Shop', 'Other','Home']:
    getDestinationZoneCounts(stopType)
print '\n\n\n\n '
    


# Count of trips by time of day; based on start time
def getCountOfTripsByTimeOfDay(stopType):
    countOfTrips = {}
    c = 0 
    for i in range(len(das)):
        if das[i][4] == stopType:
            startTime = int ( float (das[i][13]))
            if startTime in countOfTrips :
                countOfTrips[startTime] += 1 
            else:
                countOfTrips[startTime] = 1 
            c += 1 
    print '\n\Percentage of trips by time of day; based on start time; Destination Stop type:',stopType
    x = []
    y = [] 
    for key in countOfTrips :
        print key, ',',  round(countOfTrips[key]*100.0/c,2)
        x.append(key)    
        y.append(round(countOfTrips[key]*100.0/c,2))
    plt.plot(x,y, linewidth = 3 )
    plt.xlabel('Time of day (in 24 hours format)', fontsize=18)
    plt.ylabel('Percentage of trips', fontsize=18)
    plt.grid(True)
    plt.savefig(stopType + '_time_of_Day.png')
    plt.clf()
    plt.cla()
    plt.close()    
    
    
for stopType in ['Work','Education','Shop', 'Other','Home']:
    getCountOfTripsByTimeOfDay(stopType)
print '\n\n\n\n '


