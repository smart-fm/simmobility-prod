import csv
import time
from xlrd import open_workbook
import collections
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from tabulate import tabulate

DAS = 'activity_schedule.csv'
#DAS = 'DAS_Independent.csv'
#DAS = 'das_foreigners_HITS.csv'
#DAS = 'activity_schedule_LT_pop_base_lua.csv'

# person_id, tour_no, tour_type, stop_no, stop_type, stop_location, stop_zone, stop_mode, primary_stop, arrival_time, departure_time, prev_stop_location, prev_stop_zone, prev_stop_dt

Tour_Purposes = ['Work','Education','Shop','Other','WorkbasedSubTour']
Modes = ['Car','Car Sharing 2','Car Sharing 3','Motorcycle','BusTravel','MRT','PrivateBus','Taxi','Walk']

def InitDict_TBM(trips_by_mode):
    for tour_type in Tour_Purposes:
        trips_by_mode[tour_type] = {}
        for mode in Modes:
           trips_by_mode[tour_type][mode] = 0.0
        trips_by_mode['Overall'] = {}
        for mode in Modes:
           trips_by_mode['Overall'][mode] = 0.0
        
        
def InitDict_TBTOD(trips_by_TOD,trips_by_TOD_Type,trips_by_TOD_Mode):
    time_intervals = np.arange(3.25,27.25,0.5)
    for item in time_intervals:
        trips_by_TOD[item] = 0.0
        for tour in Tour_Purposes:
            trips_by_TOD_Type[(tour,item)] = 0.0
        for mode in Modes:
            trips_by_TOD_Mode[(mode,item)] = 0.0
        

def set_stop_characteristics(activity_schedule,row):
    activity_schedule[row['person_id']][int(row['tour_no'])][int(row['stop_no'])]['tour_type'] = row['tour_type']
    activity_schedule[row['person_id']][int(row['tour_no'])][int(row['stop_no'])]['stop_type'] = row['stop_type']
    activity_schedule[row['person_id']][int(row['tour_no'])][int(row['stop_no'])]['stop_location'] = row['stop_type']
    activity_schedule[row['person_id']][int(row['tour_no'])][int(row['stop_no'])]['stop_zone'] = int(row['stop_zone'])
    activity_schedule[row['person_id']][int(row['tour_no'])][int(row['stop_no'])]['stop_mode'] = row['stop_mode']
    activity_schedule[row['person_id']][int(row['tour_no'])][int(row['stop_no'])]['primary_stop'] = row['primary_stop']
    activity_schedule[row['person_id']][int(row['tour_no'])][int(row['stop_no'])]['arrival_time'] = float(row['arrival_time'])
    activity_schedule[row['person_id']][int(row['tour_no'])][int(row['stop_no'])]['departure_time'] = float(row['departure_time'])
    activity_schedule[row['person_id']][int(row['tour_no'])][int(row['stop_no'])]['prev_stop_location'] = int(row['prev_stop_location'])
    activity_schedule[row['person_id']][int(row['tour_no'])][int(row['stop_no'])]['prev_stop_zone'] = int(row['prev_stop_zone'])
    activity_schedule[row['person_id']][int(row['tour_no'])][int(row['stop_no'])]['prev_stop_dt'] = float(row['prev_stop_dt'])   

def Read_DAS_LIST(list_DAS):
    #das_reader = csv.DictReader(open(DAS))
    print('============================================== ')
    print (' Reading DAS ..................... ')
    start=time.time()
    for row in das_reader:
        list_DAS.append([row['person_id'],row['stop_mode'],row['prev_stop_dt'],row['stop_type'],row['tour_type']])
    end = time.time()
    print (' Done ..................... ')
    print ('Computational Time (seconds): ',(end-start))
    print('============================================================================== ')
    print ("Total Number of Trips: ",len(list_DAS))
    

def Read_DAS_DICT(activity_schedule):
    print('============================================================================== ')
    print (' Reading DAS ..................... ')
    start=time.time()
    count = 0 
    with open(DAS) as f:
        next(f)
        for row in f:
            listed = row.strip().replace('[','').replace(']','').split('|')       # Changed by Nishant 08112017- because the das file had '|' instead of ', as the separators'
            activity_schedule[(listed[0],int(listed[1]),int(listed[3]))] = [float(listed[13]), listed[7], listed[4],listed[2]]
            count += 1
            if (count % 1000000 == 0):
                print(count)
    end = time.time()
    print (' Done ..................... ')
    print (" Computational Time (seconds): ", (end-start))
    print('============================================================================== ')
    print("Total Number of Records: ",len(activity_schedule))

def Plot_Trips_TOD(trips_by_TOD,trips_by_TOD_Type,trips_by_TOD_Mode):
    intervals = []
    tripsTOD = []
    tripsTODType = {}
    tripsTODMode = {}
    for tour in Tour_Purposes:
        tripsTODType[tour] = []
    for mode in Modes:
        tripsTODMode[mode] = []
    for interval in trips_by_TOD:
        intervals.append(interval)
    intervals = np.array(intervals)
    intervals.sort()
    for interval in intervals:
        tripsTOD.append(trips_by_TOD[interval])
        for tour in Tour_Purposes:
            tripsTODType[tour].append(trips_by_TOD_Type[(tour,interval)])
        for mode in Modes:
            tripsTODMode[mode].append(trips_by_TOD_Mode[(mode,interval)])
                
    fig = []
    plot_index = 0
    fig.append(plt.figure(plot_index+1))
    plt.plot(intervals, tripsTOD, color='blue')
    plt.title('Trips By Time of Day')
    plt.ylabel(' Number of Trips ') ;
    plt.xlabel('Time Interval') ;
    plt.grid(True)
    plt.savefig('Trips_by_TOD.png', format='png', dpi=1000)
    for tour in Tour_Purposes:
        plot_index += 1
        fig.append(plt.figure(plot_index+1))
        plt.plot(intervals, tripsTODType[tour], color='blue')
        plt.title('Trips By Time of Day - ' + tour)
        plt.ylabel(' Number of Trips ') ;
        plt.xlabel('Time Interval') ;
        plt.grid(True)
        plt.savefig('Trips_by_TOD_'+ str(tour)+'.png', format='png', dpi=1000)

    for mode in Modes:
        plot_index += 1
        fig.append(plt.figure(plot_index+1))
        plt.plot(intervals, tripsTODMode[mode], color='blue')
        plt.title('Trips By Time of Day - ' + mode)
        plt.ylabel(' Number of Trips ') ;
        plt.xlabel('Time Interval') ;
        plt.grid(True)
        plt.savefig('Trips_by_TOD_'+ str(mode)+'.png', format='png', dpi=1000)



    plt.show() ;
    
        

def Compute_Mode_Shares(activity_schedule):
    trips_by_mode = {}
    trips_by_TOD = {}
    trips_by_TOD_Type = {}
    trips_by_TOD_Mode = {}
    InitDict_TBM(trips_by_mode)
    InitDict_TBTOD(trips_by_TOD,trips_by_TOD_Type,trips_by_TOD_Mode) 
    total_trips = {};
    total_trips['Overall'] = 0.0
    for tour_type in Tour_Purposes:
        total_trips[tour_type] = 0.0
    for (person, tour, stop) in activity_schedule:
        trips_by_mode['Overall'][activity_schedule[(person, tour, stop)][1]] = trips_by_mode['Overall'][activity_schedule[(person, tour, stop)][1]] + 1.0
        trips_by_mode[activity_schedule[(person, tour, stop)][3]][activity_schedule[(person, tour, stop)][1]] = trips_by_mode[activity_schedule[(person, tour, stop)][3]][activity_schedule[(person, tour, stop)][1]] + 1.0
        
        trips_by_TOD[activity_schedule[(person, tour, stop)][0]] = trips_by_TOD[activity_schedule[(person, tour, stop)][0]] + 1.0
        trips_by_TOD_Type[(activity_schedule[(person, tour, stop)][3],activity_schedule[(person, tour, stop)][0])] += 1.0
        trips_by_TOD_Mode[(activity_schedule[(person, tour, stop)][1],activity_schedule[(person, tour, stop)][0])] += 1.0
        total_trips['Overall'] = total_trips['Overall'] + 1.0
        total_trips[activity_schedule[(person, tour, stop)][3]] = total_trips[activity_schedule[(person, tour, stop)][3]] + 1.0
        
    print("Total Number of Trips: ", total_trips)
    print('============================================================================== ')
    print(' Trips By Mode and % Share')
    #print(trips_by_mode)



    for tour_type in trips_by_mode:
        table = []
        for key in trips_by_mode[tour_type]:
            try: # Added by Nishant to remove the divide by zero error  as the das here has 0 Work based subtours
                table.append([key,trips_by_mode[tour_type][key],float(100*trips_by_mode[tour_type][key]/total_trips[tour_type])])
            except:
                print ('Got one divide by zero error;  key : ',tour_type, ' Value:  ', trips_by_mode[tour_type] )
        print(" Tour Type: ", tour_type)
        print(tabulate(table,floatfmt=".2f"))

    
    tours_person = {}
    for (person, tour, stop) in activity_schedule:
        tours_person[person] = []
    for (person, tour, stop) in activity_schedule:
        if tour not in tours_person[person]:
            tours_person[person].append(tour)
    num_tours = 0
    for person in tours_person:
        num_tours = num_tours + len(tours_person[person])
    print("Total Number of Tours: ", num_tours)
    #print(trips_by_TOD)
    Plot_Trips_TOD(trips_by_TOD,trips_by_TOD_Type,trips_by_TOD_Mode)


activity_schedule = {}   
#list_DAS = []
#Read_DAS_LIST(list_DAS)
Read_DAS_DICT(activity_schedule)
Compute_Mode_Shares(activity_schedule)
#print(list_DAS)
        

## =======================================================================================    
## DAS Dicitonary
##        if row['person_id'] in activity_schedule.keys():
##            if int(row['tour_no']) in activity_schedule[row['person_id']].keys():
##                activity_schedule[row['person_id']][int(row['tour_no'])][int(row['stop_no'])] = {}
##                set_stop_characteristics(activity_schedule,row) 
##            else:
##                activity_schedule[row['person_id']][int(row['tour_no'])] = {}
##                activity_schedule[row['person_id']][int(row['tour_no'])][int(row['stop_no'])] = {}
##                set_stop_characteristics(activity_schedule,row)    
##        else:
##            activity_schedule[row['person_id']] = {}
##            activity_schedule[row['person_id']][int(row['tour_no'])] = {}
##            activity_schedule[row['person_id']][int(row['tour_no'])][int(row['stop_no'])] = {}
##            set_stop_characteristics(activity_schedule,row)
## =======================================================================================    

