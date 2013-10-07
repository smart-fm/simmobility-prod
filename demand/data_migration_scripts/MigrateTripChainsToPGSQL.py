#!/usr/bin/python
from pymongo import MongoClient
import psycopg2
import random
import math
import json
import argparse

db = None
collection_names = None
conn = None
pg_cursor = None

def getRandomTime(mid):
	hour = int(math.floor(mid))
	minute = int(random.uniform(0, 30) + (mid - hour - 0.25)*60)
	hour = ("0" + str(hour)) if hour < 10 else str(hour)
	minute = ("0" + str(minute)) if minute < 10 else str(minute)
	second = "00"
	return (hour+":"+minute+":"+second)

def getRandomNodeInZone(zone_code):
	zone_id = int(db[collection_names["Zone"]].find_one({"zone_code" : int(zone_code)})["zone_id"])
	nodes = list(db[collection_names["Zone Node Mapping"]].find({"Zone_ID" : zone_id}, {"Node_id":1}))
	toRemove = []
	for i in range(len(nodes)):
		node = nodes[i]
		node_str = str(node["Node_id"])
		if node_str.startswith('9999'):
			toRemove.append(node)
	for node in toRemove:
		nodes.remove(node)
	node_idx = int(random.uniform(0, len(nodes))) if len(nodes) > 0 else None
	node = nodes[node_idx]["Node_id"] if node_idx is not None else None
	return node

def insert_activity(a):
	query = "INSERT INTO \"Preday_Activities\" VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s);"
	data = (a.Activity_Id, a.Activity_Type, a.Primary_Activity, a.Flexible_Activity, a.Location_Id, a.Location_Type, a.Activity_Start_Time, a.Activity_End_Time, a.Mandatory_Activity)
	pg_cursor.execute(query, data)
	print "inserted into Preday_Activities	"

def insert_sub_trip(st):
	query = "INSERT INTO \"Preday_Sub_Trips\" VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s);"
	data = (st.TripId, st.Sub_Trip_Id, st.FromLocationId, st.FromLocationType, st.ToLocationId, st.ToLocationType, st.Mode, st.Primary_Mode_Of_Travel, st.Start_Time, st.sequence_num)
	pg_cursor.execute(query, data)
	print "inserted into Preday_Sub_Trips"

def insert_trip(t):
	query = "INSERT INTO \"Preday_Trips\" VALUES (%s, %s, %s, %s, %s);"
	data = (t.Trip_Id, t.FromLocationId, t.FromLocationType, t.ToLocationId, t.ToLocationType)
	pg_cursor.execute(query, data)
	print "inserted into Preday_Trips"
	for st in t.sub_trips:
		insert_sub_trip(st)

def insert_trip_chain(tc, pid):
	query = "INSERT INTO \"Preday_Trip_Chain\" VALUES (%s, %s, %s, %s, %s);"
	data = None
	for item in tc:
		if isinstance(item, Activity):
			insert_activity(item)
			data = (pid, item.sequence_num, "Activity", 0, item.Activity_Id)
		elif isinstance(item, Trip):
			insert_trip(item)
			data = (pid, item.sequence_num, "Trip", item.Trip_Id, 0)
		pg_cursor.execute(query, data)
		print "inserted into Preday_Trip_Chain"
	conn.commit()	
	print "Committed"
		

#class Activity
class Activity:
	def __init__(self, pid, purpose, is_primary, mode, tour_num, node, arrival, departure, seq_num):
		mode_idx_ref = { 1 : 3, 2 : 5, 3 : 3, 4 : 1, 5 : 6, 6 : 6, 7 : 8, 8 : 2, 9 : 4 }
		self.Activity_Id = pid + "_" + str(tour_num) + "_" + str(seq_num) 
		if purpose == "Work":
			self.Activity_Type = 2
		elif purpose == "Shopping":
			self.Activity_Type = 3
		elif purpose == "Education": 
			self.Activity_Type = 4
		else:
			self.Activity_Type = 9
		self.Primary_Activity = is_primary
		self.Flexible_Activity = True
		self.Location_Id = node
		self.Location_Type = "node"
		self.Activity_Start_Time = getRandomTime(arrival)
		self.Activity_End_Time = getRandomTime(departure)
		self.Mandatory_Activity = True
		self.mode = mode_idx_ref[int(mode)]
		self.tour_num = tour_num
		self.sequence_num = seq_num

	def __str__(self):
		primary_status = 'Primary' if self.Primary_Activity is True else 'Int.stop'
		out = '<' + primary_status + '|' + str(self.Activity_Type) + '|' + str(self.Activity_Start_Time) + ' ' + str(self.Activity_End_Time) + '|Destination: ' + str(self.Location_Id) + '>' 
		return out

#class Sub Trip
class SubTrip:
	def __init__(self, trip_id, fromNode, toNode, mode, is_primary_mode, start_time, seq_num):
		mode_idx_ref = { 1 : 3, 2 : 5, 3 : 3, 4 : 1, 5 : 6, 6 : 6, 7 : 8, 8 : 2, 9 : 4 }
		self.TripId = trip_id
		self.Sub_Trip_Id = trip_id + "_" + str(seq_num) 
		self.FromLocationId = fromNode
		self.FromLocationType = "node"
		self.ToLocationId = toNode
		self.ToLocationType = "node"
		self.Mode = mode_idx_ref[int(mode)]
		self.Primary_Mode_Of_Travel = is_primary_mode
		self.Start_Time = start_time
		self.sequence_num = seq_num
	
#class Trip
class Trip:
	def __init__(self, pid, fromNode, toNode, tour_num, mode, is_primary, start_time, seq_num):
		self.Trip_Id = pid + "_" + str(tour_num) + "_" + str(seq_num) 
		self.FromLocationId = fromNode
		self.FromLocationType = "node"
		self.ToLocationId = toNode
		self.ToLocationType = "node"
		self.tour_num = tour_num
		self.sequence_num = seq_num
		self.sub_trips = []
		subtrip = SubTrip(self.Trip_Id, self.FromLocationId, self.ToLocationId, mode, is_primary, start_time, 1)
		self.sub_trips.append(subtrip)

parser = argparse.ArgumentParser()
parser.add_argument("files", help="JSON files, variables files and outputfile")
args = parser.parse_args()

#Connect to PostgreSQL
try:
	conn = psycopg2.connect("dbname='SimMobility_DB_HITS_TripChains' user='postgres' host='localhost' password='secret'")
	pg_cursor = conn.cursor()
except:
	print "Unable to connect to PostgreSQL"

with open(args.files, 'r') as f:
	files = json.load(f)
	database = files["Database"]
	collection_names = files["Collections"]
	client = MongoClient(database["host"], int(database["port"]))
	db = client[database["db"]]
	person_ids = list(db[collection_names["Persons"]].find())
#	person_ids_test = []
#	for i in range(10):
#		person_ids_test.append(person_ids[i])

	print "Total:", len(person_ids), "persons"
#	for person_id in person_ids_test:
	for person_id in person_ids:
		pid = person_id['_id']
		print "Person:", pid
		p_data = db[collection_names["Person Data"]].find_one({"_id" : pid})
		p_tours = db[collection_names["Person Tours Output"]].find_one({"_id" : pid})
		num_tours = int(p_tours['num_tours']) if p_tours is not None else 0
		trip_chain = []
		seq_num = 0
		prev_node = None
		next_node = None
		prev_dep_time = None
		primary_mode = None
		curr_location = "HOME"
		for tour_num in range(num_tours):
			act_cursor = db[collection_names["Activity Output"]].find({"person_id" : pid , "tour_num" : (tour_num+1)})
			tour = db[collection_names["Tour Output"]].find_one({"person_id" : pid , "tour_num" : (tour_num+1)})
			home_node = getRandomNodeInZone(p_data['home_mtz'])
			for act_doc in act_cursor:
				next_node = getRandomNodeInZone(act_doc['destination'])
				if curr_location == "HOME":	
					# first trip in tour
					seq_num = seq_num + 1
					trip = Trip(pid, home_node, next_node, act_doc['tour_num'], act_doc['stop_mode'], act_doc['primary'], getRandomTime(tour['start_time']), seq_num)
					trip_chain.append(trip)
					curr_location = "OUTSIDE"
				else:
					#subsequent trips
					seq_num = seq_num + 1
					trip = Trip(pid, prev_node, next_node, act_doc['tour_num'], act_doc['stop_mode'], act_doc['primary'], prev_dep_time, seq_num)
					trip_chain.append(trip)
				seq_num = seq_num + 1
				act = Activity(pid, act_doc['stop_type'], act_doc['primary'], act_doc['stop_mode'], act_doc['tour_num'], next_node, act_doc['arrival'], act_doc['departure'], seq_num)
				trip_chain.append(act)
				prev_node = act.Location_Id
				prev_dep_time = act.Activity_End_Time 
			
			#last trip in tour
			seq_num = seq_num + 1
			trip = Trip(pid, prev_node, home_node, act_doc['tour_num'], act_doc['stop_mode'], act_doc['primary'], prev_dep_time, seq_num)
			trip_chain.append(trip)
			curr_location = "HOME"
		
		#insert this person into postgresql
		insert_trip_chain(trip_chain, pid)
			
			
			




