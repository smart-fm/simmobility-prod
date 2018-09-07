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

parked = []; isParkedL = lambda l : l[6] == "PARKED";
exited = []; isExitedL = lambda l : l[6] == "EXIT_PARKING";

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

parked = toTable(parked,['time','veh_type_id'])
exited = toTable(exited,['time','veh_type_id'])
veh_type_ids = parked.veh_type_id.unique()

# Group by Time
def timeResample(df, var_type, cond='5T'):
    df.time = pd.DatetimeIndex(pd.to_datetime(df.time, format="%H:%M:%S"))
    df.time = pd.DatetimeIndex(df.time)
    df.set_index('time',inplace=True)
    if var_type not in df:
        df[var_type] = 1
    return(df.resample(cond, how='sum')[var_type])

# Get Number of Parkings For Each Vehicle Type ID
def parkStats(veh_type_id):

  parkedTmp = timeResample(parked[parked.veh_type_id == veh_type_id], "spotParked")
  exitedTmp = timeResample(exited[exited.veh_type_id == veh_type_id], "spotExited")

  parkStats = pd.concat([parkedTmp, exitedTmp], axis=1,join='outer')
  parkStats.fillna(0, inplace=True)
  parkStats["cumParked"] = np.cumsum(parkStats.spotParked)
  parkStats["cumExited"] = np.cumsum(parkStats.spotExited)
  parkStats["Parked"] = parkStats["cumParked"] - parkStats["cumExited"]

  return(parkStats["Parked"])

parkCount = dict((v_id, parkStats(v_id)) for v_id in veh_type_ids)

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
park_slots = cur.fetchall()
park_slots = pd.DataFrame(park_slots)
park_slots.columns = cols
park_slots.start_time = pd.DatetimeIndex(pd.to_datetime(park_slots.start_time, format="%H:%M:%S"))
park_slots.end_time = pd.DatetimeIndex(pd.to_datetime(park_slots.end_time, format="%H:%M:%S"))

##############################
## GET PARK UTILIZATION FOR EACH VEHICLE TYPE ID
##############################

# Determine the total capacity for a particular vehicle type at a particular time
def parkUtilization(veh_type_id):
  parkCountTmp = parkCount[veh_type_id].to_frame()
  parkSlotsTmp = park_slots[park_slots.veh_type_id == int(veh_type_id)]

  # Build Park Capacities
  parkCountTmp['key'] = 0
  parkSlotsTmp['key'] = 0
  parkCap = parkCountTmp.reset_index().merge(parkSlotsTmp, on="key")
  parkCap = parkCap[(parkCap.start_time <= parkCap.time) & (parkCap.time <= parkCap.end_time)].groupby('time').sum()

  # Calculate Park Utilities
  parkUtil = pd.concat([parkCountTmp["Parked"], parkCap["capacity_pcu"]], axis=1, join="outer")
  parkUtil.columns = ["OCCUPIED FOR VEH_TYPE_ID=" + veh_type_id, "CAPACITY FOR VEH_TYPE_ID=" + veh_type_id ]
  return(parkUtil)

res = pd.concat([parkUtilization(v_id) for v_id in veh_type_ids], axis=1, join="outer")
res.plot()
plt.title("Parking Utilization")
plt.xlabel('Time')
plt.xticks([])
plt.ylabel('Parking Slots')
plt.savefig("parking_utilisation.png")

res.index  = [ pd.to_datetime(i).strftime('%H:%M') for i in res.index.values ]
res.to_csv("parking_utilisation.csv")
