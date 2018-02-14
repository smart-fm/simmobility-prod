from sys import exit
import os
from glob import glob
import psycopg2

try:
	os.remove('RMSN_records*.txt')
except:
	pass
try:
	os.remove('Iteration_Information.txt')
except:
	pass

numberOfIterationsOfPreday = 1	# Is this a good default value?
ratio = 2 # how many iterations of withinday per iteration of preday

# update alpha value in TravelTimeAggregator python script
alpha = (numberOfIterationsOfPreday + 1.0)/(numberOfIterationsOfPreday + 2.0)
os.system("sed -i 's/ALPHA.*=.*/ALPHA =" + str(alpha) + "/g' scripts/python/TravelTimeAggregator.py")

DB_NAME = 'simmobcity_backup'
DB_USER = 'postgres'
DB_HOST = 'localhost'
DB_PORT = '5432'
DB_PASSWORD = 'ITSLab2016!'

conn = psycopg2.connect("dbname=%s user=%s host=%s port=%s password=%s" %(DB_NAME, DB_USER, DB_HOST, DB_PORT, DB_PASSWORD))
cur = conn.cursor()

for i in range(numberOfIterationsOfPreday):
	# update the config file to specify to run "full" mode
	systemCommandToChangeRunMode = "sed -i 's/<mid_term_run_mode value.*/<mid_term_run_mode value = \"full\"\/>/g' data/simrun_MidTerm.xml"
	os.system(systemCommandToChangeRunMode)
	full_mode = 'Running Full Mode: Iteration Number: ' + str(i + 1)
	print '\n' + full_mode
	os.system('echo \"' + full_mode + ' \" >> Iteration_Information.txt')
	resultOfRun = os.system('Release/SimMobility_Medium')
	activity_schedule_files = sorted(glob('activity_schedule*.log'))
	for j in activity_schedule_files:
		os.remove(j)	# removing excess activity_schedule*.log files
	if resultOfRun != 0:
		# halt iteration
		print '\n\nStopping the script. Running in full mode crashed in iteration number ' + str(i + 1)
		exit(0)

	# The following query deletes all SMS trips from DAS, since this branch does not yet have supply for SMS
	cur.execute("DELETE FROM demand.das_calib WHERE stop_mode = 'SMS' OR stop_mode = 'Rail_SMS'")
	conn.commit()

	# update the config file to specify to run "supply" mode
	os.system("sed -i 's/<mid_term_run_mode value.*/<mid_term_run_mode value = \"supply\"\/>/g' data/simrun_MidTerm.xml")

	for j in range(ratio):
		print '\n\nRunning supply, iteration number %i, sub-iteration number %i.\n\n' %(i+1, j+1)
		os.system('echo \"..............Running Supply: Iteration Number: '+str(j+1)+ ' \" >> Iteration_Information.txt')
		resultOfRun = os.system('Release/SimMobility_Medium')
		out_files = sorted(glob('out_*'))
		for i in out_files:
			os.remove(i)	# removing excess out_*.txt files
		if resultOfRun != 0:
			# halt iteration
			print '\n\nStopping the script. Running supply crashed in iteration %i in sub-iteration %i' %(i+1, j+1)
			exit(0)

conn.close()

