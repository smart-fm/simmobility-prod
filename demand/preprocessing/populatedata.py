import sys
import csv
from collections import OrderedDict
import pymongo
from pymongo import MongoClient
import numpy as np
import datetime


db = None

#----------------Function to connect to mongo DB and write data-----------------------
def connect_MongoDB():
    global db
    client = MongoClient('localhost', 27017)
    print("Connected to Mongo database:", client)
    db = client.mydb
    
def removeData_MongoDB(collectionName):
    print("Removed existing data from DB")
    db[collectionName].remove()
    
def write_MongoDB(data,collectionName):
    coll=db[collectionName]
    try:
        coll.insert(data)
    except:
        print(data)
        raise RuntimeError(sys.exc_info()[0])
        
#----------------Function that populates all the data for all the variables in mongo db that is needed for the model Day pattern & exact numer of tours------------------------------
def dp():
    modelname = sys.argv[1]
    #Assinging values to rest of variables for all person data
    person_data = []
    household_data = []
    income_data = []
    all_data = []
    choiceset = []
    choiceset_data = []
    
    print("Assinging values to rest of variables for all person data")
    with open("person_database_dummy19.csv", "r") as f , open("household_database_dummy19.csv", "r") as b, open("IncomeIndex14.csv", "r") as c, open("choiceset.csv", "r") as d, open("DayPatternChoiceSet51.csv", "r") as e :
        person_reader = csv.reader(f,delimiter=",")
        household_reader = csv.reader(b,delimiter=",")
        income_reader = csv.reader(c,delimiter=",")
        choiceset_reader = csv.reader(d,delimiter=" ")
        dpchoiceset_reader = csv.reader(e,delimiter=",")
        #Reading values from hits_person.csv file
        next(person_reader, None)
        next(dpchoiceset_reader,None)
        
        for row in person_reader:
            person_data.append(row)
        #Reading values from household_database_dummy19.csv file
        for row in household_reader:
            household_data.append(row)
        #Reading values from IncomeIndex14.csv file
        for row in income_reader:
            income_data.append(row)
        #Reading values from choiceset.csv file
        for row in choiceset_reader:
            choiceset.append(row)
        #Reading values from DayPatternChoiceSet51.csv file
        for row in dpchoiceset_reader:
            choiceset_data.append(row)
    f.close()
    b.close()
    c.close()
    d.close()
    e.close()
    #***********Appending data to a list of dict where each row or document represents data for each person**************
    cnt = 0
    for i in person_data:
        eachPerson_data = OrderedDict()
        eachPerson_data["_id"] = i[0]+"-"+i[1]
        eachPerson_data["person_type_id"] = int(i[3])
        if int(i[7]) == 6:
            eachPerson_data["universitystudent"] = 1
        else:
            eachPerson_data["universitystudent"] = 0
        eachPerson_data["age_id"] = int(i[5])
        eachPerson_data["female_dummy"] = int(i[9])
        #Fetching values for variables "var_missingincome", "var_incmid"
        if int(i[11]) >= 13:
            eachPerson_data["missingincome"] = 1
        else:
            eachPerson_data["missingincome"] = 0
        for j in income_data:
            if int(j[0]) == int(i[11]):                    
                eachPerson_data["incmid"] = int(j[2])
        eachPerson_data["work_at_home_dummy"] = int(i[12])
        #Fetching values for variables "Household Composition" 
        for j in household_data:
            if i[0] == j[0]:
                eachPerson_data["HH_all_adults"] = int(j[23])
                eachPerson_data["HH_all_workers"] = int(j[24])
                if int(j[9])>= 1 :
                    eachPerson_data["HH_with_under_4"] = 1
                else:
                    eachPerson_data["HH_with_under_4"] = 0
                if int(j[11])>= 1 :
                    eachPerson_data["HH_with_under_15"] = 1
                else:
                    eachPerson_data["HH_with_under_15"] = 0
                eachPerson_data["hh_car_avail"] = int(j[5])+int(j[6])
                eachPerson_data["hh_motor_avail"] = int(j[7])
                break
        #Appending values for logsum variables
        eachPerson_data["worklogsum"] = 0
        eachPerson_data["edulogsum"] = 0
        eachPerson_data["shoplogsum"] = 0
        eachPerson_data["otherlogsum"] = 0

        #Appending availibity for choicesets of DP model
        for n,k in enumerate(choiceset):
            if int(i[3]) == 4:
                eachPerson_data["dp_"+str(k[0])+"_AV"] = 1
            else:
                if int(choiceset_data[n][2]) == 0:
                    eachPerson_data["dp_"+str(k[0])+"_AV"] = 1
                else:
                    eachPerson_data["dp_"+str(k[0])+"_AV"] = 0
        #Appending availability for exact no.of tours model
        eachPerson_data["1_AV"] = 1
        eachPerson_data["2_AV"] = 1
        eachPerson_data["3_AV"] = 1
        all_data.append(eachPerson_data)
        write_MongoDB(eachPerson_data,modelname)
    #Calling the write_DB function to write the value in mongoDB
    print("Writing values of variables to MongoDB")
    #write_MongoDB(all_data,modelname)
    print("Completed Inserting "+ str(len(all_data))+" values to Mongo DB for model:" + modelname)
           

#----------------Function that populates all the data for all the variables in mongo db that is needed for the model Mode choice------------------------------
def tm():
    modelname = sys.argv[1]
    person_data = []
    household_data = {}
    all_data = []
    income_data = []

    with open("person_database.csv", "r") as f , open("household_database.csv", "r") as b, open("IncomeIndex14.csv", "r") as c:
        person_reader = csv.reader(f,delimiter=",")
        household_reader = csv.reader(b,delimiter=",")
        income_reader = csv.reader(c,delimiter=",")
        #Reading values from hits_person.csv file
        next(person_reader, None)
        for row in person_reader:
            person_data.append(row)
        #Reading values from household_database_dummy19.csv file
        for row in household_reader:
            household_data[row[0]] = row
        #Reading values from IncomeIndex14.csv file
        for row in income_reader:
            income_data.append(row)
    f.close()
    b.close()
    c.close()
    print("Completed reading data from all person data")

    c = 0
    cnt = 0
    #Appending data to a list of dict where each row or document represents data for each person*************************************************
    for i in person_data:
        if modelname == 'tme':
            destination = int(i[17])
        elif modelname == 'tmw':
            destination = int(i[16])
        if destination == 0:
            continue
        c = c+1
        Start = datetime.datetime.now()
        eachPerson_data = OrderedDict()
        eachPerson_data["_id"] = i[0]+"-"+i[1]
        eachPerson_data["destination"] = destination
        eachPerson_data["student_type_num"] = int(i[7])
        eachPerson_data["age_num"] = int(i[5])
        eachPerson_data["Female_dummy"] = int(i[9])
        #Fetching values for variables "var_missingincome", "var_incmid"
        if int(i[11]) >= 13:
            eachPerson_data["missing_income"] = 1
        else:
            eachPerson_data["missing_income"] = 0
        for j in income_data:
            if int(j[0]) == int(i[11]):                    
                eachPerson_data["Income_mid"] = int(j[2])
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

        odsame = True if (origin == destination) else False
        am = db.AMCosts.find_one({'origin': origin, 'destin': destination})
        pm = db.PMCosts.find_one({'origin': destination, 'destin': origin})
        zone_dest = db.Zone.find_one({'zone_code': destination})
        zone_origin = db.Zone.find_one({'zone_code': origin})

        eachPerson_data["cost_public_first"] = am['pub_cost'] if odsame is False else 0
        eachPerson_data["cost_public_second"] = pm['pub_cost'] if odsame is False else 0
        eachPerson_data["cost_car_ERP_first"] = am['car_cost_erp'] if odsame is False else 0
        eachPerson_data["cost_car_ERP_second"] = pm['car_cost_erp'] if odsame is False else 0
        eachPerson_data["cost_car_OP_first"] = (am['distance'])*0.147  if odsame is False else 0
        eachPerson_data["cost_car_OP_second"] = (pm['distance'])*0.147  if odsame is False else 0
        eachPerson_data["cost_car_parking"] = 8 * zone_dest['parking_rate']
        eachPerson_data["walk_distance1"] = am['distance']  if odsame is False else 0
        eachPerson_data["walk_distance2"] = pm['distance']  if odsame is False else 0
        eachPerson_data["Central_dummy"]= zone_dest['central_dummy']
        eachPerson_data["walk_distance2_dummy"] = 0
        eachPerson_data["walk_distance1_dummy"] = 0
        #Fetching values for variables "Travel Time Related Variables"
        eachPerson_data["tt_public_ivt_first"] = am['pub_ivt'] if odsame is False else 0
        eachPerson_data["tt_public_ivt_second"] = pm['pub_ivt'] if odsame is False else 0
        eachPerson_data["tt_public_waiting_first"] = am['pub_wtt'] if odsame is False else 0
        eachPerson_data["tt_public_waiting_second"] = pm['pub_wtt'] if odsame is False else 0
        eachPerson_data["tt_public_walk_first"] = am['pub_walkt'] if odsame is False else 0
        eachPerson_data["tt_public_walk_second"] = pm['pub_walkt'] if odsame is False else 0
        eachPerson_data["tt_ivt_car_first"] = am['car_ivt'] if odsame is False else 0
        eachPerson_data["tt_ivt_car_second"] = pm['car_ivt'] if odsame is False else 0
        eachPerson_data["average_transfer_number"] = (am['avg_transfer'] + pm['avg_transfer'])/2 if odsame is False else 0
        #Fetching other variables
        eachPerson_data["resident_student"] = zone_origin['resident_students']
        eachPerson_data["education_op"] = zone_dest['total_enrollment']
        eachPerson_data["origin_area"]= zone_origin['area']
        eachPerson_data["destination_area"] = zone_dest['area']
        eachPerson_data["resident_size"] = zone_origin['resident_workers']
        eachPerson_data["work_op"] = zone_dest['employment']
        if modelname == 'tme':
            eachPerson_data["tme_Auto_AV"]= (1*(int(i[13]) * one_plus_car == 1))
            eachPerson_data["tme_Share 2+_AV"]= 1
            eachPerson_data["tme_Share 3+_AV"]= 1
            eachPerson_data["tme_Public bus_AV"]= (1*(am['pub_ivt']>0 and pm['pub_ivt']>0)) if odsame is False else 1
            eachPerson_data["tme_MRT_AV"]= (1*(am['pub_ivt']>0 and pm['pub_ivt']>0)) if odsame is False else 1
            eachPerson_data["tme_Private bus_AV"]= (1*(am['pub_ivt']>0 and pm['pub_ivt']>0)) if odsame is False else 1
            eachPerson_data["tme_Walk_AV"]= (1*(am['distance']<=5 and pm['distance']<=5)) if odsame is False else 1
            eachPerson_data["tme_Taxi_AV"]= 1
            eachPerson_data["tme_Motor_AV"]= 1            
        elif modelname == 'tmw':
            eachPerson_data["tmw_Auto_AV"]= (1*(int(i[13]) * one_plus_car == 1))
            eachPerson_data["tmw_Share 2+_AV"]= 1
            eachPerson_data["tmw_Share 3+_AV"]= 1
            eachPerson_data["tmw_Public bus_AV"]= (1*(am['pub_ivt']>0 and pm['pub_ivt']>0)) if odsame is False else 1
            eachPerson_data["tmw_MRT_AV"]= (1*(am['pub_ivt']>0 and pm['pub_ivt']>0)) if odsame is False else 1
            eachPerson_data["tmw_Private bus_AV"]= (1*(am['pub_ivt']>0 and pm['pub_ivt']>0)) if odsame is False else 1
            eachPerson_data["tmw_Walk_AV"]= (1*(am['distance']<=3 and pm['distance']<=3)) if odsame is False else 1
            eachPerson_data["tmw_Taxi_AV"]= 1
            eachPerson_data["tmw_Motor_AV"]= 1
        all_data.append(eachPerson_data)
        cnt = cnt +1
        if cnt % 100 == 0:
            write_MongoDB(all_data, modelname)
            print ('Person data inserted:', cnt)
            all_data = []
        print ('\rPerson variables fetched in %fs' %(datetime.datetime.now() - Start).total_seconds())
    #Calling the write_DB function to write the value in mongoDB
    print(c)
    write_MongoDB(all_data, modelname)
    print("Completed Inserting "+ str(len(all_data))+" values to Mongo DB for model:" + modelname)
    print ("Done!")
    return


#----------------Function that populates all the data for all the variables in mongo db that is needed for the model tour time of day------------------------------
def ttd():
    modelname = sys.argv[1]
    person_data = []
    all_data = []
    choiceset_data = []
    with open("person_database_dummy19.csv", "r") as f , open("choiceset_ttd.csv", "r") as b:
        person_reader = csv.reader(f,delimiter=",")
        choiceset_reader = csv.reader(b,delimiter=",")
        #Reading values from hits_person.csv file
        next(person_reader, None)
        for row in person_reader:
            person_data.append(row)
        #Reading values from choiceset_ttd.csv file
        for row in choiceset_reader:
            choiceset_data.append(row)
        
    f.close()
    b.close()
    print("Completed reading data from all person data")

    cnt = 0
    #Appending data to a list of dict where each row or document represents data for each person*************************************************
    for i in person_data:
        Start = datetime.datetime.now()
        eachPerson_data = OrderedDict()
        eachPerson_data["_id"] = i[0]+"-"+i[1]
        eachPerson_data["PersonTypeIndex"] = str(i[3])
        eachPerson_data["gender"] = int(i[9])
        eachPerson_data["worktime"] = int(i[18])

        #Variables asinged to zero as their parameter values are zero
        eachPerson_data["cost_HT1_am"] = 0
        eachPerson_data["cost_HT1_pm"] = 0
        eachPerson_data["cost_HT1_op"] = 0
        eachPerson_data["cost_HT2_am"] = 0
        eachPerson_data["cost_HT2_pm"] = 0
        eachPerson_data["cost_HT2_op"] = 0

        for i in range(1,49):
            eachPerson_data["TT_HT1_%s" % str(i)] = 0
            eachPerson_data["TT_HT2_%s" % str(i)] = 0

        #Appending values for availability
        for n,j in enumerate(choiceset_data):
            eachPerson_data[str(j[0])+"_AV"] = 1

        all_data.append(eachPerson_data)
        cnt = cnt +1
        if cnt % 100 == 0:
            write_MongoDB(all_data, modelname)
            print ('Person data inserted:', cnt)
            all_data = []
        print ('\rPerson variables fetched in %fs' %(datetime.datetime.now() - Start).total_seconds())
    #Calling the write_DB function to write the value in mongoDB
    print(cnt)
    write_MongoDB(all_data, modelname)
    print("Completed Inserting "+ str(len(all_data))+" values to Mongo DB for model:" + modelname)
    print ("Done!")
    return
        
#----------------Function that populates all the data for all the variables in mongo db that is needed for the model binary choice between usual an dunusual work------------------------------
def uw():
    modelname = sys.argv[1]
    person_data = []
    all_data = []
    household_data = {}
    income_data = []
    with open("person_database.csv", "r") as f , open("household_database.csv", "r") as b, open("IncomeIndex14.csv", "r") as c:
        person_reader = csv.reader(f,delimiter=",")
        household_reader = csv.reader(b,delimiter=",")
        income_reader = csv.reader(c,delimiter=",")
        #Reading values from hits_person.csv file
        next(person_reader, None)
        for row in person_reader:
            person_data.append(row)
        #Reading values from household_database_dummy19.csv file
        for row in household_reader:
            household_data[row[0]] = row
        #Reading values from IncomeIndex14.csv file
        for row in income_reader:
            income_data.append(row)
    f.close()
    b.close()
    c.close()
    print("Completed reading data from all person data")

    cnt = 0
    #Appending data to a list of dict where each row or document represents data for each person*************************************************
    for i in person_data:
        Start = datetime.datetime.now()
        eachPerson_data = OrderedDict()
        eachPerson_data["_id"] = i[0]+"-"+i[1]
        eachPerson_data["person_type_id"] = int(i[3])
        eachPerson_data["Female_dummy"] = int(i[9])
        eachPerson_data["IncomeIndex"] = int(i[11])
        eachPerson_data["work_from_home_dummy"] = int(i[12])
        eachPerson_data["fixed_place"] = 0 if int(i[16]) == 0 else 1
        eachPerson_data["fixed_work_hour"]= int(i[18])

        #Fetching values for variables "Household Composition" 
        j = household_data[i[0]]
        origin = int(j[2])
        destination = int(i[16])
        if destination == 0:
            continue        
        odsame = True if (origin == destination) else False
        am = db.AMCosts.find_one({'origin': origin, 'destin': destination})
        pm = db.PMCosts.find_one({'origin': destination, 'destin': origin})
        zone_dest = db.Zone.find_one({'zone_code': destination})

        eachPerson_data["walk_distance1"] = am['distance']  if odsame is False else 0
        eachPerson_data["walk_distance2"] = pm['distance']  if odsame is False else 0
        eachPerson_data["work_op"]= zone_dest['employment']
        eachPerson_data["first_of_multiple"] = 1
        eachPerson_data["subsequent_of_multiple"] = 0

        #appending the availability
        eachPerson_data["uw_Attend_AV"] = 1
        eachPerson_data["uw_No_Attend_AV"]= 1
        all_data.append(eachPerson_data)
        cnt = cnt +1
        if cnt % 100 == 0:
            write_MongoDB(all_data, modelname)
            print ('Person data inserted:', cnt)
            all_data = []
        print ('\rPerson variables fetched in %fs' %(datetime.datetime.now() - Start).total_seconds())
    #Calling the write_DB function to write the value in mongoDB
    print(cnt)
    if len(all_data) > 0:
        cnt = cnt + len(all_data)
        write_MongoDB(all_data, modelname)
    print("Completed Inserting "+ str(len(all_data))+" values to Mongo DB for model:" + modelname)
    print ("Done!")
    return

#----------------Function that populates all the data for all the variables in mongo db that is needed for the model Intermediate stop generation------------------------------
def isg():
    modelname = sys.argv[1]
    person_data = []
    all_data = []
    with open("person_database_dummy19.csv", "r") as f :
        person_reader = csv.reader(f,delimiter=",")
        #Reading values from hits_person.csv file
        next(person_reader, None)
        for row in person_reader:
            person_data.append(row)
    f.close()
    print("Completed reading data from all person data")

    cnt = 0
    #Appending data to a list of dict where each row or document represents data for each person*************************************************
    for i in person_data:
        Start = datetime.datetime.now()
        eachPerson_data = OrderedDict()
        eachPerson_data["_id"] = i[0]+"-"+i[1]
        eachPerson_data["has_subtour"] = 0 # no subtour in this simulation so zero
        eachPerson_data["tour_type"] = 0
        eachPerson_data["female_dummy"] = int(i[9])
        if int(i[3]) == 4:
            eachPerson_data["student_dummy"] = 1
        else:
            eachPerson_data["student_dummy"] = 0
        if int(i[3]) in [1,2,3,8,9,10]:
             eachPerson_data["worker_dummy"]= 1
        else:
             eachPerson_data["worker_dummy"]= 0 
        eachPerson_data["driver_dummy"] = 0
        eachPerson_data["passenger_dummy"]= 0
        eachPerson_data["public_dummy"] = 0
        eachPerson_data["time_window_h"] = 0
        
        eachPerson_data["distance"] = 0
        eachPerson_data["p_700a_930a"] = 0
        eachPerson_data["p_930a_1200a"] = 0
        eachPerson_data["p_300p_530p"] = 0
        eachPerson_data["p_530p_730p"] = 0
        eachPerson_data["p_730p_1000p"] = 0
        eachPerson_data["p_1000p_700a"] = 0
        
        eachPerson_data["worklogsum"] = 0
        eachPerson_data["edulogsum"] = 0
        eachPerson_data["shoplogsum"] = 0
        eachPerson_data["otherlogsum"] = 0
        
        eachPerson_data["first_stop"] = 0
        eachPerson_data["first_bound"] = 0
        eachPerson_data["second_stop"] = 0
        eachPerson_data["three_plus_stop"] = 0
        eachPerson_data["second_bound"] = 0
        eachPerson_data["first_tour_dummy"] = 0
        eachPerson_data["tour_remain"] = 0

        #appending the availability
        eachPerson_data["isg_Work_AV"] = 1
        eachPerson_data["isg_Education_AV"]= 1
        eachPerson_data["isg_Shopping_AV"]= 1
        eachPerson_data["isg_Others_AV"]= 1
        eachPerson_data["isg_Quit_AV"]= 1
        
        all_data.append(eachPerson_data)
        cnt = cnt +1
        if cnt % 100 == 0:
            write_MongoDB(all_data, modelname)
            print ('Person data inserted:', cnt)
            all_data = []
        print ('\rPerson variables fetched in %fs' %(datetime.datetime.now() - Start).total_seconds())
    #Calling the write_DB function to write the value in mongoDB
    print(cnt)
    if len(all_data) > 0:
        cnt = cnt + len(all_data)
        write_MongoDB(all_data, modelname)
    print("Completed Inserting "+ str(cnt)+" values to Mongo DB for model:" + modelname)
    print ("Done!")
    return


#----------------Function that populates all the data for all the variables in mongo db that is needed for the model Intermediate stop generation------------------------------
def itd():
    modelname = sys.argv[1]
    person_data = []
    all_data = []
    income_data = []
    choiceset_data = []
    with open("person_database.csv", "r") as f ,  open("IncomeIndex14.csv", "r") as c, open("choiceset_itd.csv", "r") as b:
        person_reader = csv.reader(f,delimiter=",")
        choiceset_reader = csv.reader(b,delimiter=",")
        income_reader = csv.reader(c,delimiter=",")
        #Reading values from hits_person.csv file
        next(person_reader, None)
        for row in person_reader:
            person_data.append(row)
        #Reading values from IncomeIndex14.csv file
        for row in income_reader:
            income_data.append(row)
        #Reading values from choiceset_itd.csv file
        for row in choiceset_reader:
            choiceset_data.append(row)
    f.close()
    c.close()
    b.close()
    print("Completed reading data from all person data")

    cnt = 0
    #Appending data to a list of dict where each row or document represents data for each person*************************************************
    for i in person_data:
        Start = datetime.datetime.now()
        eachPerson_data = OrderedDict()
        eachPerson_data["_id"] = i[0]+"-"+i[1]
        eachPerson_data["first_bound"] = 0
        eachPerson_data["second_bound"] = 0
        eachPerson_data["stop_type"] = 0

        #Fetching values for variables "var_missingincome", "var_incmid"
        if int(i[11]) >= 13:
            eachPerson_data["missing_income_dummy"] = 1
        else:
            eachPerson_data["missing_income_dummy"] = 0
        for j in income_data:
            if int(j[0]) == int(i[11]):                    
                eachPerson_data["income_mid"] = int(j[2])                

        for i in range(1,49):

            eachPerson_data["TT_%s"%i] = 0
            eachPerson_data["cost_%s"%i] = 0

            eachPerson_data["high_tod"] = 0
            eachPerson_data["low_tod"] = 0

        #appending the availability
        for n,j in enumerate(choiceset_data):
            eachPerson_data[str(j[0])+"_AV"] = 1
        
        
        all_data.append(eachPerson_data)
        cnt = cnt +1
        if cnt % 100 == 0:
            write_MongoDB(all_data, modelname)
            print ('Person data inserted:', cnt)
            all_data = []
        print ('\rPerson variables fetched in %fs' %(datetime.datetime.now() - Start).total_seconds())
    #Calling the write_DB function to write the value in mongoDB
    print(cnt)
    if len(all_data) > 0:
        cnt = cnt + len(all_data)
        write_MongoDB(all_data, modelname)
    print("Completed Inserting "+ str(len(all_data))+" values to Mongo DB for model:" + modelname)
    print ("Done!")
    return
    
#----------------Main function which in turn calls the function specific to each model------------------------------------------------------------------------
def main():
    modelname = sys.argv[1]
    print("Calling the function for model: %s" % (modelname))
    connect_MongoDB()
    removeData_MongoDB(modelname)
    #dp()
    tm()
    #ttd()
    #uw()
    #isg()
    #itd()
    print (modelname+" done!")
    return
    


if  __name__ =='__main__':
    if (len(sys.argv)) != 2:
          print ("Enter the command as python filename.py modelname")
    else:
          main()
          print ("main done!")
