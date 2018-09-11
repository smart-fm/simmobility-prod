"""
sage: controller-statistics.py [-h] --das DAS [--clog CLOG] [--dbname DBNAME]
                                [--dbhost DBHOST] [--dbuser DBUSER]
                                [--dbpwd DBPWD] [--time T]

Script to plot demand, requests, assignments, pickups and dropoff statistics
by time of day.

optional arguments:
  -h, --help       show this help message and exit
  --das DAS        DAS Stored Procedure (default: None)
  --clog CLOG      Path of controller.log (default: controller.log)
  --dbname DBNAME  Database Name Containing DAS (default: simmobility_l2nic2b)
  --dbhost DBHOST  Database IP Address (default: 172.25.184.156)
  --dbuser DBUSER  Database Username (default: postgres)
  --dbpwd DBPWD    Database Password (default: HPCdb@2018)
  --time T         Start Time (in seconds) (default: 0)

Author: Lemuel Kumarga
Date: 06.09.2018

Output:
  1) parking_utilisation.png: The plot
  2) parking_utilisation.csv: The underlying data
"""

import pandas as pd
import numpy as np
import sys
import csv
import datetime
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.markers as mkr
import psycopg2
import argparse
from argparse import ArgumentParser

########################
## INPUT PARAMETERS
########################
class HelpParser(argparse.ArgumentParser):
    def error(self, message):
        sys.stderr.write('error: %s\n' % message)
        self.print_help()
        sys.exit(2)

parser = HelpParser(description="Script to plot parking utilisation by time of day.",
                        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("--park",dest="park", type=str, required=True,
                  help="Parking Stored Procedure")
parser.add_argument("--pfile",dest="pfile", type=str, default="parkingInfo.csv",
                  help="Path of parkingInfo.csv")
parser.add_argument("--dbname",dest="dbname", type=str, default="simmobility_l2nic2b",
                  help="Database Name Containing DAS")
parser.add_argument("--dbhost",dest="dbhost",type=str, default="172.25.184.156",
                  help="Database IP Address")
parser.add_argument("--dbuser",dest="dbuser", type=str, default="postgres",
                  help="Database Username")
parser.add_argument("--dbpwd",dest="dbpwd", type=str, default="HPCdb@2018",
                  help="Database Password")
parser.add_argument("--veh_type",dest="veht", type=str, default="supply2.vehicle_type",
                  help="Vehicle Type Table")
parser.add_argument("--granular",dest="g",type=int, default=5,
                    help="Granularity of plot (Points are plotted for every G minutes.)")
options = parser.parse_args()

##############################
## HELPER FUNCTIONS
##############################

def ceilTime(dt):
  new_dt = datetime.datetime(dt.year, dt.month, dt.day, dt.hour,options.g*(dt.minute / options.g))
  if (dt.minute % options.g == 0 and dt.second == 0):
    return (new_dt)
  else:
    return (new_dt + datetime.timedelta(minutes=options.g))

##############################
## LOAD PARKINGINFO STATISTICS FOR EACH vehicle id type
##############################

timeLimits = [None,None]; 
parked = []; isParkedL = lambda l : l[7] == "PARKED";
exited = []; isExitedL = lambda l : (l[7] == "EXIT_PARKING" or l[7] == "SHIFT_END");

with open(options.pfile, 'rt') as file:
    reader = csv.reader(file)

    for line in reader:

      curTime = pd.to_datetime(line[3], format="%H:%M:%S")
      if (timeLimits[0] is None or timeLimits[0] > curTime):
        timeLimits[0] = curTime
      if (timeLimits[1] is None or timeLimits[1] < curTime):
        timeLimits[1] = curTime

      if isParkedL(line):
        parked.append({
          "time" : curTime,
          "parking_id" : line[4],
          "veh_type_id" : line[1]
        })
      elif isExitedL(line):
        exited.append({
          "time" : curTime,
          "parking_id": line[4],
          "veh_type_id" : line[1]
        })

# Create Data Frame
def toTable(arr, col_names):
    output = pd.DataFrame(arr)
    if output.empty==False:
        output=output[col_names]
    return(output)

parked = toTable(parked,['time','parking_id', 'veh_type_id'])
exited = toTable(exited,['time','parking_id', 'veh_type_id'])

# Group by Time
def timeResample(df, var_type):
    df.time = pd.DatetimeIndex(df.time)
    df.time = df.time.apply(ceilTime)
    if var_type not in df:
        df[var_type] = 1
    return(df.groupby(['time', 'parking_id', 'veh_type_id']).sum())

parkedTmp = timeResample(parked, "spotParked")
exitedTmp = timeResample(exited, "spotExited")

# Get Cumulative Parked and Exited to Find out the # of parking spots occupied then
parked = parkedTmp.merge(exitedTmp, left_index=True, right_index=True, how="outer")
parked.fillna(0, inplace=True)

##############################
## LOAD PARKING CAPACITIES
##############################

# Initialize Database 
dbConn = psycopg2.connect("dbname='" + options.dbname + "' " + \
                          "user='" + options.dbuser + "' " + \
                          "host='" + options.dbhost + "' " + \
                          "password='" + options.dbpwd + "'")
cur = dbConn.cursor()

# Execute Query
cols = {'parking_id' : 'parking_id', 
        'f1.veh_type_id' : 'veh_type_id', 
        'start_time' : 'start_time', 
        'end_time' : 'end_time', 
        'capacity_pcu' : 'capacity_pcu',
        'pcu' : 'pcu' }
query = "SELECT " + ", ".join(cols.keys()) + " FROM " + options.park + "('00:00:00','23:59:59') f1 JOIN " + \
        options.veht + " f2 ON f1.veh_type_id = f2.veh_type_id"
cur.execute(query)
dbConn.commit()

# Create time slots 
time_slots = pd.date_range(start=ceilTime(timeLimits[0]), 
                           end=ceilTime(timeLimits[1]), freq='5T')
time_slots = pd.DataFrame(time_slots)
time_slots.columns = ['time']
time_slots['key'] = 0

# Get Parking ID X Vehicle Type Data Frame
park_slots = pd.DataFrame(cur.fetchall())
park_slots.columns = cols.values()
park_slots.veh_type_id = [ str(v) for v in park_slots.veh_type_id ]
park_slots.start_time = pd.DatetimeIndex(pd.to_datetime(park_slots.start_time, format="%H:%M:%S"))
park_slots.end_time = pd.DatetimeIndex(pd.to_datetime(park_slots.end_time, format="%H:%M:%S"))
park_slots['key'] = 0

# Create Time X Parking ID X Vehicle Type data frame
slots = time_slots.merge(park_slots, on="key")
slots = slots[(slots.start_time <= slots.time) & (slots.time <= slots.end_time)].groupby(['time','parking_id','veh_type_id']).mean()
slots = slots[['capacity_pcu','pcu']]
slots.columns = ['PCUCapacity','PCU']
slots["UnitCapacity"] = slots["PCUCapacity"] / slots["PCU"]

##############################
## GET PARK UTILIZATION
##############################

parkUtil = slots.merge(parked, how='left', left_index=True, right_index=True)
parkUtil.fillna(0, inplace=True)

# Get Occupied Slots
parkOccupied = parkUtil[['spotParked','spotExited']].groupby(level=[1,2]).cumsum()
parkOccupied["UnitOccupied"] = parkOccupied['spotParked'] - parkOccupied['spotExited']

parkStats = pd.concat([parkUtil[['PCU','UnitCapacity','PCUCapacity']], parkOccupied['UnitOccupied'].to_frame()], axis=1, join='outer')
parkStats["PCUOccupied"] = parkStats["PCU"] * parkStats["UnitOccupied"]

# Generate Statistics of Parking Slots
parkPct = parkStats.drop(['PCU'], axis=1).groupby(level=[0,1]).sum()
parkPct["nSlots"] = 1
parkPct["pctOccupied"] = (parkPct["UnitOccupied"] > 0)*1
parkPct["pct25Pct"] = ((parkPct["UnitOccupied"] / parkPct["UnitCapacity"]) >= 0.25) * 1
parkPct["pct50Pct"] = ((parkPct["UnitOccupied"] / parkPct["UnitCapacity"]) >= 0.5) * 1
parkPct["pctFull"] = (parkPct["UnitOccupied"] == parkPct["UnitCapacity"])*1
parkPct = parkPct.groupby(level=[0]).sum()
for v in ["pctOccupied","pct25Pct","pct50Pct","pctFull"]:
  parkPct[v] = parkPct[v] / parkPct['nSlots']

# Plot
res = parkPct[["pctOccupied","pct25Pct","pct50Pct","pctFull"]]
res.columns = ["Spots with >0 Parked","Spots with 25% Full", "Spots with 50% Full", "Spots with 100% Full"]
xbreaks = [i for i in res.index if (i.hour % 3 == 0 and i.minute == 0 and i.second == 0)]

ax = res.plot()
plt.title("Parking Utilization")
plt.xlabel('Time')
plt.xticks(xbreaks, [i.strftime('%H:%M') for i in xbreaks])
plt.ylabel('% of Parking Spots')
ax.set_yticklabels(['{:,.2%}'.format(x) for x in ax.get_yticks()])
plt.savefig("parking_utilisation.png")

# Save
parkPct.index  = [ pd.to_datetime(i).strftime('%H:%M') for i in parkPct.index.values ]
parkPct.to_csv("parking_utilisation.csv")
