--[[
Model - Mode choice for education tour
Type - NL
Authors - Siyu Li, Harish Loganathan
]]

-- all require statements do not work with C++. They need to be commented. The order in which lua files are loaded must be explicitly controlled in C++. 
--require "NLogit"

--Estimated values for all betas
--Note: the betas that not estimated are fixed to zero.

--!! see the documentation on the definition of AM,PM and OP table!!
local beta_cons_bus = -1.866
local beta_cons_mrt = -2.799
local beta_cons_privatebus= -2.074
local beta_cons_drive1= 0
local beta_cons_share2= -5.180
local beta_cons_share3= -4.861
local beta_cons_motor= -7.309
local beta_cons_walk= 2.583
local beta_cons_taxi= -4.865

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


local beta_cost = 0


local beta_central_bus = 0.330
local beta_central_mrt = 0.502 
local beta_central_privatebus = 0.851
local beta_central_share2 = 0.507
local beta_central_share3 = 0.602
local beta_central_motor = 0.136
local beta_central_taxi = 1.04
local beta_central_walk = 0.205


local beta_female_bus = 0.804
local beta_female_mrt = 0.916
local beta_female_privatebus = 0.875

local beta_female_drive1 = 0
local beta_female_share2 = 0.877
local beta_female_share3 = 0.762

local beta_female_motor = 0
local beta_female_taxi = 0.567
local beta_female_walk = 0.877

local beta_zero_drive1 = 0
local beta_oneplus_drive1 = 0
local beta_twoplus_drive1 = 0
local beta_threeplus_drive1 = 0

local beta_zero_share2 = 0
local beta_oneplus_share2 = 2.77
local beta_twoplus_share2 = 1.31
local beta_threeplus_share2 = 0.140

local beta_zero_share3 = 0
local beta_oneplus_share3 = 2.79
local beta_twoplus_share3 = 0.998
local beta_threeplus_share3 = 0


local beta_zero_motor = 0
local beta_oneplus_motor = 0
local beta_twoplus_motor = 0
local beta_threeplus_motor = 0


local beta_transfer = 0

local beta_distance = -0.0116
local beta_residence = -0.488
local beta_residence_2 = 0
local beta_attraction = -0.0429
local beta_attraction_2 = 0

local beta_age_over_15_bus = 1.95
local beta_age_over_15_mrt = 2.46
local beta_age_over_15_private_bus = 0
local beta_age_over_15_drive1 = 0
local beta_age_over_15_share2 = 0.376
local beta_age_over_15_share3 = 0
local beta_age_over_15_motor = 0
local beta_age_over_15_walk = 1.28
local beta_age_over_15_taxi = 0.772

local beta_university_student_bus = -0.157
local beta_university_student_mrt = 0.308
local beta_university_student_private_bus = 0.227
local beta_university_student_drive1 = 0
local beta_university_student_share2 = 0.267
local beta_university_student_share3 = 0
local beta_university_student_motor = 0
local beta_university_student_walk = 0
local beta_university_student_taxi = 2.10

local beta_distance_motor = 0


--choice set
-- 1 for public bus; 2 for MRT/LRT; 3 for private bus; 4 for drive1;
-- 5 for shared2; 6 for shared3+; 7 for motor; 8 for walk; 9 for taxi
local choice = {}
choice["PT"] = {1,2,3}
choice["car"] = {4,5,6,7}
choice["other"] = {8,9}

--utility
-- 1 for public bus; 2 for MRT/LRT; 3 for private bus; 4 for drive1;
-- 5 for shared2; 6 for shared3+; 7 for motor; 8 for walk; 9 for taxi
local utility = {}
local function computeUtilities(pparams,mparams)
	local cost_increase = 0

	--dbparams.cost_public_first = AM[(origin,destination)]['pub_cost']
	--origin is home, destination is tour destination
	--0 if origin == destination
	

	--dbcost_public_second = PM[(destination,origin)]['pub_cost']
	--origin is home, destination is tour destination
	--0 if origin == destination
	local distance_remained = mparams.distance_remaining

	local cost_public = 0.77 + distance_remained*0 -- min fare + distance based fare
	local cost_bus=cost_public
	local cost_mrt=cost_public
	local cost_privatebus=cost_public

	--dbparams.cost_car_ERP_first = AM[(origin,destination)]['car_cost_erp']
	--dbparams.cost_car_ERP_second = PM[(destination,origin)]['car_cost_erp']
	--dbparams.cost_car_OP_first = AM[(origin,destination)]['distance']*0.147
	--dbparams.cost_car_OP_second = PM[(destination,origin)]['distance']*0.147
	--dbparams.cost_car_parking = 8 * ZONE[destination]['parking_rate']
	--for the above 5 variables, origin is home, destination is tour destination
	--0 if origin == destination
	
	
	local cost_car_voc = distance_remained * 0.147
	
	local cost_car_parking = 8 * mparams.parking_rate

	local cost_cardriver = cost_car_voc + cost_car_parking
	local cost_carpassenger= cost_car_voc + cost_car_parking
	local cost_motor=0.5*(cost_car_voc)+0.65*cost_car_parking

	--dbparams.walk_distance1= AM[(origin,destination)]['AM2dis']
	--origin is home mtz, destination is usual work location mtz
	--0 if origin == destination
	--dbparams.walk_distance2= PM[(destination,origin)]['PM2dis']
	--origin is home mtz, destination is usual work location mtz
	--0 if origin == destination
	local d1 = distance_remained
	

	--dbparams.central_dummy=ZONE[destination]['central_dummy']
	--destination is tour destination
	local central_dummy = mparams.central_dummy
	
	local female_dummy = pparams.female_dummy
	local income_id = pparams.income_id
	local income_cat = {500,1250,1750,2250,2750,3500,4500,5500,6500,7500,8500,0,99999,99999}
	local income_mid = income_cat[income_id]
	local missing_income = (pparams.income_id >= 13) and 1 or 0
	
	local age_id = pparams.age_id
	local age_over_15 = age_id >= 3 and 1 or 0
	local student_type_id = pparams.student_type_id
	local university_student = student_type_id == 6 and 1 or 0


	local cost_taxi=3.4+((d1*(d1>10 and 1 or 0)-10*(d1>10 and 1 or 0))/0.35+(d1*(d1<=10 and 1 or 0)+10*(d1>10 and 1 or 0))/0.4)*0.22+ central_dummy*3
	
	

	local cost_over_income_bus=30*cost_bus/(0.5+income_mid)
	local cost_over_income_mrt=30*cost_mrt/(0.5+income_mid)
	local cost_over_income_privatebus=30*cost_privatebus/(0.5+income_mid)
	local cost_over_income_cardriver=30*cost_cardriver/(0.5+income_mid)
	local cost_over_income_carpassenger=30*cost_carpassenger/(0.5+income_mid)
	local cost_over_income_motor=30*cost_motor/(0.5+income_mid)
	local cost_over_income_taxi=30*cost_taxi/(0.5+income_mid)

	--dbparams.tt_public_ivt_first = AM[(origin,destination)]['pub_ivt']
	--dbparams.tt_public_ivt_second = PM[(destination,origin)]['pub_ivt']
	--dbparams.tt_public_waiting_first = AM[(origin,destination)]['pub_wtt']
	--dbparams.tt_public_waiting_second = PM[(destination,origin)]['pub_wtt']
	--dbparams.tt_public_walk_first = AM[(origin,destination)]['pub_walkt']
	--dbparams.tt_public_walk_second = PM[(destination,origin)]['pub_walkt']
	--for the above 6 variables, origin is home, destination is tour destination
	--0 if origin == destination
	local tt_public_ivt = mparams.tt_public_ivt / 3600.0
	local tt_public_waiting = mparams.tt_public_waiting / 3600.0
	local tt_public_walk =  mparams.tt_public_walk / 3600.0
	local tt_ivt_car = mparams.tt_ivt_car / 3600.0
	
	local tt_bus_ivt=tt_public_ivt
	local tt_bus_wait=tt_public_waiting
	local tt_bus_walk=tt_public_walk
	local tt_bus_all=tt_bus_ivt+tt_bus_wait+tt_bus_walk

	local tt_mrt_ivt=tt_public_ivt
	local tt_mrt_wait=tt_public_waiting
	local tt_mrt_walk=tt_public_walk
	local tt_mrt_all=tt_mrt_ivt+tt_mrt_wait+tt_mrt_walk

	local tt_privatebus_ivt=tt_ivt_car
	local tt_privatebus_wait=tt_public_waiting
	local tt_privatebus_walk=tt_public_walk
	local tt_privatebus_all=tt_privatebus_ivt+tt_privatebus_wait+tt_privatebus_walk

	local tt_cardriver_ivt=tt_ivt_car
	local tt_cardriver_out=1.0/6
	local tt_cardriver_all=tt_cardriver_ivt+tt_cardriver_out

	local tt_carpassenger_ivt=tt_ivt_car
	local tt_carpassenger_out=1.0/6
	local tt_carpassenger_all=tt_carpassenger_ivt+tt_carpassenger_out

	local tt_motor_ivt=tt_ivt_car
	local tt_motor_out=1.0/6 
	local tt_motor_all=tt_motor_ivt+tt_motor_out

	local tt_walk=(d1)/5

	local tt_taxi_ivt=tt_ivt_car
	local tt_taxi_out=1.0/6 + 0.25
	local tt_taxi_all=tt_cardriver_ivt+tt_cardriver_out

	--dbparams.average_transfer_number = (AM[(origin,destination)]['avg_transfer'] + PM[(destination,origin)]['avg_transfer'])/2
	--origin is home, destination is tour destination
	-- 0 if origin == destination
	local average_transfer_number = mparams.average_transfer_number

	--params.car_own_normal is from household table
	local zero_car = pparams.car_own_normal == 0 and 1 or 0
	local one_plus_car = pparams.car_own_normal >= 1 and 1 or 0
	local two_plus_car = pparams.car_own_normal >= 2 and 1 or 0
	local three_plus_car = pparams.car_own_normal >= 3 and 1 or 0

	--params.motor_own is from household table
	local zero_motor = pparams.motor_own == 0 and 1 or 0
	local one_plus_motor = pparams.motor_own >=1 and 1 or 0
	local two_plus_motor = pparams.motor_own >=2 and 1 or 0
	local three_plus_motor = pparams.motor_own >= 3 and 1 or 0

	--dbparams.resident_size = ZONE[origin]['resident workers']
	--dbparams.education_op = ZONE[destination]['education_op'] --total student 
	--dbparams.origin_area= ZONE[origin]['area'] -- in square km 
	--dbparams.destination_area = ZONE[destination]['area'] -- in square km
	--origin is home, destination is tour destination
	local resident_size = mparams.resident_size
	local education_op = mparams.education_op
	local origin_area = mparams.origin_area
	local destination_area = mparams.destination_area

	local residential_size=resident_size/origin_area/10000.0
	local school_attraction=education_op/destination_area/10000.0

	utility[1] = beta_cons_bus + beta1_1_tt * tt_bus_ivt + beta1_2_tt * tt_bus_walk + beta1_3_tt * tt_bus_wait + beta_cost * cost_bus + beta_central_bus * 		central_dummy + beta_transfer * average_transfer_number + beta_female_bus * female_dummy + age_over_15 * beta_age_over_15_bus + university_student * 		beta_university_student_bus
	
	utility[2] = beta_cons_mrt + beta1_1_tt * tt_mrt_ivt + beta1_2_tt * tt_mrt_walk + beta1_3_tt * tt_mrt_wait + beta_cost * cost_mrt + beta_central_mrt * 		central_dummy + beta_transfer * average_transfer_number + beta_female_mrt * female_dummy + age_over_15 * beta_age_over_15_mrt + university_student * 		beta_university_student_mrt
	
	utility[3] = beta_cons_privatebus + beta_private_1_tt * tt_privatebus_ivt + beta_cost * cost_privatebus + beta_central_privatebus * central_dummy + 		beta_distance*(d1) + beta_residence * residential_size + beta_attraction * school_attraction + beta_residence_2*math.pow(residential_size,2)		+beta_attraction_2*math.pow(school_attraction,2)+beta_female_privatebus* female_dummy + age_over_15 * beta_age_over_15_private_bus + university_student * beta_university_student_private_bus

	utility[4] = beta_cons_drive1 + beta2_tt_drive1 * tt_cardriver_all + beta_cost * cost_cardriver + beta_female_drive1 * female_dummy + beta_zero_drive1 * zero_car + beta_oneplus_drive1 * one_plus_car + beta_twoplus_drive1 * two_plus_car + beta_threeplus_drive1 * three_plus_car + age_over_15 * beta_age_over_15_drive1 + university_student * beta_university_student_drive1

	utility[5] = beta_cons_share2 + beta2_tt_share2 * tt_carpassenger_all + beta_cost * cost_carpassenger/2.0  + beta_central_share2 * central_dummy + beta_female_share2 * female_dummy + beta_zero_share2 * zero_car + beta_oneplus_share2 * one_plus_car + beta_twoplus_share2 * two_plus_car + beta_threeplus_share2 * three_plus_car + age_over_15*beta_age_over_15_share2 + university_student * beta_university_student_share2

	utility[6] = beta_cons_share3 + beta2_tt_share3 * tt_carpassenger_all + beta_cost * cost_carpassenger/3.0  + beta_central_share3 * central_dummy + beta_female_share3 * female_dummy + beta_zero_share3 * zero_car + beta_oneplus_share3 * one_plus_car + beta_twoplus_share3 * two_plus_car + beta_threeplus_share3 * three_plus_car + age_over_15*beta_age_over_15_share3 + university_student * beta_university_student_share3

	utility[7] = beta_cons_motor + beta2_tt_motor * tt_motor_all + beta_cost * cost_motor + beta_central_motor * central_dummy + beta_zero_motor * zero_motor + beta_oneplus_motor * one_plus_motor + beta_twoplus_motor * two_plus_motor + beta_threeplus_motor * three_plus_motor + beta_female_motor * female_dummy + age_over_15*beta_age_over_15_motor + university_student * beta_university_student_motor + beta_distance_motor * (d1)

	utility[8] = beta_cons_walk  + beta_tt_walk * tt_walk + beta_central_walk * central_dummy+ beta_female_walk * female_dummy + age_over_15*beta_age_over_15_walk + university_student * beta_university_student_walk

	utility[9] = beta_cons_taxi + beta_tt_taxi * tt_taxi_all + beta_cost * cost_taxi + beta_central_taxi * central_dummy + beta_female_taxi * female_dummy + age_over_15*beta_age_over_15_taxi + university_student * beta_university_student_taxi

end

--availability
--the logic to determine availability is the same with current implementation
local availability = {}
local function computeAvailabilities(pparams,mparams)
	availability = {
		mparams.publicbus_AV, 
		mparams.mrt_AV, 
		mparams.privatebus_AV, 
		mparams.drive1_AV, 
		mparams.share2_AV, 
		mparams.share3_AV, 
		mparams.motor_AV, 
		mparams.walk_AV, 
		mparams.taxi_AV
	}
end

--scale
local scale={}
scale["PT"] = 1.60
scale["car"] = 1.51
scale["other"] = 1

-- function to call from C++ preday simulator
-- params and dbparams tables contain data passed from C++
-- to check variable bindings in params or dbparams, refer PredayLuaModel::mapClasses() function in dev/Basic/medium/behavioral/lua/PredayLuaModel.cpp
function choose_wdme(pparams,mparams)
	computeUtilities(pparams,mparams) 
	computeAvailabilities(pparams,mparams)
	local probability = calculate_probability("nl", choice, utility, availability, scale)
	return make_final_choice(probability)
end
