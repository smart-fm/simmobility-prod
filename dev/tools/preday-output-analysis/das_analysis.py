#A script to plot DAS and do consistency checks
# Author: Arun Prakash Akkinepally

#Inputs parameters : 
#        inputFolder : folder path with the activity schedule file
#        dasFilename : A csv file containing the activity_schedule 
#        outFolder: A folder path to store the generated plots


#Outputs:
#        Consistency checks see functions checkTrips() and checkTour()
#        Plots and graphs

#-------------------------------------------------------------------------------------------------

from plotsDas import * 
import os
import sys 


###############   Harcoded Values ##################


###############  Input section #####################
inputFolder = '/home/harish/simmobility/code_to_be_merged/dev/Basic'
dasFile = 'activity_schedule'
print dasFile


dasFile = os.path.join(inputFolder,dasFile)
# sorting the das file with personid, tour_number and trip_number
resultOfSysCall = os.system('(head -n1 ' + dasFile + ' && tail -n+2 ' + dasFile + ' | sort -t"," -k1,1 -n -k2,2 -k4,4 ) > ' + os.path.join(inputFolder,'sortedfile.csv'))
if resultOfSysCall != 0 :
    print ("Sorting of the activity_schedule could not be done! \n Stopping execution")
    sys.exit(0)

dasFile = 'sortedfile.csv'
dasFile = os.path.join(inputFolder,dasFile)


import csv
with open(dasFile, 'rb') as csvfile:
    checkHeader = csv.Sniffer()
    has_header = checkHeader.has_header(csvfile.read(5000))
    csvfile.seek(0)

    if not has_header: 
        os.system("echo 'person_id,tour_no,tour_type,stop_no,stop_type,stop_location,stop_zone,stop_mode,primary_stop,arrival_time,departure_time,prev_stop_location,prev_stop_zone,prev_stop_departure_time,pid ' >  temp_file.csv ")
    os.system("cat " + dasFile + " >> temp_file.csv")
    os.system("mv temp_file.csv " +dasFile)

#--------------------------------------------------------------------------------------------------



def plotAndWrite(tripModeCount,tripStopCount,tripTodActiCount,tripTodModeCount,tripDistCount,tourModeCount,tourActivityCount,tourDistCount,totalNum):
    '''
    '''
    
    plotModeActivityShares(tripModeCount,tripStopCount,tripTodActiCount,tripTodModeCount,tripDistCount,tourModeCount,tourActivityCount,tourDistCount,totalNum)

#-----------------------------------------------------------------------------------------------------

def checkTrips(person_das):
    '''
        checkTrips : Checks the following 
                    1. if trips are in increasing order of time.
                    2. if the trips are continuous in location
    '''
    
    departArriveTimes = []
    stopLocationNodes = []
    stopLocationZones = []
    for atrip in person_das:
        departTime, arrTime = float(atrip['prev_stop_departure_time']), float(atrip['arrival_time'])
        stop, prevStop = atrip['stop_location'], atrip['prev_stop_location']
        stopZone, PrevStopZone = atrip['stop_zone'], atrip['prev_stop_zone']
        departArriveTimes.extend([departTime,arrTime])
        stopLocationNodes.extend([prevStop,stop])
        stopLocationZones.extend([PrevStopZone,stopZone ])
    
    # Checking cronological order in time
    if sorted(departArriveTimes) != departArriveTimes:
        print "The das of person %s is not cronological"%(person_das[0]['person_id'])
    
   # Checking continuity in space in nodes and zones
    stopLocationNodes = stopLocationNodes[1:-1]
    if stopLocationNodes[0::2] != stopLocationNodes[1::2]:
       print "The das of person %s is not continuous in nodes"%(person_das[0]['person_id'])
       print "stopNodes :",stopLocationNodes

    stopLocationZones = stopLocationZones[1:-1]
    if stopLocationZones[0::2] != stopLocationZones[1::2]:
       print "The das of person %s is not continuous in Zones"%(person_das[0]['person_id'])
       print "Zones :",stopLocationZones

#--------------------------------------------------------------------------------------------------


def checkTour(person_tour):
    '''
    '''
    modeTourList = []
    for atrip in person_tour:
        modeTourList.append(atrip['stop_mode'])
    
    # This check assumes that there are no subtrips. The idea is that if person has a Car trip, then he needs to have car with him
    # through out the tour
    if "Car" in modeTourList:
        setModes = set(modeTourList)
        if len(setModes) > 1:
            print "Person %s and tour %s has inconsistent modes"%(person_tour[0]['person_id'], person_tour[0]['tour_no'])
            print modeTourList
            # pdb.set_trace()
            
                

#--------------------------------------------------------------------------------------------------

def processPersonDas(person_das,tripModeCount,tripStopCount,tripTodActiCount,tripTodModeCount,tripDistCount,tourModeCount,tourActivityCount,tourDistCount,totalNum):
    '''
        processPersonDas(): Processes individual person's das to coherently get measures
                            The data is assumed to be sorted by person_id
    '''
    
    numTrips = len(person_das)
    # processing each trip
    for atrip in person_das:
        stop_mode,stop_activity,tour_activity,tod = atrip['stop_mode'],atrip['stop_type'],atrip['tour_type'],atrip['prev_stop_departure_time']
        tripModeCount[stop_mode] += 1
        tripStopCount[stop_activity] += 1
        tripTodActiCount['total'][tod] += 1
        tripTodActiCount[tour_activity][tod] += 1
        tripTodModeCount[stop_mode][tod] += 1
    
    if numTrips in tripDistCount:
        tripDistCount[numTrips] += 1
    else:
        tripDistCount[numTrips] = 1
    
    
    checkTrips(person_das)
    
    # processing each tour at once
    numTours = max([int(atrip['tour_no']) for atrip in person_das]) 
    for tour_no in xrange(1,numTours+1):
        person_tour = [item for item in person_das if int(item['tour_no'])==tour_no]
        checkTour(person_tour)
        tour_mode, tour_activity = person_tour[0]['stop_mode'], person_tour[0]['tour_type']
        tourModeCount[tour_mode] += 1
        tourActivityCount[tour_activity] += 1

    if numTours in tourDistCount:
        tourDistCount[numTours] += 1
    else:
        tourDistCount[numTours] = 0
    
    totalNum['travelpersons'] += 1
    totalNum['trips'] += len(person_das)
    totalNum['tours'] += numTours
    
#--------------------------------------------------------------------------------------------------

def processDas(dasFile):
    '''
        processDas(): Reads the complete DAS sequentially and extracts statistics
    '''
    
    # Initializing all the dictionaries to store aggregatre numbers
    tripModeCount = {mode:0 for mode in modeList}
    tripStopCount = {stop:0 for stop in stopList}
    tripTodActiCount = {act:{tod:0 for tod in timeOfDayList} for act in activityList+['total']}
    tripTodModeCount = {mode:{tod:0 for tod in timeOfDayList} for mode in modeList}
    tripDistCount = {}
    tourModeCount = {mode:0 for mode in modeList}
    tourActivityCount = {act:0 for act in activityList}    
    tourDistCount = {}
    
    totalNum = {'trips':0,'tours':0,'travelpersons':0,'persons':totalPopulation}
    
    # Opening the sorted das and procession each person activity schedule iteratively
    with open(dasFile,'r') as ifile:
        reader = csv.DictReader(ifile)
        person_das = []
        person_id = None
        
        for atrip in reader:
            if person_id != None and person_id != atrip['person_id']:
                # as we colected 1 person activity-schedule we process it
                processPersonDas(person_das,tripModeCount,tripStopCount,tripTodActiCount,tripTodModeCount,tripDistCount,tourModeCount,tourActivityCount,tourDistCount,totalNum)
                person_id,person_das = atrip['person_id'],[atrip]
                #pdb.set_trace()
            else:
                person_das.append(atrip)
                person_id = atrip['person_id']
    
    # pdb.set_trace()
    plotAndWrite(tripModeCount,tripStopCount,tripTodActiCount,tripTodModeCount,tripDistCount,tourModeCount,tourActivityCount,tourDistCount,totalNum)

#--------------------------------------------------------------------------------------------------



processDas(dasFile)




    
