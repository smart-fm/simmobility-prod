"""
Script to extract requests, pickups and dropoffs by time of day
By default, the script will look for controller.log in the current directory. However,
this may be overriden by specifying the file path as an argument.


Usage: 
python2.7 controller-statistics.py [optional: path of controller.log] [optional: starttime in seconds]

Example:
python2.7 controller-statistics.py /home/.../controller.log 0

Author: Unknown (Last Updated by Lemuel Kumarga)
Date: 31.08.2018

Output:
  1) request_pickup_dropoffs.png: Plot of Requests, Pickups, Dropoffs by time of day
  2) request_pickup_dropoff.csv: # of Requests, Pickups, Dropoffs by time of day
"""

import pandas as pd
import sys
import os
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.markers as mkr
import re

#import brewer2mpl as b2m # Brewer colors; view here: http://bl.ocks.org/mbostock/5577023
#set1H = b2m.get_map('Set1','Qualitative', 5).hex_colors)

set1H = ['#E41A1C', '#377EB8', '#4DAF4A', '#984EA3', '#FF7F00']

markers_array  = ["-","--","-.",":"]

log_path = "controller.log"
if (len(sys.argv) > 1):
    log_path = sys.argv[1]

start_time = 0 * 60 * 60
if (len(sys.argv) > 2):
    start_time = float(sys.argv[2])

def getStats(logfile,park=True,controller=["AMOD"]):
    # shared_requests=dict.fromkeys(('time', 'user_id','origin', 'destination'),0)
    # amod_requests=dict.fromkeys(('time', 'user_id','origin', 'destination'),0)
    # shared_subscriptions=dict.fromkeys(('time', 'driver_id'),0)
    # amod_subscriptions=dict.fromkeys(('time', 'driver_id'),0)
    # shared_assignments=dict.fromkeys(('time', 'driver_id', 'num_rides'),0)
    # amod_assignments=dict.fromkeys(('time', 'driver_id', 'num_rides'),0)
    # pickups=dict.fromkeys(('time', 'driver_id', 'user_id','origin', 'destination'),0)
    # dropoffs=dict.fromkeys(('time', 'driver_id', 'user_id','origin', 'destination'),0)
    # parking=dict.fromkeys(('time', 'driver_id', 'parking_node'))
    
    '''
    SERVICE_CONTROLLER_UNKNOWN = 0,
    SERVICE_CONTROLLER_ON_HAIL = 1,
    SERVICE_CONTROLLER_GREEDY = 2,
    SERVICE_CONTROLLER_SHARED = 3,
    SERVICE_CONTROLLER_FRAZZOLI = 4,
    SERVICE_CONTROLLER_INCREMENTAL = 5,
    SERVICE_CONTROLLER_PROXIMITY = 6,
    SERVICE_CONTROLLER_AMOD = 7
    '''

    # read file for choice model
    with open(logfile, "r") as fi:
            controllerLogLines = fi.readlines()
    counter = 0
    fleet_size = {}
    for controller_id in  controller:
        temp_subscriptions=[]
        temp_requests=[]
        temp_assignments=[]
        pickups=[]
        dropoffs=[]
        parking=[]
        prev_line_controller_spec = False
        for line in controllerLogLines:
            if line.startswith('Subscription received by the controller of type SERVICE_CONTROLLER_'+controller_id):
                if 'time frame' in line:
                    # For older versions where controller.log is printing time frame instead of time                
                    temp_subscriptions.append({
                        'time':float(line.split('at time frame ')[1].split(',')[0]),
                        'driver_id':line.split('Subscription from driver ')[1].split(' at time')[0]
                        })
                else:
                    temp_subscriptions.append({
                        'time':float(line.split('at time ')[1].split('.')[0]),
                        'driver_id':line.split('Subscription from driver ')[1].split(' at time')[0]
                        })
            elif line.startswith('Request sent to controller of type SERVICE_CONTROLLER_' +controller_id):
                temp_requests.append({
                    'time':float(line.split('at frame ')[1].split(' ')[1].split('s')[0]),
                    'user_id':line.split('request issued by ')[1].split(' person')[0], 
                    'origin':float(line.split('from node ')[1].split(', to node ')[0]),
                    'destination':float(line.split('to node ')[1].split('.')[0])
                    })
            elif line.startswith('SERVICE_CONTROLLER_'+controller_id+' controller sent this assignment'):
                pattern = re.compile(r'\b{}\b'.format('ScheduleItem PICKUP'), re.I)
                num_amod_rides=len(pattern.findall(line))
                #print(float(line.split('is sent at frame ')[1].split(', ')[1].split('sto')[0]))
                #print(line.split('to driver ')[1].split(':')[1].split('(')[0])
                temp_assignments.append({
                    'time':float(line.split('is sent at frame ')[1].split(', ')[1].split('sto')[0]),
                    'driver_id':line.split('to driver ')[1].split(':')[1].split('(')[0],#line.split('. ')[1].split('to driver ')[1].split('\n')[0],
                    'num_rides':int(num_amod_rides)
                    })
            elif line.startswith('Pickup succeeded for'):
                pickups.append({
                    'time':float(line.split('frame ')[1].split(', ')[1].split('s with')[0]),
                    'driver_id':line.split('driverId ')[1].split('\n')[0], 
                    'user_id':line.split('Pickup succeeded for ')[1].split(' ')[0],
                    'origin':line.split('startNodeId ')[1].split(',')[0], 
                    'destination':line.split('destinationNodeId ')[1].split(',')[0]
                    })

            elif line.startswith('Drop-off of user '):
                dropoffs.append({
                    'time':float(line.split('frame ')[1].split(', ')[1].split('s')[0]),
                    'driver_id':line.split('driverId ')[1].split('\n')[0], 
                    'user_id':line.split('Drop-off of user ')[1].split(' ')[0],
                    'destination':line.split('destinationNodeId ')[1].split('and')[0]
                    })

            elif 'Begin driving to park,' in line:
                parking.append({
                    'time':float(line.split(': ')[0].split('ms')[0]),
                    'driver_id':line.split('OnCallDriver ')[1].split(': ')[0],
                    'parking_node':line.split('to parking node ')[1].split('\n')[0]
                    })
            elif line.startswith('For Controller type: SERVICE_CONTROLLER_'+controller_id):
                prev_line_controller_spec = True
            elif prev_line_controller_spec and line.startswith('Max. fleet size configured:'):
                fleet_size[controller_id] = (line.split(": ")[1].split("\n")[0])
                prev_line_controller_spec = False

        temp_requests = pd.DataFrame(temp_requests)
        if temp_requests.empty==False:
            temp_requests=temp_requests[['time','user_id','origin','destination']]

        temp_subscriptions = pd.DataFrame(temp_subscriptions)
        if temp_subscriptions.empty==False:
            temp_subscriptions=temp_subscriptions[['time','driver_id']]

        temp_assignments = pd.DataFrame(temp_assignments)
        if temp_assignments.empty==False:
            temp_assignments=temp_assignments[['time','driver_id']]
        #print assignments

        pickups = pd.DataFrame(pickups)
        pickups=pickups[['time','driver_id','user_id','origin', 'destination']]

        dropoffs = pd.DataFrame(dropoffs)
        dropoffs=dropoffs[['time','driver_id','user_id','destination']]

        if park:
            parking = pd.DataFrame(parking)
            parking = parking[['time', 'driver_id', 'parking_node']]
            parking.time = parking.time*.001
            parking.time = parking.time + start_time
            parking.time = pd.to_datetime(parking.time, unit='s')
            parking.time = pd.DatetimeIndex(parking.time)
            parking.set_index('time',inplace=True)
            parking['num_parking'] = 1
            num_parking = parking.resample('5T', how='sum')['num_parking']
            #print len(requests), len(subscriptions), len(assignments), len(pickups), len(dropoffs), len(parking)
        else:
            pass
            #print len(requests), len(subscriptions), len(assignments), len(pickups), len(dropoffs)

        dropoffs.time = dropoffs.time + start_time
        dropoffs.time = pd.to_datetime(dropoffs.time, unit='s')
        dropoffs.time = pd.DatetimeIndex(dropoffs.time)
        dropoffs.set_index('time',inplace=True)
        dropoffs['num_dropoffs'] = 1
        num_dropoffs = dropoffs.resample('5T', how='sum')['num_dropoffs']


        pickups.time = pickups.time + start_time
        pickups.time = pd.to_datetime(pickups.time, unit='s')
        pickups.time = pd.DatetimeIndex(pickups.time)
        pickups.set_index('time',inplace=True)
        pickups['num_pickups'] = 1
        num_pickups = pickups.resample('5T', how='sum')['num_pickups']


        if temp_subscriptions.empty:
            pass
        else:
            temp_subscriptions.time = temp_subscriptions.time + start_time
            temp_subscriptions.time = pd.to_datetime(temp_subscriptions.time, unit='s')
            temp_subscriptions.time = pd.DatetimeIndex(temp_subscriptions.time)
            temp_subscriptions.set_index('time',inplace=True)
            temp_subscriptions['num_'+controller_id+'_subscriptions'] = 1
            num_temp_subscriptions = temp_subscriptions.resample('5T', how='sum')['num_'+controller_id+'_subscriptions']

        if temp_requests.empty:
            pass
        else:
            temp_requests.time = temp_requests.time + start_time
            temp_requests.time = pd.to_datetime(temp_requests.time, unit='s')
            temp_requests.time = pd.DatetimeIndex(temp_requests.time)
            temp_requests.set_index('time',inplace=True)
            temp_requests['num_'+controller_id+'_requests'] = 1
            num_temp_requests = temp_requests.resample('5T', how='sum')['num_'+controller_id+'_requests']

        if temp_assignments.empty:
            pass
        else:
            temp_assignments['time'] = temp_assignments['time'].astype(float)
            temp_assignments.time = temp_assignments.time + start_time
            temp_assignments['time'] = pd.to_datetime(temp_assignments['time'], unit='s')
            temp_assignments['time'] = pd.DatetimeIndex(temp_assignments['time'])
            temp_assignments.set_index('time',inplace=True)
            temp_assignments['num_'+controller_id+'_assignments'] = 1
            num_temp_assignments = temp_assignments.resample('5T', how='sum')['num_'+controller_id+'_assignments']
            #num_amod_rides = amod_assignments.resample('5T', how='sum')['num_rides']


        if park:
            stats = pd.concat([#num_shared_subscriptions, num_amod_subscriptions,
                        num_shared_requests, num_amod_requests,
                        num_shared_assignments, #num_shared_rides, 
                        num_amod_assignments, #num_amod_rides,
                        num_pickups, num_dropoffs, num_parking], axis=1,join='outer')
        else:
            stats = pd.concat([#num_shared_subscriptions, 
                        #num_temp_subscriptions,
                        #num_shared_requests, 
                        num_temp_requests,
                        #num_shared_assignments, #num_shared_rides, 
                       # num_temp_assignments, #num_amod_rides,
                        num_pickups, num_dropoffs], axis=1,join='outer')

        stats.index  = [ pd.to_datetime(i).strftime('%H:%M') for i in stats.index.values ]
        stats.columns = ['Requests', 'Pickups', 'Dropoffs']
        #stats.index = pd.DatetimeIndex(stats.index).time
        #stats.index = pd.Series([val.time() for val in stats.index])
    return { "data": stats, "fleet_size" : fleet_size } 

#st0dist = getStats('controller_basecase.log',park=False)
res = getStats(log_path,park=False)
st = res["data"]
st.to_csv("request_pickup_dropoff.csv")

st.plot()
plt.title(" | ".join(k + " Fleet Size: " + v for k,v in res["fleet_size"].items()))
plt.xlabel('Time')
plt.ylabel('Counts')
plt.savefig("request_pickup_dropoff.png")
