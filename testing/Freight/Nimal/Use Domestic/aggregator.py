import csv
import numpy as np
import string
from datetime import datetime
import pandas as pd
import math
import time

cols = [0,5,1,2,14,16,7,8,6]
colnames = ['A', 'B', 'C', 'D']
data = pd.read_csv('Incsv.csv', names=colnames)
HS2 = data.D.tolist()
cntL = len(HS2)

HS4 = data.B.tolist()
IO1 = data.A.tolist()
replace = data.C.tolist()
listofzeros = [0] * cntL
apndd =  [0] * cntL
for j in range(0,cntL,1):
    apndd[j] = str(HS4[j]).zfill(4)


j = 0
with open('Incsv.csv') as csvfile:
    readCSV = csv.reader(csvfile, delimiter=',')
    for j in range(0,cntL,1):

        if HS2[j] > 0:
           for i in range(0, cntL, 1):
               lst2 = str(apndd[i])
               if str(HS2[j]) == lst2[:2]:
                 # #print 'we have a hit' ,lst2[-2:]
                  listofzeros[i] = replace[j]
                  if math.isnan(replace[j]):
                      listofzeros[i] = 0
                 # #print listofzeros[i]
    newlines = []
    cnt =0

#with open('CleanData.csv', 'wb') as store:
#    storeWriter = csv.writer(store)
#    for i in range(0, cntL, 1):
#        # [x.replace(",", ".") if x == "," else x for x in row]
#        if listofzeros[i]:
#            storeWriter.writerow([IO1[i], HS4[i], listofzeros[i]])
#store.close()

#from itertools import groupby
#[len(list(group)) for key, group in groupby(listofzeros)]
colnames = ['A', 'B', 'C']
data = pd.read_csv('CleanData.csv', names=colnames)
IOa = data.A.tolist()
print len(IOa),IOa[1336]
cntL = len(IO1)
HSc = data.B.tolist()
datalst = []
cc=data.C.tolist()

#print len(cc),len(IO1)
import collections
j = 0
pj = 0
vallist = []
keylist = []


for i in range(1,33):

    templist = []#[0] * cntL
    k = 0
    #for j in range(0,len(IOa)):
    #    j = pj
    #    print i,IOa[j]
    #    if IOa[j] == i:
    #        templist.append(cc[j])
    #        IOa[j] = 0
    #    elif IOa[j]:
    #        templist.append(0)
    #

        #else:
          #  print "append"
            #templist.append(0)
    set = 1

    while IOa[j] == i:
          templist.append(cc[j])
          #print IOa[j],j,len(IOa)

          set =0
          j = j + 1
          k = k + 1
          if j == len(IOa)-1:
              break

    if set:
       templist.append((0))
    counter=collections.Counter(templist)

    for tt in range(1,13):
        counter.setdefault(tt,0)
    countlist = counter
    #/#print (countlist) ,countlist[3]
        # Counter({1: 4, 2: 4, 3: 2, 5: 2, 4: 1})
    print i,counter.values()
    vallist.append(counter.values())
    keylist.append(counter.keys())
    #print i,counter.most_common()
   # print i,keylist
for ik in range(0,len(keylist)):
    print len(keylist[ik]),len(vallist),keylist[ik]

##print "value:",len(vallist),vallist[0][14]





apndlist=[0]*13

###########>>> from operator import add
##########>>> map(add, list1, list2

fulllist71 = []
from numpy import genfromtxt
my_data = genfromtxt('ValueFromUse.csv',delimiter=";")  #Replace commas for 1000s speartion using gedit
first   = my_data[:,1]
second  = my_data[1,:]
print len(my_data)
##print len(four[0]),my_data[:,0][:len(four)]
#print len(first),len(second)
import locale
additionlist = []
for ff  in range(0,71):
    tempolist = [0] * 12
    list32 = []
    for i in range(0,len(my_data)):
         templistt = []

         for j in range(0, 12):
             #if my_data[:,1][i]>0:

             #print vallist[i][j]
             if (my_data[:,ff][i])>0:
                # print i,sum(vallist[i]),my_data[:,ff][i]
                 if(my_data[:, ff][i] * np.true_divide(vallist[i][j], sum(vallist[i]))):
                     templistt.append(my_data[:, ff][i] * np.true_divide(vallist[i][j], sum(vallist[i])))
                 else:
                     templistt.append(0)
             else:
                # print i, my_data[:, ff][i]
                 templistt.append(0)
         #print i,templistt
        #
         list32.append((templistt))
         #print len(list32),'**'
         #print list32
         #time.sleep(1)
        # for i in range(0,len(my_data)): #31
    for j in range(0,12):
         lst2 = [item[j] for item in list32]
         #print lst2
            #if math.isnan(list32[i][j]):
            #    list32[i][j] =0
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

             #for ii in range(1,73):
#            if math.isnan(my_data[i,:][ii]):
#                  vallist[i][j]/sum(vallist[i])
#               if math.isnan(my_data[j, :][ii]):
#                  my_data[j, :][ii] = 00
#               else:
                   ##print my_data[i,:][ii],int(my_data[j,:][ii])
               #   my_data[i,:][ii] =((my_data[i,:][ii])+(my_data[j,:][ii]))
                #  my_data[j,:][0] = -1
######from numpy import genfromtxt
######my_data = genfromtxt('addup.csv', delimiter=';')
######first  = my_data[0,:]
######second = my_data[1,:]
######third  = my_data[2,:]
######four   = my_data[4,:]
########print len(four[0]),my_data[:,0][:len(four)]
######import locale
######
######
######for i in range(0,len(my_data)):
######    for j in range(i+1, len(my_data)):
######        if my_data[i,:][0]>0:
######            if int(my_data[i,:][0]) == int(my_data[j,:][0]):
######
######                for ii in range(1,73):
######                    if math.isnan(my_data[i,:][ii]):
######
######                        my_data[i,:][ii] = 00
######                    if math.isnan(my_data[j, :][ii]):
######                        my_data[j, :][ii] = 00
######                    else:
######                        ##print my_data[i,:][ii],int(my_data[j,:][ii])
######
######                        my_data[i,:][ii] =((my_data[i,:][ii])+(my_data[j,:][ii]))
######                        my_data[j,:][0] = -1
######
print "hiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
with open('nuu.csv', 'wb') as store:
    storeWriter = csv.writer(store, delimiter=';')

    for i in range(0, 71):
        row = [fulllist71[i][ii] for ii in range(0,12)]
        ##print row
        storeWriter.writerow(row)  # [my_data[i,:]])
    store.close()




#with open ('aggre.csv','wb') as store:
#     storeWriter=csv.writer(store, delimiter=';')
#     for i in range(0, 30, 1):
#        row = [my_data[i,:][ii] for ii in range(0,73)]
#  #[x.replace(",", ".") if x == "," else x for x in row]
#        if (my_data[i,:][0]) != -1:
#            storeWriter.writerow(row)#[my_data[i,:]])
#     store.close()