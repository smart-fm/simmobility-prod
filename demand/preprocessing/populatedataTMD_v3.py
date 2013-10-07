import sys
from multiprocessing import Process, Lock
import csv
from collections import OrderedDict
import pymongo
from pymongo import MongoClient
import numpy as np
import datetime
import time
import math

db = None
lock = Lock()
procs = []

#----------------Mongo functions-----------------------
def connect_MongoDB():
	global db
	client = MongoClient('localhost', 27017)
	print("Connected to Mongo database:", client)
	db = client.mydb
#	db = client.vardb
	
def removeData_MongoDB(collectionName):
	db[collectionName].remove()

def dropCollection_MongoDB(collectionName):
	db[collectionName].drop()
	
def write_MongoDB(data,collectionName):
	coll=db[collectionName]
	#lock.acquire()
	try:
		coll.insert(data)
		#lock.release()
	except:
		#lock.release()
		error = sys.exc_info()[0]
		raise RuntimeError(error)

#----------------Function that populates all the data for all the variables in mongo db that is needed for the model Mode choice------------------------------
def tmd(model_name, person_data_file, tid):
	model_name = "tmd"
	person_data = []
	household_data = {}
	income_data = []
	all_data = []

	with open(person_data_file, "r") as f, open("household_database.csv", "r") as b, open("IncomeIndex14.csv", "r") as c:
		person_reader = csv.reader(f,delimiter=",")
		household_reader = csv.reader(b,delimiter=",")
		income_reader = csv.reader(c,delimiter=",")
		#Reading values from hits_person.csv file
		next(person_reader, None)
		for row in person_reader:
			person_data.append(row)
		#Reading values from household_database.csv file
		for row in household_reader:
			#household_data.append(row)
			household_data[row[0]] = row
		#Reading values from IncomeIndex14.csv file
		for row in income_reader:
			income_data.append(row)
	f.close()
	b.close()
	c.close()
	print("Completed reading data from all person data")

	#loading am, pm, zones into list
	zones = list(db.Zone.find())
	cnt = 0
	#Appending data to a list of dict where each row or document represents data for each person*************************************************
	for i in person_data:
		cnt = cnt + 1
		start = datetime.datetime.now()
		eachPerson_data = OrderedDict()
		eachPerson_data["_id"] = i[0]+"-"+i[1]
		#Fetching values for variables "var_missingincome", "var_incmid"
		if int(i[11]) >= 13:
			eachPerson_data["missing_income"] = 1
		else:
			eachPerson_data["missing_income"] = 0
		for j in income_data:
			if int(j[0]) == int(i[11]):					
				eachPerson_data["Income_mid"] = int(j[2])
		eachPerson_data["Female_dummy"] = int(i[9])
		eachPerson_data["stop_type"] = 0 
		#Computing values for "Cost Related Variables"		
		#Fetching values for variables "Household Composition" 
		j = household_data[i[0]]
		origin = int(j[2])
		eachPerson_data["zero_car"] = (1*(int(j[5])==0))
		eachPerson_data["one_plus_car"] = (1*(int(j[5])>=1))
		eachPerson_data["two_plus_car"] = (1*(int(j[5])>=2))
		eachPerson_data["three_plus_car"] = (1*(int(j[5])>=3))
		eachPerson_data["zero_motor"] = (1*(int(j[7])==0))
		eachPerson_data["one_plus_motor"] = (1*(int(j[7])>=1))
		eachPerson_data["two_plus_motor"] = (1*(int(j[7])>=2))
		eachPerson_data["three_plus_motor"] = (1*(int(j[7])>=3))  
		one_plus_car = (1*(int(j[5])>=1))

		c = 0
		for row in zones:
			destination = int(row['zone_code'])
			odsame = True if origin == destination else False
			
			destinationid = row['zone_id']

			am = db.AMCosts.find_one({'origin': origin, 'destin': destination})
			pm = db.PMCosts.find_one({'origin': destination, 'destin': origin})

			cost_public_first = am['pub_cost'] if odsame is False else 0
			cost_public_second = pm['pub_cost'] if odsame is False else 0
			eachPerson_data["cost_public_%s" % destinationid] = cost_public_first + cost_public_second
			cost_car_ERP_first = am['car_cost_erp'] if odsame is False else 0
			cost_car_ERP_second = pm['car_cost_erp'] if odsame is False else 0
			cost_car_OP_first = (am['distance'])*0.147 if odsame is False else 0
			cost_car_OP_second = (pm['distance'])*0.147 if odsame is False else 0
			
			eachPerson_data["cost_car_parking_%s" % destinationid] = 8 * (row['parking_rate'])
			eachPerson_data["cost_car_ERP_%s" % destinationid] = cost_car_ERP_first + cost_car_ERP_second
			eachPerson_data["cost_car_OP_%s" % destinationid] = cost_car_OP_first + cost_car_OP_second
			eachPerson_data["walk_distance_first_%s" % destinationid] = am['distance'] if odsame is False else 0
			eachPerson_data["walk_distance_second_%s" % destinationid] = pm['distance'] if odsame is False else 0
			eachPerson_data["central_dummy_%s" % destinationid]=row['central_dummy']
			
			#Fetching values for variables "Travel Time Related Variables"
			tt_public_ivt_first = am['pub_ivt'] if odsame is False else 0
			tt_public_ivt_second = pm['pub_ivt'] if odsame is False else 0
			tt_public_out_first = am['pub_wtt'] if odsame is False else 0
			tt_public_out_second = pm['pub_wtt'] if odsame is False else 0
			tt_ivt_car_first = am['car_ivt'] if odsame is False else 0
			tt_ivt_car_second = pm['car_ivt'] if odsame is False else 0
			eachPerson_data["tt_public_ivt_%s" % destinationid] = tt_public_ivt_first + tt_public_ivt_second 
			eachPerson_data["tt_public_out_%s" % destinationid] = tt_public_out_first + tt_public_out_second 
			eachPerson_data["tt_car_ivt_%s" % destinationid] = 99.0 if math.isnan(tt_ivt_car_first + tt_ivt_car_second) else (tt_ivt_car_first + tt_ivt_car_second)

			c = c + 1
			eachPerson_data["employment_%s" % c] = row['employment'] 
			eachPerson_data["population_%s" % c] = row['population'] 
			eachPerson_data["area_%s" % c] = row['area'] 
			eachPerson_data["shop_%s" % c] = row['shop'] 

			eachPerson_data["drive1,%s_AV" % destination]= (1*(int(i[13]) * one_plus_car == 1)) if odsame is False else 0
			eachPerson_data["share2,%s_AV" % destination]= 1 if odsame is False else 0
			eachPerson_data["share3,%s_AV" % destination]= 1 if odsame is False else 0
			eachPerson_data["bus,%s_AV" % destination]= (1*(am['pub_ivt']>0 and pm['pub_ivt']>0)) if odsame is False else 0
			eachPerson_data["mrt,%s_AV" % destination]= (1*(am['pub_ivt']>0 and pm['pub_ivt']>0)) if odsame is False else 0
			eachPerson_data["private_bus,%s_AV" % destination]= (1*(am['pub_ivt']>0 and pm['pub_ivt']>0)) if odsame is False else 0
			eachPerson_data["walk,%s_AV" % destination]= (1*(am['distance']<=3 and pm['distance']<=3)) if odsame is False else 0
			eachPerson_data["taxi,%s_AV" % destination]= 1 if odsame is False else 0
			eachPerson_data["motor,%s_AV" % destination]= 1 if odsame is False else 0

			eachPerson_data["mrt,%s_AV" % destination] = 0 if eachPerson_data["tt_public_ivt_%s" % destinationid] == 0 else (eachPerson_data["mrt,%s_AV" % destination] if odsame is False else 0)
			eachPerson_data["bus,%s_AV" % destination] = 0 if eachPerson_data["tt_public_ivt_%s" % destinationid] == 0 else (eachPerson_data["bus,%s_AV" % destination] if odsame is False else 0)

		all_data.append(eachPerson_data)
		#print 'Process:', tid, '|Person:', (i[0]+"-"+i[1]), '|Time:', (datetime.datetime.now() - start).total_seconds(), 's|variables:', len(eachPerson_data)
		if cnt % 10 == 0:
			write_MongoDB(all_data, model_name)
			print 'Person data inserted from Proc', tid, ':' , cnt
			all_data = []
		
	if len(all_data) > 0:
		cnt = cnt + len(all_data)
		write_MongoDB(all_data, model_name)
		all_data = []
	print "Proc", tid, "Completed Inserting ", cnt," documents to Mongo DB for model:", model_name
	return

#----------------Main function which in turn calls the function specific to each model------------------------------------------------------------------------
def main():
	starttime = datetime.datetime.now()
	modelname = sys.argv[1]
	print("Calling the function for model: %s" % (modelname))
	connect_MongoDB()
	#removeData_MongoDB("dummy")
	dropCollection_MongoDB(modelname)	

	# Create new threads
#	proc = Process(target=tmd, args = ("tmd", "person_database_dummy20.csv", 1))
	proc1 = Process(target=tmd, args = ("tmd", "person_database1.csv", 1))
	proc2 = Process(target=tmd, args = ("tmd", "person_database2.csv", 2))
	proc3 = Process(target=tmd, args = ("tmd", "person_database3.csv", 3))
	proc4 = Process(target=tmd, args = ("tmd", "person_database4.csv", 4))

	# Start new Processes
#	proc.start()
	proc1.start()
	proc2.start()
	proc3.start()
	proc4.start()

	# Add threads to thread list
#	procs.append(proc)
	procs.append(proc1)
	procs.append(proc2)
	procs.append(proc3)
	procs.append(proc4)

	# Wait for all threads to complete
	for p in procs:
		p.join()
	print "Exiting Main"
	endtime = datetime.datetime.now()
	print 'Time taken: ' + str((endtime - starttime).total_seconds()) + 's'

if  __name__ =='__main__':
	if (len(sys.argv)) != 2:
		print ("Enter the command as python filename.py modelname")
	else:
		main()



