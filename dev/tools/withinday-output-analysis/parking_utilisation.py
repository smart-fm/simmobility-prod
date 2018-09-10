"""
usage: parking_utilisation.py [-h] [--pfile PFILE] [--dbname DBNAME]
                              [--dbhost DBHOST] [--dbuser DBUSER]
                              [--dbpwd DBPWD] [--park PARK]

Script to plot parking utilisation by time of day.

optional arguments:
  -h, --help       show this help message and exit
  --pfile PFILE    Path of parkingInfo.csv (default: parkingInfo.csv)
  --dbname DBNAME  Database Name Containing DAS (default: simmobility_l2nic2b)
  --dbhost DBHOST  Database IP Address (default: 172.25.184.156)
  --dbuser DBUSER  Database Username (default: postgres)
  --dbpwd DBPWD    Database Password (default: d84dmiN)
  --park PARK      Parking Stored Procedure (default:
                   public.get_parking_infra)

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
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.markers as mkr
import matplotlib.dates as mdates
import psycopg2
import argparse
from argparse import ArgumentParser

########################
## INPUT PARAMETERS
########################
parser = ArgumentParser(description="Script to plot parking utilisation by time of day.",
                        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("--pfile",dest="pfile", type=str, default="parkingInfo.csv",
                  help="Path of parkingInfo.csv")
parser.add_argument("--dbname",dest="dbname", type=str, default="simmobility_l2nic2b",
                  help="Database Name Containing DAS")
parser.add_argument("--dbhost",dest="dbhost",type=str, default="172.25.184.156",
                  help="Database IP Address")
parser.add_argument("--dbuser",dest="dbuser", type=str, default="postgres",
                  help="Database Username")
parser.add_argument("--dbpwd",dest="dbpwd", type=str, default="d84dmiN",
                  help="Database Password")
parser.add_argument("--park",dest="park", type=str, default="public.get_parking_infra",
                  help="Parking Stored Procedure")
options = parser.parse_args()

##############################
## LOAD PARKINGINFO STATISTICS FOR EACH vehicle id type
##############################

parked = []; isParkedL = lambda l : l[7] == "PARKED";
exited = []; isExitedL = lambda l : l[7] == "EXIT_PARKING";

with open(options.pfile, 'rt') as file:
    reader = csv.reader(file)

    for line in reader:
      if isParkedL(line):
        parked.append({
          "time" : line[3],
          "parking_id" : line[4],
          "veh_type_id" : line[1]
        })
      elif isExitedL(line):
        exited.append({
          "time" : line[3],
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
veh_type_ids = parked.veh_type_id.unique()

# Group by Time
def timeResample(df, var_type, g=5):
    df.time = pd.DatetimeIndex(pd.to_datetime(df.time, format="%H:%M:%S"))
    df.time = df.time.apply(lambda dt: pd.datetime(dt.year, dt.month, dt.day, dt.hour,g*(dt.minute / g)))
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
cols = ['parking_id', 'veh_type_id', 'start_time', 'end_time', 'capacity_pcu']
query = "SELECT " + ", ".join(cols) + " FROM " + options.park + "('00:00:00','23:59:59') "
cur.execute(query)
dbConn.commit()

# Create time slots 
time_slots = parked.groupby(level=[0]).sum()
time_slots['key'] = 0

# Get Parking ID X Vehicle Type Data Frame
park_slots = pd.DataFrame(cur.fetchall())
park_slots.columns = cols
park_slots.veh_type_id = [ str(v) for v in park_slots.veh_type_id ]
park_slots.start_time = pd.DatetimeIndex(pd.to_datetime(park_slots.start_time, format="%H:%M:%S"))
park_slots.end_time = pd.DatetimeIndex(pd.to_datetime(park_slots.end_time, format="%H:%M:%S"))
park_slots['key'] = 0

# Create Time X Parking ID X Vehicle Type data frame
slots = time_slots.reset_index().merge(park_slots, on="key")
slots = slots[(slots.start_time <= slots.time) & (slots.time <= slots.end_time)].groupby(['time','parking_id','veh_type_id']).mean()
slots = slots['capacity_pcu'].to_frame()
slots.columns = ['Capacity']

##############################
## GET PARK UTILIZATION
##############################

parkUtil = slots.merge(parked, how='left', left_index=True, right_index=True)
parkUtil.fillna(0, inplace=True)

# Get Occupied Slots
parkOccupied = parkUtil[['spotParked','spotExited']].groupby(level=[1,2]).cumsum()
parkOccupied["Occupied"] = parkOccupied['spotParked'] - parkOccupied['spotExited']

parkStats = pd.concat([parkUtil['Capacity'], parkOccupied['Occupied']], axis=1, join='outer')

# Generate Statistics of Parking Slots
parkPct = parkStats.groupby(level=[0,1]).sum()
parkPct["nSlots"] = 1
parkPct["pctOccupied"] = (parkPct["Occupied"] > 0)*1
parkPct["pct25Pct"] = ((parkPct["Occupied"] / parkPct["Capacity"]) >= 0.25) * 1
parkPct["pct50Pct"] = ((parkPct["Occupied"] / parkPct["Capacity"]) >= 0.5) * 1
parkPct["pctFull"] = (parkPct["Occupied"] == parkPct["Capacity"])*1
parkPct = parkPct.groupby(level=[0]).sum()
for v in ["pctOccupied","pct25Pct","pct50Pct","pctFull"]:
  parkPct[v] = parkPct[v] / parkPct['nSlots']

# Plot
res = parkPct[["pctOccupied","pct25Pct","pct50Pct","pctFull"]]
res.columns = ["Spots with >0 Parked","Spots with 25% Full", "Spots with 50% Full", "Spots with 100% Full"]

ax = res.plot()
plt.title("Parking Utilization")
plt.xlabel('Time')
#ax.xaxis.set_major_formatter(mdates.DateFormatter("%H:%M"))
plt.ylabel('% of Parking Spots')
ax.set_yticklabels(['{:,.2%}'.format(x) for x in ax.get_yticks()])
plt.savefig("parking_utilisation.png")

# Save
parkPct.index  = [ pd.to_datetime(i).strftime('%H:%M') for i in res.index.values ]
parkPct.to_csv("parking_utilisation.csv")
