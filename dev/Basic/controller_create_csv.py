import matplotlib.pyplot as plt
import csv

start_time = 1080
end_time = 2520

##################################################
##################################################
##################################################

with open('controller.log') as f:
	content = f.readlines()

lines = [x.strip() for x in content]

max_fleet_size = -1
curr_fleet_size = -1

rows = {}

curr_tick = 0

# Parse log and write to CSV
with open('mobilityService.csv', 'wb') as csvfile:
	# writer = csv.writer(csvfile, delimiter=' ', quotechar='|', quoting=csv.QUOTE_MINIMAL)
	# writer.writerow(['User ID', 'Service ID', 'Driver ID', 'Request Creation Time', 'Origin Location Center',
	# 	'Origin Location Radius', 'Destination Location Center', 'Destination Location Radius',
	# 	'Earliest Requested Pickup', 'Latest Requested Pickup', 'Earliest Requested Drop-off',
	# 	'Latest Requested Drop-off', 'Actual Start Location', 'Actual Destination Location',
	# 	'Request Processed Timestamp', 'Actual Pickup Time', 'Actual Drop-off Time'])

	writer = csv.DictWriter(csvfile, ['User ID', 'Service ID', 'Driver ID', 'Request Creation Time', 'Origin Location Center',
		'Origin Location Radius', 'Destination Location Center', 'Destination Location Radius',
		'Earliest Requested Pickup', 'Latest Requested Pickup', 'Earliest Requested Drop-off',
		'Latest Requested Drop-off', 'Actual Start Location', 'Actual Destination Location',
		'Request Processed Timestamp', 'Actual Pickup Time', 'Actual Drop-off Time'])
	writer.writeheader()

	for line in lines:
		if 'Maximum number of taxis' in line:
			max_fleet_size = int(line[24:])
		elif 'Number of taxis' in line:
			curr_fleet_size = int(line[16:])
			if curr_fleet_size > max_fleet_size:
				print "Warning: number of taxis was thought to be", curr_fleet_size,\
					"but was actually", max_fleet_size

		if 'Approximate Tick Boundary:' in line:
			end = line.find(',')
			curr_tick = int(line[27:end])

		if 'Request made from' in line:
			if curr_tick >= start_time and curr_tick < end_time:
				user_id_end = line.find(' at time')

				user_id = line[18:user_id_end]

				# time_end = line.find('. Message was sent at')
				origin_location_center_start = line.find('startNodeId ') + 12
				origin_location_center_end = line.find(', destinationNodeId ')
				destination_location_center_start = origin_location_center_end + 20
				destination_location_center_end = line.find(', and driverId')

				if user_id in rows and 'Actual Destination Location' in rows[user_id][-1]:
					rows[user_id].append({'User ID': user_id})
				else:
					rows[user_id] = [{'User ID': user_id}]

				request_dict = rows[user_id][-1]

				# request_dict['Request Creation Time'] = int(line[user_id_end+9:time_end])
				request_dict['Request Creation Time'] = curr_tick
				request_dict['Origin Location Center'] = int(line[origin_location_center_start:origin_location_center_end])
				request_dict['Origin Location Radius'] = 0
				request_dict['Destination Location Center'] = int(line[destination_location_center_start:destination_location_center_end])
				request_dict['Destination Location Radius'] = 0
				request_dict['Earliest Requested Pickup'] = 0
				request_dict['Latest Requested Pickup'] = 1000000
				request_dict['Earliest Requested Drop-off'] = 0
				request_dict['Latest Requested Drop-off'] = 1000000

		if 'Request received from' in line:
			user_id_end = line.find(' at time')

			user_id = line[22:user_id_end]

			if user_id in rows and 'Actual Destination Location' not in rows[user_id][-1]:
				time_start = user_id_end + 9
				time_end = line.find('. Message was sent at')

				rows[user_id][-1]['Request Processed Timestamp'] = int(line[time_start:time_end])

		if 'Pickup succeeded for' in line:
			user_id_end = line.find(' at time')

			user_id = line[21:user_id_end]

			if user_id in rows and 'Actual Start Location' not in rows[user_id][-1]:
				# time_start = user_id_end + 9
				# time_end = line.find('. Message was sent at')
				origin_location_center_start = line.find('startNodeId ') + 12
				origin_location_center_end = line.find(', destinationNodeId ')
				driver_id_start = line.find('driverId ') + 9

				request_dict = rows[user_id][-1]

				request_dict['Driver ID'] = int(line[driver_id_start:])
				request_dict['Actual Start Location'] = int(line[origin_location_center_start:origin_location_center_end])
				# request_dict['Actual Pickup Time'] = int(line[time_start:time_end])
				request_dict['Actual Pickup Time'] = curr_tick

		if 'Drop-off for' in line:
			user_id_end = line.find(' at time')

			user_id = line[13:user_id_end]

			if user_id in rows and 'Actual Destination Location' not in rows[user_id][-1]:
				# time_start = user_id_end + 9
				# time_end = line.find('. Message was sent at')
				destination_location_center_start = line.find(', destinationNodeId ') + 20
				destination_location_center_end = line.find(', and driverId')

				request_dict = rows[user_id][-1]

				request_dict['Actual Destination Location'] = int(line[destination_location_center_start:destination_location_center_end])
				# request_dict['Actual Drop-off Time'] = int(line[time_start:time_end])
				request_dict['Actual Drop-off Time'] = curr_tick


	for k in rows:
		for i in rows[k]:
			writer.writerow(i)
