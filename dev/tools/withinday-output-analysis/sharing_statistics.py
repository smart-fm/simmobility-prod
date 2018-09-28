"""
usage: sharing_statistics.py [-h] [--tfile TFILE] [--granular G]

Script to plot vehicle occupancy of on-call taxi drivers throughout the day.

optional arguments:
  -h, --help     show this help message and exit
  --tfile TFILE  Path of taxi trajectory csv file. (default:
                 onCall_taxi_trajectory.csv)
  --granular G   Granularity of plot (Points are plotted for every G minutes.)
                 (default: 5)
  --capacity C   The maximum aggregated requests value supplied in
                 simulation.xml (default: 6)
  --plot_only_shared  Ignore rides containing 0 or 1 person. (default: true)
Author: Param
Date: 26.09.2018

Output:
  1) sharing_statistics.png: The plot
  2) sharing_statistics.csv: The underlying data
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

parser = HelpParser(description="Script to plot vehicle occupancy of on-call taxi drivers throughout the day.",
                        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("--tfile", dest="tfile", type=str, default="onCall_taxi_trajectory.csv",
                    help="Path of taxi trajectory csv file.")
parser.add_argument("--granular",dest="g",type=int, default=1,
                    help="Granularity of plot (Points are plotted for every G minutes.)")
parser.add_argument("--capacity",dest="c",type=int, default=6,
                    help="The maximum aggregated requests value supplied in simulation.xml")
parser.add_argument("--plot_only_shared",dest="s",type=bool, default=True,
                    help="Ignore rides containing 0 or 1 person.")
options = parser.parse_args()


requiredlines = OrderedDict()
with open (options.tfile, 'rt') as file:
    reader = csv.reader(file)

    for line in reader:
        if line[3].split(':')[-1] == '00' and int(line[3].split(':')[1]) % options.g == 0:
            key = line[3]
            value = (line[1], line[6], int(line[7]))
            if key not in requiredlines:
                requiredlines[key] = set()
            requiredlines[key].add(value)

plots = []
end = options.c
if (options.s):
    end = end - 2
for i in range(end+1):
    plots.append([0]*len(requiredlines))

for i, time in enumerate(requiredlines.values()):
    for action in time:
        if action[1] == 'DRIVE_ON_CALL' or action[1] == 'DRIVE_WITH_PASSENGER':
            if options.s and action[2] > 1:
                plots[action[2]-2][i] += 1
            elif not options.s:
                plots[action[2]][i] += 1

res = pd.DataFrame(plots).transpose()
if options.s:
    res.columns = range(2, 2+len(plots))
res["time"] = requiredlines.keys()
res.time = pd.DatetimeIndex(pd.to_datetime(res.time, format="%H:%M:%S"))
res.set_index("time", inplace=True)
xbreaks = [i for i in res.index if (i.hour % 3 == 0 and i.minute == 0 and i.second == 0)]

ax = res.plot()
plt.title('Sharing Statistics')
plt.xlabel('Time')
plt.xticks(xbreaks, [i.strftime('%H:%M') for i in xbreaks])
plt.ylabel('Number of Drivers')
plt.savefig("sharing_statistics.png")

res.index  = [ pd.to_datetime(i).strftime('%H:%M') for i in res.index.values ]
res.to_csv("sharing_statistics.csv")

