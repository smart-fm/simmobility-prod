--[[
Model - Mode choice for education tour
Type - NL
Authors - Siyu Li, Harish Loganathan
]]

-- all require statements do not work with C++. They need to be commented. The order in which lua files are loaded must be explicitly controlled in C++. 
--require "NLogit"

--Estimated values for all betas
--Note: the betas that not estimated are fixed to zero.

-------------------------------------------------
-- The variables having name format as [ beta_cons_<modeName> ] are used to store the Alternate Specific Constants(also called ASCs)
-- These constants are added into the utility calculation later
-- An increase in the [ beta_cons_<modeName> ] for any mode will result in an increase in the percentage of mode shares being increased for this model
local beta_cons_bus = -7.5
local beta_cons_mrt = - 8
local beta_cons_privatebus= -8.02
local beta_cons_drive1= 2.252
local beta_cons_share2= -10
local beta_cons_share3= -9.957
local beta_cons_motor= 9.992
local beta_cons_walk= -7 
local beta_cons_taxi=  -11.334
local beta_cons_SMS = -10.8
local beta_cons_Rail_SMS =-12.4
local beta_cons_SMS_Pool = -14
local beta_cons_Rail_SMS_Pool = -12



-------------------------------------------------
-- The variables having name format as  [ beta_<modeName>_tt ]  are coefficients for travel time 
-- These are multiplied by the travel time for the respective modes
local beta1_1_tt = -0.687
local beta1_2_tt = -0.690
local beta1_3_tt = -1.06
local beta_private_1_tt= -0.659
local beta2_tt_drive1 = -0.736
local beta2_tt_share2 = -1.42
local beta2_tt_share3 = -1.18
local beta2_tt_motor = 0
local beta_tt_walk = -3.72
local beta_tt_taxi = -2.84
local beta_tt_SMS = -2.84
local beta_tt_SMS_Pool = -2.84


-------------------------------------------------
-- The variables having name format as  [ beta_cost ]  are coefficients for travel cost 
-- These are multiplied by the travel cost for the respective modes; 
-- In the case of tme, all the modes have a common cost coefficient 
local beta_cost = 0


local beta_cost_erp = 0    -- coefficient for ERP cost
local beta_cost_parking = 0    -- coefficient for Parking cost


-------------------------------------------------
-- The variables having name format as  [ beta_central_<modeName> ]  are coefficients for centralDummy
-- centralDummy is a dummy varible taking values 0 or 1 based on whether the O/D is in the CBD region of the city
local beta_central_bus = 0.330
local beta_central_mrt = 0.502 
local beta_central_privatebus = 0.851
local beta_central_share2 = 0.507
local beta_central_share3 = 0.602
local beta_central_motor = 0.136
local beta_central_taxi = 1.04
local beta_central_walk = 0.205
local beta_central_SMS = 1.04
local beta_central_Rail_SMS = 0.502
local beta_central_SMS_Pool = 1.04
local beta_central_Rail_SMS_Pool = 0.502



-------------------------------------------------
-- The variables having name format as  [ beta_female_<modeName> ]  are joint coefficients for femaleDummy and modeName variables
-- femaleDummy is a dummy varible taking values 0 or 1 based on whether the individual is a female or not
local beta_female_bus = 0.804
local beta_female_mrt = 0.916
local beta_female_Rail_SMS = 0.916
local beta_female_Rail_SMS_Pool = 0.916
local beta_female_privatebus = 0.875
local beta_female_drive1 = 0
local beta_female_share2 = 0.877
local beta_female_share3 = 0.762
local beta_female_motor = 0
local beta_female_taxi = 0.567
local beta_female_SMS = 0.567
local beta_female_SMS_Pool = 0.567
local beta_female_walk = 0.877
local beta_zero_drive1 = 0
local beta_oneplus_drive1 = 0
local beta_twoplus_drive1 = 0
local beta_threeplus_drive1 = 0
local beta_zero_share2 = 0
local beta_oneplus_share2 = 2.77
local beta_twoplus_share2 = 2.77
local beta_threeplus_share2 = 0.140



-------------------------------------------------
-- The variables having name format as  [ beta_<vehicleOwnerShipCategoryDummy>_<modeName> ]  are coefficients for vehicleOwnershipDummy variables
-- vehicleOwnershipDummy is a dummy varible taking values 0 or 1 based on whether the individual owns has a particular set of vehicles(like oneCar, onePlusCar etc.. )
local beta_zero_share3 = 0
local beta_oneplus_share3 = 2.79
local beta_twoplus_share3 = 0.998
local beta_threeplus_share3 = 0
local beta_zero_motor = 0
local beta_oneplus_motor = 0
local beta_twoplus_motor = 0
local beta_threeplus_motor = 0



local beta_transfer = 0                 -- Coefficient for average transfer number if the mode in question is used
local beta_distance = -0.0116           -- Coefficient for walk distance if the mode in question is used
local beta_residence = -0.488           -- Coefficient for residential size (defined as resident_size/origin_area/10000.0)
local beta_residence_2 = 0              -- Coefficient for residential size squared
local beta_attraction = -0.0429         -- Coefficient for work_attraction (defined as work_op/destination_area/10000.0)
local beta_attraction_2 = 0             -- Coefficient for work_attraction squared 



-------------------------------------------------
-- The variables having name format as  [ beta_<ageRange>_<vehicleOwnerShipCategoryDummy>_<modeName> ]  are joint coefficients for ageGroup and vehicleOwnershipDummy variables
local beta_age_over_15_bus = 1.95
local beta_age_over_15_mrt = 2.46
local beta_age_over_15_Rail_SMS = 2.46
local beta_age_over_15_Rail_SMS_Pool = 2.46
local beta_age_over_15_private_bus = 0
local beta_age_over_15_drive1 = 0
local beta_age_over_15_share2 = 0.376
local beta_age_over_15_share3 = 0
local beta_age_over_15_motor = 0
local beta_age_over_15_walk = 1.28
local beta_age_over_15_taxi = 0.772
local beta_age_over_15_SMS = 0.772
local beta_age_over_15_SMS_Pool = 0.772



-------------------------------------------------
-- The variables having name format as  [ beta_university_student_<modeName> ]  are joint coefficients for university_student and modeName variables
-- university_student is a dummy varible taking values 0 or 1 based on whether the individual is a university student
local beta_university_student_bus = -0.157
local beta_university_student_mrt = 0.308
local beta_university_student_Rail_SMS = 0.308
local beta_university_student_Rail_SMS_Pool = 0.308
local beta_university_student_private_bus = 0.227
local beta_university_student_drive1 = 0
local beta_university_student_share2 = 0.267
local beta_university_student_share3 = 0
local beta_university_student_motor = 0
local beta_university_student_walk = 0
local beta_university_student_taxi = 2.10
local beta_university_student_SMS = 2.10
local beta_university_student_SMS_Pool = 2.10

local beta_distance_motor = 0


local modes = {['BusTravel'] = 1 , ['MRT'] =2, ['PrivateBus'] =3 ,  ['Car'] = 4,  ['Car_Sharing_2'] = 5,['Car_Sharing_3'] = 6, ['Motorcycle'] = 7,['Walk'] = 8, ['Taxi'] = 9, ['SMS'] =10, ['Rail_SMS'] = 11, ['SMS_Pool'] = 12, ['Rail_SMS_Pool'] = 13 }




-------------------------------------------------
-- choice set contains the set of choices(mode serial number) which are available in this model 
-- The serial number of modes in the choice set corresponds the order of modes as listed in the config file data/simulation.xml
-- 1 for public bus; 2 for MRT/LRT; 3 for private bus; 4 for drive1;
-- 5 for shared2; 6 for shared3+; 7 for motor; 8 for walk; 9 for taxi, 
-- 10 for SMS; 12 for SMS_Pool
-- 11 for Rail_SMS; 13 for Rail_SMS_Pool
local choice = {}
choice["PT"] = {1,2,3,11,13}
choice["car"] = {4,5,6,7}
choice["other"] = {8,9,10,12}



-- utility[] is a lua table which will store the computed utilities for various modes
-- 1 for public bus; 2 for MRT/LRT; 3 for private bus; 4 for drive1;
-- 5 for shared2; 6 for shared3+; 7 for motor; 8 for walk; 9 for taxi
-- 10 for SMS; 11 for Rail SMS; 12 for SMS_Pool; 13 for Rail_SMS_Pool
local utility = {}


-- params contain the individual specific data like age, income etc
-- dbparams contain the network specific data like travel time/ travel cost between two OD pairs etc
local function computeUtilities(params,dbparams)
	local cost_increase = dbparams.cost_increase
	local d1 = dbparams.walk_distance1
	local d2 = dbparams.walk_distance2
	

	-- ivt: in-vehicle time;  
	-- first: first half tour; -- second: second half tour
	-- public: name of mode
	-- public_walk : time spent in walking if the public mode chosen
	local tt_public_ivt_first = dbparams.tt_public_ivt_first
	local tt_public_ivt_second = dbparams.tt_public_ivt_second
	local tt_public_waiting_first = dbparams.tt_public_waiting_first
	local tt_public_waiting_second = dbparams.tt_public_waiting_second
	local tt_public_walk_first =  dbparams.tt_public_walk_first
	local tt_public_walk_second = dbparams.tt_public_walk_second

	
	
	-------------------------------------------------
	-- Expressions for calculating travel costs of various modes
	-- first: first half tour; -- second: second half tour
	-- OP: Off-Peak
	local cost_public_first = dbparams.cost_public_first
	local cost_public_second = dbparams.cost_public_second
	local cost_bus=cost_public_first+cost_public_second+cost_increase
	local cost_mrt=cost_public_first+cost_public_second+cost_increase
	local cost_privatebus=cost_public_first+cost_public_second+cost_increase
	local cost_car_ERP_first = dbparams.cost_car_ERP_first
	local cost_car_ERP_second = dbparams.cost_car_ERP_second
	local cost_car_OP_first = dbparams.cost_car_OP_first
	local cost_car_OP_second = dbparams.cost_car_OP_second
	local cost_car_parking = dbparams.cost_car_parking
	local cost_cardriver=cost_car_ERP_first+cost_car_ERP_second+cost_car_OP_first+cost_car_OP_second+cost_car_parking+cost_increase
	local cost_carpassenger=cost_car_ERP_first+cost_car_ERP_second+cost_car_OP_first+cost_car_OP_second+cost_car_parking+cost_increase
	local cost_motor=0.5*(cost_car_ERP_first+cost_car_ERP_second+cost_car_OP_first+cost_car_OP_second)+0.65*cost_car_parking+cost_increase


	-- dummy variables can take values 0 or 1
	local central_dummy = dbparams.central_dummy
	local female_dummy = params.female_dummy
	local income_id = params.income_id
	local income_cat = {500,1250,1750,2250,2750,3500,4500,5500,6500,7500,8500,0,99999,99999}
	local income_mid = income_cat[income_id]
	local missing_income = (params.income_id >= 13) and 1 or 0
	

	-- age group related variables
	local age_id = params.age_id
	local age_over_15 = age_id >= 3 and 1 or 0
	
	-- student type related variables
	local student_type_id = params.student_type_id
	local university_student = student_type_id == 6 and 1 or 0


 -- Cost of travelling by taxi is computed using three components: an initial flag down cost (3.4), a fixed rate per km, upto 10 kms and another rate per km after 10 kms travelled  
	local cost_taxi_1=3.4+((d1*(d1>10 and 1 or 0)-10*(d1>10 and 1 or 0))/0.35+(d1*(d1<=10 and 1 or 0)+10*(d1>10 and 1 or 0))/0.4)*0.22+ cost_car_ERP_first + central_dummy*3
	local cost_taxi_2=3.4+((d2*(d2>10 and 1 or 0)-10*(d2>10 and 1 or 0))/0.35+(d2*(d2<=10 and 1 or 0)+10*(d2>10 and 1 or 0))/0.4)*0.22+ cost_car_ERP_second + central_dummy*3
	local cost_taxi=cost_taxi_1+cost_taxi_2+cost_increase
	
   	-- Cost of SMS defined as a percentage of cost of Taxi (72% in the example below)	
	local cost_SMS_1=3.4+((d1*(d1>10 and 1 or 0)-10*(d1>10 and 1 or 0))/0.35+(d1*(d1<=10 and 1 or 0)+10*(d1>10 and 1 or 0))/0.4)*0.22+ cost_car_ERP_first + central_dummy*3
	local cost_SMS_2=3.4+((d2*(d2>10 and 1 or 0)-10*(d2>10 and 1 or 0))/0.35+(d2*(d2<=10 and 1 or 0)+10*(d2>10 and 1 or 0))/0.4)*0.22+ cost_car_ERP_second + central_dummy*3
	local cost_SMS=(cost_SMS_1+cost_SMS_2)*0.72 + cost_increase
	
   	-- Cost of SMS_Pool defined as a percentage of cost of SMS (70 % in the example below)	
	local cost_SMS_Pool_1=3.4+((d1*(d1>10 and 1 or 0)-10*(d1>10 and 1 or 0))/0.35+(d1*(d1<=10 and 1 or 0)+10*(d1>10 and 1 or 0))/0.4)*0.22+ cost_car_ERP_first + central_dummy*3
	local cost_SMS_Pool_2=3.4+((d2*(d2>10 and 1 or 0)-10*(d2>10 and 1 or 0))/0.35+(d2*(d2<=10 and 1 or 0)+10*(d2>10 and 1 or 0))/0.4)*0.22+ cost_car_ERP_second + central_dummy*3
	local cost_SMS_Pool=(cost_SMS_1+cost_SMS_2)*0.72*0.7 + cost_increase
	
	local aed_1 = (5*tt_public_walk_first) -- Access egress distance (AED1)
	local aed_2 = (5*tt_public_walk_second) -- Access egress distance (AED2)
	
	-- Cost of Rail_SMS calculated similar to SMS but by using AED in place of walking distance 	
	local cost_Rail_SMS_AE_1 = 3.4+((aed_1*(aed_1>10 and 1 or 0)-10*(aed_1>10 and 1 or 0))/0.35+(aed_1*(aed_1<=10 and 1 or 0)+10*(aed_1>10 and 1 or 0))/0.4)*0.22+ cost_car_ERP_first + central_dummy*3
	local cost_Rail_SMS_AE_2 = 3.4+((aed_2*(aed_2>10 and 1 or 0)-10*(aed_2>10 and 1 or 0))/0.35+(aed_2*(aed_2<=10 and 1 or 0)+10*(aed_2>10 and 1 or 0))/0.4)*0.22+ cost_car_ERP_second + central_dummy*3
	local cost_Rail_SMS = cost_public_first + cost_public_second + cost_increase + (cost_Rail_SMS_AE_1 + cost_Rail_SMS_AE_2) * 0.72	
	
	
    -- Cost of Rail_SMS_Pool defined as a percentage of cost of Rail_SMS (70 % in the example below)
	local cost_Rail_SMS_AE_Pool_1 = 3.4+((aed_1*(aed_1>10 and 1 or 0)-10*(aed_1>10 and 1 or 0))/0.35+(aed_1*(aed_1<=10 and 1 or 0)+10*(aed_1>10 and 1 or 0))/0.4)*0.22+ cost_car_ERP_first + central_dummy*3
	local cost_Rail_SMS_AE_Pool_2 = 3.4+((aed_2*(aed_2>10 and 1 or 0)-10*(aed_2>10 and 1 or 0))/0.35+(aed_2*(aed_2<=10 and 1 or 0)+10*(aed_2>10 and 1 or 0))/0.4)*0.22+ cost_car_ERP_second + central_dummy*3
	local cost_Rail_SMS_Pool = cost_public_first + cost_public_second + cost_increase + (cost_Rail_SMS_AE_Pool_1 + cost_Rail_SMS_AE_Pool_2) * 0.72 * 0.7	
	
	

	-- Cost over income: Cost of the mode divided by income of the individual	
	local cost_over_income_bus=30*cost_bus/(0.5+income_mid)
	local cost_over_income_mrt=30*cost_mrt/(0.5+income_mid)
	local cost_over_income_privatebus=30*cost_privatebus/(0.5+income_mid)
	local cost_over_income_cardriver=30*cost_cardriver/(0.5+income_mid)
	local cost_over_income_carpassenger=30*cost_carpassenger/(0.5+income_mid)
	local cost_over_income_motor=30*cost_motor/(0.5+income_mid)
	local cost_over_income_taxi=30*cost_taxi/(0.5+income_mid)
	local cost_over_income_SMS=30*cost_SMS/(0.5+income_mid)
	local cost_over_income_SMS_Pool=30*cost_SMS_Pool/(0.5+income_mid) 	
	local cost_over_income_Rail_SMS=30*cost_Rail_SMS/(0.5+income_mid)
	local cost_over_income_Rail_SMS_Pool=30*cost_Rail_SMS_Pool/(0.5+income_mid)
		
	
	
    -- ivt: in-vehicle time;  
	-- first: first half tour; -- second: second half tour
	-- public: name of mode
	-- public_walk : time spent in walking if the public mode chosen	
	local tt_ivt_car_first = dbparams.tt_ivt_car_first
	local tt_ivt_car_second = dbparams.tt_ivt_car_second
	local tt_bus_ivt=tt_public_ivt_first+tt_public_ivt_second
	local tt_bus_wait=tt_public_waiting_first+tt_public_waiting_second
	local tt_bus_walk=tt_public_walk_first+tt_public_walk_second
	local tt_bus_all=tt_bus_ivt+tt_bus_wait+tt_bus_walk
	local tt_mrt_ivt=tt_public_ivt_first+tt_public_ivt_second
	local tt_mrt_wait=tt_public_waiting_first+tt_public_waiting_second
	local tt_mrt_walk=tt_public_walk_first+tt_public_walk_second
	local tt_mrt_all=tt_mrt_ivt+tt_mrt_wait+tt_mrt_walk
	local tt_Rail_SMS_ivt=tt_public_ivt_first+tt_public_ivt_second
	local tt_Rail_SMS_wait=1/6.0+1/6.0+tt_public_waiting_first+tt_public_waiting_second
	local tt_Rail_SMS_walk=(tt_public_walk_first+tt_public_walk_second)/8.0
	local tt_Rail_SMS_all=tt_mrt_ivt+tt_mrt_wait+tt_mrt_walk
	local tt_Rail_SMS_Pool_ivt=tt_public_ivt_first+tt_public_ivt_second+(aed_1+aed_2)/60
	local tt_Rail_SMS_Pool_wait=1/6.0+1/6.0+tt_public_waiting_first+tt_public_waiting_second+1/10
	local tt_Rail_SMS_Pool_walk=(tt_public_walk_first+tt_public_walk_second)/8.0
	local tt_Rail_SMS_Pool_all=tt_mrt_ivt+tt_mrt_wait+tt_mrt_walk
	local tt_privatebus_ivt=tt_ivt_car_first+tt_ivt_car_second
	local tt_privatebus_wait=tt_public_waiting_first+tt_public_waiting_second
	local tt_privatebus_walk=tt_public_walk_first+tt_public_walk_second
	local tt_privatebus_all=tt_privatebus_ivt+tt_privatebus_wait+tt_privatebus_walk
	local tt_cardriver_ivt=tt_ivt_car_first+tt_ivt_car_second
	local tt_cardriver_out=1.0/6
	local tt_cardriver_all=tt_cardriver_ivt+tt_cardriver_out
	local tt_carpassenger_ivt=tt_ivt_car_first+tt_ivt_car_second
	local tt_carpassenger_out=1.0/6
	local tt_carpassenger_all=tt_carpassenger_ivt+tt_carpassenger_out
	local tt_motor_ivt=tt_ivt_car_first+tt_ivt_car_second
	local tt_motor_out=1.0/6
	local tt_motor_all=tt_motor_ivt+tt_motor_out
	local tt_walk=(d1+d2)/5
	local tt_taxi_ivt=tt_ivt_car_first+tt_ivt_car_second
	local tt_taxi_out=1.0/6
	local tt_taxi_all=tt_cardriver_ivt+tt_cardriver_out
	local tt_SMS_ivt=tt_ivt_car_first+tt_ivt_car_second
	local tt_SMS_out=dbparams.wtt_sms_first + dbparams.wtt_sms_second;
	local tt_SMS_all=tt_cardriver_ivt+tt_cardriver_out
	local tt_SMS_Pool_ivt=tt_ivt_car_first+tt_ivt_car_second+(d1+d2)/2/60
	local tt_SMS_Pool_out=dbparams.wtt_sms_pool_first + dbparams.wtt_sms_pool_second;
	local tt_SMS_Pool_all=tt_cardriver_ivt+tt_cardriver_out
	

	local average_transfer_number = dbparams.average_transfer_number


	-- Vehicle ownership dummies
	local zero_car,one_plus_car,two_plus_car,three_plus_car, zero_motor,one_plus_motor,two_plus_motor,three_plus_motor = 0,0,0,0,0,0,0,0
	local veh_own_cat = params.vehicle_ownership_category
	if veh_own_cat == 0 or veh_own_cat == 1 or veh_own_cat ==2 then 
		zero_car = 1 
	end
	if veh_own_cat == 3 or veh_own_cat == 4 or veh_own_cat == 5  then 
		one_plus_car = 1 
	end
	if veh_own_cat == 5  then 
		two_plus_car = 1 
	end
	if veh_own_cat == 5  then 
		three_plus_car = 1 
	end
	if veh_own_cat == 0 or veh_own_cat == 3  then 
		zero_motor = 1 
	end
	if veh_own_cat == 1 or veh_own_cat == 2 or veh_own_cat == 4 or veh_own_cat == 5  then 
		one_plus_motor = 1 
	end
	if veh_own_cat == 1 or veh_own_cat == 2 or veh_own_cat == 4 or veh_own_cat == 5  then 
		two_plus_motor = 1 
	end
	if veh_own_cat == 1 or veh_own_cat == 2 or veh_own_cat == 4 or veh_own_cat == 5  then 
		three_plus_motor = 1 
	end

	
	
	local resident_size = dbparams.resident_size        -- Number of workers in the household
	local education_op = dbparams.education_op          -- Number of schools in the zone
	local origin_area = dbparams.origin_area            -- Area of Origin Taz
	local destination_area = dbparams.destination_area  -- Area of Destination Taz
	local residential_size=resident_size/origin_area/10000.0        -- derived variable
	local school_attraction=education_op/destination_area/10000.0   -- derived variable



	utility[1] = beta_cons_bus + beta1_1_tt * tt_bus_ivt + beta1_2_tt * tt_bus_walk + beta1_3_tt * tt_bus_wait + beta_cost * cost_bus + beta_central_bus * central_dummy + beta_transfer * average_transfer_number + beta_female_bus * female_dummy + age_over_15 * beta_age_over_15_bus + university_student * beta_university_student_bus
	utility[2] = beta_cons_mrt + beta1_1_tt * tt_mrt_ivt + beta1_2_tt * tt_mrt_walk + beta1_3_tt * tt_mrt_wait + beta_cost * cost_mrt + beta_central_mrt * central_dummy + beta_transfer * average_transfer_number + beta_female_mrt * female_dummy + age_over_15 * beta_age_over_15_mrt + university_student * beta_university_student_mrt
	utility[3] = beta_cons_privatebus + beta_private_1_tt * tt_privatebus_ivt + beta_cost * cost_privatebus + beta_central_privatebus * central_dummy + beta_distance*(d1+d2) + beta_residence * residential_size + beta_attraction * school_attraction + beta_residence_2*math.pow(residential_size,2)+beta_attraction_2*math.pow(school_attraction,2)+beta_female_privatebus* female_dummy + age_over_15 * beta_age_over_15_private_bus + university_student * beta_university_student_private_bus
	utility[4] = beta_cons_drive1 + beta2_tt_drive1 * tt_cardriver_all + beta_cost * cost_cardriver + beta_female_drive1 * female_dummy + beta_zero_drive1 * zero_car + beta_oneplus_drive1 * one_plus_car + beta_twoplus_drive1 * two_plus_car + beta_threeplus_drive1 * three_plus_car + age_over_15 * beta_age_over_15_drive1 + university_student * beta_university_student_drive1
	utility[5] = beta_cons_share2 + beta2_tt_share2 * tt_carpassenger_all + beta_cost * cost_carpassenger/2.0  + beta_central_share2 * central_dummy + beta_female_share2 * female_dummy + beta_zero_share2 * zero_car + beta_oneplus_share2 * one_plus_car + beta_twoplus_share2 * two_plus_car + beta_threeplus_share2 * three_plus_car + age_over_15*beta_age_over_15_share2 + university_student * beta_university_student_share2
	utility[6] = beta_cons_share3 + beta2_tt_share3 * tt_carpassenger_all + beta_cost * cost_carpassenger/3.0  + beta_central_share3 * central_dummy + beta_female_share3 * female_dummy + beta_zero_share3 * zero_car + beta_oneplus_share3 * one_plus_car + beta_twoplus_share3 * two_plus_car + beta_threeplus_share3 * three_plus_car + age_over_15*beta_age_over_15_share3 + university_student * beta_university_student_share3
	utility[7] = beta_cons_motor + beta2_tt_motor * tt_motor_all + beta_cost * cost_motor + beta_central_motor * central_dummy + beta_zero_motor * zero_motor + beta_oneplus_motor * one_plus_motor + beta_twoplus_motor * two_plus_motor + beta_threeplus_motor * three_plus_motor + beta_female_motor * female_dummy + age_over_15*beta_age_over_15_motor + university_student * beta_university_student_motor + beta_distance_motor * (d1+d2)
	utility[8] = beta_cons_walk  + beta_tt_walk * tt_walk + beta_central_walk * central_dummy+ beta_female_walk * female_dummy + age_over_15*beta_age_over_15_walk + university_student * beta_university_student_walk
	utility[9] = beta_cons_taxi + beta_tt_taxi * tt_taxi_all + beta_cost * cost_taxi + beta_central_taxi * central_dummy + beta_female_taxi * female_dummy + age_over_15*beta_age_over_15_taxi + university_student * beta_university_student_taxi
	utility[10] = beta_cons_SMS + beta_tt_SMS * tt_SMS_all + beta_cost * cost_SMS + beta_central_SMS * central_dummy + beta_female_SMS * female_dummy + age_over_15*beta_age_over_15_SMS + university_student * beta_university_student_SMS
	utility[11] = beta_cons_Rail_SMS + beta1_1_tt * tt_Rail_SMS_ivt + beta1_2_tt * tt_Rail_SMS_walk + beta1_3_tt * tt_Rail_SMS_wait + beta_cost * cost_Rail_SMS + beta_central_Rail_SMS * central_dummy + beta_transfer * average_transfer_number + beta_female_Rail_SMS * female_dummy + age_over_15 * beta_age_over_15_Rail_SMS + university_student * beta_university_student_Rail_SMS
	utility[12] = beta_cons_SMS_Pool + beta_tt_SMS_Pool * tt_SMS_Pool_all + beta_cost * cost_SMS_Pool + beta_central_SMS_Pool * central_dummy + beta_female_SMS_Pool * female_dummy + age_over_15*beta_age_over_15_SMS_Pool + university_student * beta_university_student_SMS_Pool
	utility[13] = beta_cons_Rail_SMS_Pool + beta1_1_tt * tt_Rail_SMS_Pool_ivt + beta1_2_tt * tt_Rail_SMS_Pool_walk + beta1_3_tt * tt_Rail_SMS_Pool_wait + beta_cost * cost_Rail_SMS_Pool + beta_central_Rail_SMS_Pool * central_dummy + beta_transfer * average_transfer_number + beta_female_Rail_SMS_Pool * female_dummy + age_over_15 * beta_age_over_15_Rail_SMS_Pool + university_student * beta_university_student_Rail_SMS_Pool
end

--availability
--the logic to determine availability is the same with current implementation
local availability = {}
local function computeAvailabilities(params,dbparams)
 availability = {
	dbparams:getModeAvailability(modes.BusTravel),
		dbparams:getModeAvailability(modes.MRT),
		dbparams:getModeAvailability(modes.PrivateBus),
		dbparams:getModeAvailability(modes.Car),
		dbparams:getModeAvailability(modes.Car_Sharing_2),
		dbparams:getModeAvailability(modes.Car_Sharing_3),
		dbparams:getModeAvailability(modes.Motorcycle),
		dbparams:getModeAvailability(modes.Walk),
		dbparams:getModeAvailability(modes.Taxi),
		dbparams:getModeAvailability(modes.SMS),
		dbparams:getModeAvailability(modes.Rail_SMS),
		dbparams:getModeAvailability(modes.SMS_Pool),
		dbparams:getModeAvailability(modes.Rail_SMS_Pool)


}
end

--scale
--scale can be used to control the variance of selection of choices
local scale={}
scale["PT"] = 1.60
scale["car"] = 1.51
scale["other"] = 1

-- function to call from C++ preday simulator
-- params and dbparams tables contain data passed from C++
-- to check variable bindings in params or dbparams, refer PredayLuaModel::mapClasses() function in dev/Basic/medium/behavioral/lua/PredayLuaModel.cpp
function choose_tme(params,dbparams)
	computeUtilities(params,dbparams) 
	computeAvailabilities(params,dbparams)
	local probability = calculate_probability("nl", choice, utility, availability, scale)
	return make_final_choice(probability)
end


-- function to call from C++ preday simulator for logsums computation
-- params and dbparams tables contain data passed from C++
-- to check variable bindings in params or dbparams, refer PredayLuaModel::mapClasses() function in dev/Basic/medium/behavioral/lua/PredayLuaModel.cpp
function compute_logsum_tme(params,dbparams)
	computeUtilities(params,dbparams) 
	computeAvailabilities(params,dbparams)
	return compute_mnl_logsum(utility, availability)



end


