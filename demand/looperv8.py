#!/usr/bin/python
import random
import datetime
import json
import argparse
import csv
import pickle
from collections import OrderedDict
from decimal import Decimal
from pymongo import MongoClient
from multiprocessing import Process, Lock
import dcm
import pdb
import sys
import logging

#############################---global functions and variables---#####################################################################
files = None
client = None
db = None
collection_names = None
output_file = None
choice_idx = None
header_idx = {} #header index map for input data file
lock = Lock()
procs = []

def construct_person_tour_doc(person):
	person_tour = {}
	person_tour["_id"] = person.pid
	person_tour["num_tours"] = len(person.tours)
	person_tour["person_type_id"] = int(person.p_data["person_type_id"])
	return person_tour

def construct_tours_doc(person, tour, tour_ctr):
	tour_doc = {}
	tour_doc["person_id"] = person.pid
	tour_doc["tour_num"] = tour_ctr
	tour_doc["tour_type"] = tour.purpose
	tour_doc["tour_mode"] = tour.mode
	tour_doc["num_stops"] = len(tour.trip_chain) - 1
	tour_doc["start_time"] = tour.start_time
	tour_doc["end_time"] = tour.end_time
	tour_doc["person_type_id"] = int(person.p_data["person_type_id"])
	tour_doc["num_tours"] = len(person.tours)
	prim_act = None
	for act in tour.trip_chain:
		if act.is_primary is True:
			prim_act = act
			break
	tour_doc["prim_arr"] = prim_act.arrival
	tour_doc["prim_dept"] = prim_act.departure
	tour_doc["destination"] = prim_act.destination

	if tour.mode == 8:
		if tour.purpose == "Education" and person.tme_data is not None:
			tour_doc["walk_distance1"] = person.tme_data["walk_distance1"]
			tour_doc["walk_distance2"] = person.tme_data["walk_distance2"]
		elif person.tmw_data is not None and person.tmw_data["destination"] == prim_act.destination:
			tour_doc["walk_distance1"] = person.tmw_data["walk_distance1"]
			tour_doc["walk_distance2"] = person.tmw_data["walk_distance2"]
		elif person.tmd_data is not None:
			zone_id = db.Zone.find_one({"zone_code" : prim_act.destination})["zone_id"]
			tour_doc["walk_distance1"] = person.tmd_data["walk_distance_first_" + str(zone_id)]
			tour_doc["walk_distance2"] = person.tmd_data["walk_distance_second_" + str(zone_id)]
	else:
		tour_doc["walk_distance1"] = 0
		tour_doc["walk_distance2"] = 0

	return tour_doc

def construct_activity_doc(person, tour_ctr, stop_ctr, activity, tour):
	act_doc = {}
	act_doc["person_id"] = person.pid
	act_doc["person_type_id"] = int(person.p_data["person_type_id"])
	act_doc["tour_num"] = tour_ctr
	act_doc["stop_ctr"] = stop_ctr
	act_doc["primary"] = activity.is_primary
	act_doc["stop_type"] = activity.purpose
	act_doc["tour_type"] = tour.purpose
	act_doc["stop_mode"] = activity.mode
	act_doc["tour_mode"] = tour.mode
	act_doc["arrival"] = activity.arrival
	act_doc["departure"] = activity.departure
	act_doc["destination"] = activity.destination
	return act_doc

# returns a random integer in range(0, num_choices)
def monte_carlo_prediction(num_choices):
	if type(num_choices) is not int or num_choices <= 0 : return None
	guess = random.random()
	choice_num = 1.0
	while (choice_num < num_choices):
		if guess <= choice_num/num_choices: 
			break
		choice_num += 1.0
	return int(choice_num)

def contains_window(parent_window, candidate):
	lst = parent_window.split(',')
	pstart = float(lst[0]) if float(lst[0]) > 3.0 else (float(lst[0]) + 24.0)

	pend = float(lst[1]) if float(lst[1]) > 3.0 else (float(lst[1]) + 24.0)

	lst = candidate.split(',')
	cstart = float(lst[0]) if float(lst[0]) > 3.0 else (float(lst[0]) + 24.0)
	cend = float(lst[1]) if float(lst[1]) > 3.0 else (float(lst[1]) + 24.0)

	if (pstart <= cstart and pend >= cend):
		return 1 # True if candidate is contained within parent_window
	else:
		return 0 # False otherwise

def calculate_logsum(model_name):
	mnl = dcm.MultinomialLogit()
	mnl.load_model(dcm.model_file[model_name])
	
	utility = None
	availability = None
	
	#for each person do the following
		#for each alternative do

def predict_day_pattern(person, verbose=False):
	if verbose: print 'predict_day_pattern()'
	day_pattern_index_reference = [ "WorkT", "EduT", "ShopT", "OthersT", "WorkI", "EduI", "ShopI", "OthersI" ] #used for constructing day pattern dictionary 
	mnl = dcm.MultinomialLogit()
	mnl.load_model(dcm.model_file["Day Pattern"]) # load model for Day Pattern
	(utility,probability,availability,final_choice) = mnl.simulate(person.dp_data)
	final_choice = final_choice.split(',')
	# construct day_pattern dictionary
	person.day_pattern = OrderedDict()
	for i in range(len(day_pattern_index_reference)):
		person.day_pattern[day_pattern_index_reference[i]] = int(final_choice[i])

	if verbose: print 'Day Pattern: ', person.day_pattern

	#write out output to mongo for debugging
	utility["person_id"] = person.pid
	probability["person_id"] = person.pid
	availability["person_id"] = person.pid
	utility["final_choice"] = final_choice
	probability["final_choice"] = final_choice
	#logging.info(utility)
	#logging.info(probability)
	#logging.info(availability)

def predict_num_tours(person, verbose=False):
	if verbose: print 'predict_num_tours()'
	mnl = dcm.MultinomialLogit()
	
	# construct num_tours dictionary
	person.num_tours = OrderedDict()

	# number of work tours
	person.num_tours["WorkT"] = 0
	if person.day_pattern["WorkT"] > 0:
		mnl.load_model(dcm.model_file["Number Of Tours Work"]) # Number Of Tours Work
		(utility,probability,availability,final_choice) = mnl.simulate(person.dp_data)
		person.num_tours["WorkT"] = int(final_choice)

		utility["person_id"] = person.pid
		probability["person_id"] = person.pid
		availability["person_id"] = person.pid
		utility["tour_type"] = "Work"
		probability["tour_type"] = "Work"
		availability["tour_type"] = "Work"
		utility["final_choice"] = final_choice
		probability["final_choice"] = final_choice
		#logging.info(utility)
		#logging.info(probability)
		#logging.info(availability)

	# number of Education tours
	person.num_tours["EduT"] = 0
	if person.day_pattern["EduT"] > 0:
		mnl.load_model(dcm.model_file["Number Of Tours Education"]) # load model for Number Of Tours Education
		(utility,probability,availability,final_choice) = mnl.simulate(person.dp_data)
		person.num_tours["EduT"] = int(final_choice)

		utility["person_id"] = person.pid
		probability["person_id"] = person.pid
		availability["person_id"] = person.pid
		utility["tour_type"] = "Education"
		probability["tour_type"] = "Education"
		availability["tour_type"] = "Education"
		utility["final_choice"] = final_choice
		probability["final_choice"] = final_choice
		#logging.info(utility)
		#logging.info(probability)
		#logging.info(availability)

	# number of Shopping tours
	person.num_tours["ShopT"] = 0
	if person.day_pattern["ShopT"] > 0:
		mnl.load_model(dcm.model_file["Number Of Tours Shopping"]) # load model for Number Of Tours Shopping
		(utility,probability,availability,final_choice) = mnl.simulate(person.dp_data)
		person.num_tours["ShopT"] = int(final_choice)
		
		utility["person_id"] = person.pid
		probability["person_id"] = person.pid
		availability["person_id"] = person.pid
		utility["tour_type"] = "Shop"
		probability["tour_type"] = "Shop"
		availability["tour_type"] = "Shop"
		utility["final_choice"] = final_choice
		probability["final_choice"] = final_choice
		#logging.info(utility)
		#logging.info(probability)
		#logging.info(availability)
	
	# number of Others tours
	person.num_tours["OthersT"] = 0
	if person.day_pattern["OthersT"] > 0:
		mnl.load_model(dcm.model_file["Number Of Tours Others"]) # load model for Number Of Tours Others
		(utility,probability,availability,final_choice) = mnl.simulate(person.dp_data)
		person.num_tours["OthersT"] = int(final_choice)

		utility["person_id"] = person.pid
		probability["person_id"] = person.pid
		availability["person_id"] = person.pid
		utility["tour_type"] = "Others"
		probability["tour_type"] = "Others"
		availability["tour_type"] = "Others"
		utility["final_choice"] = final_choice
		probability["final_choice"] = final_choice
		#logging.info(utility)
		#logging.info(probability)
		#logging.info(availability)

	if verbose: print 'Number of Tours:', person.num_tours

def predict_usual_location(person, verbose=False):
	if verbose: print 'predict_usual_location()'
	mnl = dcm.MultinomialLogit()
	mnl.load_model(dcm.model_file["Attend Usual Work"]) # load model for Number Of Tours Others
	(utility,probability,availability,final_choice) = mnl.simulate(person.uw_data)
	attends_usual_work_location = True  if final_choice == "Attend" else False
	if verbose: print 'Attends usual work location:' , attends_usual_work_location
	return attends_usual_work_location

def predict_mode(person, tour, verbose=False):
	# 1 = public bus  
	# 2 = MRT
	# 3 = private bus
	# 4 = drive alone
	# 5 = shared2
	# 6 = shared3+
	# 7 = motor
	# 8 = walk
	# 9 = taxi
	mode_idx_ref = { "Walk": 8, "Taxi": 9,  "Auto": 4, "Share 2+" : 5, "Share 3+" : 6, "Motor": 7, "Public bus": 1, "MRT": 2, "Private bus": 3}
	if verbose: print 'predict_mode()'
	data = None
	mnl = dcm.MultinomialLogit()
	if tour.purpose == "Education":
		mnl.load_model(dcm.model_file["Tour Mode Education"]) 
		data = person.tme_data
		tour.destination = person.p_data["school_location_mtz"]
	elif tour.purpose == "Work":
		mnl.load_model(dcm.model_file["Tour Mode Work"]) 
		data = person.tmw_data
		tour.destination = person.p_data["fix_work_location_mtz"]
	else:
		raise RuntimeError("predict_mode() should be called only for (usual) work or education tours")

	(utility,probability,availability,final_choice) = mnl.simulate(data)
	tour.mode = mode_idx_ref[final_choice]
	if verbose: print 'mode:', tour.mode, 'destination:', tour.destination

def predict_mode_destination_dummy(person, tour_or_activity, verbose=False):
	if verbose: print 'predict_mode_destination_dummy()'
	mode_idx_ref = { "walk": 8, "taxi": 9,  "drive1": 4, "share2" : 5, "share3" : 6, "motor": 7, "bus": 1, "mrt": 2, "private_bus": 3}
	tour_or_activity.mode = random.choice([1,2,3,4,5,6,7,8,9])
	tour_or_activity.destination = db[collection_names["Zone"]].find_one({"zone_id" : int(random.uniform(1, 1092))}, {"zone_code" : 1, "_id":0})["zone_code"]
	if verbose: print 'tour_or_activity:',tour_or_activity

def predict_mode_destination(person, tour_or_activity, verbose=False):
	#if verbose: print '\npredict_mode_destination()'
	# 1 = public bus  
	# 2 = MRT
	# 3 = private bus
	# 4 = drive alone
	# 5 = shared2
	# 6 = shared3+
	# 7 = motor
	# 8 = walk
	# 9 = taxi
	mode_idx_ref = { "bus": 1, "mrt": 2, "private_bus": 3, "drive1": 4, "share2" : 5, "share3" : 6, "motor": 7, "walk": 8, "taxi": 9 }
	if verbose: print 'predict_mode_destination()'
	mnl = dcm.MultinomialLogit()
	if tour_or_activity.__class__.__name__ is "Activity":
		mnl.load_model(dcm.model_file["Intermediate Stop Mode/Destination"])
		tour_mode = tour_or_activity.parent_tour.mode
		available_modes_for_stop = None
		if tour_mode == 1 or tour_mode == 2:
			available_modes_for_stop = ["bus", "mrt", "drive1", "share2", "share3", "motor", "walk", "taxi"]
		elif tour_mode == 3:
			available_modes_for_stop = ["bus", "mrt", "private_bus", "drive1", "share2", "share3", "motor", "walk", "taxi"]
		elif tour_mode == 4:
			available_modes_for_stop = ["drive1", "motor", "walk", "taxi"]
		elif tour_mode == 5:
			available_modes_for_stop = ["drive1", "share2", "motor", "walk", "taxi"]
		elif tour_mode == 6:
			available_modes_for_stop = ["drive1", "share2", "share3", "motor", "walk", "taxi"]
		elif tour_mode == 7:
			available_modes_for_stop = ["motor", "walk"]
		elif tour_mode == 8:
			available_modes_for_stop = ["walk"]	
		elif tour_mode == 9:
			available_modes_for_stop = ["motor", "walk", "taxi"]	
		for choice in mnl.Choiceset:
			if choice.split(',')[0] in available_modes_for_stop:
				mnl.Availability[choice] == 1
			else:
				mnl.Availability[choice] == 0

	elif tour_or_activity.purpose == "Work":
		mnl.load_model(dcm.model_file["Tour Mode/Destination Work"])
	elif tour_or_activity.purpose == "Shopping":
		mnl.load_model(dcm.model_file["Tour Mode/Destination Shopping"]) 
	elif tour_or_activity.purpose == "Others":
		mnl.load_model(dcm.model_file["Tour Mode/Destination Others"])
	elif tour_or_activity.purpose == "Education" and tour_or_activity.__class__.__name__ is not "Activity":
		raise RuntimeError("predict_mode_destination() should be called only for (unusual) work, Shopping or Other purposes")

	stop_ref_idx = ['Work', 'Education', 'Shopping', 'Others']
	person.tmd_data["stop_type"] = stop_ref_idx.index(tour_or_activity.purpose) + 1

	(utility,probability,availability,final_choice) = mnl.simulate(person.tmd_data)
	final_choice = final_choice.split(',')
	tour_or_activity.mode = mode_idx_ref[final_choice[0]]
	tour_or_activity.destination = int(final_choice[1])
	if verbose: print 'mode:', tour_or_activity.mode, 'destination:', tour_or_activity.destination

def predict_work_based_sub_tours(person, tour, verbose=False):
	if verbose: print 'predict_work_based_sub_tours()'
	mnl = dcm.MultinomialLogit()
	mnl.load_model(dcm.model_file["Work Based Sub-Tours"]) # load model for Work Based Sub-Tours
	current_tour = tour
	next_tour = None
	ctr = 0
	(utility,probability,availability,final_choice) = mnl.simulate(person.data)

	while (final_choice != "Quit" and ctr < 3):
		ctr = ctr + 1
		next_tour = Tour(person, final_choice, False, True, tour)
		person.tours.insert(person.tours.index(current_tour) + 1, next_tour)
		current_tour = next_tour
		(utility,probability,availability,final_choice) = mnl.simulate(person.data)

def predict_tour_time_of_day(person, tour, initialized_available_time_windows, verbose=False):
	if verbose: print 'predict_tour_time_of_day()'
	mnl = dcm.MultinomialLogit()
	if tour.purpose == "Work":
		mnl.load_model(dcm.model_file["Tour Time Of Day Work"]) # load model for Tour Time Of Day Work
	elif tour.purpose == "Education":
		mnl.load_model(dcm.model_file["Tour Time Of Day Education"]) # load model for Tour Time Of Day Education
	elif tour.purpose == "Others":
		mnl.load_model(dcm.model_file["Tour Time Of Day Others"]) # load model for Tour Time Of Day Others
	elif tour.purpose == "Shopping":
		mnl.load_model(dcm.model_file["Tour Time Of Day Others"]) # load model for Tour Time Of Day Others for shopping as well
	else:
		raise RuntimeError("Invalid tour purpose: " + tour.purpose)
	
	if initialized_available_time_windows is False:
		person.available_time_windows = mnl.Choiceset

	if tour.is_sub_tour is False:
		for utw in person.unavailable_time_windows: #turn-off availability of time windows taken by other tours
			mnl.Availability[utw] = 0

		origin = tour.destination
		destination = person.p_data['home_mtz']
		
		if(origin != destination):
			#Mode should be determined from mode or mode destination models
			tcostBus = db.tcost_bus.find_one({'origin': origin, 'destination': destination})
			tcostCar = db.tcost_car.find_one({'origin': origin, 'destination': destination})
			am_dist = db.AMCosts.find_one({'origin': destination, 'destin': origin})
			pm_dist = db.PMCosts.find_one({'origin': destination, 'destin': origin})
			distance_min = float(am_dist['distance']) - float(pm_dist['distance'])
			for i in range(1,49):
				if (tour.mode in [4,5,6,7,9] and person.itd_data["first_bound"] == 1):
					person.itd_data['TT_HT1_%s'%i] = tcostCar['TT_car_arrival_%s'%i] if tcostCar['TT_car_arrival_%s'%i] != "NULL" else 999
				elif (tour.mode in [4,5,6,7,9] and person.itd_data["first_bound"] == 0):
					person.itd_data['TT_HT2_%s'%i] = tcostCar['TT_car_departure_%s'%i] if tcostCar['TT_car_departure_%s'%i] != "NULL" else 999
				elif (tour.mode <= 3 and person.itd_data["first_bound"] == 1):
					person.itd_data['TT_HT1_%s'%i] = tcostBus['TT_bus_arrival_%s'%i] if tcostBus['TT_bus_arrival_%s'%i] != "NULL" else 999
				elif (tour.mode <= 3 and person.itd_data["first_bound"] == 0):
					person.itd_data['TT_HT2_%s'%i] = tcostBus['TT_bus_departure_%s'%i] if tcostBus['TT_bus_departure_%s'%i] != "NULL" else 999
				elif (tour.mode == 8):
					person.itd_data['TT_HT1_%s'%i] = distance_min/5
					person.itd_data['TT_HT2_%s'%i] = distance_min/5
		else: #if origin == destination
			for i in range(1,49):
				person.itd_data['TT_HT1_%s'%i] = 0
				person.itd_data['TT_HT2_%s'%i] = 0

		(utility,probability,availability,final_choice) = mnl.simulate(person.ttd_data)
		tour.take_time_window(final_choice)
		if verbose: print final_choice
		return final_choice

# 	must revisit and edit this section when work-based sub tours are activated
#	else: #if tour.is_sub_tour is True
#		parent_tour = tour.parent_tour
#		primary_activity_parent_tour = None
#		for n,act in enumerate(parent_tour.trip_chain):
#			if act.is_primary is True:
#				primary_activity_parent_tour = act
#
#		for wndw, avail in mnl.Availability: #only time contained within the duration of primary_activity_parent_tour is available
#			mnl.Availability[wndw] = contains_window(primary_activity_parent_tour.time_window, wndw)
#
#		for wndw in primary_activity_parent_tour.unavailable_time_windows: # remove the contained time windows which are already taken
#			mnl.Availability[wndw] = 0
#
#		(utility,probability,availability,final_choice) = mnl.simulate(person.data)
#		primary_activity_parent_tour.take_time_window(final_choice)
#		if verbose: print 'final_choice:', final_choice
#		return final_choice

def generate_intermediate_activities(person, tour, verbose=False):
	if verbose: print 'generate_intermediate_stops()'
	mnl = dcm.MultinomialLogit()

	try:
		primary_activity = tour.trip_chain[0] # the only activity at this point is the primary activity
	except IndexError:
		raise RuntimeError('Primary activity was not generated')

	generated_stop = None
	mnl.load_model(dcm.model_file["Intermediate Stop Generation"]) # load model for Intermediate Stop Generation

	unavailable_stop_types_cnt =0
	if person.day_pattern['WorkI'] == 0: 
		mnl.Availability['Work'] = 0 
		unavailable_stop_types_cnt = unavailable_stop_types_cnt + 1
	if person.day_pattern['EduI'] == 0: 
		mnl.Availability['Education'] = 0 
		unavailable_stop_types_cnt = unavailable_stop_types_cnt + 1
	if person.day_pattern['ShopI'] == 0: 
		mnl.Availability['Shopping'] = 0 
		unavailable_stop_types_cnt = unavailable_stop_types_cnt + 1
	if person.day_pattern['OthersI'] == 0: 
		mnl.Availability['Others'] = 0 
		unavailable_stop_types_cnt = unavailable_stop_types_cnt + 1

	if unavailable_stop_types_cnt == 4:
		return

	#Tour type will be obtained form exact no.of tours model
	tour_type_ref_idx = ['Work', 'Education', 'Shopping', 'Others']
	person.isg_data["tour_type"] = tour_type_ref_idx.index(tour.purpose) + 1

	#Mode should be determined from mode or mode destination models
	if tour.mode == 4:
		person.isg_data["driver_dummy"] = 1
	else:
		person.isg_data["driver_dummy"] = 0

	if tour.mode in [5,6]:
		person.isg_data["passenger_dummy"] = 1
	else:
		person.isg_data["passenger_dummy"] = 0

	if tour.mode in [1,2,3]:
		person.isg_data["public_dummy"] = 1
	else:
		person.isg_data["public_dummy"] = 0

	origin = person.p_data["home_mtz"]
	destination = primary_activity.destination
	arrivalTime = primary_activity.arrival #arrival time of tour primary activity
	deptureTime = primary_activity.departure #departure time of tour primary activity 
	
	person.isg_data["first_tour_dummy"] = 1 if person.tours.index(tour) == 0 else 0
	person.isg_data["tour_remain"] = len(person.tours) - (person.tours.index(tour) + 1)

	#First half
	person.isg_data["distance"] = db[collection_names["AMCosts"]].find_one({'origin': destination, 'destin': origin}, {'distance': 1, '_id': 0})['distance'] if origin != destination else 0
	person.isg_data["p_700a_930a"] = 1 if arrivalTime > 7 and arrivalTime <= 9.5 else 0
	person.isg_data["p_930a_1200a"] = 1 if arrivalTime > 9.5 and arrivalTime <= 12 else 0
	person.isg_data["p_300p_530p"] = 1 if arrivalTime > 15 and arrivalTime <= 17.5 else 0
	person.isg_data["p_530p_730p"] = 1 if arrivalTime > 17.5 and arrivalTime <= 19.5 else 0
	person.isg_data["p_730p_1000p"] = 1 if arrivalTime > 19.5 and arrivalTime <= 22 else 0
	person.isg_data["p_1000p_700a"] = 1 if (arrivalTime > 22 and arrivalTime <= 27) or (arrivalTime > 0 and arrivalTime <= 7) else 0

	person.isg_data["first_bound"] = 1
	person.isg_data["second_bound"] = 0
	
	prev_idx = person.tours.index(tour) - 1
	prev_departure = float(person.tours[prev_idx].end_time) if prev_idx >= 0 else 3.25 #end time of the previous tour
	next_arrival = primary_activity.arrival # arrival time of the primary activity

	ctr = 0
	final_choice = None
	insert_idx = 0
	while (final_choice != "Quit" and ctr < 3): 
#		person.isg_data["time_window_h"] = (next_arrival - prev_departure) if (next_arrival - prev_departure) <= 1.5 else 1.5
		person.isg_data["first_stop"] = 1 if ctr == 0 else 0 
		person.isg_data["second_stop"] = 1 if ctr == 1 else 0
		person.isg_data["three_plus_stop"] = 1 if ctr >= 2 else 0
		(utility,probability,availability,final_choice) = mnl.simulate(person.isg_data)
		if final_choice != "Quit":
			generated_stop = Activity(tour, final_choice, False)
			tour.insert_trip_chain_item(insert_idx, generated_stop)
			predict_mode_destination(person, generated_stop) #Stop mode destination
			next_activity = tour.trip_chain[tour.trip_chain.index(generated_stop) + 1]
			estimate_departure_time(generated_stop, next_activity)
			if generated_stop.departure <= 3.25: 
				tour.trip_chain.remove(generated_stop)
				if verbose: print 'Invalid departure time for stop. Removed this stop from trip_chain of tour'
				ctr = ctr + 1
				continue
			predict_stop_time_of_day(person, generated_stop, True) #stop time of day
			tour.take_time_window(str(generated_stop.arrival) + "," + str(generated_stop.departure))
			next_arrival = float(generated_stop.arrival) # arrival time of the generated stop is the next arrival for the subsequent stop
		ctr = ctr + 1

	#Second half
	person.isg_data["distance"] = db[collection_names["PMCosts"]].find_one({'origin': destination, 'destin': origin}, {'distance': 1, '_id': 0})['distance'] if origin != destination else 0
	person.isg_data["p_700a_930a"] = 1 if deptureTime > 7 and deptureTime <= 9.5 else 0
	person.isg_data["p_930a_1200a"] = 1 if deptureTime > 9.5 and deptureTime <= 12 else 0
	person.isg_data["p_300p_530p"] = 1 if deptureTime > 15 and deptureTime <= 17.5 else 0
	person.isg_data["p_530p_730p"] = 1 if deptureTime > 17.5 and deptureTime <= 19.5 else 0
	person.isg_data["p_730p_1000p"] = 1 if deptureTime > 19.5 and deptureTime <= 22 else 0
	person.isg_data["p_1000p_700a"] = 1 if (deptureTime > 22 and deptureTime <= 27) or (deptureTime > 0 and deptureTime <= 7) else 0

	person.isg_data["first_bound"] = 0
	person.isg_data["second_bound"] = 1

	next_idx = person.tours.index(tour) + 1
	prev_departure = float(primary_activity.departure) # departure time of the primary activity 
	next_arrival = 26.75 

	ctr = 0
	final_choice = None
	insert_idx = len(tour.trip_chain)
	while (final_choice != "Quit" and ctr < 3): 
#		person.isg_data["time_window_h"] = (next_arrival - prev_departure) if (next_arrival - prev_departure) <= 1.5 else 1.5
		person.isg_data["first_stop"] = 1 if ctr == 0 else 0
		person.isg_data["second_stop"] = 1 if ctr == 1 else 0
		person.isg_data["three_plus_stop"] = 1 if ctr >= 2 else 0
		(utility,probability,availability,final_choice) = mnl.simulate(person.isg_data)
		if final_choice != "Quit":
			generated_stop = Activity(tour, final_choice, False)
			tour.insert_trip_chain_item(insert_idx, generated_stop)
			insert_idx = insert_idx + 1
			predict_mode_destination(person, generated_stop) #Stop mode destination
			prev_activity = tour.trip_chain[tour.trip_chain.index(generated_stop) - 1]
			estimate_arrival_time(generated_stop, prev_activity)
			if generated_stop.arrival >= 26.75: 
				tour.trip_chain.remove(generated_stop)
				if verbose: print 'Invalid arrival time for stop. Removed this stop from trip_chain of tour'
				ctr = ctr + 1
				continue
			predict_stop_time_of_day(person, generated_stop, False) #stop time of day
			tour.take_time_window(str(generated_stop.arrival) + "," + str(generated_stop.departure))
			prev_departure = float(generated_stop.departure) # departure time of the generated stop is the previous departure for the subsequent stop
		ctr = ctr + 1

def predict_stop_time_of_day(person, activity, is_before_primary, verbose=False):
	if verbose: print 'predict_stop_time_of_day()'
	mnl = dcm.MultinomialLogit()
	mnl.load_model(dcm.model_file["Intermediate Stop Time Of Day"]) # load model for Intermediate Stop Generation

	for utw in person.unavailable_time_windows: # mark availability
		mnl.Availability[utw] = 0
	
	stop_ref_idx = ['Work', 'Education', 'Shopping', 'Others']
	person.itd_data["stop_type"] = stop_ref_idx.index(activity.purpose) + 1
	person.itd_data["first_bound"] = int(is_before_primary)
	person.itd_data["second_bound"] = int(not is_before_primary)
	origin = activity.destination
	destination = person.p_data['home_mtz']
	
	if origin != destination:
		#Mode should be determined from mode or mode destination models
		tcostBus = db.tcost_bus.find_one({'origin': origin, 'destination': destination})
		tcostCar = db.tcost_car.find_one({'origin': origin, 'destination': destination})
		am_dist = db.AMCosts.find_one({'origin': destination, 'destin': origin})
		pm_dist = db.PMCosts.find_one({'origin': destination, 'destin': origin})
		distance_min = float(am_dist['distance']) - float(pm_dist['distance'])
		for i in range(1,49):
			if (activity.mode in [4,5,6,7,9] and person.itd_data["first_bound"] == 1):
				person.itd_data['TT_%s'%i] = tcostCar['TT_car_arrival_%s'%i] if tcostCar['TT_car_arrival_%s'%i] != "NULL" else 999
			elif (activity.mode in [4,5,6,7,9] and person.itd_data["first_bound"] == 0):
				person.itd_data['TT_%s'%i] = tcostCar['TT_car_departure_%s'%i] if tcostCar['TT_car_departure_%s'%i] != "NULL" else 999
			elif (activity.mode <= 3 and person.itd_data["first_bound"] == 1):
				person.itd_data['TT_%s'%i] = tcostBus['TT_bus_arrival_%s'%i] if tcostBus['TT_bus_arrival_%s'%i] != "NULL" else 999
			elif (activity.mode <= 3 and person.itd_data["first_bound"] == 0):
				person.itd_data['TT_%s'%i] = tcostBus['TT_bus_departure_%s'%i] if tcostBus['TT_bus_departure_%s'%i] != "NULL" else 999
			elif (activity.mode == 8):
				person.itd_data['TT_%s'%i] = distance_min/5
	else: #if origin == destination
		for i in range(1,49):
			person.itd_data['TT_%s'%i] = 0
			
	#calculation of high_tod and low_tod
	if verbose: print 'activity.departure:', activity.departure, 'activity.arrival:', activity.arrival
	if is_before_primary is True:
		person.itd_data["high_tod"] = activity.departure
		curr_tour_idx = person.tours.index(activity.parent_tour)
		person.itd_data["low_tod"] = 3.25 if curr_tour_idx == 0 else person.tours[curr_tour_idx - 1].end_time
	else:
		person.itd_data["low_tod"] = activity.arrival
		person.itd_data["high_tod"] = 26.75

	#calculating cost
	cost = {}

	am = db.AMCosts.find_one({'origin': origin, 'destin': destination})
	pm = db.PMCosts.find_one({'origin': origin, 'destin': destination})
	op = db.OPCosts.find_one({'origin': origin, 'destin': destination})
	zone_origin = db.Zone.find_one({'zone_code': origin})

	am_alternative=range(10,14)
	pm_alternative=range(30,34)
	op_alternative=range(1,10)+range(14,30)+range(34,49)
	for i in range(1,49):
		if activity.mode in [4,5,6,7,9]:
			if person.itd_data["first_bound"] == 1:
				duration=person.itd_data["high_tod"]-i+1
			elif person.itd_data["second_bound"] == 1:
				duration = i-person.itd_data["low_tod"]+1
			duration=0.25+(duration-1)*0.5
			parking_rate=zone_origin['parking_rate']
			cost_car_parking=(8*(duration>8)+duration*(duration<=8))*parking_rate
			if i in am_alternative:
				cost_car_ERP=am['car_cost_erp']
				cost_car_OP=(am['distance'])*0.147
				walk_distance=am['distance']
			elif i in pm_alternative:
				cost_car_ERP=pm['car_cost_erp']
				cost_car_OP=(pm['distance'])*0.147
				walk_distance=pm['distance']
			else:
				cost_car_ERP=op['car_cost_erp']
				cost_car_OP=(op['distance'])*0.147
				walk_distance=op['distance']
			if activity.mode in [4,5,6]:#drive1 shared2 shared3
				cost[i-1]=(cost_car_parking+cost_car_OP+cost_car_ERP)/(activity.mode-3.0)
			elif activity.mode==7:#motor
				cost[i-1]=0.5*cost_car_ERP+0.5*cost_car_OP+0.65*cost_car_parking
			else:#taxi
				central_dummy = person.p_data["female_dummy"]
				cost_taxi=3.4+cost_car_ERP+3*central_dummy+((walk_distance*(walk_distance>10)-10*(walk_distance>10))/0.35+(walk_distance*(walk_distance<=10)+10*(walk_distance>10))/0.4)*0.22
				cost[i-1]=cost_taxi
		elif activity.mode in [1,2,3]:
			if i in am_alternative:
				cost[i-1]=am['pub_cost']
			elif i in pm_alternative:
				cost[i-1]=pm['pub_cost']
			else:
				cost[i-1]=op['pub_cost']
		elif activity.mode == 8:
			cost[i-1]=0

	for i in range(1,49):
		person.itd_data["cost_%s"%i] = cost[i-1]

	for i in mnl.Choiceset:
		if float(i) <= person.itd_data["low_tod"] or float(i) >= person.itd_data["high_tod"]:
			mnl.Availability[i] = 0

	(utility,probability,availability,final_choice) = mnl.simulate(person.itd_data)
	if is_before_primary is True:
		activity.arrival = float(final_choice)
	else:
		activity.departure = float(final_choice)
	if verbose: print 'activity', activity

def estimate_departure_time(curr_activity, next_activity, verbose=False):
	if verbose: print 'estimate_departure_time()'
	twnd_idx_ref = [3.25,3.75,4.25,4.75,5.25,5.75,6.25,6.75,7.25,7.75,8.25,8.75,9.25,9.75,10.25,10.75,11.25,11.75,\
12.25,12.75,13.25,13.75,14.25,14.75,15.25,15.75,16.25,16.75,17.25,17.75,18.25,18.75,19.25,19.75,20.25,20.75,21.25,\
21.75,22.25,22.75,23.25,23.75,24.25,24.75,25.25,25.75,26.25,26.75]
	nxt_act_arr_idx = twnd_idx_ref.index(next_activity.arrival)+1
	if curr_activity.destination != next_activity.destination:
		if (next_activity.mode in [4,5,6,7,9]):
			doc = db[collection_names["Travel Cost Car"]].find_one({'origin': curr_activity.destination, 'destination': next_activity.destination})
			tt =  float(doc['TT_car_arrival_%s'%nxt_act_arr_idx]) if doc['TT_car_arrival_%s'%nxt_act_arr_idx] != "NULL" else 999
		elif (next_activity.mode <= 3):
			doc = db[collection_names["Travel Cost Bus"]].find_one({'origin': curr_activity.destination, 'destination': next_activity.destination})
			tt = float(doc['TT_bus_arrival_%s'%nxt_act_arr_idx]) if doc['TT_bus_arrival_%s'%nxt_act_arr_idx] != "NULL" else 999
		elif (next_activity.mode == 8):
			am_dist = db[collection_names["AMCosts"]].find_one({'origin': curr_activity.destination, 'destin': next_activity.destination})
			pm_dist = db[collection_names["PMCosts"]].find_one({'origin': curr_activity.destination, 'destin': next_activity.destination})
			distance_min = float(am_dist['distance']) - float(pm_dist['distance'])
			tt = distance_min/5
	else:
		tt = 0
	curr_activity.departure = next_activity.arrival - tt
	curr_activity.departure = int(curr_activity.departure) + 0.25 if int(curr_activity.departure) < (int(curr_activity.departure) + 0.5) else int(curr_activity.departure) + 0.75 
	if verbose: print 'curr_activity', curr_activity

def estimate_arrival_time(curr_activity, prev_activity, verbose=False):
	if verbose: print 'estimate_arrival_time()'
	twnd_idx_ref = [3.25,3.75,4.25,4.75,5.25,5.75,6.25,6.75,7.25,7.75,8.25,8.75,9.25,9.75,10.25,10.75,11.25,11.75,\
12.25,12.75,13.25,13.75,14.25,14.75,15.25,15.75,16.25,16.75,17.25,17.75,18.25,18.75,19.25,19.75,20.25,20.75,21.25,\
21.75,22.25,22.75,23.25,23.75,24.25,24.75,25.25,25.75,26.25,26.75]
	prev_act_arr_idx = twnd_idx_ref.index(prev_activity.departure)+1
	if curr_activity.destination != prev_activity.destination:
		if (prev_activity.mode in [4,5,6,7,9]):
			doc = db[collection_names["Travel Cost Car"]].find_one({'origin': curr_activity.destination, 'destination': prev_activity.destination})
			tt =  float(doc['TT_car_departure_%s'%prev_act_arr_idx]) if doc['TT_car_departure_%s'%prev_act_arr_idx] != "NULL" else 999
		elif (prev_activity.mode <= 3):
			doc = db[collection_names["Travel Cost Bus"]].find_one({'origin': curr_activity.destination, 'destination': prev_activity.destination})
			tt = float(doc['TT_bus_departure_%s'%prev_act_arr_idx]) if doc['TT_bus_departure_%s'%prev_act_arr_idx] != "NULL" else 999
		elif (prev_activity.mode == 8):
			am_dist = db[collection_names["AMCosts"]].find_one({'origin': curr_activity.destination, 'destin': prev_activity.destination})
			pm_dist = db[collection_names["PMCosts"]].find_one({'origin': curr_activity.destination, 'destin': prev_activity.destination})
			distance_min = float(am_dist['distance']) - float(pm_dist['distance'])
			tt = distance_min/5
	else:
		tt = 0
	curr_activity.arrival = prev_activity.departure + tt
	curr_activity.arrival = int(curr_activity.arrival) + 0.25 if int(curr_activity.arrival) < (int(curr_activity.arrival) + 0.5) else int(curr_activity.arrival) + 0.75 
	if verbose: print 'curr_activity', curr_activity

def estimate_tour_start_time(person, tour, verbose=False):
	if verbose: print 'estimate_tour_start_time()'
	twnd_idx_ref = [3.25,3.75,4.25,4.75,5.25,5.75,6.25,6.75,7.25,7.75,8.25,8.75,9.25,9.75,10.25,10.75,11.25,11.75,\
12.25,12.75,13.25,13.75,14.25,14.75,15.25,15.75,16.25,16.75,17.25,17.75,18.25,18.75,19.25,19.75,20.25,20.75,21.25,\
21.75,22.25,22.75,23.25,23.75,24.25,24.75,25.25,25.75,26.25,26.75]
	first_activity = tour.trip_chain[0]
	first_act_arr_idx = twnd_idx_ref.index(first_activity.arrival)+1
	if int(person.p_data["home_mtz"]) != first_activity.destination:
		if (first_activity.mode in [4,5,6,7,9]):
			doc = db[collection_names["Travel Cost Car"]].find_one({'origin': int(person.p_data["home_mtz"]), 'destination': first_activity.destination})
			tt =  float(doc['TT_car_arrival_%s'%first_act_arr_idx]) if doc['TT_car_arrival_%s'%first_act_arr_idx] != "NULL" else 999
		elif (first_activity.mode <= 3):
			doc = db[collection_names["Travel Cost Bus"]].find_one({'origin': int(person.p_data["home_mtz"]), 'destination': first_activity.destination})
			tt = float(doc['TT_bus_arrival_%s'%first_act_arr_idx]) if doc['TT_bus_arrival_%s'%first_act_arr_idx] != "NULL" else 999
		elif (first_activity.mode == 8):
			am_dist = db[collection_names["AMCosts"]].find_one({'origin': int(person.p_data["home_mtz"]), 'destin': first_activity.destination})
			pm_dist = db[collection_names["PMCosts"]].find_one({'origin': int(person.p_data["home_mtz"]), 'destin': first_activity.destination})
			distance_min = float(am_dist['distance']) - float(pm_dist['distance'])
			tt = distance_min/5
	else: 
		tt = 0

	tour.start_time = first_activity.arrival - tt
	tour.start_time = int(tour.start_time) + 0.25 if tour.start_time < (tour.start_time + 0.5) else int(tour.start_time) + 0.75
	tour.start_time = tour.start_time if tour.start_time >= 3.25 else 3.25 

def estimate_tour_end_time(person, tour, verbose=False):
	if verbose: print 'estimate_tour_end_time()'
	twnd_idx_ref = [3.25,3.75,4.25,4.75,5.25,5.75,6.25,6.75,7.25,7.75,8.25,8.75,9.25,9.75,10.25,10.75,11.25,11.75,\
12.25,12.75,13.25,13.75,14.25,14.75,15.25,15.75,16.25,16.75,17.25,17.75,18.25,18.75,19.25,19.75,20.25,20.75,21.25,\
21.75,22.25,22.75,23.25,23.75,24.25,24.75,25.25,25.75,26.25,26.75]
	last_activity = tour.trip_chain[len(tour.trip_chain) -1]
	last_act_arr_idx = twnd_idx_ref.index(last_activity.departure)+1

	if int(person.p_data["home_mtz"]) != last_activity.destination:
		if (last_activity.mode in [4,5,6,7,9]):
			doc = db[collection_names["Travel Cost Car"]].find_one({'origin': int(person.p_data["home_mtz"]), 'destination': last_activity.destination})
			tt =  float(doc['TT_car_departure_%s'%last_act_arr_idx]) if doc['TT_car_departure_%s'%last_act_arr_idx] != "NULL" else 999
		elif (last_activity.mode <= 3):
			doc = db[collection_names["Travel Cost Bus"]].find_one({'origin': int(person.p_data["home_mtz"]), 'destination': last_activity.destination})
			tt = float(doc['TT_bus_departure_%s'%last_act_arr_idx]) if doc['TT_bus_departure_%s'%last_act_arr_idx] != "NULL" else 999
		elif (last_activity.mode == 8):
			am_dist = db[collection_names["AMCosts"]].find_one({'origin': int(person.p_data["home_mtz"]), 'destin': last_activity.destination})
			pm_dist = db[collection_names["PMCosts"]].find_one({'origin': int(person.p_data["home_mtz"]), 'destin': last_activity.destination})
			distance_min = float(am_dist['distance']) - float(pm_dist['distance'])
			tt = distance_min/5
	else: 
		tt = 0

	tour.end_time = last_activity.departure + tt
	tour.end_time = int(tour.end_time) + 0.25 if tour.end_time < (int(tour.end_time) + 0.5) else int(tour.end_time) + 0.75
	tour.end_time = tour.end_time if tour.end_time <= 26.75 else 26.75 
	
	
#############################---Class Time Window---#####################################################################
class TimeWindow:
	# defining __str__ for pretty printing
	def __str__(self):
		string = '['
		if self.start is not None: 
			string  = string + ' Arrival:' + self.start.strftime("%H:%M") 
		if self.end is not None:
			string = string + ' Departure:' + self.end.strftime("%H:%M")
		string = string + ' ]'
		return string
	
	# constructor
	def __init__(self, start_hour = None, start_minute = None, end_hour = None, end_minute = None):
		self.start = None
		self.end = None
		if type(start_hour) is int and start_hour in range(0,24) and type(start_minute) is int and start_minute in range(0,60):
			if start_hour >= 3 : self.start = datetime.datetime(1900,1,1,start_hour,start_minute)
			else: self.start = datetime.datetime(1900,1,2,start_hour,start_minute)
		if type(end_hour) is int and end_hour in range(0,24) and type(end_minute) is int and end_minute in range(0,60):
			if end_hour >= 3 : self.end = datetime.datetime(1900,1,1,end_hour,end_minute)
			else: self.end = datetime.datetime(1900,1,2,end_hour,end_minute)
		if self.start is not None and self.end is not None and self.start > self.end: 
			error =  'TimeWindow: start time is greater than end time: [' + str(self.start) +','+  str(self.end) + ']'
			raise RuntimeError(error)
		if self.start is None and self.end is None:
			error =  'TimeWindow constructor called incorrectly: atleast one of start and end must be provided'
			raise RuntimeError(error)
		
#############################---Class Tour---#########################################################################
class Tour:
	def __init__(self, person, purpose, usual_loc = False, is_sub_tour = False, parent_tour = None):
		person.tour_ctr = person.tour_ctr + 1
		self.tour_num = person.tour_ctr
		self.person = person
		self.purpose = purpose
		self.usual_loc = usual_loc
		self.is_sub_tour = is_sub_tour
		self.parent_tour = parent_tour
		self.trip_chain = []
		self.mode = None
		self.destination = None
		self.start_time = None
		self.end_time = None

	def __str__(self):
		tour_type = 'sub-tour' if self.is_sub_tour is True else 'tour'
		out = '[' + str(self.purpose) + '|' + tour_type  + '|start ' + str(self.start_time) 
		for item in self.trip_chain:
			out = out + '|' + str(item)
		out = out + '|end ' + str(self.end_time) + ']'
		return out

	def add_trip_chain_item(self, tci):
		self.trip_chain.append(tci)

	def insert_trip_chain_item(self, index, tci):
		self.trip_chain.insert(index, tci)

	def take_time_window(self, tw):
		tw = tw.split(',')
		self.person.block_time(float(tw[0]), float(tw[1]))
		

#############################---- class Trip-----#######################################################################
#currently not used
class Trip:
	def __init__(self, parent_tour):
		self.parent_tour = parent_tour
		self.mode = None
		self.origin = None
		self.destination = None

#############################---Class Activity---#######################################################################
class Activity:
	def __init__(self, parent_tour, purpose, is_primary, time_window = None):
		self.parent_tour = parent_tour
		self.purpose = purpose
		self.is_primary = is_primary
		self.time_window = time_window
		self.arrival = None
		self.departure = None
		self.set_time_window(time_window)
		self.mode = None
		self.destination = None
 
	def __str__(self):
		primary_status = 'Primary' if self.is_primary is True else 'Int.stop'
		out = '<' + primary_status + '|' + str(self.purpose) + '|' + str(self.arrival) + ' ' + str(self.departure) + '|Mode: ' + str(self.mode) + '|Destination: ' + str(self.destination) + '>' 
		return out
	
	def set_time_window(self, tw):
		if tw is not None:
			tw = tw.split(',')
			self.arrival = float(tw[0])
			self.departure = float(tw[1])
	
#############################---Class Person---#########################################################################
class Person:
	def __init__(self, pid):
		self.pid = pid
		self.isStudent = False
		self.tour_ctr = 0 
		self.day_pattern = None
		self.num_tours = None
		self.tours = []
		self.available_time_windows = []
		self.unavailable_time_windows = []

		#data holders
		self.p_data = None
		self.dp_data = None
		self.uw_data = None
		self.tmd_date = None
		self.tme_data = None
		self.tmw_data = None
		self.ttd_data = None
		self.isg_data = None
		self.itd_data = None

	def block_time(self, start, end):
		if start == end: 
			return
		else:
			for tw in self.available_time_windows:
				twl = tw.split(',')
				tw_start = float(twl[0])
				tw_end = float(twl[1])
				if (tw_start >= start and tw_start <= end) or (tw_end >= start and tw_end <= end):
					self.available_time_windows.remove(tw)

#############################---Loop---#################################################################################

def person_loop(pid, verbose=False):
	person = Person(pid)
	start = datetime.datetime.now()
	person.p_data = db[collection_names["Person Data"]].find_one({"_id" : person.pid})
	person.dp_data = db[collection_names["Day Pattern"]].find_one({"_id" : person.pid})
	person.uw_data = db[collection_names["Attend Usual Work"]].find_one({"_id" : person.pid})
	person.tmd_data = db[collection_names["Tour Mode/Destination"]].find_one({"_id" : person.pid})
	person.tme_data = db[collection_names["Tour Mode Education"]].find_one({"_id" : person.pid})
	person.tmw_data = db[collection_names["Tour Mode Work"]].find_one({"_id" : person.pid})
	person.ttd_data = db[collection_names["Tour Time Of Day"]].find_one({"_id" : person.pid})
	person.isg_data = db[collection_names["Intermediate Stop Generation"]].find_one({"_id" : person.pid})
	person.itd_data = db[collection_names["Intermediate Stop Time Of Day"]].find_one({"_id" : person.pid})

	person_tours_coll = db[collection_names["Person Tours Output"]]
	tours_coll = db[collection_names["Tour Output"]]
	activity_coll = db[collection_names["Activity Output"]]

	end = datetime.datetime.now()
	if verbose: print 'Reading data from Mongo:', str((end-start).total_seconds()) + 's'

	person.isStudent = (int(person.dp_data["person_type_id"]) == 4)

	# Day Pattern
	predict_day_pattern(person)
	
	# Number Of Tours
	predict_num_tours(person)
	
	# priority for tours based on student/non-student status of person
	if person.isStudent is True :
		if person.num_tours["EduT"] > 0:
			for i in range(person.num_tours["EduT"]): 
				person.tours.append(Tour(person, "Education", True))
		if person.num_tours["WorkT"] > 0:
			first_of_multiple = True
			for i in range(person.num_tours["WorkT"]): 
				attends_usual_work_location = False
				if person.p_data["fix_work_location_mtz"] != 0 : 
					person.uw_data["first_of_multiple"] = int(first_of_multiple)
					person.uw_data["subsequent_of_multiple"] = int(not first_of_multiple)
					first_of_multiple = False
					# Attend usual work location
					attends_usual_work_location = predict_usual_location(person)
				person.tours.append(Tour(person, "Work", attends_usual_work_location))
	else : #person.isStudent is False
		if person.num_tours["WorkT"] > 0 :
			first_of_multiple = True
			for i in range(person.num_tours["WorkT"]): 
				attends_usual_work_location = False
				if person.p_data["fix_work_location_mtz"] != 0 : 
					person.uw_data["first_of_multiple"] = int(first_of_multiple)
					person.uw_data["subsequent_of_multiple"] = int(not first_of_multiple)
					first_of_multiple = False
					# Attend usual work location
					attends_usual_work_location = predict_usual_location(person)
				person.tours.append(Tour(person, "Work", attends_usual_work_location))
		if person.num_tours["EduT"] > 0 :
			for i in range(person.num_tours["EduT"]): 
				person.tours.append(Tour(person, "Education", True))
	if person.num_tours["ShopT"] > 0:
		for i in range(person.num_tours["ShopT"]): 
			person.tours.append(Tour(person, "Shopping"))
	if person.num_tours["OthersT"] > 0:
		for i in range(person.num_tours["OthersT"]): 
			person.tours.append(Tour(person, "Others"))

	initialized_available_time_windows = False

	for tour in person.tours:
		if tour.usual_loc is True:
			# Tour Mode
			predict_mode(person, tour)
		else : #tour.usual_loc is False
			# Tour Mode Destination
			predict_mode_destination(person, tour)
		
		#if tour.purpose == "Work" and tour.is_sub_tour is False:
		#	# Work based sub-tours
		#	predict_work_based_sub_tours(person, tour)
			
		time_wndw = predict_tour_time_of_day(person, tour, initialized_available_time_windows)
		initialized_available_time_windows = True
	
		if time_wndw is None:
			person.tours.remove(tour)
		else: # if time_wndw is not None
			prim_act = Activity(tour, tour.purpose, True, time_wndw)
			prim_act.mode = tour.mode
			prim_act.destination = tour.destination
			tour.add_trip_chain_item(prim_act)
			tour.take_time_window(prim_act.time_window)

			# Intermediate stop generation (generates a stop, calls mode/destination and stop-tod models)
			generate_intermediate_activities(person, tour)

		estimate_tour_start_time(person, tour)
		estimate_tour_end_time(person, tour)
		if tour.start_time > 3 and tour.end_time > 3 and tour.end_time >= tour.start_time:
			person.block_time(tour.start_time, tour.end_time)
		else:
			print "invalid start and end time for the tour" + str(tour)
			
	if verbose: 
		for tour in person.tours:
			print tour

	#write output to mongo
	person_tours_doc = 	construct_person_tour_doc(person)
	person_tours_coll.insert(person_tours_doc)
	tour_ctr = 0
	for tour in person.tours:
		tour_ctr = tour_ctr + 1
		tours_doc = construct_tours_doc(person, tour, tour_ctr)
		tours_coll.insert(tours_doc)
		stop_ctr = 0
		for act in tour.trip_chain:
			stop_ctr = stop_ctr + 1
			activity_doc = construct_activity_doc(person, tour_ctr, stop_ctr, act, tour)
			activity_coll.insert(activity_doc)
		
#############################--doMain---################################################################################
def doMain(person_ids, proc):
	log_file = "proc" + str(proc) +".log"
	logging.basicConfig(filename=log_file, level=logging.INFO)
	logging.info('Process-' + str(proc) + ' will process ' + str(len(person_ids)) + ' persons')
	for record in person_ids:
		starttime = datetime.datetime.now()
		try: 
			print record["_id"]
			person_loop(record["_id"])
		except KeyboardInterrupt:
			raise
		except: 
			logging.info('Person:' + str(record["_id"]) + ' is discarded due to ' + str(sys.exc_info()[0]) + '.')
			
		endtime = datetime.datetime.now()
		logging.info('Person: '+ str(record["_id"])+ '|Time: ' + str((endtime - starttime).total_seconds()) + 's')

def doMain_unpickle(files_count, db):
	file_ctr = 1
	person_ctr = 0
	tour_ctr = 0
	pt_coll = db.person_tours
	while file_ctr <= files_count:
		pickled_file = "output" + str(file_ctr) + ".txt"
		print 'unpickling', pickled_file
		with open(pickled_file, 'rb') as f_in:
			unpcklr = pickle.Unpickler(f_in)
			EOF_reached = False
			while EOF_reached is False:
				try:
					person = unpcklr.load()
					person_ctr = person_ctr + 1
					person_tour = construct_person_tour_doc(person, person_ctr)
					pid = pt_coll.insert(person_tour)
					print person_ctr, ':', str(person.pid)
				except EOFError:
					EOF_reached = True
					file_ctr = file_ctr + 1
		
		
#############################---main---#################################################################################
		
if __name__ == "__main__":
	s = datetime.datetime.now()
	parser = argparse.ArgumentParser()
	parser.add_argument("files", help="JSON files, variables files and outputfile")
	parser.add_argument("num_procs", default=1, help="Number of processes to spawn")
	args = parser.parse_args()
	
	with open(args.files, 'r') as f:
		files = json.load(f)
		dcm.model_file = files["JsonFiles"] # model_file must be kept local to this module when json files for all models are ready
		database = files["Database"]
		collection_names = files["Collections"]
		client = MongoClient(database["host"], int(database["port"]))
		db = client[database["db"]]

		person_ids = list(db[collection_names["Persons"]].find())
#		print 'Total Persons:', len(person_ids)
		nprocs = int(args.num_procs)
		splitCnt = int(len(person_ids)/nprocs)	

		# Create new processes
		for i in range(nprocs):
			lb = i 
			ub = (i+1) if (i+1) < nprocs else len(person_ids)
			p = Process(target=doMain, args = (person_ids[lb*splitCnt:ub*splitCnt], (i+1)))
			procs.append(p)

		# Start new Processes 
		for p in procs:
			p.start()

		#Wait for all threads to complete
		try:
			for p in procs:
				p.join()
		except KeyboardInterrupt:
			for p in procs:
				p.terminate()
				raise

	print "Exiting Main"
	e = datetime.datetime.now()
	print 'Running Time: ' + str((e-s).total_seconds()) + 's'

