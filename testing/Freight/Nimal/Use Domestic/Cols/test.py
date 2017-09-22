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
data = pd.read_csv('hs4.csv', names=colnames)
HS4srch = data.A.tolist()
Pounds = data.B.tolist()

colnames = ['A','B','C']
data = pd.read_csv('Cleandata.csv', names=colnames)
HS4 = data.B.tolist()

poundlist = 0*[len(Pounds)]

for i in range(0,len(HS4srch)):
    for j in range(0,len(HS4)):
        if HS4srch[i] == HS4[j]:
            poundlist.append(Pounds[i])


