"""
Script to update the waiting times in the zone to zone learned am/pm/op cost tables
Usage: python2.7 WaitingTimeAggregator.py traveltime.csv 0.5
Important: The cost tables have similar names, hence a single variable called COSTS_TABLE_FORMAT
specifies the name of these. For example:
COSTS_TABLE_FORMAT = ['demand.learned_', 'am/pm/op', 'costs_modified_for_waiting_time']
        implies the three table names are:
            demand.learned_amcosts_modified_for_waiting_time
            demand.learned_pmcosts_modified_for_waiting_time
            demand.learned_opcosts_modified_for_waiting_time



More details on the usage in the constants section 

 Author: Nishant Kumar
 Date: 06.05.2019
 

"""
import sys
import argparse
import datetime
import csv
import psycopg2
import time
import numpy


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ constants ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
DB_HOST = '127.0.0.1'
DB_PORT = '5432'
DB_USER = 'postgres'
DB_PASSWORD = 'postgres'
DB_NAME = 'simmobcity'

ZONE_TABLE = 'demand.taz_2012'

# the list COSTS_TABLE_FORMAT is a quick fix solution to specify a format of the names for the zone to zone tables
COSTS_TABLE_FORMAT = ['demand.learned_', 'am/pm/op', 'costs_modified_for_waiting_time']


TRAVEL_TIME_TABLE_NAME = 'output.traveltime'   
NODE_ZONE_MAP_TABLE = 'demand.node_taz_map'
listOfAllowedModesForWaitingTime = ['WAIT_SMS','WAIT_SMS_Pool']


# update rule:  (1-alpha)* <old_TT> + (alpha)*<new_TT> )
ALPHA = float(sys.argv[2])
print ('ALPHA VALUE: ', ALPHA) 


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ helper functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
#identify whether time is in AM peak period
#AM Peak : 7:30:00 - 9:29:00 AM
def isAM(time):
    splitTime = time.split(":")
    hour = int(splitTime[0])
    if hour==7:
        minute = int(splitTime[1])
        if minute >= 30: return True
        else: return False
    elif hour==8: return True
    elif hour==9:
        minute = int(splitTime[1])
        if minute < 30: return True
        else: return False

#identify whether time is in PM peak period
#PM Peak : 17:30:00 - 19:29:00 PM
def isPM(time):
    splitTime = time.split(":")
    hour = int(splitTime[0])
    if hour==17:
        minute = int(splitTime[1])
        if minute >= 30: return True
        else: return False
    elif hour==18: return True
    elif hour==19:
        minute = int(splitTime[1])
        if minute < 30: return True
        else: return False



def get_AM_PM_OP(startTime):
    if isAM(startTime):
        return 'AM'
    elif isPM(startTime):
        return 'PM'
    else:
        return 'OP'

#convert value to float
def toFloat(value):
    if str(value) == "NULL":
        return 0
    return float(value)

#find idx of 30 minute window from time
def getWindowIdx(time):
    res = 0
    splitTime = time.split(":")
    hour = int(splitTime[0])
    if hour > 24 or hour < 0: raise RuntimeError('invalid time - ' + time)
    if hour < 3: hour = hour + 24 #0, 1 and 2 AM are 24, 25 and 26 hours respectively
    minute = int(splitTime[1])
    if minute > 59 or minute < 0: raise RuntimeError('invalid time - ' + time)
    totalMins = ((hour*60) + minute) - 180 #day starts at 3:00 AM
    return (totalMins/30)+1

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~WT_Aggregator class~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
class WT_Aggregator:
    def __init__(self, csv_name):
        connection_string = "dbname='" + DB_NAME + "' user='" + DB_USER + "' host='" + DB_HOST + "' port='" + DB_PORT + "' password='" + DB_PASSWORD + "'"
        conn = psycopg2.connect(connection_string)
        cur = conn.cursor()

        # fetch zones
        fetch_zones_query = "SELECT zone_id, zone_code FROM " + ZONE_TABLE
        cur.execute(fetch_zones_query)

        self.NUM_ZONES = cur.rowcount #meant to be constant
        self.zoneId = {} # dictionary of <zone_code> : <zone_id>
        self.zoneCode = {} #dictionary of <zone_id> : <zone_code>
        rows = cur.fetchall()

        for row in rows:
            zone_id = int(row[0])
            zone_code = int(row[1])
            self.zoneId[zone_code] = zone_id
            self.zoneCode[zone_id] = zone_code
            
            
            
       # fetch node-zone map
        fetch_nodezone_map_query = "SELECT node_id, taz FROM " + NODE_ZONE_MAP_TABLE
        cur.execute(fetch_nodezone_map_query)
        self.nodeZoneMap = {}
        rows = cur.fetchall()
        for row in rows:
            node_id = int(row[0])
            zone_code = int(row[1])
            self.nodeZoneMap[node_id] = zone_code
            

        # truncate existing travel time  table
        truncate_script = "DROP TABLE IF EXISTS " + TRAVEL_TIME_TABLE_NAME
        cur.execute(truncate_script)
        print ("Old travel time table dropped")
            
            
            
        # create table output.traveltime
        create_table_query = "CREATE TABLE " + TRAVEL_TIME_TABLE_NAME + "( \
        person_id character varying,                \
        trip_origin_id integer,                     \
        trip_dest_id integer,                       \
        subtrip_origin_id character varying,                  \
        subtrip_dest_id character varying,          \
        subtrip_origin_type character varying,      \
        subtrip_dest_type character varying,        \
        travel_mode character varying,              \
        arrival_time character varying,             \
        travel_time double precision  )"             
        # subtrip_dest_id and subtrip_origin_id is character varying because for Public Transit it can take alphanumeric values
        cur.execute(create_table_query)
        print ("New travel time table created")
        

        # creating an intermediate tables to retain the first 10 columns in the travel time csv 
        with open('intermediateFile','w') as fw:
            csvwriter = csv.writer(fw)
            with open(inputFile,'r') as fr:
                for row in fr:
                    csvwriter.writerow(row.strip().split(',')[0:10])  # using comma as the delimiter since the travel time csv file is comma separated
                    

        # copy csv data into table
        csvFile = open('intermediateFile', 'r')
        cur.copy_from(csvFile, TRAVEL_TIME_TABLE_NAME, ',')
        csvFile.close()


       
        # load aggregate records from copied table 
        fetch_agg_tt_query = "SELECT trip_origin_id, trip_dest_id, travel_mode as mode, arrival_time as end_time, travel_time FROM " + TRAVEL_TIME_TABLE_NAME 
        cur.execute(fetch_agg_tt_query)
        self.inputCsv = cur.fetchall()

        # close the cursor
        cur.close()

        ##2-dimensional list structures for each OD zone pair
        # to hold total wating time for the particular modes/ timeOfDay (AM/PM/OP)       
        
        # create dictionaries; such that the key will denote the name mode
        self.WTT = {}
        self.WTT_count = {}

        
        for mode in listOfAllowedModesForWaitingTime:
            for am_pm_op in ['AM', 'PM', 'OP']:
                if am_pm_op not in self.WTT:
                    self.WTT[am_pm_op] = {}
                self.WTT[am_pm_op][mode] = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
            
                #to hold number of people who waited for this mode
                if am_pm_op not in self.WTT_count:
                    self.WTT_count[am_pm_op] = {}
                self.WTT_count[am_pm_op][mode] = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
                    
    ##functions to add items into the data structures defined above
    def add_WTT(self, origin, destination, value, mode, am_pm_op):
        orgZid = self.zoneId[origin] - 1
        desZid = self.zoneId[destination] - 1
        self.WTT[am_pm_op][mode][orgZid][desZid] += value
        self.WTT_count[am_pm_op][mode][orgZid][desZid] += 1
        return

    
    # process input csv
    def processInput(self):
        # process each row of input csv
        for row in self.inputCsv:
            #origin_taz, destination_taz, mode, start_time, end_time, travel_time
            orgZ = self.nodeZoneMap[int(row[0])]
            desZ = self.nodeZoneMap[int(row[1])]
            mode = str(row[2]).strip()
            tripWaitingTime = float(row[4])
            tripStartTime = str(row[3])
            am_pm_op = get_AM_PM_OP(tripStartTime)    
                
            if mode in listOfAllowedModesForWaitingTime:
                self.add_WTT(orgZ, desZ, tripWaitingTime, mode, am_pm_op )
            else:
                # print ('ignoring record with mode ' + mode)  #  warning message to keep track of rogue modes
                doNothing = 1
        return

                
    def computeMeans(self):
        for i in range(self.NUM_ZONES):
            for j in range(self.NUM_ZONES):
                for mode in listOfAllowedModesForWaitingTime:
                    for am_pm_op in ['AM', 'PM', 'OP']:
                        if self.WTT[am_pm_op][mode][i][j] > 0 :  # implying if there was an output in the latest simulation corresponding to this time <window-mode-OD> combination
                            self.WTT[am_pm_op][mode][i][j] = (float(self.WTT[am_pm_op][mode][i][j]/self.WTT_count[am_pm_op][mode][i][j]))
                        else:
                            self.WTT[am_pm_op][mode][i][j] = 0 
        return
                

    def updatePostgres(self):
        connection_string = "dbname='" + DB_NAME + "' user='" + DB_USER + "' host='" + DB_HOST + "' port='" + DB_PORT + "' password='" + DB_PASSWORD + "'"
        conn = psycopg2.connect(connection_string)
        cur = conn.cursor()

        listOfModesFor_SQL_Query = ''
        for mode in listOfAllowedModesForWaitingTime:
            listOfModesFor_SQL_Query +=  (mode + ', ') 
        listOfModesFor_SQL_Query = listOfModesFor_SQL_Query[0:-2] # removing the last comma and trailing space

        
        
        self.oldValsWTT = {}
        for am_pm_op in ['AM','PM','OP']:
            queryToRetrieveOlderValues = 'SELECT origin_zone, destination_zone,' + listOfModesFor_SQL_Query + ' FROM '+  ( COSTS_TABLE_FORMAT[0] + am_pm_op + COSTS_TABLE_FORMAT[2]) 
            cur.execute(queryToRetrieveOlderValues)
            self.querydump = cur.fetchall()
            if am_pm_op not in self.oldValsWTT:
                self.oldValsWTT[am_pm_op] = {}
            for mode in listOfAllowedModesForWaitingTime:
                self.oldValsWTT[am_pm_op][mode] = {}
                modeCount = 0 
                for row in self.querydump:
                    orgZ = int(row[0])
                    desZ = int(row[1])
                    # The order as specified in listOfModesFor_SQL_Query is preserved

                    self.oldValsWTT[am_pm_op][mode][(orgZ,desZ)] = float(row[2+modeCount])
                modeCount += 1

            

        prevWTTValues = {}
        currentWTTValues = {}
        for mode in listOfAllowedModesForWaitingTime:
            prevWTTValues[mode] = []
            currentWTTValues[mode] = []
                
        for am_pm_op in ['AM','PM','OP']:
            for i in range(self.NUM_ZONES):
                orgZ = self.zoneCode[i+1]
                for j in range(self.NUM_ZONES):
                    desZ = self.zoneCode[j+1]
                    if orgZ == desZ: continue

                    newWTT = {}
                    for mode in listOfAllowedModesForWaitingTime:
                        # The new value is divided by 3600 because
                        # the simulation output generates the waiting time in seconds
                        # whereas the learned_am/pm/op_cost tables store the waiting time in hours
                        newWTT[mode] = self.WTT[am_pm_op][mode][i][j]/3600.0

                    if (sum(newWTT.values())) > 0:   # no need to do anything if there is no update in the values
                        comma_required = False
                        update_param_tuple = ()
                        update_query = "UPDATE " + ( COSTS_TABLE_FORMAT[0] + am_pm_op + COSTS_TABLE_FORMAT[2])  + " SET "
                        for mode in listOfAllowedModesForWaitingTime:
                            if newWTT[mode] > 0:
                                if comma_required:   # adding comma to allow for updates of multiple columns in the update query
	    					    	update_query = update_query + ","

                                update_query = update_query +  mode + "=(%s * %s+ " + ( COSTS_TABLE_FORMAT[0] + am_pm_op + COSTS_TABLE_FORMAT[2])  + "."+ mode + " * %s)"
                                update_param_tuple = update_param_tuple + (newWTT[mode], ALPHA, 1- ALPHA)
                                prevWTTValues[mode].append(self.oldValsWTT[am_pm_op][mode][(orgZ,desZ)])
                                currentWTTValues[mode].append(newWTT[mode])
                                comma_required = True
                        update_query = update_query + " WHERE origin_zone=%s and destination_zone=%s"
                        update_param_tuple = update_param_tuple + (orgZ, desZ)
                        print (update_query)
                        cur.execute(update_query, update_param_tuple)
                        conn.commit()

          
                
        # calculating the RMSN
        # taking into only those values which are non zeros both in observed and simulated
        for mode in listOfAllowedModesForWaitingTime:
            prevValues = numpy.array(prevWTTValues[mode])
            currentValues = numpy.array(currentWTTValues[mode])
            rmsn = {}
            rmsn[mode] = numpy.sqrt(numpy.mean(numpy.square(prevValues - currentValues))) / (numpy.mean(prevValues))
            prevValues = list(prevValues)
            currentValues = list(currentValues)
            with open(mode + '_WTT.csv','a') as f:
                csvwriter = csv.writer(f)
                csvwriter.writerow(['Old Values','New Values'])
                for i in range(len(prevValues)):
                    csvwriter.writerow([prevValues[i], currentValues[i]])

            with open('RMSN_records_zone_to_zone_WTT.txt','a') as f:
                f.write("RMSN value for differences in zone to zone " + mode + " waiting time "+ str(rmsn[mode]) + "\n")

            print ("\nRMSN value for differences in zone to zone waiting time for Mode<" + mode + ">: " , rmsn[mode])
        return


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
s = datetime.datetime.now()

#1.
print ("1. initializing and loading CSV")
inputFile = sys.argv[1]
aggregator = WT_Aggregator(inputFile)
#2.
print ("2. processing input")
aggregator.processInput()
#3.
print ("3. computing means")
aggregator.computeMeans()

#4.
print ("4. updating data in postgres db")
aggregator.updatePostgres()

print ("Done. Exiting Main")

e = datetime.datetime.now()
print ('Running Time: ' + str((e-s).total_seconds()) + 's')

