import csv
import numpy as np
import string
from datetime import datetime
import pandas as pd
import math
import time

from numpy import genfromtxt


#colnames = ['A']
#data = pd.read_csv('hs4.csv', names=colnames)
#IOa = data.A.tolist()
#
#colnames = ['B']
#data = pd.read_csv('usd_per_tonne.csv', names=colnames)
#IOb = data.B.tolist()
#print IOb[1],len(IOa)
#with open('HSDollar.csv', 'wb') as store:
#    storeWriter = csv.writer(store, delimiter=',')
#    for i in range(0,len(IOa)/2):
#        row = [IOa[i],IOa[(len(IOa)/2)+i]]
#        ##print row
#        storeWriter.writerow(row)  # [my_data[i,:]])
#    store.close()

colnames = ['A','B']
data = pd.read_csv('HSDollar.csv', names=colnames)
HS4srch = data.A.tolist()
Pounds = data.B.tolist()

colnames = ['A','B','C']
data = pd.read_csv('CleanData.csv', names=colnames)
HS4 = data.B.tolist()
print len(HS4srch),len(HS4)
thirteens = data.C.tolist()
apndd =  [0] * len(HS4srch)
zeroup = [0] * len(HS4srch)
for j in range(0,len(HS4srch)):
    apndd[j] = int(HS4srch[j])
    zeroup[j] = (str(apndd[j]).zfill(4))

#print zeroup
thirteentoDollar = []

#for i, j in enumerate(HS4):

a = range(10)
print a
del a[-2]
print a

j=0

for i in range(0,len(HS4)):
    poundlist = []
    lent = len(HS4srch)
#    for j in range(0,lent):
    while j<lent:
        #print i, int(HS4srch[i]), str(HS4[j])

         if str(int(HS4srch[j])) == str(HS4[i]):
            poundlist.append(Pounds[j])
            del HS4srch[j],Pounds[j]
            if len(HS4srch) == 0:
                break
            print j
         else:
           j =j+1

               #HS4srch[j] = 0
            #lent = len(HS4srch)
           # print i, (HS4srch[j]), str(HS4[i])
    print i,sum(poundlist),str(HS4[i])
    if len(poundlist):
        thirteentoDollar.append((sum(poundlist))/len(poundlist))
    else:
        thirteentoDollar.append((0))
print len(thirteentoDollar),thirteentoDollar

with open('haldanhour.csv', 'wb') as store:
    storeWriter = csv.writer(store, delimiter=',')
    for i in range(0, len(thirteentoDollar)):
# [x.replace(",", ".") if x == "," else x for x in row]
         storeWriter.writerow([thirteentoDollar[i]])
store.close()



        ##print row
        #         storeWriter.writerow(row)  # [my_data[i,:]])
        #     store.close()