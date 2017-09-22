
import csv
import numpy as np
import string
from datetime import datetime
import pandas as pd
import math
import time

from numpy import genfromtxt


colnames = ['A','B','C']
data = pd.read_csv('value-to-weights_of_13_commodities.csv', names=colnames)
USDpPound = data.C.tolist()

#df = pd.read_csv('1345.csv')

#newmul = df.mul(int(str(USDpPound)),0)


from numpy import genfromtxt
my_data = genfromtxt('1345.csv', delimiter=';')
first   = my_data[1,:]
print  (first[1])

finishedlist = []


for ff in range(0,12):
    rowwise = []
    for j in range(0,len(my_data[ff,:])):
        rowwise.append(my_data[ff,:][j] / USDpPound[ff])
    finishedlist.append(rowwise)

print len(finishedlist[0])

with open('1345PoundConverted.csv', 'wb') as store:
    storeWriter = csv.writer(store, delimiter=',')
    for i in range(0, 12):
        row = [finishedlist[i][ii] for ii in range(0, len(finishedlist[i]))]
        ##print row
        storeWriter.writerow(row)  # [my_data[i,:]])
    store.close()

with open('final_proption_1345.csv', 'wb') as store:
    storeWriter = csv.writer(store, delimiter=';')

    for i in range(0, 12):
        row = [my_data[:,ii][i]/sum(my_data[:,ii]) for ii in range(0,len(finishedlist[i]))]
        ##print row
        storeWriter.writerow(row)  # [my_data[i,:]])
    store.close()