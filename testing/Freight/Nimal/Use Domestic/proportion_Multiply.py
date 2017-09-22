from numpy import genfromtxt
propo = genfromtxt('Proportions.csv', delimiter=';')
print my_data[0][0]

value_Data = genfromtxt('ValueforProportion.csv', delimiter=';')
print value_Data[0]
templist = []
for jj in range(0,71):
    for i in range(0,32):
        for j in range(1,12):
            value_Data[i][j] * propo[i][j]