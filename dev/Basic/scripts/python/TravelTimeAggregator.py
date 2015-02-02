from pymongo import MongoClient
import argparse
import datetime
import csv

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
		self.inputCsv = csv.DictReader(open(str(args.csv_name)))
		
		#connect to local mongodb
		self.client = MongoClient('localhost', 27017)
		self.db = self.client.mydb
		
		self.nodeMTZ = self.db.node_mtz
		self.zone = self.db.Zone
		self.amCosts = self.db.LearnedAMCosts
		self.pmCosts = self.db.LearnedPMCosts
		self.opCosts = self.db.LearnedOPCosts
		self.ttCar = self.db.learned_tcost_car
		self.ttBus = self.db.learned_tcost_bus
		
		self.zoneId = {} # dictionary of <zone_code> : <zone_id>
		self.zoneCode = {} #dictionary of <zone_id> : <zone_code>
		self.NUM_ZONES = self.zone.count() #meant to be constant
		
		for z in range(1,1093):
			zoneDoc = self.zone.find_one({"zone_id" : z})
			self.zoneId[int(zoneDoc["zone_code"])] = z
			self.zoneCode[z] = int(zoneDoc["zone_code"])
		
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
	
	#fetch zone from node
	def getZone(self, node):
		return int(self.nodeMTZ.find_one({"_id" : node})["MTZ_1092"])
	
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
			orgZ = self.getZone(int(row["origin"]))
			desZ = self.getZone(int(row["destination"]))
			tripStartTime = str(row["start_time"])
			tripEndTime = str(row["end_time"])
			mode = str(row["mode"])
			travelTime = float(row["travel_time"])
			if mode == "Car" or mode == "Motorcycle" or mode == "Taxi":
				if isAM(tripStartTime):
					self.addAMCarIvt(orgZ, desZ, travelTime)
				elif isPM(tripStartTime):
					self.addPMCarIvt(orgZ, desZ, travelTime)
				else:
					self.addOPCarIvt(orgZ, desZ, travelTime)
				self.addTTCar(orgZ, desZ, tripStartTime, tripEndTime, travelTime)
			elif mode == "Bus":
				if isAM(tripStartTime):
					self.addAMPubIvt(orgZ, desZ, travelTime)
					self.addAMPubWtt(orgZ, desZ, float(row["pt_wtt"]))
					self.addAMPubWalkt(orgZ, desZ, float(row["pt_walkt"]))
				elif isPM(tripStartTime):
					self.addPMPubIvt(orgZ, desZ, travelTime)
					self.addPMPubWtt(orgZ, desZ, float(row["pt_wtt"]))
					self.addPMPubWalkt(orgZ, desZ, float(row["pt_walkt"]))
				else:
					self.addOPPubIvt(orgZ, desZ, travelTime)
					self.addOPPubWtt(orgZ, desZ, float(row["pt_wtt"]))
					self.addOPPubWalkt(orgZ, desZ, float(row["pt_walkt"]))
				self.addTTBus(orgZ, desZ, tripStartTime, tripEndTime, travelTime)
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

	def updateMongo(self):
		for i in range(self.NUM_ZONES):
			orgZ = self.zoneCode[i+1]
			for j in range(self.NUM_ZONES):
				desZ = self.zoneCode[j+1]
				if orgZ == desZ: continue
				query = { "origin" : orgZ, "destin" : desZ }
				
				# AM updates
				updates = {}
				newCarIvt = amCarIvt[i][j]
				newPubIvt = amPubIvt[i][j]
				newPubWtt = amPubWtt[i][j]
				newPubWalkt = amPubWalkt[i][j]
				if (newCarIvt+newPubIvt+newPubWtt+newPubWalkt) > 0: 
					amDoc = amCosts.find_one(query)
					if newCarIvt > 0: updates["car_ivt"] = (newCarIvt + toFloat(amDoc["car_ivt"]))/2
					if newPubIvt > 0: updates["pub_ivt"] = (newPubIvt + toFloat(amDoc["pub_ivt"]))/2
					if newPubWtt > 0: updates["pub_wtt"] = (newPubWtt + toFloat(amDoc["pub_wtt"]))/2
					if newPubWalkt > 0: updates["pub_walkt"] = (newPubWalkt + toFloat(amDoc["pub_walkt"]))/2
					print 'OD:[',orgZ,desZ,'] updating AMCosts newCarIvt:',newCarIvt,' newPubIvt:',newPubIvt,' newPubWtt:',newPubWtt,' newPubWalkt:',newPubWalkt
					amCosts.update(query, {"$set" : updates }, upsert=False, multi=False)
				
				# PM updates
				updates = {}
				newCarIvt = pmCarIvt[i][j]
				newPubIvt = pmPubIvt[i][j]
				newPubWtt = pmPubWtt[i][j]
				newPubWalkt = pmPubWalkt[i][j]
				if (newCarIvt+newPubIvt+newPubWtt+newPubWalkt) > 0: 
					pmDoc = pmCosts.find_one(query)
					if newCarIvt > 0: updates["car_ivt"] = (newCarIvt + toFloat(pmDoc["car_ivt"]))/2
					if newPubIvt > 0: updates["pub_ivt"] = (newPubIvt + toFloat(pmDoc["pub_ivt"]))/2
					if newPubWtt > 0: updates["pub_wtt"] = (newPubWtt + toFloat(pmDoc["pub_wtt"]))/2
					if newPubWalkt > 0: updates["pub_walkt"] = (newPubWalkt + toFloat(pmDoc["pub_walkt"]))/2
					print 'OD:[',orgZ,desZ,'] updating PMCosts newCarIvt:',newCarIvt,' newPubIvt:',newPubIvt,' newPubWtt:',newPubWtt,' newPubWalkt:',newPubWalkt
					pmCosts.update(query, {"$set" : updates }, upsert=False, multi=False)
				
				# OP updates
				updates = {}
				newCarIvt = opCarIvt[i][j]
				newPubIvt = opPubIvt[i][j]
				newPubWtt = opPubWtt[i][j]
				newPubWalkt = opPubWalkt[i][j]
				if (newCarIvt+newPubIvt+newPubWtt+newPubWalkt) > 0: 
					opDoc = opCosts.find_one(query)
					if newCarIvt > 0: updates["car_ivt"] = (newCarIvt + toFloat(opDoc["car_ivt"]))/2
					if newPubIvt > 0: updates["pub_ivt"] = (newPubIvt + toFloat(opDoc["pub_ivt"]))/2
					if newPubWtt > 0: updates["pub_wtt"] = (newPubWtt + toFloat(opDoc["pub_wtt"]))/2
					if newPubWalkt > 0: updates["pub_walkt"] = (newPubWalkt + toFloat(opDoc["pub_walkt"]))/2
					print 'OD:[',orgZ,desZ,'] updating OPCosts newCarIvt:',newCarIvt,' newPubIvt:',newPubIvt,' newPubWtt:',newPubWtt,' newPubWalkt:',newPubWalkt
					opCosts.update(query, {"$set" : updates }, upsert=False, multi=False)
				
				#time dependent tt updates
				query = { "origin" : orgZ, "destination" : desZ }
				updates = {}
				for k in range(48):
					newTTCarArr = ttArrivalCar[i][j][k]
					newTTCarDep = ttDepartureCar[i][j][k]
					if (newTTCarArr+newTTCarDep) > 0:
						ttCarDoc = ttCar.find_one(query)
						if newTTCarArr > 0: updates["TT_car_arrival_"+str(k+1)] = (newTTCarArr + toFloat(ttCarDoc["TT_car_arrival_"+str(k+1)]))/2
						if newTTCarDep > 0: updates["TT_car_departure_"+str(k+1)] = (newTTCarDep + toFloat(ttCarDoc["TT_car_departure_"+str(k+1)]))/2
					if orgZ == 506130 and desZ == 519060:
						print 'newTTCarArr:',newTTCarArr,' newTTCarDep:',newTTCarDep,' k:',k,' updates:',updates
				if updates: 
					print 'OD:[',orgZ,desZ,'] car ',updates
					ttCar.update(query, {"$set" : updates }, upsert=False , multi=False)
				
				updates = {}
				ttBusDoc = ttBus.find_one(query)
				for k in range(48):
					newTTBusArr = ttArrivalBus[i][j][k]
					newTTBusDep = ttDepartureBus[i][j][k]
					if (newTTBusArr+newTTBusDep) > 0:
						ttBusDoc = ttBus.find_one(query)
						if newTTBusArr > 0: updates["TT_bus_arrival_"+str(k+1)] = (newTTBusArr + toFloat(ttBusDoc["TT_bus_arrival_"+str(k+1)]))/2
						if newTTBusDep > 0: updates["TT_bus_departure_"+str(k+1)] = (newTTBusDep + toFloat(ttBusDoc["TT_bus_departure_"+str(k+1)]))/2
					if orgZ == 506130 and desZ == 519060:
						print 'newTTBusArr:',newTTBusArr,' newTTBusDep:',newTTBusDep,' k:',k,' updates:',updates
				if updates: 
					print 'OD:[',orgZ,desZ,'] bus ',updates
					ttBus.update(query, {"$set" : updates }, upsert=False , multi=False)
		return

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
s = datetime.datetime.now()
parser = argparse.ArgumentParser()
parser.add_argument("csv_name", default="tt.csv", help="travel times experienced by persons in withinday")
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
print "4. updating mongodb"
aggregator.updateMongo()

print "Done. Exiting Main"

e = datetime.datetime.now()
print 'Running Time: ' + str((e-s).total_seconds()) + 's'

