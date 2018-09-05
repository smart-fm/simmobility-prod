import csv
import matplotlib.pyplot as plt
from collections import OrderedDict
import numpy as np

GRANULARITY = 5
requiredlines = OrderedDict()

with open ('onCall_taxi_trajectory.csv', 'rb') as file:
	reader = csv.reader(file)

	for line in reader:
		if line[3].split(':')[-1] == '00' and int(line[3].split(':')[1]) % GRANULARITY == 0:
			key = line[3]
			value = (line[1], line[6])
			if key not in requiredlines:
				requiredlines[key] = set()
			requiredlines[key].add(value)

cruising = [0]*len(requiredlines)
driving = [0]*len(requiredlines)
parked = [0]*len(requiredlines)
withPassenger = [0]*len(requiredlines)
driveparking = [0]*len(requiredlines)
total = [0]*len(requiredlines)

for i, time in enumerate(requiredlines.values()):
	for action in time:
		if action[1] == 'CRUISING':
			cruising[i] += 1
		elif action[1] == 'DRIVE_ON_CALL':
			driving[i] += 1
		elif action[1] == 'PARKED':
			parked[i] += 1
		elif action[1] == 'DRIVE_WITH_PASSENGER':
			withPassenger[i] += 1
		elif action[1] == 'DRIVE_TO_PARKING':
			driveparking[i] +=1

		total[i] += 1

plt.plot(cruising, label='Cruising')
plt.plot(driving, label='Drive to Pickup')
plt.plot(parked, label='Parked')
plt.plot(withPassenger, label='Drive with Passenger')
plt.plot(driveparking, label='Drive to Parking')
plt.plot(total, label='Total')

plt.title('Vehicle Utilisation')
plt.legend()
plt.xlabel('Time')
plt.xticks([])
plt.ylabel('Number of Drivers')
plt.show()
