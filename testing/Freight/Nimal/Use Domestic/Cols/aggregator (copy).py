import csv
import numpy as np
import string
from datetime import datetime
import pandas as pd
import math
import time

cols = [0,5,1,2,14,16,7,8,6]
colnames = ['A', 'B', 'C', 'D']
data = pd.read_csv('Industry_SSIC.csv', names=colnames)
HS2 = data.D.tolist()
SS = data.C.tolist()
cntL = len(HS2)

HS4 = data.B.tolist()
IO1 = data.A.tolist()

#for jj in range(0, len(HS4)):
 #   print HS2[jj]

replace = data.C.tolist()
listofzeros = [0] * cntL
apndd =  [0] * len(HS4)
zeroup = [0] * len(HS4)
for j in range(0,len(HS4)):
    if math.isnan(SS[j]):
        SS[j] = 0
    apndd[j] = int(SS[j])
    zeroup[j] = (str(apndd[j]).zfill(2))
    #print SS[j],len(str(apndd[j]).zfill(2))
   # print zeroup[j]

newlist = [0] * len(HS4)
for j in   range(0,len(HS4)):
    nn  = len(zeroup[j])
    for jj in range(0,len(HS4)):
        tmp = str(HS4[jj]).zfill(5)
        #print tmp[:nn],zeroup[j]
        if zeroup[j] >0:
            if tmp[:nn] == (zeroup[j]):
              newlist[jj] = int(HS2[j])
          # print j,jj,nn,tmp[:nn],apndd[j],HS2[j]
        #print j,newlist[j]

#with open('CleanDataCols1.csv', 'wb') as store:
#    storeWriter = csv.writer(store, delimiter=',')
#    for i in range(0, len(HS4)):
#        # [x.replace(",", ".") if x == "," else x for x in row]
#        if newlist[i]:
#            storeWriter.writerow([IO1[i], HS4[i], newlist[i]])
#        else:
#            print "howlyvwfe"
#store.close()


j = 0
for j in range(0,cntL,1):
     if HS2[j] > 0:
        for i in range(0, cntL, 1):
            lst2 = str(apndd[i])
            if str(HS2[j]) == lst2[:2]:
              # #print 'we have a hit' ,lst2[-2:]
               listofzeros[i] = replace[j]
              # #print listofzeros[i]
newlines = []
cnt =0

#with open('CleanData.csv', 'wb') as store:
#    storeWriter = csv.writer(store, delimiter=';')
#    for i in range(0, cntL, 1):
#        # [x.replace(",", ".") if x == "," else x for x in row]
#        storeWriter.writerow([IO1[i], HS4[i], listofzeros[i]])
#store.close()

#from itertools import groupby
#[len(list(group)) for key, group in groupby(listofzeros)]
colnames = ['A', 'B', 'C']
data = pd.read_csv('CleanDataCols1.csv', names=colnames)
IOa = data.A.tolist()
cntL = len(IO1)
HSc = data.B.tolist()
datalst = []
cc=data.C.tolist()
import collections
counter=collections.Counter(cc)

#print counter.values()
#print counter.keys()
j = 0
vallist = []
keylist = []

for i in range(0,71,1):
    templist = []#[0] * cntL
    k = 0
    while IOa[j] == i:
         templist.append(cc[j])
         j = j+1
         k = k+1
        # print i,j,IOa[j],cc[j]
    counter=collections.Counter(templist)
    for tt in range(1,43):
        counter.setdefault(tt,0)
    countlist = counter
    #/#print (countlist) ,countlist[3]
        # Counter({1: 4, 2: 4, 3: 2, 5: 2, 4: 1})
    vallist.append(counter.values())
    keylist.append(counter.keys())
   # #print counter

for ik in range(0,len(keylist)):
    print len(keylist[ik]),len(vallist),keylist[ik]



apndlist=[0]*len(keylist[0])

###########>>> from operator import add
##########>>> map(add, list1, list2

fulllist71 = []
from numpy import genfromtxt
my_data = genfromtxt('nuu.csv', delimiter=';')
first   = my_data[:,1]
second  = my_data[1,:]
print first
##print len(four[0]),my_data[:,0][:len(four)]
#print len(my_data)
import locale
additionlist = []
for ff  in range(0,12):
    tempolist = [0] * 71
    list71 = []
    for i in range(1,len(my_data)):
         templistt = []

         for j in range(0, len(vallist[i])):
             #if my_data[:,1][i]>0:

             if int(my_data[:,ff][i]>0):
                 if(my_data[:, ff][i] * np.true_divide(vallist[i][j], sum(vallist[i]))):
                     templistt.append(my_data[:, ff][i] * np.true_divide(vallist[i][j], sum(vallist[i])))
                 else:
                     templistt.append(0)
             else:
                 templistt.append(0)
         #print templistt
        #
         list71.append((templistt))
         #print len(list32),'**'
         #print list32
         #time.sleep(1)
        # for i in range(0,len(my_data)): #31
    for j in range(0,42):
         lst2 = [item[j] for item in list71]
         #print lst2
         for kk in range (0,len(lst2)):
             if math.isnan(lst2[kk]):
                lst2[kk] =0
         tempolist[j] = sum(lst2)
             #print ff,tempolist
         #time.sleep(1)
    fulllist71.append(tempolist)
#    #print "\n"
    ##print tempolist

    ##print ff,fulllist71
print len(fulllist71[0])




with open('1345.csv', 'wb') as store:
    storeWriter = csv.writer(store, delimiter=';')

    for i in range(0, 12):
        row = [fulllist71[i][ii] for ii in range(0,45)]
        ##print row
        storeWriter.writerow(row)  # [my_data[i,:]])
    store.close()


from numpy import genfromtxt
my_data = genfromtxt('1345.csv', delimiter=';')
first   = my_data[:,0]

print sum(first)

with open('final_proption_1345.csv', 'wb') as store:
    storeWriter = csv.writer(store, delimiter=';')

    for i in range(0, 12):
        row = [my_data[:,ii][i]/sum(my_data[:,ii]) for ii in range(0,45)]
        ##print row
        storeWriter.writerow(row)  # [my_data[i,:]])
    store.close()
