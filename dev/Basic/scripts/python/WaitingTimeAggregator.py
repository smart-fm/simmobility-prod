import argparse
import datetime
import csv
import psycopg2
import time
import numpy
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ constants ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
DB_HOST = '172.25.184.48'
DB_PORT = '5432'
DB_USER = 'postgres'
DB_PASSWORD = '5M_S1mM0bility'
DB_NAME = 'simmobcity'

ZONE_TABLE = 'demand.taz_2012'
AM_COSTS_TABLE = 'demand.learned_amcosts'
PM_COSTS_TABLE = 'demand.learned_pmcosts'
OP_COSTS_TABLE = 'demand.learned_opcosts'
TCOST_CAR_TABLE = 'demand.learned_tcost_car'
TCOST_BUS_TABLE = 'demand.learned_tcost_bus'
TRAVEL_TIME_TABLE_NAME = 'output.traveltime'
NODE_ZONE_MAP_TABLE = 'demand.nodeZoneMap'
listOfAllowedModesForWaitingTime = ['SMS']


# update rule:  (1-alpha)* <old_TT> + (alpha)*<new_TT> )
ALPHA = 0.25


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
        fetch_nodezone_map_query = "SELECT node_id, zone_code FROM " + NODE_ZONE_MAP_TABLE
        cur.execute(fetch_nodezone_map_query)

        self.nodeZoneMap = {}
        for row in rows:
            node_id = int(row[0])
            zone_code = int(row[1])
            self.nodeZoneMap[node_id] = zone_code

        # truncate existing travel time  table
        truncate_script = "truncate table " + TRAVEL_TIME_TABLE
        cur.execute(truncate_script)

        # copy csv data into table
        csvFile = open(str(args.csv_name), 'r')
        cur.copy_from(csvFile, TRAVEL_TIME_TABLE, ',')
        csvFile.close()

        # load aggregate records from copied table
        fetch_agg_tt_query = "SELECT origin_taz, destination_taz, min(mode) as mode, min(start_time) as start_time, max(end_time) as end_time, sum(travel_time) as travel_time FROM " + SUBTRIP_METRICS_TABLE + " GROUP BY person_id, trip_id, origin_taz, destination_taz"
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
            for sectionOfDay in ['AM', 'PM', 'OP']:
                self.WTT[sectionOfDay] = {}
                self.WTT[sectionOfDay][mode] = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
            
                #to hold number of people who waited for this mode
                self.WTT_count[sectionOfDay] = {}
                self.WTT_count[sectionOfDay][mode] = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
                   
                    
    ##functions to add items into the data structures defined above
    def add_WTT(self, origin, destination, value, mode, am_pm_op):
        orgZid = self.zoneId[origin] - 1
        desZid = self.zoneId[destination] - 1
        self.WTT[am_pm_op][mode][orgZid][desZid] += value
        return

    
    # process input csv
    def processInput(self):
        # process each row of input csv
        for row in self.inputCsv:
            #origin_taz, destination_taz, mode, start_time, end_time, travel_time
            orgZ = self.nodeZoneMap[int(row[0])]
            desZ = self.nodeZoneMap[int(row[1])]
            mode = str(row[2]).strip()
            tripWaitingTime = str(row[
            tripStartTime = str(row[3])
            am_pm_op = get_AM_PM_OP(tripStartTime)    
                
            if mode in listOfAllowedModesForWaitingTime:
                self.add_WTT(orgZ, desZ, tripWaitingTime, mode, am_pm_op )
            else:
                print ('ignoring record with mode ' + mode)  #  warning message to keep track of rogue modes
                doNothing = 1
        return

                
    def computeMeans(self):
        for i in range(self.NUM_ZONES):
            for j in range(self.NUM_ZONES):
                for mode in listOfAllowedModesForWaitingTime:
                    for sectionOfDay in ['AM', 'PM', 'OP']:
                        if self.amCarIvtCount[i][j] > 0 :
                            self.WTT[sectionOfDay][mode][i][j] = (float(self.WTT[sectionOfDay][mode][i][j]/self.WTT_count[sectionOfDay][mode][i][j]))
                        else:
                            self.WTT[sectionOfDay][mode][i][j] = 0 
        return
                

    def updatePostgres(self):
        connection_string = "dbname='" + DB_NAME + "' user='" + DB_USER + "' host='" + DB_HOST + "' port='" + DB_PORT + "' password='" + DB_PASSWORD + "'"
        conn = psycopg2.connect(connection_string)
        cur = conn.cursor()

        listOfModesFor_SQL_Query = ''
        for mode in listOfAllowedModesForWaitingTime:
            listOfModesFor_SQL_Query.append( 'wait_time_'+ mode + ', ')
                
        
        self.oldValsWTT = {}
        for am_pm_op in ['AM','PM','OP']:
            queryToRetrieveOlderValues = 'SELECT origin_zone,destination_zone,' + listOfModesFor_SQL_Query ' FROM ' + am_pm_op+ 'COSTS_TABLE'
            cur.execute(queryToRetrieveOlderValues)
            self.querydump = cur.fetchall()
            self.oldValsWTT[am_pm_op] = {}
            for mode in listOfAllowedModesForWaitingTime:
                self.oldValsWTT[am_pm_op][mode] = {}
            for row in self.querydump:
                orgZ = int(row[0])
                desZ = int(row[1])
                # The order as specified in listOfModesFor_SQL_Query is preserved
                count = 0 
                for mode in listOfAllowedModesForWaitingTime:
                    self.oldValsWTT[am_pm_op][mode][(orgZ,desZ)] = float(row[2+count])
                    count += 1

        

        prevWTTValues = {}
        currentWTTValues = {}
        for mode in listOfAllowedModesForWaitingTime:
            prevWTTValues[mode] = []
            currentWTTValuesp[mode] = []
                
        for am_pm_op in ['AM','PM','OP']:
            for i in range(self.NUM_ZONES):
                orgZ = self.zoneCode[i+1]
                for j in range(self.NUM_ZONES):
                    desZ = self.zoneCode[j+1]
                    if orgZ == desZ: continue

                    newWTT = {}
                    for mode in listOfAllowedModesForWaitingTime:
                        newWTT[mode] = self.WTT[am_pm_op][mode][i][j]


                    if (sum(newWTT.values())) > 0:   # no need to do anything if there is no update in the values
                        comma_required = False
                        update_param_tuple = ()
                        update_query = "UPDATE " + am_pm_op + '_COSTS_TABLE' + " SET"
                        for mode in listOfAllowedModesForWaitingTime:
                            if newWTT[mode] > 0:
                                update_query = update_query + 'wait_time_'+ mode + "=(%s * %s+ " + am_pm_op + "_COSTS_TABLE + ".wait_time_"+ mode +" * %s)"
                                update_param_tuple = update_param_tuple + (newWTT[mode], ALPHA, 1- ALPHA)
                                prevWTTValues[mode].append(self.oldVals[am_pm_op][mode][(orgZ,desZ)])
                                currentWTTValues.append(newWTT[mode])
                                comma_required = True

                        update_query = update_query + " WHERE origin_zone=%s and destination_zone=%s"
                        update_param_tuple = update_param_tuple + (orgZ, desZ)
                        cur.execute(update_query, update_param_tuple)
                        conn.commit()

          
                
        # calculating the RMSN
        # taking into only those values which are non zeros both in observed and simulated
        for mode in listOfAllowedModesForWaitingTime:
            prevValues = numpy.array(prevCarIvtValues)
            currentValues = numpy.array(currentCarIvtValues)
            rmsn = {}
            rmsn[mode] = numpy.sqrt(numpy.mean(numpy.square(prevValues - currentValues))) / (numpy.mean(prevValues))
            prevValues = list(prevValues)
            currentValues = list(currentValues)
            with open(mode + '_WTT'.csv','a') as f:
                csvwriter = csv.writer(f)
                csvwriter.writerow(['Old Values','New Values'])
                for i in range(len(prevValues)):
                    csvwriter.writerow([prevValues[i], currentValues[i]])

            with open('RMSN_records_zone_to_zone_WTT.txt','a') as f:
                f.write("RMSN value for differences in zone to zone " + mode + " waiting time "+ str(rmsn[mode]) + "\n")

            print ()"\nRMSN value for differences in zone to zone car ivt:", rmsn[mode])
        return


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
s = datetime.datetime.now()
parser = argparse.ArgumentParser()
parser.add_argument("csv_name", default="traveltime.csv", help="waiting times experienced by persons in withinday")
args = parser.parse_args()

#1.
print "1. initializing and loading CSV"
aggregator = WT_Aggregator(str(args.csv_name))
#2.
print "2. processing input"
aggregator.processInput()
#3.
print "3. computing means"
aggregator.computeMeans()

#4.
print "4. updating data in postgres db"
aggregator.updatePostgres()

print "Done. Exiting Main"

e = datetime.datetime.now()
print 'Running Time: ' + str((e-s).total_seconds()) + 's'

