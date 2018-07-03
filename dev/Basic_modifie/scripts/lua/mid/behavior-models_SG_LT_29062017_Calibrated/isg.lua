--[[
Model - intermediate stop generation
Type - MNL
Authors - Siyu Li, Harish Loganathan
]]

-- require statements do not work with C++. They need to be commented. The order in which lua files are loaded must be explicitly controlled in C++. 
--require "Logit"

--Estimated values for all betas
--Note= the betas that not estimated are fixed to zero.

local beta_cons_work = -5.24
local beta_cons_edu = -1.78
local beta_cons_shopping = -3.87
local beta_cons_other = -1.61
local beta_cons_Q = 0


local beta_first_stop_inbound = 0
local beta_second_stop_inbound = 5.31
local beta_threeplus_stop_inbound = 1.73
local beta_first_stop_outbound = 0.233
local beta_second_stop_outbound = 2.3
local beta_threeplus_stop_outbound = 2.23


local beta_work_tour_dummy_Q = 0
local beta_edu_tour_dummy_Q = 0
local beta_shopping_tour_dummy_Q = 0
local beta_other_tour_dummy_Q = 0


local beta_first_tour_dummy_Q = -1.4
local beta_sub_tour_dummy_Q = -0.0218
local beta_zero_tour_remain_Q = 0
local beta_one_tour_remain_Q = 0.506
local beta_twoplus_tour_remain_Q = 0


local beta_work_tour_dummy_W = 0
local beta_edu_tour_dummy_W = 0
local beta_shopping_tour_dummy_W = 0
local beta_other_tour_dummy_W = 0
local beta_female_dummy_W = 0.0908
local beta_student_dummy_W = 0
local beta_worker_dummy_W = 0
local beta_driver_dummy_W = 0.354
local beta_passenger_dummy_W = 0.119
local beta_public_dummy_W = -0.0553


local beta_work_tour_dummy_E = 0
local beta_edu_tour_dummy_E = 0
local beta_shopping_tour_dummy_E = 0
local beta_other_tour_dummy_E = 0
local beta_female_dummy_E = -0.174
local beta_student_dummy_E = 0
local beta_worker_dummy_E = 0
local beta_driver_dummy_E = -0.756
local beta_passenger_dummy_E = 0.105
local beta_public_dummy_E = 0.00416


local beta_work_tour_dummy_S = 0
local beta_edu_tour_dummy_S = 0
local beta_shopping_tour_dummy_S = 0
local beta_other_tour_dummy_S = 0.0289
local beta_female_dummy_S = 0.0469
local beta_student_dummy_S = 0
local beta_worker_dummy_S = 0.250
local beta_driver_dummy_S = 0.185
local beta_passenger_dummy_S = 0.0916
local beta_public_dummy_S = 0.119


local beta_work_tour_dummy_O = 0.165
local beta_edu_tour_dummy_O = -0.594
local beta_shopping_tour_dummy_O = -0.106
local beta_other_tour_dummy_O = 0
local beta_female_dummy_O = -0.0377
local beta_student_dummy_O = 0.616
local beta_worker_dummy_O = -0.0365
local beta_driver_dummy_O = 0.161
local beta_passenger_dummy_O = 0.194
local beta_public_dummy_O = -0.135


local beta_work_logsum = 0
local beta_edu_logsum = 0
local beta_shop_logsum = 0
local beta_other_logsum = 0


local beta_time_window_work_first_stop_first_bound = 2.31
local beta_time_window_work_second_stop_first_bound = 3.43
local beta_time_window_work_3plus_stop_first_bound = 0
local beta_time_window_work_first_stop_second_bound = 0.669
local beta_time_window_work_second_stop_second_bound = 3.34
local beta_time_window_work_3plus_stop_second_bound = 0

local beta_time_window_edu_first_stop_first_bound = 0.464
local beta_time_window_edu_second_stop_first_bound = 0
local beta_time_window_edu_3plus_stop_first_bound = 0
local beta_time_window_edu_first_stop_second_bound = 0.389
local beta_time_window_edu_second_stop_second_bound = 0 
local beta_time_window_edu_3plus_stop_second_bound = 0

local beta_time_window_shopping_first_stop_first_bound = 0.790
local beta_time_window_shopping_second_stop_first_bound = 1.65
local beta_time_window_shopping_3plus_stop_first_bound = 0
local beta_time_window_shopping_first_stop_second_bound = 1.31
local beta_time_window_shopping_second_stop_second_bound = 0.493
local beta_time_window_shopping_3plus_stop_second_bound = 0

local beta_time_window_other_first_stop_first_bound = 0.232
local beta_time_window_other_second_stop_first_bound = 1.39
local beta_time_window_other_3plus_stop_first_bound = 0
local beta_time_window_other_first_stop_second_bound = 0.164
local beta_time_window_other_second_stop_second_bound = -0.106
local beta_time_window_other_3plus_stop_second_bound = 0




local beta_tour_distance_work = 0.0981
local beta_tour_distance_edu = 0.131
local beta_tour_distance_shopping = 0.0860
local beta_tour_distance_other = 0.142


local beta_a700_a930_work = -2.64
local beta_a930_a1200_work = -1.45
local beta_p300_p530_work = 0.442
local beta_p530_p730_work = 0.726
local beta_p730_p1000_work = 1.21
local beta_p1000_a700_work = 0.838


local beta_a700_a930_edu = -1.21
local beta_a930_a1200_edu = -2.34
local beta_p300_p530_edu = -0.0204
local beta_p530_p730_edu = -0.187
local beta_p730_p1000_edu =-0.292
local beta_p1000_a700_edu = -0.322


local beta_a700_a930_shopping = -2.57
local beta_a930_a1200_shopping = -0.777
local beta_p300_p530_shopping = 0.663
local beta_p530_p730_shopping = 0.872
local beta_p730_p1000_shopping = -0.0707
local beta_p1000_a700_shopping = -1.56


local beta_a700_a930_other = -0.196
local beta_a930_a1200_other = -0.532
local beta_p300_p530_other = -0.0650
local beta_p530_p730_other = 0.0719
local beta_p730_p1000_other = -0.411
local beta_p1000_a700_other = -0.560

--choice set
--1 for work; 2 for edu; 3 for shopping; 4 for other; 5 for quit
local choice = {
	1,
	2,
	3,
	4,
	5}

local utility = {}
local function computeUtilities(params,dbparams)

	-- when generating intermediate stops for a particular tour, 
	--this variable indicate the tour type. 
	--it can take values from 1 to 4. 
	--1 for work tour, 2 for education tour, 3 for shopping tour and 4 for other tour.
	local tour_type = dbparams.tour_type

	local female_dummy = params.female_dummy
	
	--1 if person_type_id=4, 0 otherwise
	local student_dummy = params.student_dummy

	--1 if person_type_id in [1,2,3,8,9,10], 0 otherwise
	local worker_dummy = params.worker_dummy

	--1 if the tour mode is drive alone (mode id =4), 0 otherwise
	local driver_dummy = dbparams.driver_dummy

	--1 if the tour mode is shared ride 2 or shared ride 3+ (mode id=5,6), 0 otherwise
	local passenger_dummy = dbparams.passenger_dummy

	--1 if the tour mode id is in [1,2,3], 0 otherwise
	local public_dummy = dbparams.public_dummy

	--if we are modeling stops on the first half tour, 
	--the distance is AM[(destination,origin)][’AM2dis’]. 
	--If we are modeling stops on the second half tour, 
	--the distance is PM[(destination,origin)][’PM2dis’]. 
	--origin is home mtz and destination is primary activity location of the tour.
	local distance = dbparams.distance
	local log_constant = 0.0001

	--updated on Sep 11 2014
	local time_window_first_bound = dbparams.time_window_first_bound
	local time_window_second_bound = dbparams.time_window_second_bound 
	
	local worklogsum = 0
	local edulogsum = 0
	local shoplogsum = 0
	local otherlogsum = 0

	--if we are modeling stops on the first half tour, the dummy variable is 1 if the arrival time of tour primary activity is between 7 am to 9:30 am. If we are modeling stops on the second half tour, the dummy variable is 1 if the departure time of tour primary activity is between 7 am to 9:30 am.
	p_700a_930a = dbparams.p_700a_930a
	p_930a_1200a = dbparams.p_930a_1200a 
	p_300p_530p = dbparams.p_300p_530p
	p_530p_730p = dbparams.p_530p_730p
	p_730p_1000p = dbparams.p_730p_1000p
	p_1000p_700a = dbparams.p_1000p_700a

	--1 if the current modeled stop is the first stop on the half tour, 0 otherwise
	first_stop = dbparams.first_stop
	--1 if the current modeled stop is the second stop on the half tour, 0 otherwise
	second_stop = dbparams.second_stop
	--1 if the current modeled stop is the 3 or 3+ stop on the half tour, 0 otherwise
	three_plus_stop = dbparams.three_plus_stop

	--1 if the current modeled stop is on first half tour, 0 otherwise
	first_bound = dbparams.first_bound
	--1 if the current modeled stop is on second half tour, 0 otherwise
	second_bound = dbparams.second_bound

	--1 if the current modeled stop is on the fist tour, 0 otherwise
	first_tour_dummy = dbparams.first_tour_dummy
	
	--1 if the current tour has subtour, 0 otherwise
	has_subtour = dbparams.has_subtour
	
	--number of tours remained to run intermediate stop generation
	tour_remain = dbparams.tour_remain

	utility[1]= beta_cons_work+
	beta_work_tour_dummy_W * 1 * (tour_type==1 and 1 or 0) +
	beta_edu_tour_dummy_W * 1 * (tour_type==2 and 1 or 0) +
	beta_shopping_tour_dummy_W * 1 * (tour_type==3 and 1 or 0) +
	beta_other_tour_dummy_W * 1 * (tour_type==4 and 1 or 0) +
	beta_female_dummy_W * female_dummy +
	beta_student_dummy_W * student_dummy +
	beta_worker_dummy_W * worker_dummy +
	beta_driver_dummy_W * driver_dummy +
	beta_passenger_dummy_W * passenger_dummy +
	beta_public_dummy_W * public_dummy +
	beta_work_logsum * worklogsum+
	beta_time_window_work_first_stop_first_bound * first_bound * first_stop * math.log(log_constant+time_window_first_bound)+
	beta_time_window_work_second_stop_first_bound * first_bound * second_stop * math.log(log_constant+time_window_first_bound)+
	beta_time_window_work_3plus_stop_first_bound * first_bound * three_plus_stop * math.log(log_constant+time_window_first_bound)+
	beta_time_window_work_first_stop_second_bound * second_bound * first_stop * math.log(log_constant+time_window_second_bound)+
	beta_time_window_work_second_stop_second_bound * second_bound * second_stop * math.log(log_constant+time_window_second_bound)+
	beta_time_window_work_3plus_stop_second_bound * second_bound * three_plus_stop * math.log(log_constant+time_window_second_bound)+
	beta_tour_distance_work * math.log(1 + distance) +
	beta_a700_a930_work * p_700a_930a +
	beta_a930_a1200_work * p_930a_1200a +
	beta_p300_p530_work * p_300p_530p +
	beta_p530_p730_work * p_530p_730p +
	beta_p730_p1000_work * p_730p_1000p +
	beta_p1000_a700_work * p_1000p_700a

	utility[2] = beta_cons_edu +
	beta_work_tour_dummy_E * 1 * (tour_type==1 and 1 or 0) +
	beta_edu_tour_dummy_E * 1 * (tour_type==2 and 1 or 0) +
	beta_shopping_tour_dummy_E * 1 * (tour_type==3 and 1 or 0) +
	beta_other_tour_dummy_E * 1 * (tour_type==4 and 1 or 0) +
	beta_female_dummy_E * female_dummy +
	beta_student_dummy_E * student_dummy +
	beta_worker_dummy_E * worker_dummy +
	beta_driver_dummy_E * driver_dummy +
	beta_passenger_dummy_E * passenger_dummy +
	beta_public_dummy_E * public_dummy +
	beta_edu_logsum * edulogsum +
	beta_time_window_edu_first_stop_first_bound * first_bound * first_stop * math.log(log_constant+time_window_first_bound)+
	beta_time_window_edu_second_stop_first_bound * first_bound * second_stop * math.log(log_constant+time_window_first_bound)+
	beta_time_window_edu_3plus_stop_first_bound * first_bound * three_plus_stop * math.log(log_constant+time_window_first_bound)+
	beta_time_window_edu_first_stop_second_bound * second_bound * first_stop * math.log(log_constant+time_window_second_bound)+
	beta_time_window_edu_second_stop_second_bound * second_bound * second_stop * math.log(log_constant+time_window_second_bound)+
	beta_time_window_edu_3plus_stop_second_bound * second_bound * three_plus_stop * math.log(log_constant+time_window_second_bound)+
	beta_tour_distance_edu * math.log(1+distance) +
	beta_a700_a930_edu * p_700a_930a +
	beta_a930_a1200_edu * p_930a_1200a +
	beta_p300_p530_edu * p_300p_530p +
	beta_p530_p730_edu * p_530p_730p +
	beta_p730_p1000_edu * p_730p_1000p +
	beta_p1000_a700_edu * p_1000p_700a

	utility[3] = beta_cons_shopping +
	beta_work_tour_dummy_S * 1 * (tour_type==1 and 1 or 0) +
	beta_edu_tour_dummy_S * 1 * (tour_type==2 and 1 or 0) +
	beta_shopping_tour_dummy_S * 1 * (tour_type==3 and 1 or 0) +
	beta_other_tour_dummy_S * 1 * (tour_type==4 and 1 or 0) +
	beta_female_dummy_S * female_dummy +
	beta_student_dummy_S * student_dummy +
	beta_worker_dummy_S * worker_dummy +
	beta_driver_dummy_S * driver_dummy +
	beta_passenger_dummy_S * passenger_dummy +
	beta_public_dummy_S * public_dummy +
	beta_shop_logsum * shoplogsum +
	beta_time_window_shopping_first_stop_first_bound * first_bound * first_stop * math.log(log_constant+time_window_first_bound)+
	beta_time_window_shopping_second_stop_first_bound * first_bound * second_stop * math.log(log_constant+time_window_first_bound)+
	beta_time_window_shopping_3plus_stop_first_bound * first_bound * three_plus_stop * math.log(log_constant+time_window_first_bound)+
	beta_time_window_shopping_first_stop_second_bound * second_bound * first_stop * math.log(log_constant+time_window_second_bound)+
	beta_time_window_shopping_second_stop_second_bound * second_bound * second_stop * math.log(log_constant+time_window_second_bound)+
	beta_time_window_shopping_3plus_stop_second_bound * second_bound * three_plus_stop * math.log(log_constant+time_window_second_bound)+
	beta_tour_distance_shopping * math.log(1+distance) +
	beta_a700_a930_shopping * p_700a_930a +
	beta_a930_a1200_shopping * p_930a_1200a +
	beta_p300_p530_shopping * p_300p_530p +
	beta_p530_p730_shopping * p_530p_730p +
	beta_p730_p1000_shopping * p_730p_1000p +
	beta_p1000_a700_shopping * p_1000p_700a

	utility[4] = beta_cons_other +
	beta_work_tour_dummy_O * 1 * (tour_type==1 and 1 or 0) +
	beta_edu_tour_dummy_O * 1 * (tour_type==2 and 1 or 0) +
	beta_shopping_tour_dummy_O * 1 * (tour_type==3 and 1 or 0) +
	beta_other_tour_dummy_O * 1 * (tour_type==4 and 1 or 0) +
	beta_female_dummy_O * female_dummy +
	beta_student_dummy_O * student_dummy +
	beta_worker_dummy_O * worker_dummy +
	beta_driver_dummy_O * driver_dummy +
	beta_passenger_dummy_O * passenger_dummy +
	beta_public_dummy_O * public_dummy +
	beta_other_logsum * otherlogsum +
	beta_time_window_other_first_stop_first_bound * first_bound * first_stop * math.log(log_constant+time_window_first_bound)+
	beta_time_window_other_second_stop_first_bound * first_bound * second_stop * math.log(log_constant+time_window_first_bound)+
	beta_time_window_other_3plus_stop_first_bound * first_bound * three_plus_stop * math.log(log_constant+time_window_first_bound)+
	beta_time_window_other_first_stop_second_bound * second_bound * first_stop * math.log(log_constant+time_window_second_bound)+
	beta_time_window_other_second_stop_second_bound * second_bound * second_stop * math.log(log_constant+time_window_second_bound)+
	beta_time_window_other_3plus_stop_second_bound * second_bound * three_plus_stop * math.log(log_constant+time_window_second_bound)+
	beta_tour_distance_other * math.log(1+distance) +
	beta_a700_a930_other * p_700a_930a +
	beta_a930_a1200_other * p_930a_1200a +
	beta_p300_p530_other * p_300p_530p +
	beta_p530_p730_other * p_530p_730p +
	beta_p730_p1000_other * p_730p_1000p +
	beta_p1000_a700_other * p_1000p_700a

	utility[5] = beta_cons_Q +
	beta_first_stop_inbound * first_stop * first_bound +
	beta_second_stop_inbound * second_stop * first_bound +
	beta_threeplus_stop_inbound * three_plus_stop * first_bound +
	beta_first_stop_outbound * first_stop * second_bound +
	beta_second_stop_outbound * second_stop * second_bound +
	beta_threeplus_stop_outbound * three_plus_stop * second_bound +
	beta_work_tour_dummy_Q * 1 * (tour_type==1 and 1 or 0) +
	beta_edu_tour_dummy_Q * 1 * (tour_type==2 and 1 or 0) +
	beta_shopping_tour_dummy_Q * 1 * (tour_type==3 and 1 or 0) +
	beta_other_tour_dummy_Q * 1 * (tour_type==4 and 1 or 0) +
	beta_first_tour_dummy_Q * first_tour_dummy +
	beta_sub_tour_dummy_Q * has_subtour +
	beta_zero_tour_remain_Q * 1 * (tour_remain==0 and 1 or 0)+
	beta_one_tour_remain_Q * 1 * (tour_remain==1 and 1 or 0)+ 
	beta_twoplus_tour_remain_Q*1*(tour_remain>=2 and 1 or 0)
end

--availability
--the logic to determine availability is the same with current implementation
local availability = {}
local function computeAvailabilities(params,dbparams)
	for i = 1, 5 do 
		availability[i] = dbparams:availability(i)
	end
end

--scale
local scale = 1 -- for all choices

-- function to call from C++ preday simulator
-- params and dbparams tables contain data passed from C++
-- to check variable bindings in params or dbparams, refer PredayLuaModel::mapClasses() function in dev/Basic/medium/behavioral/lua/PredayLuaModel.cpp
function choose_isg(params,dbparams)
	computeUtilities(params,dbparams) 
	computeAvailabilities(params,dbparams)
	local probability = calculate_probability("mnl", choice, utility, availability, scale)
	return make_final_choice(probability)
end

