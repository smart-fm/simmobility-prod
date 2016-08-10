from pymongo import MongoClient
import argparse
import datetime
import csv
import psycopg2

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ constants ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
DB_NAME = 'simmobility_virtualcity'
DB_HOST = '172.25.184.48'
DB_PORT = '5432'
DB_USER = 'postgres'
DB_PASSWORD = '5M_S1mM0bility'
DB_NAME = 'simmobility_virtualcity'

ZONE_TABLE = 'demand.taz_2012'
AM_COSTS_TABLE = 'demand.learned_amcosts'
PM_COSTS_TABLE = 'demand.learned_pmcosts'
OP_COSTS_TABLE = 'demand.learned_opcosts'
TCOST_CAR_TABLE = 'demand.learned_tcost_car'
TCOST_BUS_TABLE = 'demand.learned_tcost_bus'
SUBTRIP_METRICS_TABLE = 'output.subtrip_metrics'

HALF = 0.5

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

#identify whether time is in Off-peak period
#OffPeak : rest of the day
def isOP(time):
	return ((not isAM(time)) and (not isPM(time)))

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

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~TT_Aggregator class~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
class TT_Aggregator:
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

		# truncate existing subtrip_metrics table
		truncate_script = "truncate table " + SUBTRIP_METRICS_TABLE
		cur.execute(truncate_script)
		
		# copy csv data into table 		
		csvFile = open(str(args.csv_name), 'r')
		cur.copy_from(csvFile, SUBTRIP_METRICS_TABLE, ',')
		csvFile.close()

		# load aggregate records from copied table
		fetch_agg_tt_query = "SELECT origin_taz, destination_taz, min(mode) as mode, min(start_time) as start_time, max(end_time) as end_time, sum(travel_time) as travel_time FROM " + SUBTRIP_METRICS_TABLE + " GROUP BY person_id, trip_id, origin_taz, destination_taz"
		cur.execute(fetch_agg_tt_query)
		self.inputCsv = cur.fetchall()

		# close the cursor
		cur.close()

		##2-dimensional list structures for each OD zone pair
		#to hold cumulative car in-vehicle travel time, in hours
		self.amCarIvt = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		self.pmCarIvt = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		self.opCarIvt = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		
		#to hold number of agents who travelled in car between each OD zone pair
		self.amCarIvtCount = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		self.pmCarIvtCount = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		self.opCarIvtCount = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		
		#to hold cumulative public transit in-vehicle travel time, in hours
		self.amPubIvt = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		self.pmPubIvt = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		self.opPubIvt = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		
		#to hold public transit in-vehicle travel count
		self.amPubIvtCount = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		self.pmPubIvtCount = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		self.opPubIvtCount = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		
		#to hold cumulative public transit waiting time, in hours
		self.amPubWtt = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		self.pmPubWtt = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		self.opPubWtt = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		
		#to hold number of people who waited at public transit stops
		self.amPubWttCount = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		self.pmPubWttCount = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		self.opPubWttCount = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		
		#to hold cumulative public transit walk transfer times, in hours
		self.amPubWalkt = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		self.pmPubWalkt = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		self.opPubWalkt = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		
		#to hold number of people who had walk transfers
		self.amPubWalktCount = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		self.pmPubWalktCount = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		self.opPubWalktCount = [[0 for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		
		#3d tt structures for each tod for each od pair to hold time dependent travel times
		self.ttArrivalCar = [[[0 for k in xrange(48)] for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		self.ttDepartureCar = [[[0 for k in xrange(48)] for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		self.ttArrivalBus = [[[0 for k in xrange(48)] for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		self.ttDepartureBus = [[[0 for k in xrange(48)] for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		
		#3d tt structures for each tod for each od pair to hold time dependent travel counts
		self.ttArrivalCarCount = [[[0 for k in xrange(48)] for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		self.ttDepartureCarCount = [[[0 for k in xrange(48)] for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		self.ttArrivalBusCount = [[[0 for k in xrange(48)] for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]
		self.ttDepartureBusCount = [[[0 for k in xrange(48)] for j in xrange(self.NUM_ZONES)] for i in xrange(self.NUM_ZONES)]

		#connect to local mongodb
		self.client = MongoClient('localhost', 27017)
		self.db = self.client.simmobcity

	##functions to add items into the data structures defined above
	def addAMCarIvt(self, origin, destination, value):
		orgZid = self.zoneId[origin] - 1
		desZid = self.zoneId[destination] - 1
		self.amCarIvt[orgZid][desZid] = self.amCarIvt[orgZid][desZid] + value
		self.amCarIvtCount[orgZid][desZid] = self.amCarIvtCount[orgZid][desZid] + 1
		return
	
	def addAMPubIvt(self, origin, destination, value):
		orgZid = self.zoneId[origin] - 1
		desZid = self.zoneId[destination] - 1
		self.amPubIvt[orgZid][desZid] = self.amPubIvt[orgZid][desZid] + value
		self.amPubIvtCount[orgZid][desZid] = self.amPubIvtCount[orgZid][desZid] + 1
		return
	
	def addAMPubWtt(self, origin, destination, value):
		orgZid = self.zoneId[origin] - 1
		desZid = self.zoneId[destination] - 1
		self.amPubWtt[orgZid][desZid] = self.amPubWtt[orgZid][desZid] + value
		self.amPubWttCount[orgZid][desZid] = self.amPubWttCount[orgZid][desZid] + 1
		return
	
	def addAMPubWalkt(self, origin, destination, value):
		orgZid = self.zoneId[origin] - 1
		desZid = self.zoneId[destination] - 1
		self.amPubWalkt[orgZid][desZid] = self.amPubWalkt[orgZid][desZid] + value
		self.amPubWalktCount[orgZid][desZid] = self.amPubWalktCount[orgZid][desZid] + 1
		return
	
	def addPMCarIvt(self, origin, destination, value):
		orgZid = self.zoneId[origin] - 1
		desZid = self.zoneId[destination] - 1
		self.pmCarIvt[orgZid][desZid] = self.pmCarIvt[orgZid][desZid] + value
		self.pmCarIvtCount[orgZid][desZid] = self.pmCarIvtCount[orgZid][desZid] + 1
		return
	
	def addPMPubIvt(self, origin, destination, value):
		orgZid = self.zoneId[origin] - 1
		desZid = self.zoneId[destination] - 1
		self.pmPubIvt[orgZid][desZid] = self.pmPubIvt[orgZid][desZid] + value
		self.pmPubIvtCount[orgZid][desZid] = self.pmPubIvtCount[orgZid][desZid] + 1
		return
	
	def addPMPubWtt(self, origin, destination, value):
		orgZid = self.zoneId[origin] - 1
		desZid = self.zoneId[destination] - 1
		self.pmPubWtt[orgZid][desZid] = self.pmPubWtt[orgZid][desZid] + value
		self.pmPubWttCount[orgZid][desZid] = self.pmPubWttCount[orgZid][desZid] + 1
		return
	
	def addPMPubWalkt(self, origin, destination, value):
		orgZid = self.zoneId[origin] - 1
		desZid = self.zoneId[destination] - 1
		self.pmPubWalkt[orgZid][desZid] = self.pmPubWalkt[orgZid][desZid] + value
		self.pmPubWalktCount[orgZid][desZid] = self.pmPubWalktCount[orgZid][desZid] + 1
		return
	
	def addOPCarIvt(self, origin, destination, value):
		orgZid = self.zoneId[origin] - 1
		desZid = self.zoneId[destination] - 1
		self.opCarIvt[orgZid][desZid] = self.opCarIvt[orgZid][desZid] + value
		self.opCarIvtCount[orgZid][desZid] = self.opCarIvtCount[orgZid][desZid] + 1
		return
	
	def addOPPubIvt(self, origin, destination, value):
		orgZid = self.zoneId[origin] - 1
		desZid = self.zoneId[destination] - 1
		self.opPubIvt[orgZid][desZid] = self.opPubIvt[orgZid][desZid] + value
		self.opPubIvtCount[orgZid][desZid] = self.opPubIvtCount[orgZid][desZid] + 1
		return
	
	def addOPPubWtt(self, origin, destination, value):
		orgZid = self.zoneId[origin] - 1
		desZid = self.zoneId[destination] - 1
		self.opPubWtt[orgZid][desZid] = self.opPubWtt[orgZid][desZid] + value
		self.opPubWttCount[orgZid][desZid] = self.opPubWttCount[orgZid][desZid] + 1
		return
	
	def addOPPubWalkt(self, origin, destination, value):
		orgZid = self.zoneId[origin] - 1
		desZid = self.zoneId[destination] - 1
		self.opPubWalkt[orgZid][desZid] = self.opPubWalkt[orgZid][desZid] + value
		self.opPubWalktCount[orgZid][desZid] = self.opPubWalktCount[orgZid][desZid] + 1
		return
	
	def addTTCar(self, origin, destination, departure, arrival, value):
		orgZid = self.zoneId[origin] - 1
		desZid = self.zoneId[destination] - 1
		arrIdx = getWindowIdx(arrival) - 1
		depIdx = getWindowIdx(departure) - 1
		self.ttArrivalCar[orgZid][desZid][arrIdx] = self.ttArrivalCar[orgZid][desZid][arrIdx] + value
		self.ttArrivalCarCount[orgZid][desZid][arrIdx] = self.ttArrivalCarCount[orgZid][desZid][arrIdx] + 1
		self.ttDepartureCar[orgZid][desZid][depIdx] = self.ttDepartureCar[orgZid][desZid][depIdx] + value
		self.ttDepartureCarCount[orgZid][desZid][depIdx] = self.ttDepartureCarCount[orgZid][desZid][depIdx] + 1
		return
	
	def addTTBus(self, origin, destination, departure, arrival, value):
		orgZid = self.zoneId[origin] - 1
		desZid = self.zoneId[destination] - 1
		arrIdx = getWindowIdx(arrival) - 1
		depIdx = getWindowIdx(departure) - 1
		self.ttArrivalBus[orgZid][desZid][arrIdx] = self.ttArrivalBus[orgZid][desZid][arrIdx] + value
		self.ttArrivalBusCount[orgZid][desZid][arrIdx] = self.ttArrivalBusCount[orgZid][desZid][arrIdx] + 1
		self.ttDepartureBus[orgZid][desZid][depIdx] = self.ttDepartureBus[orgZid][desZid][depIdx] + value
		self.ttDepartureBusCount[orgZid][desZid][depIdx] = self.ttDepartureBusCount[orgZid][desZid][depIdx] + 1
		return
	
	#process input csv
	def processInput(self):
		#process each row of input csv
		for row in self.inputCsv:
			#origin_taz, destination_taz, mode, start_time, end_time, travel_time
			orgZ = int(row[0])
			desZ = int(row[1])
			mode = str(row[2]).strip()
			tripStartTime = str(row[3])
			tripEndTime = str(row[4])
			travelTime = float(row[5])
			if mode == "Car" or mode == "Motorcycle" or mode == "Taxi":
				if isAM(tripStartTime):
					self.addAMCarIvt(orgZ, desZ, travelTime)
				elif isPM(tripStartTime):
					self.addPMCarIvt(orgZ, desZ, travelTime)
				else:
					self.addOPCarIvt(orgZ, desZ, travelTime)
				self.addTTCar(orgZ, desZ, tripStartTime, tripEndTime, travelTime)
			elif mode == "BusTravel" or mode == "MRT":
				if isAM(tripStartTime):
					self.addAMPubIvt(orgZ, desZ, travelTime)
					#self.addAMPubWtt(orgZ, desZ, float(row[6]))
					#self.addAMPubWalkt(orgZ, desZ, float(row[7]))
				elif isPM(tripStartTime):
					self.addPMPubIvt(orgZ, desZ, travelTime)
					#self.addPMPubWtt(orgZ, desZ, float(row[6]))
					#self.addPMPubWalkt(orgZ, desZ, float(row[7]))
				else:
					self.addOPPubIvt(orgZ, desZ, travelTime)
					#self.addOPPubWtt(orgZ, desZ, float(row[6]))
					#self.addOPPubWalkt(orgZ, desZ, float(row[7]))
				self.addTTBus(orgZ, desZ, tripStartTime, tripEndTime, travelTime)
			else:
				print 'ignoring record with mode ' + mode
		return
	
	def computeMeans(self):
		for i in range(self.NUM_ZONES):
			for j in range(self.NUM_ZONES):
				self.amCarIvt[i][j] = (float(self.amCarIvt[i][j])/self.amCarIvtCount[i][j]) if self.amCarIvtCount[i][j] > 0 else 0
				self.pmCarIvt[i][j] = (float(self.pmCarIvt[i][j])/self.pmCarIvtCount[i][j]) if self.pmCarIvtCount[i][j] > 0 else 0
				self.opCarIvt[i][j] = (float(self.opCarIvt[i][j])/self.opCarIvtCount[i][j]) if self.opCarIvtCount[i][j] > 0 else 0
			
				self.amPubIvt[i][j] = (float(self.amPubIvt[i][j])/self.amPubIvtCount[i][j]) if self.amPubIvtCount[i][j] > 0 else 0
				self.pmPubIvt[i][j] = (float(self.pmPubIvt[i][j])/self.pmPubIvtCount[i][j]) if self.pmPubIvtCount[i][j] > 0 else 0
				self.opPubIvt[i][j] = (float(self.opPubIvt[i][j])/self.opPubIvtCount[i][j]) if self.opPubIvtCount[i][j] > 0 else 0
			
				self.amPubWtt[i][j] = (float(self.amPubWtt[i][j])/self.amPubWttCount[i][j]) if self.amPubWttCount[i][j] > 0 else 0
				self.pmPubWtt[i][j] = (float(self.pmPubWtt[i][j])/self.pmPubWttCount[i][j]) if self.pmPubWttCount[i][j] > 0 else 0
				self.opPubWtt[i][j] = (float(self.opPubWtt[i][j])/self.opPubWttCount[i][j]) if self.opPubWttCount[i][j] > 0 else 0
			
				self.amPubWalkt[i][j] = (float(self.amPubWalkt[i][j])/self.amPubWalktCount[i][j]) if self.amPubWalktCount[i][j] > 0 else 0
				self.pmPubWalkt[i][j] = (float(self.pmPubWalkt[i][j])/self.pmPubWalktCount[i][j]) if self.pmPubWalktCount[i][j] > 0 else 0
				self.opPubWalkt[i][j] = (float(self.opPubWalkt[i][j])/self.opPubWalktCount[i][j]) if self.opPubWalktCount[i][j] > 0 else 0
			
				for k in range(48):
					self.ttArrivalCar[i][j][k] = (float(self.ttDepartureCar[i][j][k])/self.ttArrivalCarCount[i][j][k]) if self.ttArrivalCarCount[i][j][k] > 0 else 0
					self.ttDepartureCar[i][j][k] = (float(self.ttDepartureCar[i][j][k])/self.ttDepartureCarCount[i][j][k]) if self.ttDepartureCarCount[i][j][k] > 0 else 0
					self.ttArrivalBus[i][j][k] = (float(self.ttArrivalBus[i][j][k])/self.ttArrivalBusCount[i][j][k]) if self.ttArrivalBusCount[i][j][k] > 0 else 0
					self.ttDepartureBus[i][j][k] = (float(self.ttDepartureBus[i][j][k])/self.ttDepartureBusCount[i][j][k]) if self.ttDepartureBusCount[i][j][k] > 0 else 0
		return

	def updatePostgres(self):
		connection_string = "dbname='" + DB_NAME + "' user='" + DB_USER + "' host='" + DB_HOST + "' port='" + DB_PORT + "' password='" + DB_PASSWORD + "'"
		conn = psycopg2.connect(connection_string)
		cur = conn.cursor()
		for i in range(self.NUM_ZONES):
			orgZ = self.zoneCode[i+1]
			for j in range(self.NUM_ZONES):
				desZ = self.zoneCode[j+1]
				if orgZ == desZ: continue
				
				# AM updates
				newCarIvt = self.amCarIvt[i][j]
				newPubIvt = self.amPubIvt[i][j]
				newPubWtt = self.amPubWtt[i][j]
				newPubWalkt = self.amPubWalkt[i][j]
				if (newCarIvt+newPubIvt+newPubWtt+newPubWalkt) > 0: 
					comma_required = False
					update_param_tuple = ()
					update_query = "UPDATE " + AM_COSTS_TABLE + " SET" 
					if newCarIvt > 0: 
						update_query = update_query + " car_ivt=((%s + " + AM_COSTS_TABLE + ".car_ivt) * %s)" 
						update_param_tuple = update_param_tuple + (newCarIvt, HALF)
						comma_required = True
					if newPubIvt > 0: 
						if comma_required: 
							update_query = update_query + ","
						update_query = update_query + " pub_ivt=((%s + " + AM_COSTS_TABLE + ".pub_ivt) * %s)" 
						update_param_tuple = update_param_tuple + (newPubIvt, HALF)
						comma_required = True
					if newPubWtt > 0: 
						if comma_required: 
							update_query = update_query + ","
						update_query = update_query + " pub_wtt=((%s + " + AM_COSTS_TABLE + ".pub_wtt) * %s)" 
						update_param_tuple = update_param_tuple + (newPubWtt, HALF)
						comma_required = True
					if newPubWalkt > 0: 
						if comma_required: 
							update_query = update_query + ","
						update_query = update_query + " pub_walkt=((%s + " + AM_COSTS_TABLE + ".pub_walkt) * %s)" 
						update_param_tuple = update_param_tuple + (newPubWalkt, HALF)
					update_query = update_query + " WHERE origin_zone=%s and destination_zone=%s"
					update_param_tuple = update_param_tuple + (orgZ, desZ)
					cur.execute(update_query, update_param_tuple)
					conn.commit()

				# PM updates
				newCarIvt = self.pmCarIvt[i][j]
				newPubIvt = self.pmPubIvt[i][j]
				newPubWtt = self.pmPubWtt[i][j]
				newPubWalkt = self.pmPubWalkt[i][j]
				if (newCarIvt+newPubIvt+newPubWtt+newPubWalkt) > 0: 
					comma_required = False
					update_param_tuple = ()
					update_query = "UPDATE " + PM_COSTS_TABLE + " SET" 
					if newCarIvt > 0: 
						update_query = update_query + " car_ivt=((%s + " + PM_COSTS_TABLE + ".car_ivt) * %s)" 
						update_param_tuple = update_param_tuple + (newCarIvt, HALF)
						comma_required = True
					if newPubIvt > 0: 
						if comma_required: 
							update_query = update_query + ","
						update_query = update_query + " pub_ivt=((%s + " + PM_COSTS_TABLE + ".pub_ivt) * %s)" 
						update_param_tuple = update_param_tuple + (newPubIvt, HALF)
						comma_required = True
					if newPubWtt > 0: 
						if comma_required: 
							update_query = update_query + ","
						update_query = update_query + " pub_wtt=((%s + " + PM_COSTS_TABLE + ".pub_wtt) * %s)" 
						update_param_tuple = update_param_tuple + (newPubWtt, HALF)
						comma_required = True
					if newPubWalkt > 0: 
						if comma_required: 
							update_query = update_query + ","
						update_query = update_query + " pub_walkt=((%s + " + PM_COSTS_TABLE + ".pub_walkt) * %s)" 
						update_param_tuple = update_param_tuple + (newPubWalkt, HALF)
					update_query = update_query + " WHERE origin_zone=%s and destination_zone=%s"
					update_param_tuple = update_param_tuple + (orgZ, desZ)
					cur.execute(update_query, update_param_tuple)
					conn.commit()	
			
				# OP updates
				newCarIvt = self.opCarIvt[i][j]
				newPubIvt = self.opPubIvt[i][j]
				newPubWtt = self.opPubWtt[i][j]
				newPubWalkt = self.opPubWalkt[i][j]
				if (newCarIvt+newPubIvt+newPubWtt+newPubWalkt) > 0: 
					comma_required = False
					update_param_tuple = ()
					update_query = "UPDATE " + OP_COSTS_TABLE + " SET" 
					if newCarIvt > 0: 
						update_query = update_query + " car_ivt=((%s + " + OP_COSTS_TABLE + ".car_ivt) * %s)" 
						update_param_tuple = update_param_tuple + (newCarIvt, HALF)
						comma_required = True
					if newPubIvt > 0: 
						if comma_required: 
							update_query = update_query + ","
						update_query = update_query + " pub_ivt=((%s + " + OP_COSTS_TABLE + ".pub_ivt) * %s)" 
						update_param_tuple = update_param_tuple + (newPubIvt, HALF)
						comma_required = True
					if newPubWtt > 0: 
						if comma_required: 
							update_query = update_query + ","
						update_query = update_query + " pub_wtt=((%s + " + OP_COSTS_TABLE + ".pub_wtt) * %s)" 
						update_param_tuple = update_param_tuple + (newPubWtt, HALF)
						comma_required = True
					if newPubWalkt > 0: 
						if comma_required: 
							update_query = update_query + ","
						update_query = update_query + " pub_walkt=((%s + " + OP_COSTS_TABLE + ".pub_walkt) * %s)" 
						update_param_tuple = update_param_tuple + (newPubWalkt, HALF)
					update_query = update_query + " WHERE origin_zone=%s and destination_zone=%s"
					update_param_tuple = update_param_tuple + (orgZ, desZ)
					cur.execute(update_query, update_param_tuple)
					conn.commit()	
				
				#time dependent tt updates
				updatesAvailable = False
				update_query = "UPDATE " + TCOST_CAR_TABLE + " SET" 
				update_param_tuple = ()
				comma_required = False
				for k in range(48):
					newTTCarArr = self.ttArrivalCar[i][j][k]
					newTTCarDep = self.ttDepartureCar[i][j][k]
					arrival_col = "tt_arrival_" + str(k+1)
					departure_col = "tt_departure_" + str(k+1)
					if (newTTCarArr+newTTCarDep) > 0:
						updatesAvailable = True
						if newTTCarArr > 0: 
							if comma_required: 
								update_query = update_query + ","
							update_query = update_query + " " + arrival_col + "=((%s + " + TCOST_CAR_TABLE + "." + arrival_col + ") * %s)" 
							update_param_tuple = update_param_tuple + (newTTCarArr, HALF)
							comma_required = True
						if newTTCarDep > 0: 
							if comma_required: 
								update_query = update_query + ","
							update_query = update_query + " " + departure_col + "=((%s + " + TCOST_CAR_TABLE + "." + departure_col + ") * %s)" 
							update_param_tuple = update_param_tuple + (newTTCarDep, HALF)
							comma_required = True
				if updatesAvailable:
					update_query = update_query + " WHERE origin_zone=%s and destination_zone=%s"
					update_param_tuple = update_param_tuple + (orgZ, desZ)
					cur.execute(update_query, update_param_tuple)
					conn.commit()							
				
				updatesAvailable = False
				update_query = "UPDATE " + TCOST_BUS_TABLE + " SET" 
				update_param_tuple = ()
				comma_required = False
				for k in range(48):
					newTTBusArr = self.ttArrivalBus[i][j][k]
					newTTBusDep = self.ttDepartureBus[i][j][k]
					arrival_col = "tt_arrival_" + str(k+1)
					departure_col = "tt_departure_" + str(k+1)
					if (newTTBusArr+newTTBusDep) > 0:
						updatesAvailable = True
						if newTTBusArr > 0: 
							if comma_required: 
								update_query = update_query + ","
							update_query = update_query + " " + arrival_col + "=((%s + " + TCOST_BUS_TABLE + "." + arrival_col + ") * %s)" 
							update_param_tuple = update_param_tuple + (newTTBusArr, HALF)
							comma_required = True
						if newTTBusDep > 0: 
							if comma_required: 
								update_query = update_query + ","
							update_query = update_query + " " + departure_col + "=((%s + " + TCOST_BUS_TABLE + "." + departure_col + ") * %s)" 
							update_param_tuple = update_param_tuple + (newTTBusDep, HALF)
							comma_required = True
				if updatesAvailable:
					update_query = update_query + " WHERE origin_zone=%s and destination_zone=%s"
					update_param_tuple = update_param_tuple + (orgZ, desZ)
					cur.execute(update_query, update_param_tuple)
					conn.commit()
		return


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
s = datetime.datetime.now()
parser = argparse.ArgumentParser()
parser.add_argument("csv_name", default="subtrip_metrics.csv", help="travel times experienced by persons in withinday")
args = parser.parse_args()

#1.
print "1. initializing and loading CSV"
aggregator = TT_Aggregator(str(args.csv_name))
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

