"""
usage: vehicle_utilisation.py [-h] [--tfile TFILE] [--granular G]

Script to plot on-call service statuses throughout the day.

optional arguments:
  -h, --help     show this help message and exit
  --tfile TFILE  Path of taxi trajectory csv file. (default:
                 onCall_taxi_trajectory.csv)
  --granular G   Granularity of plot (Points are plotted for every G minutes.)
                 (default: 5)

Author: Param (Last Updated by Lemuel Kumarga)
Date: 31.08.2018

Output:
  1) vehicle_utilisation.png: The plot
  2) vehicle_utilisation.csv: The underlying data
"""

import pandas as pd
import csv
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from collections import OrderedDict
import numpy as np
import argparse
from argparse import ArgumentParser

class HelpParser(argparse.ArgumentParser):
    def error(self, message):
        sys.stderr.write('error: %s\n' % message)
        self.print_help()
        sys.exit(2)

parser = HelpParser(description="Script to plot on-call service statuses throughout the day.",
                        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("--tfile", dest="tfile", type=str, default="onCall_taxi_trajectory.csv",
                    help="Path of taxi trajectory csv file.")
parser.add_argument("--granular",dest="g",type=int, default=5,
                    help="Granularity of plot (Points are plotted for every G minutes.)")
options = parser.parse_args()


requiredlines = OrderedDict()
with open (options.tfile, 'rt') as file:
    reader = csv.reader(file)

    for line in reader:
        if line[3].split(':')[-1] == '00' and int(line[3].split(':')[1]) % options.g == 0:
            key = line[3]
            value = (line[1], line[6])
            if key not in requiredlines:
                requiredlines[key] = set()
            requiredlines[key].add(value)

cruising = [0]*len(requiredlines)
driving = [0]*len(requiredlines)
parked = [0]*len(requiredlines)
withPassenger = [0]*len(requiredlines)
driveparking = [0]*len(requiredlines)
total = [0]*len(requiredlines)

for i, time in enumerate(requiredlines.values()):
    for action in time:
        if action[1] == 'CRUISING':
            cruising[i] += 1
        elif action[1] == 'DRIVE_ON_CALL':
            driving[i] += 1
        elif action[1] == 'PARKED':
            parked[i] += 1
        elif action[1] == 'DRIVE_WITH_PASSENGER':
            withPassenger[i] += 1
        elif action[1] == 'DRIVE_TO_PARKING':
            driveparking[i] +=1

        total[i] += 1

res = pd.DataFrame([cruising, driving, parked, withPassenger, driveparking, total]).transpose()
res.columns = ["Cruising","Drive to Pickup","Parked","Drive With Passenger","Drive to Parking","Total"]
res["time"] = requiredlines.keys()
res.time = pd.DatetimeIndex(pd.to_datetime(res.time, format="%H:%M:%S"))
res.set_index("time", inplace=True)
xbreaks = [i for i in res.index if (i.hour % 3 == 0 and i.minute == 0 and i.second == 0)]

ax = res.plot()
plt.title('Vehicle Utilisation')
plt.xlabel('Time')
plt.xticks(xbreaks, [i.strftime('%H:%M') for i in xbreaks])
plt.ylabel('Number of Drivers')
plt.savefig("vehicle_utilisation.png")

res.index  = [ pd.to_datetime(i).strftime('%H:%M') for i in res.index.values ]
res.to_csv("vehicle_utilisation.csv")

