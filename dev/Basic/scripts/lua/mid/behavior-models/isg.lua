--[[
Model - intermediate stop generation
Type - MNL
Authors - Siyu Li, Harish Loganathan
]]

-- require statements do not work with C++. They need to be commented. The order in which lua files are loaded must be explicitly controlled in C++. 
--require "Logit"

--Estimated values for all betas
--Note= the betas that not estimated are fixed to zero.

local beta_cons_work = -0.428
local beta_cons_edu = -1.25
local beta_cons_shopping = -1.42
local beta_cons_other = -1.30
local beta_cons_Q = 0


local beta_first_stop_inbound = 0
local beta_second_stop_inbound = 1.52
local beta_threeplus_stop_inbound = 1.57
local beta_first_stop_outbound = -0.175
local beta_second_stop_outbound = 1.38
local beta_threeplus_stop_outbound = 2.28


local beta_work_tour_dummy_Q = 0
local beta_edu_tour_dummy_Q = 0
local beta_shopping_tour_dummy_Q = 0
local beta_other_tour_dummy_Q = 0


local beta_first_tour_dummy_Q = -0.936
local beta_sub_tour_dummy_Q = -0.667
local beta_zero_tour_remain_Q = 0
local beta_one_tour_remain_Q = 0.396
local beta_twoplus_tour_remain_Q = 0.419


local beta_work_tour_dummy_W = 0
local beta_edu_tour_dummy_W = -1.03
local beta_shopping_tour_dummy_W = -1.71
local beta_other_tour_dummy_W = -0.684
local beta_female_dummy_W = -0.0880
local beta_student_dummy_W = 0.337
local beta_worker_dummy_W = 0.112
local beta_driver_dummy_W = 0.429
local beta_passenger_dummy_W = 0.250
local beta_public_dummy_W = -0.0911


local beta_work_tour_dummy_E = -0.289
local beta_edu_tour_dummy_E = 0
local beta_shopping_tour_dummy_E = 0.166
local beta_other_tour_dummy_E = 0.208
local beta_female_dummy_E = -0.0171
local beta_student_dummy_E = 0.328
local beta_worker_dummy_E = 0.713
local beta_driver_dummy_E = 0.629
local beta_passenger_dummy_E = 0.355
local beta_public_dummy_E = 0.259


local beta_work_tour_dummy_S = 0.467
local beta_edu_tour_dummy_S = 0.190
local beta_shopping_tour_dummy_S = 0
local beta_other_tour_dummy_S = 0.369
local beta_female_dummy_S = 0.0175
local beta_student_dummy_S = 0.118
local beta_worker_dummy_S = -0.0733
local beta_driver_dummy_S = 0.215
local beta_passenger_dummy_S = -0.0831
local beta_public_dummy_S = 0.0421


local beta_work_tour_dummy_O = 0.0036
local beta_edu_tour_dummy_O = -0.229
local beta_shopping_tour_dummy_O = 0.0129
local beta_other_tour_dummy_O = 0
local beta_female_dummy_O = 0.0164
local beta_student_dummy_O = 0.171
local beta_worker_dummy_O = -0.127
local beta_driver_dummy_O = 0.800
local beta_passenger_dummy_O = 0.566
local beta_public_dummy_O = 0.160


local beta_work_logsum = 0
local beta_edu_logsum = 0
local beta_shop_logsum = 0
local beta_other_logsum = 0


local beta_time_window_work = 0 
local beta_time_window_edu = 0
local beta_time_window_shopping = 0
local beta_time_window_other = 0


local beta_tour_distance_work = 0.0802 
local beta_tour_distance_edu = 0.0891 
local beta_tour_distance_shopping = 0.108
local beta_tour_distance_other = 0.163


local beta_a700_a930_work = -3.18
local beta_a930_a1200_work = -1.31
local beta_p300_p530_work = -0.371
local beta_p530_p730_work = -0.333
local beta_p730_p1000_work = -0.951
local beta_p1000_a700_work = -2.04


local beta_a700_a930_edu = -2.14
local beta_a930_a1200_edu = -0.989
local beta_p300_p530_edu = -0.470
local beta_p530_p730_edu = -0.484
local beta_p730_p1000_edu = -2.62
local beta_p1000_a700_edu = -2.36


local beta_a700_a930_shopping = -3.20
local beta_a930_a1200_shopping = -1.08
local beta_p300_p530_shopping = 0.293
local beta_p530_p730_shopping = 0.282
local beta_p730_p1000_shopping = -0.280
local beta_p1000_a700_shopping = -2.53


local beta_a700_a930_other = -0.422
local beta_a930_a1200_other = -0.790
local beta_p300_p530_other = -0.250
local beta_p530_p730_other = -0.126
local beta_p730_p1000_other = -0.766
local beta_p1000_a700_other = -1.11

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
	
	local time_window_h = 0
	
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
	beta_time_window_work * time_window_h +
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
	beta_time_window_edu * time_window_h +
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
	beta_time_window_shopping * time_window_h +
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
	beta_time_window_other * time_window_h +
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
local scale={}
for i = 1, 5 do
	scale[i]=1
end

function choose_isg(params,dbparams)
	computeUtilities(params,dbparams) 
	computeAvailabilities(params,dbparams)
	local probability = calculate_probability("mnl", choice, utility, availability, scale)
	return make_final_choice(probability)
end

