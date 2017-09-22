import csv
import numpy as np
#import matplotlib.pyplot as plt
import string
from datetime import datetime
import pandas as pd


cols = [0,5,1,2,14,16,7,8,6]
with open('rawdata.csv') as csvfile:
    readCSV = csv.reader(csvfile, delimiter=';')
    newlines = []
    cnt =0
    with open ('CleanData.csv','wb') as store:
         storeWriter=csv.writer(store, delimiter=';')
         for row in readCSV:
             for index, item in enumerate(cols):
                 if row[item] == '':
                    row[item] ='NAN'
                 row[item] = row[item].replace(',','.')
#[x.replace(",", ".") if x == "," else x for x in row]
             storeWriter.writerow([row[0],row[5],row[1],row[2],row[14],row[16],row[7],row[8],row[6]])
             #'TRIP_ID; START_DATE; START_TIME;Seconds;Latitude; Longitude;Speed(km/h);RPM;
            #adding TIME extra at end for graph plot

store.close()
with open('CleanData.csv') as csvfile:
    #has_header = csv.Sniffer().has_header(csvfile.read(1024))
    #csvfile.seek(0)  # rewind
    readCSV = csv.reader(csvfile,delimiter=';')
    #if has_header:
    next(readCSV)
    #readCSV = csv.reader(csvfile, delimiter=';')
    maxspeed   = 0
    idletime = 0;
    traveltime = 0
    cnt        = 0
    with open ('Output.csv','wb') as store1:
         storeWriter=csv.writer(store1, delimiter=';')
         for row in readCSV:
             if cnt == 0:
                cnt = 1
                TripID = int(row[0])
                maxspeed = float(row[4])
                if float(row[4])==0:
                    idletime = idletime + 1

             else:
                Tripnu = int(row[0])
                if float(row[4]) == 0:
                    idletime = idletime + 1
                if TripID != Tripnu:
                    storeWriter.writerow([TripID,traveltime,maxspeed,idletime])
                    TripID = int(row[0])
                    idletime = 0
                else:
                    traveltime = int(row[1])
                    if float(row[4]) == 0:
                        idletime = idletime + 1
                    if maxspeed < float(row[4]):
                       maxspeed = float(row[4])
         storeWriter.writerow([TripID, traveltime,maxspeed,idletime])
store1.close()

csvdata = np.genfromtxt ('Output.csv', delimiter=";")
first = csvdata[:,0];
second = csvdata[:,1]
third = csvdata[:,2]
print second
i= second.argsort()

#print i,len(i)
maxtravelTrip = first[len(i)-1]
MaxTotalTravelTime = second[len(i)-1]
print "Max Total Travel Time: %d \nTrip ID: %d" % (MaxTotalTravelTime,maxtravelTrip)
csvdata = np.genfromtxt ('CleanData.csv', delimiter=";")

Timestr = np.array([])
y = np.array([])
with open('CleanData.csv') as csvfile:
    #has_header = csv.Sniffer().has_header(csvfile.read(1024))
    #csvfile.seek(0)  # rewind
    readCSV = csv.reader(csvfile,delimiter=';')
    next(readCSV)
    for row in readCSV:
        if float(row[0]) == maxtravelTrip:
            if row[8] != 'NAN':
                Timestr = np.append(Timestr,row[8],)
                y = np.append(y,float(row[4]))

        label1 = "start time: %s \n Total TravelTime %d s" %(row[3],MaxTotalTravelTime)
fmt = '%Y-%m-%d %H:%M:%S.%f'

x=pd.to_datetime(Timestr)

#print x
#print y
#datetime.strptime("2012-may-31 19:00", "%Y-%b-%d %H:%M")
#datetime.datetime(2012, 5, 31, 19, 0)

#x = np.array([datetime.datetime(2013, 9, 28, i, 0) for i in range(24)])
#y = np.random.randint(100, size=x.shape)
#plt.plot(x,y,  label = label1)
#plt.xlabel('Timeperiod')
#plt.ylabel('Speed Km/h')
#plt.legend()
#plt.show()

