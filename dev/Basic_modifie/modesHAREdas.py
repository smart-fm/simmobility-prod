import os
os.system('cat activity_schedule*.log > activity_schedule')


modeShareDAS = {}
# reading the simulated data
with open('activity_schedule', 'r') as f:
    for row in f:
        listed = row.strip().split(',')
        if (1==1):# (listed[2] == 'Other') : #(no bound on time limits!) and float(listed[9]) >= 17.00 and float(listed[9]) <= 21.00 :

            if listed[7] in modeShareDAS:
                modeShareDAS[listed[7]] += 1
            else:
                modeShareDAS[listed[7]] = 1


f.close()
#print modeShareDAS

s = sum(modeShareDAS.values())*1.0
for i in modeShareDAS :
    modeShareDAS[i] /= (s/100)
    modeShareDAS[i] = round(modeShareDAS[i],2)
#print modeShareDAS


for i in modeShareDAS :

        print i, ' : ', '       ',':',modeShareDAS[i]

