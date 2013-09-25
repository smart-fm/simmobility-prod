import sys
import csv
import pymongo
from pymongo import MongoClient

client = MongoClient('localhost', 27017)
db = client.mydb
print("Connected to Mongo database:", client)

persons = list(db.person_data.find({"person_type" : "Full time student", "school_location_mtz" :0}, {"_id":1}))
print len(persons), 'persons will be removed'
for row in persons:
	personid = row['_id']
#	db.person_ids.remove({"_id": personid})
#	db.person_data.remove({"_id": personid})
#	db.dp.remove({"_id": personid})
#	db.isg.remove({"_id": personid})
	db.itd.remove({"_id": personid})
#	db.tme.remove({"_id": personid})
#	db.tmw.remove({"_id": personid})
#	db.ttd.remove({"_id": personid})
#	db.uw.remove({"_id": personid})
#	db.tmd.remove({"_id": personid})
	print personid, 'is removed'
	
print("removed the data from collections")





