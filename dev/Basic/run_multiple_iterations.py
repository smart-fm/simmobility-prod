import os
os.system('rm RMSN_records*.txt Iteration_Information.txt')

numberOfIterationsOfPreday = 3
ratio = 3 # how many iterations of withinday per iteration of preday
for i in range(numberOfIterationsOfPreday):

	# update alpha value in TravelTimeAggregator python script
	k = numberOfIterationsOfPreday + 1
	alpha = k / (k + 1.0 )
	systemCommandToUpdateAlphaValue = "sed -i 's/ALPHA.*/ALPHA="+ str(alpha) + "/g' "  + "scripts/python/TravelTimeAggregator.py" ;
	os.system(systemCommandToUpdateAlphaValue)

	# update the config file to specify whether to run "full" mode
	systemCommandToChangeRunMode = "sed -i 's/<mid_term_run_mode value.*/<mid_term_run_mode value = \"full\"\/>/g' data/simrun_MidTerm.xml"
	os.system(systemCommandToChangeRunMode)

	os.system('echo \"Running Full Mode: Iteration Number: '+str(i+1)+ ' \" >> Iteration_Information.txt')
	os.system('Release/SimMobility_Medium')


	for j in range(ratio):
		# update the config file to specify whether to run "full" mode
		systemCommandToChangeRunMode = "sed -i 's/<mid_term_run_mode value.*/<mid_term_run_mode value = \"supply\"\/>/g' data/simrun_MidTerm.xml"
		os.system(systemCommandToChangeRunMode)

		os.system('echo \"..............Running Supply: Iteration Number: '+str(j+1)+ ' \" >> Iteration_Information.txt')
		os.system('Release/SimMobility_Medium')