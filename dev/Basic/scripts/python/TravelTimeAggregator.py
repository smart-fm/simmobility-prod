from pymongo import MongoClient
import argparse
import datetime
import csv

inputCsv = None

#connect to local mongodb
client = MongoClient('localhost', 27017)
db = client.mydb
nodeMTZ = db.node_mtz
zone = db.Zone
amCosts = db.LearnedAMCosts
pmCosts = db.LearnedPMCosts
opCosts = db.LearnedOPCosts
ttCar = db.learned_tcost_car
ttBus = db.learned_tcost_bus

zoneId = {} # dictionary of <zone_code> : <zone_id>
zoneCode = {} #dictionary of <zone_id> : <zone_code>

NUM_ZONES = zone.count() #meant to be constant

for z in range(1,1093):
	zoneDoc = zone.find_one({"zone_id" : z})
	zoneId[int(zoneDoc["zone_code"])] = z
	zoneCode[z] = int(zoneDoc["zone_code"])
 
##2-dimensional list structures for each OD zone pair
#to hold cumulative car in-vehicle travel time, in hours
amCarIvt = [[0]*NUM_ZONES for i in range(NUM_ZONES)]
pmCarIvt = [[0]*NUM_ZONES for i in range(NUM_ZONES)]
opCarIvt = [[0]*NUM_ZONES for i in range(NUM_ZONES)]

#to hold number of agents who travelled in car between each OD zone pair
amCarIvtCount = [[0]*NUM_ZONES for i in range(NUM_ZONES)]
pmCarIvtCount = [[0]*NUM_ZONES for i in range(NUM_ZONES)]
opCarIvtCount = [[0]*NUM_ZONES for i in range(NUM_ZONES)]

#to hold cumulative public transit in-vehicle travel time, in hours
amPubIvt = [[0]*NUM_ZONES for i in range(NUM_ZONES)]
pmPubIvt = [[0]*NUM_ZONES for i in range(NUM_ZONES)]
opPubIvt = [[0]*NUM_ZONES for i in range(NUM_ZONES)]

#to hold public transit in-vehicle travel count
amPubIvtCount = [[0]*NUM_ZONES for i in range(NUM_ZONES)]
pmPubIvtCount = [[0]*NUM_ZONES for i in range(NUM_ZONES)]
opPubIvtCount = [[0]*NUM_ZONES for i in range(NUM_ZONES)]

#to hold cumulative public transit waiting time, in hours
amPubWtt = [[0]*NUM_ZONES for i in range(NUM_ZONES)]
pmPubWtt = [[0]*NUM_ZONES for i in range(NUM_ZONES)]
opPubWtt = [[0]*NUM_ZONES for i in range(NUM_ZONES)]

#to hold number of people who waited at public transit stops
amPubWttCount = [[0]*NUM_ZONES for i in range(NUM_ZONES)]
pmPubWttCount = [[0]*NUM_ZONES for i in range(NUM_ZONES)]
opPubWttCount = [[0]*NUM_ZONES for i in range(NUM_ZONES)]

#to hold cumulative public transit walk transfer times, in hours
amPubWalkt = [[0]*NUM_ZONES for i in range(NUM_ZONES)]
pmPubWalkt = [[0]*NUM_ZONES for i in range(NUM_ZONES)]
opPubWalkt = [[0]*NUM_ZONES for i in range(NUM_ZONES)]

#to hold number of people who had walk transfers
amPubWalktCount = [[0]*NUM_ZONES for i in range(NUM_ZONES)]
pmPubWalktCount = [[0]*NUM_ZONES for i in range(NUM_ZONES)]
opPubWalktCount = [[0]*NUM_ZONES for i in range(NUM_ZONES)]

#3d tt structures for each tod for each od pair to hold time dependent travel times
ttArrivalCar = [[[0]*48]*NUM_ZONES for i in range(NUM_ZONES)]
ttDepartureCar = [[[0]*48]*NUM_ZONES for i in range(NUM_ZONES)]
ttArrivalBus = [[[0]*48]*NUM_ZONES for i in range(NUM_ZONES)]
ttDepartureBus = [[[0]*48]*NUM_ZONES for i in range(NUM_ZONES)]

#3d tt structures for each tod for each od pair to hold time dependent travel counts
ttArrivalCarCount = [[[0]*48]*NUM_ZONES for i in range(NUM_ZONES)]
ttDepartureCarCount = [[[0]*48]*NUM_ZONES for i in range(NUM_ZONES)]
ttArrivalBusCount = [[[0]*48]*NUM_ZONES for i in range(NUM_ZONES)]
ttDepartureBusCount = [[[0]*48]*NUM_ZONES for i in range(NUM_ZONES)]

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
	
#fetch zone from node
def getZone(node):
	return int(nodeMTZ.find_one({"_id" : node})["MTZ_1092"])

##functions to add items into the data structures defined above
def addAMCarIvt(origin, destination, value):
	orgZid = zoneId[origin] - 1
	desZid = zoneId[destination] - 1
	amCarIvt[orgZid][desZid] = amCarIvt[orgZid][desZid] + value
	amCarIvtCount[orgZid][desZid] = amCarIvtCount[orgZid][desZid] + 1

def addAMPubIvt(origin, destination, value):
	orgZid = zoneId[origin] - 1
	desZid = zoneId[destination] - 1
	amPubIvt[orgZid][desZid] = amPubIvt[orgZid][desZid] + value
	amPubIvtCount[orgZid][desZid] = amPubIvtCount[orgZid][desZid] + 1

def addAMPubWtt(origin, destination, value):
	orgZid = zoneId[origin] - 1
	desZid = zoneId[destination] - 1
	amPubWtt[orgZid][desZid] = amPubWtt[orgZid][desZid] + value
	amPubWttCount[orgZid][desZid] = amPubWttCount[orgZid][desZid] + 1

def addAMPubWalkt(origin, destination, value):
	orgZid = zoneId[origin] - 1
	desZid = zoneId[destination] - 1
	amPubWalkt[orgZid][desZid] = amPubWalkt[orgZid][desZid] + value
	amPubWalktCount[orgZid][desZid] = amPubWalktCount[orgZid][desZid] + 1

def addPMCarIvt(origin, destination, value):
	orgZid = zoneId[origin] - 1
	desZid = zoneId[destination] - 1
	pmCarIvt[orgZid][desZid] = pmCarIvt[orgZid][desZid] + value
	pmCarIvtCount[orgZid][desZid] = pmCarIvtCount[orgZid][desZid] + 1

def addPMPubIvt(origin, destination, value):
	orgZid = zoneId[origin] - 1
	desZid = zoneId[destination] - 1
	pmPubIvt[orgZid][desZid] = pmPubIvt[orgZid][desZid] + value
	pmPubIvtCount[orgZid][desZid] = pmPubIvtCount[orgZid][desZid] + 1

def addPMPubWtt(origin, destination, value):
	orgZid = zoneId[origin] - 1
	desZid = zoneId[destination] - 1
	pmPubWtt[orgZid][desZid] = pmPubWtt[orgZid][desZid] + value
	pmPubWttCount[orgZid][desZid] = pmPubWttCount[orgZid][desZid] + 1

def addPMPubWalkt(origin, destination, value):
	orgZid = zoneId[origin] - 1
	desZid = zoneId[destination] - 1
	pmPubWalkt[orgZid][desZid] = pmPubWalkt[orgZid][desZid] + value
	pmPubWalktCount[orgZid][desZid] = pmPubWalktCount[orgZid][desZid] + 1

def addOPCarIvt(origin, destination, value):
	orgZid = zoneId[origin] - 1
	desZid = zoneId[destination] - 1
	opCarIvt[orgZid][desZid] = opCarIvt[orgZid][desZid] + value
	opCarIvtCount[orgZid][desZid] = opCarIvtCount[orgZid][desZid] + 1

def addOPPubIvt(origin, destination, value):
	orgZid = zoneId[origin] - 1
	desZid = zoneId[destination] - 1
	opPubIvt[orgZid][desZid] = opPubIvt[orgZid][desZid] + value
	opPubIvtCount[orgZid][desZid] = opPubIvtCount[orgZid][desZid] + 1

def addOPPubWtt(origin, destination, value):
	orgZid = zoneId[origin] - 1
	desZid = zoneId[destination] - 1
	opPubWtt[orgZid][desZid] = opPubWtt[orgZid][desZid] + value
	opPubWttCount[orgZid][desZid] = opPubWttCount[orgZid][desZid] + 1

def addOPPubWalkt(origin, destination, value):
	orgZid = zoneId[origin] - 1
	desZid = zoneId[destination] - 1
	opPubWalkt[orgZid][desZid] = opPubWalkt[orgZid][desZid] + value
	opPubWalktCount[orgZid][desZid] = opPubWalktCount[orgZid][desZid] + 1

def addTTCar(origin, destination, departure, arrival, value):
	orgZid = zoneId[origin] - 1
	desZid = zoneId[destination] - 1
	arrIdx = getWindowIdx(arrival)
	depIdx = getWindowIdx(departure)
	ttArrivalCar[orgZid][desZid][arrIdx] = ttArrivalCar[orgZid][desZid][arrIdx] + value
	ttArrivalCarCount[orgZid][desZid][arrIdx] = ttArrivalCarCount[orgZid][desZid][arrIdx] + 1
	ttDepartureCar[orgZid][desZid][depIdx] = ttDepartureCar[orgZid][desZid][depIdx] + value
	ttDepartureCarCount[orgZid][desZid][depIdx] = ttDepartureCarCount[orgZid][desZid][depIdx] + 1
	print 'OD:[',origin,destination,'] arr:',arrIdx,' ttArrivalCar:',ttArrivalCar[orgZid][desZid][arrIdx],' ttArrivalCarCount:',ttArrivalCarCount[orgZid][desZid][arrIdx],' dep:',depIdx,' ttDepartureCar:',ttDepartureCar[orgZid][desZid][depIdx],' ttDepartureCarCount:',ttDepartureCarCount[orgZid][desZid][depIdx]

def addTTBus(origin, destination, departure, arrival, value):
	orgZid = zoneId[origin] - 1
	desZid = zoneId[destination] - 1
	arrIdx = getWindowIdx(arrival)
	depIdx = getWindowIdx(departure)
	ttArrivalBus[orgZid][desZid][arrIdx] = ttArrivalBus[orgZid][desZid][arrIdx] + value
	ttArrivalBusCount[orgZid][desZid][arrIdx] = ttArrivalBusCount[orgZid][desZid][arrIdx] + 1
	ttDepartureBus[orgZid][desZid][depIdx] = ttDepartureBus[orgZid][desZid][depIdx] + value
	ttDepartureBusCount[orgZid][desZid][depIdx] = ttDepartureBusCount[orgZid][desZid][depIdx] + 1

	
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

#process input csv
def processInput():
	#process each row of input csv
	for row in inputCsv:
		orgZ = getZone(int(row["origin"]))
		desZ = getZone(int(row["destination"]))
		tripStartTime = str(row["start_time"])
		tripEndTime = str(row["end_time"])
		mode = str(row["mode"])
		travelTime = float(row["travel_time"])
		if mode == "Car" or mode == "Motorcycle" or mode == "Taxi":
			if isAM(tripStartTime): 
				addAMCarIvt(orgZ, desZ, travelTime)
			elif isPM(tripStartTime): 
				addPMCarIvt(orgZ, desZ, travelTime)
			else : 
				addOPCarIvt(orgZ, desZ, travelTime)
			addTTCar(orgZ, desZ, tripStartTime, tripEndTime, travelTime)
			
		elif mode == "Bus":
			if isAM(tripStartTime):
				addAMPubIvt(orgZ, desZ, travelTime)
				addAMPubWtt(orgZ, desZ, float(row["pt_wtt"]))
				addAMPubWalkt(orgZ, desZ, float(row["pt_walkt"]))
			elif isPM(tripStartTime): 
				addPMPubIvt(orgZ, desZ, travelTime)
				addPMPubWtt(orgZ, desZ, float(row["pt_wtt"]))
				addPMPubWalkt(orgZ, desZ, float(row["pt_walkt"]))
			else: 
				addOPPubIvt(orgZ, desZ, travelTime)
				addOPPubWtt(orgZ, desZ, float(row["pt_wtt"]))
				addOPPubWalkt(orgZ, desZ, float(row["pt_walkt"]))
			addTTBus(orgZ, desZ, tripStartTime, tripEndTime, travelTime)

def computeMeans():
	for i in range(NUM_ZONES):
		for j in range(NUM_ZONES):
			amCarIvt[i][j] = (float(amCarIvt[i][j])/amCarIvtCount[i][j]) if amCarIvtCount[i][j] > 0 else 0
			pmCarIvt[i][j] = (float(pmCarIvt[i][j])/pmCarIvtCount[i][j]) if pmCarIvtCount[i][j] > 0 else 0
			opCarIvt[i][j] = (float(opCarIvt[i][j])/opCarIvtCount[i][j]) if opCarIvtCount[i][j] > 0 else 0
			
			amPubIvt[i][j] = (float(amPubIvt[i][j])/amPubIvtCount[i][j]) if amPubIvtCount[i][j] > 0 else 0
			pmPubIvt[i][j] = (float(pmPubIvt[i][j])/pmPubIvtCount[i][j]) if pmPubIvtCount[i][j] > 0 else 0
			opPubIvt[i][j] = (float(opPubIvt[i][j])/opPubIvtCount[i][j]) if opPubIvtCount[i][j] > 0 else 0	
			
			amPubWtt[i][j] = (float(amPubWtt[i][j])/amPubWttCount[i][j]) if amPubWttCount[i][j] > 0 else 0
			pmPubWtt[i][j] = (float(pmPubWtt[i][j])/pmPubWttCount[i][j]) if pmPubWttCount[i][j] > 0 else 0
			opPubWtt[i][j] = (float(opPubWtt[i][j])/opPubWttCount[i][j]) if opPubWttCount[i][j] > 0 else 0
			
			amPubWalkt[i][j] = (float(amPubWalkt[i][j])/amPubWalktCount[i][j]) if amPubWalktCount[i][j] > 0 else 0
			pmPubWalkt[i][j] = (float(pmPubWalkt[i][j])/pmPubWalktCount[i][j]) if pmPubWalktCount[i][j] > 0 else 0
			opPubWalkt[i][j] = (float(opPubWalkt[i][j])/opPubWalktCount[i][j]) if opPubWalktCount[i][j] > 0 else 0

			for k in range(48):
				ttArrivalCar[i][j][k] = (float(ttArrivalCar[i][j][k])/ttArrivalCarCount[i][j][k]) if ttArrivalCarCount[i][j][k] > 0 else 0
				ttDepartureCar[i][j][k] = (float(ttDepartureCar[i][j][k])/ttDepartureCarCount[i][j][k]) if ttDepartureCarCount[i][j][k] > 0 else 0
				ttArrivalBus[i][j][k] = (float(ttArrivalBus[i][j][k])/ttArrivalBusCount[i][j][k]) if ttArrivalBusCount[i][j][k] > 0 else 0
				ttDepartureBus[i][j][k] = (float(ttDepartureBus[i][j][k])/ttDepartureBusCount[i][j][k]) if ttDepartureBusCount[i][j][k] > 0 else 0

def updateMongo():
	for i in range(NUM_ZONES):
		orgZ = zoneCode[i+1]
		for j in range(NUM_ZONES):
			desZ = zoneCode[j+1]
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
			ttCarDoc = ttCar.find_one(query)
			for k in range(48):
				newTT = ttArrivalCar[i][j][k]
				if newTT > 0: updates["TT_car_arrival_"+str(k+1)] = (newTT + toFloat(ttCarDoc["TT_car_arrival_"+str(k+1)]))/2
				newTT = ttDepartureCar[i][j][k]
				if newTT > 0: updates["TT_car_departure_"+str(k+1)] = (newTT + toFloat(ttCarDoc["TT_car_departure_"+str(k+1)]))/2
			if updates: 
				print 'OD:[',orgZ,desZ,'] car ',updates
				ttCar.update(query, {"$set" : updates }, upsert=False , multi=False)

			updates = {}
			ttBusDoc = ttBus.find_one(query)
			for k in range(48):
				newTT = ttArrivalBus[i][j][k]
				if newTT > 0: updates["TT_bus_arrival_"+str(k+1)] = (newTT + toFloat(ttBusDoc["TT_bus_arrival_"+str(k+1)]))/2
				newTT = ttDepartureBus[i][j][k]
				if newTT > 0: updates["TT_bus_departure_"+str(k+1)] = (newTT + toFloat(ttBusDoc["TT_bus_departure_"+str(k+1)]))/2
			if updates: 
				print 'OD:[',orgZ,desZ,'] bus ',updates
				ttBus.update(query, {"$set" : updates }, upsert=False , multi=False)

if __name__ == "__main__":
	s = datetime.datetime.now()
	parser = argparse.ArgumentParser()
	parser.add_argument("csv_name", default="tt.csv", help="travel times experienced by persons in withinday")
	args = parser.parse_args()

	#1.
	print "1. loading CSV"
	inputCsv = csv.DictReader(open(str(args.csv_name)))
	#2.
	print "2. processing input"
	processInput()
	#3.
	print "3. computing means"
	computeMeans()
	#4.
	print "4. updating mongodb"
	updateMongo()
	
	print "Done. Exiting Main"

	e = datetime.datetime.now()
	print 'Running Time: ' + str((e-s).total_seconds()) + 's'

			
		
