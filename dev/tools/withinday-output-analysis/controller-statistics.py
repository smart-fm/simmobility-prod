"""
usage: parking_utilisation.py [-h] --park PARK [--pfile PFILE]
                              [--dbname DBNAME] [--dbhost DBHOST]
                              [--dbuser DBUSER] [--dbpwd DBPWD]
                              [--veh_type VEHT] [--granular G]

Script to plot parking utilisation by time of day.

optional arguments:
  -h, --help       show this help message and exit
  --park PARK      Parking Stored Procedure (default: None)
  --pfile PFILE    Path of parkingInfo.csv (default: parkingInfo.csv)
  --dbname DBNAME  Database Name Containing DAS (default: simmobility_l2nic2b)
  --dbhost DBHOST  Database IP Address (default: 172.25.184.156)
  --dbuser DBUSER  Database Username (default: postgres)
  --dbpwd DBPWD    Database Password (default: HPCdb@2018)
  --veh_type VEHT  Vehicle Type Table (default: supply2.vehicle_type)
  --granular G     Granularity of plot (Points are plotted for every G
                   minutes.) (default: 5)

Author: Lemuel Kumarga
Date: 31.08.2018

Output:
  1) controller-statistics.png: The plot
  2) controller-statistics.csv: The underlying data
"""

import pandas as pd
import sys
import os
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.markers as mkr
import matplotlib.dates as mdates
import re
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

parser = HelpParser(description="Script to plot demand, requests, assignments, pickups and dropoff statistics by time of day.",
                        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("--das",dest="das", type=str, required=True,
                  help="DAS Stored Procedure")
parser.add_argument("--clog",dest="clog", type=str, default="controller.log",
                  help="Path of controller.log")
parser.add_argument("--dbname",dest="dbname", type=str, default="simmobility_l2nic2b",
                  help="Database Name Containing DAS")
parser.add_argument("--dbhost",dest="dbhost",type=str, default="172.25.184.156",
                  help="Database IP Address")
parser.add_argument("--dbuser",dest="dbuser", type=str, default="postgres",
                  help="Database Username")
parser.add_argument("--dbpwd",dest="dbpwd", type=str, default="HPCdb@2018",
                  help="Database Password")
parser.add_argument("--time",dest="t", type=int, default=0,
                  help="Start Time (in seconds)")
options = parser.parse_args()

########################
## SHARED FUNCTIONS
########################
def timeResample(df, var_type, cond='30T'):
    df.time = options.t + df.time
    df.time = pd.to_datetime(df.time, unit='s')
    df.time = pd.DatetimeIndex(df.time)
    df.set_index('time',inplace=True)
    if var_type not in df:
        df[var_type] = 1
    return(df.resample(cond, how='sum')[var_type])


##############################
## PARSE STATS FROM CONTROLLER
##############################
def getTrips(logfile, controller_id="AMOD"):
    ############################
    # Parse Values From Log File
    ############################
    requests = []; isRequestL = lambda l : l.startswith('Request sent to controller of type SERVICE_CONTROLLER_' +controller_id);
    assignments = []; isAssignL = lambda l : l.startswith('SERVICE_CONTROLLER_'+controller_id+' controller') and ('sent this assignment to driver' in l);
    pickups = []; isPickupL = lambda l : l.startswith('Pickup succeeded for');
    dropoffs = []; isDropoffL = lambda l : l.startswith('Drop-off of user ');

    with open(logfile, "r") as fi:
        controllerLogLines = fi.readlines();

    for line in controllerLogLines:
        if isRequestL(line):
            requests.append({
                'time':float(line.split('at frame ')[1].split(' ')[1].split('s')[0]),
                'user_id':line.split('request issued by ')[1].split(' person')[0], 
                'origin':float(line.split('from node ')[1].split(', to node ')[0]),
                'destination':float(line.split('to node ')[1].split('.')[0])
            })
        elif isAssignL(line):
            assignments.append({
                'time':float(line.split('is sent at frame ')[1].split(', ')[1].split('sto')[0]),
                'driver_id':line.split('to driver ')[1].split(':')[1].split('(')[0]
                })
        elif isPickupL(line):
            pickups.append({
                'time':float(line.split('frame ')[1].split(', ')[1].split('s with')[0]),
                'driver_id':line.split('driverId ')[1].split('\n')[0], 
                'user_id':line.split('Pickup succeeded for ')[1].split(' ')[0],
                'origin':line.split('startNodeId ')[1].split(',')[0], 
                'destination':line.split('destinationNodeId ')[1].split(',')[0]
                })
        elif isDropoffL(line):
            dropoffs.append({
                'time':float(line.split('frame ')[1].split(', ')[1].split('s')[0]),
                'driver_id':line.split('driverId ')[1].split('\n')[0], 
                'user_id':line.split('Drop-off of user ')[1].split(' ')[0],
                'destination':line.split('destinationNodeId ')[1].split('and')[0]
                })            

    ############################
    # Create Data Frame
    ############################
    def toTable(arr, col_names):
        output = pd.DataFrame(arr)
        if output.empty==False:
            output=output[col_names]
        return(output)

    requests = toTable(requests, ['time','user_id','origin','destination'])
    assignments = toTable(assignments, ['time','driver_id'])
    pickups = toTable(pickups, ['time','driver_id','user_id','origin', 'destination'])
    dropoffs = toTable(dropoffs, ['time','driver_id','user_id','destination'])

    ############################
    # Create Index and Resample
    ############################
    requests = timeResample(requests, "Requests")
    assignments = timeResample(assignments, "Assignments")
    pickups = timeResample(pickups, "Pickups")
    dropoffs = timeResample(dropoffs, "Dropoffs")

    # Combine All Plots Together
    output = pd.concat([requests, assignments, pickups, dropoffs], axis=1,join='outer')
    return(output)

##############################
## PARSE DEMAND FROM DAS
##############################
def getDemand(das_stored_proc, stop_modes = ["SMS","Rail_SMS","SMS_Pool","Rail_SMS_Pool"]):

    ############################
    # Initialize Database
    ############################    
    dbConn = psycopg2.connect("dbname='" + options.dbname + "' " + \
                              "user='" + options.dbuser + "' " + \
                              "host='" + options.dbhost + "' " + \
                              "password='" + options.dbpwd + "'")
    cur = dbConn.cursor()

    ############################
    # Execute Query
    ############################        
    query = "SELECT prev_stop_departure_time, COUNT(*) FROM " + das_stored_proc + "(0,99) " + \
            "WHERE stop_mode IN ('" + "','".join(stop_modes) + "') GROUP BY 1 ORDER BY 1"
    cur.execute(query)
    dbConn.commit()
    demand = cur.fetchall()

    ############################
    # Post Process
    ############################ 
    demand = pd.DataFrame(demand)
    demand["time"] = [ int(v) - options.t for v in demand[0]*60*60 ]
    demand["Demand"] = [ int(v) for v in demand[1] ]
    demand = timeResample(demand,"Demand")
    return demand

res = getTrips(options.clog).merge(getDemand(options.das).to_frame(), left_index=True, right_index=True, how="left")
res.fillna(0, inplace=True)
xbreaks = [i for i in res.index if (i.hour % 3 == 0 and i.minute == 0 and i.second == 0)]

ax = res[["Demand","Requests","Assignments","Pickups","Dropoffs"]].plot()
plt.title("Controller Statistics")
plt.xlabel('Time')
plt.xticks(xbreaks, [i.strftime('%H:%M') for i in xbreaks])
plt.ylabel('Counts')
plt.savefig("controller-statistics.png")

res.index  = [ pd.to_datetime(i).strftime('%H:%M') for i in res.index.values ]
res.to_csv("controller-statistics.csv")
