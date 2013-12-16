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

local beta_cons_bus = -1.94
local beta_cons_mrt = -2.93
local beta_cons_privatebus= -2.21
local beta_cons_drive1= 0
local beta_cons_share2= -5.66
local beta_cons_share3= -5.44
local beta_cons_motor= -6.5
local beta_cons_walk= -0.386
local beta_cons_taxi= -6.12

local beta1_1_tt = -0.623
local beta1_2_tt = -0.703
local beta1_3_tt = -1.09

local beta_private_1_tt= -0.692

local beta2_tt_drive1 = -0.9
local beta2_tt_share2 = -1.31
local beta2_tt_share3 = -1.13
local beta2_tt_motor = 0

local beta_tt_walk = -3.52
local beta_tt_taxi = -2.54


local beta_cost = -0.0184

local beta_cost_erp = 0
local beta_cost_parking = 0

local beta_central_bus = 0.214
local beta_central_mrt = 0.397 
local beta_central_privatebus = 0.753
local beta_central_share2 = 0.395
local beta_central_share3 = 0.486
local beta_central_motor = 0.00692
local beta_central_taxi = 0.959
local beta_central_walk = 0.0671


local beta_female_bus = 0.826
local beta_female_mrt = 0.948
local beta_female_privatebus = 0.886

local beta_female_drive1 = 0
local beta_female_share2 = 0.864
local beta_female_share3 = 0.753

local beta_female_motor = 0
local beta_female_taxi = 0.724
local beta_female_walk = 0.953

local beta_zero_drive1 = 0
local beta_oneplus_drive1 = 0
local beta_twoplus_drive1 = 0
local beta_threeplus_drive1 = 0

local beta_zero_share2 = 0
local beta_oneplus_share2 = 2.73
local beta_twoplus_share2 = 1.28
local beta_threeplus_share2 = 0.121

local beta_zero_share3 = 0
local beta_oneplus_share3 = 2.61
local beta_twoplus_share3 = 0.963
local beta_threeplus_share3 = 0


local beta_zero_motor = 0
local beta_oneplus_motor = 0
local beta_twoplus_motor = 0
local beta_threeplus_motor = 0


local beta_transfer = 0

local beta_distance = -0.00836
local beta_residence = -0.466
local beta_residence_2 = 0
local beta_attraction = -0.0393
local beta_attraction_2 = 0

local beta_age_over_15_bus = 2.0
local beta_age_over_15_mrt = 2.54
local beta_age_over_15_private_bus = 0
local beta_age_over_15_drive1 = 0
local beta_age_over_15_share2 = 0.414
local beta_age_over_15_share3 = 0
local beta_age_over_15_motor = 0
local beta_age_over_15_walk = 1.36
local beta_age_over_15_taxi = 1.14

local beta_university_student_bus = -0.184
local beta_university_student_mrt = 0.341
local beta_university_student_private_bus = 0.22
local beta_university_student_drive1 = 0
local beta_university_student_share2 = 0.191
local beta_university_student_share3 = 0
local beta_university_student_motor = 0
local beta_university_student_walk = 0
local beta_university_student_taxi = 1.88

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
local function computeUtilities(params,dbparams)
	
	--dbparams.cost_public_first = AM[(origin,destination)]['pub_cost']
	--origin is home, destination is tour destination
	--0 if origin == destination
	local cost_public_first = dbparams.cost_public_first

	--dbcost_public_second = PM[(destination,origin)]['pub_cost']
	--origin is home, destination is tour destination
	--0 if origin == destination
	local cost_public_second = dbparams.cost_public_second

	local cost_bus=cost_public_first+cost_public_second
	local cost_mrt=cost_public_first+cost_public_second
	local cost_privatebus=cost_public_first+cost_public_second

	--dbparams.cost_car_ERP_first = AM[(origin,destination)]['car_cost_erp']
	--dbparams.cost_car_ERP_second = PM[(destination,origin)]['car_cost_erp']
	--dbparams.cost_car_OP_first = AM[(origin,destination)]['distance']*0.147
	--dbparams.cost_car_OP_second = PM[(destination,origin)]['distance']*0.147
	--dbparams.cost_car_parking = 8 * ZONE[destination]['parking_rate']
	--for the above 5 variables, origin is home, destination is tour destination
	--0 if origin == destination
	local cost_car_ERP_first = dbparams.cost_car_ERP_first
	local cost_car_ERP_second = dbparams.cost_car_ERP_second
	local cost_car_OP_first = dbparams.cost_car_OP_first
	local cost_car_OP_second = dbparams.cost_car_OP_second
	local cost_car_parking = dbparams.cost_car_parking

	local cost_cardriver=cost_car_ERP_first+cost_car_ERP_second+cost_car_OP_first+cost_car_OP_second+cost_car_parking
	local cost_carpassenger=cost_car_ERP_first+cost_car_ERP_second+cost_car_OP_first+cost_car_OP_second+cost_car_parking
	local cost_motor=0.5*(cost_car_ERP_first+cost_car_ERP_second+cost_car_OP_first+cost_car_OP_second)+0.65*cost_car_parking

	--dbparams.walk_distance1= AM[(origin,destination)]['AM2dis']
	--origin is home mtz, destination is usual work location mtz
	--0 if origin == destination
	--dbparams.walk_distance2= PM[(destination,origin)]['PM2dis']
	--origin is home mtz, destination is usual work location mtz
	--0 if origin == destination
	local d1 = dbparams.walk_distance1
	local d2 = dbparams.walk_distance2

	--dbparams.central_dummy=ZONE[destination]['central_dummy']
	--destination is tour destination
	local central_dummy = dbparams.central_dummy
	
	local female_dummy = params.female_dummy
	local income_id = params.income_id
	local income_cat = {500,1250,1750,2250,2750,3500,4500,5500,6500,7500,8500,0,99999,99999}
	local income_mid = income_cat[income_id]
	local missing_income = (params.income_id >= 12) and 1 or 0
	
	local age_id = params.age_id
	local age_over_15 = age_id >= 3 and 1 or 0
	local student_type_id = params.student_type_id
	local university_student = student_type_id == 6 and 1 or 0


	local cost_taxi_1=3.4+((d1*(d1>10 and 1 or 0)-10*(d1>10 and 1 or 0))/0.35+(d1*(d1<=10 and 1 or 0)+10*(d1>10 and 1 or 0))/0.4)*0.22+ cost_car_ERP_first + central_dummy*3
	local cost_taxi_2=3.4+((d2*(d2>10 and 1 or 0)-10*(d2>10 and 1 or 0))/0.35+(d2*(d2<=10 and 1 or 0)+10*(d2>10 and 1 or 0))/0.4)*0.22+ cost_car_ERP_second + central_dummy*3
	local cost_taxi=cost_taxi_1+cost_taxi_2

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
	local tt_public_ivt_first = dbparams.tt_public_ivt_first
	local tt_public_ivt_second = dbparams.tt_public_ivt_second
	local tt_public_waiting_first = dbparams.tt_public_waiting_first
	local tt_public_waiting_second = dbparams.tt_public_waiting_second
	local tt_public_walk_first =  dbparams.tt_public_walk_first
	local tt_public_walk_second = dbparams.tt_public_walk_second

	--dbparams.tt_ivt_car_first = AM[(origin,destination)]['car_ivt']
	--dbparams.tt_ivt_car_second = PM[(destination,origin)]['car_ivt']
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

	--dbparams.average_transfer_number = (AM[(origin,destination)]['avg_transfer'] + PM[(destination,origin)]['avg_transfer'])/2
	--origin is home, destination is tour destination
	-- 0 if origin == destination
	local average_transfer_number = dbparams.average_transfer_number

	--params.car_own_normal is from household table
	local zero_car = params.car_own_normal == 0 and 1 or 0
	local one_plus_car = params.car_own_normal >= 1 and 1 or 0
	local two_plus_car = params.car_own_normal >= 2 and 1 or 0
	local three_plus_car = params.car_own_normal >= 3 and 1 or 0

	--params.motor_own is from household table
	local zero_motor = params.motor_own == 0 and 1 or 0
	local one_plus_motor = params.motor_own >=1 and 1 or 0
	local two_plus_motor = params.motor_own >=2 and 1 or 0
	local three_plus_motor = params.motor_own >= 3 and 1 or 0

	--dbparams.resident_size = ZONE[origin]['resident workers']
	--dbparams.education_op = ZONE[destination]['education_op'] --total student 
	--dbparams.origin_area= ZONE[origin]['area'] -- in square km 
	--dbparams.destination_area = ZONE[destination]['area'] -- in square km
	--origin is home, destination is tour destination
	local resident_size = dbparams.resident_size
	local education_op = dbparams.education_op
	local origin_area = dbparams.origin_area
	local destination_area = dbparams.destination_area

	local residential_size=resident_size/origin_area/10000.0
	local school_attraction=education_op/destination_area/10000.0

	utility[1] = beta_cons_bus + beta1_1_tt * tt_bus_ivt + beta1_2_tt * tt_bus_walk + beta1_3_tt * tt_bus_wait + beta_cost * cost_bus + beta_central_bus * central_dummy + beta_transfer * average_transfer_number + beta_female_bus * female_dummy + age_over_15 * beta_age_over_15_bus + university_student * beta_university_student_bus
	utility[2] = beta_cons_mrt + beta1_1_tt * tt_mrt_ivt + beta1_2_tt * tt_mrt_walk + beta1_3_tt * tt_mrt_wait + beta_cost * cost_mrt + beta_central_mrt * central_dummy + beta_transfer * average_transfer_number + beta_female_mrt * female_dummy + age_over_15 * beta_age_over_15_mrt + university_student * beta_university_student_mrt
	utility[3] = beta_cons_privatebus + beta_private_1_tt * tt_privatebus_ivt + beta_cost * cost_privatebus + beta_central_privatebus * central_dummy + beta_distance*(d1+d2) + beta_residence * residential_size + beta_attraction * school_attraction + beta_residence_2*math.pow(residential_size,2)+beta_attraction_2*math.pow(school_attraction,2)+beta_female_privatebus* female_dummy + age_over_15 * beta_age_over_15_private_bus + university_student * beta_university_student_private_bus
	utility[4] = beta_cons_drive1 + beta2_tt_drive1 * tt_cardriver_all + beta_cost * cost_cardriver + beta_female_drive1 * female_dummy + beta_zero_drive1 * zero_car + beta_oneplus_drive1 * one_plus_car + beta_twoplus_drive1 * two_plus_car + beta_threeplus_drive1 * three_plus_car + age_over_15 * beta_age_over_15_drive1 + university_student * beta_university_student_drive1
	utility[5] = beta_cons_share2 + beta2_tt_share2 * tt_carpassenger_all + beta_cost * cost_carpassenger/2.0  + beta_central_share2 * central_dummy + beta_female_share2 * female_dummy + beta_zero_share2 * zero_car + beta_oneplus_share2 * one_plus_car + beta_twoplus_share2 * two_plus_car + beta_threeplus_share2 * three_plus_car + age_over_15*beta_age_over_15_share2 + university_student * beta_university_student_share2
	utility[6] = beta_cons_share3 + beta2_tt_share3 * tt_carpassenger_all + beta_cost * cost_carpassenger/3.0  + beta_central_share3 * central_dummy + beta_female_share3 * female_dummy + beta_zero_share3 * zero_car + beta_oneplus_share3 * one_plus_car + beta_twoplus_share3 * two_plus_car + beta_threeplus_share3 * three_plus_car + age_over_15*beta_age_over_15_share3 + university_student * beta_university_student_share3
	utility[7] = beta_cons_motor + beta2_tt_motor * tt_motor_all + beta_cost * cost_motor + beta_central_motor * central_dummy + beta_zero_motor * zero_motor + beta_oneplus_motor * one_plus_motor + beta_twoplus_motor * two_plus_motor + beta_threeplus_motor * three_plus_motor + beta_female_motor * female_dummy + age_over_15*beta_age_over_15_motor + university_student * beta_university_student_motor + beta_distance_motor * (d1+d2)
	utility[8] = beta_cons_walk  + beta_tt_walk * tt_walk + beta_central_walk * central_dummy+ beta_female_walk * female_dummy + age_over_15*beta_age_over_15_walk + university_student * beta_university_student_walk
	utility[9] = beta_cons_taxi + beta_tt_taxi * tt_taxi_all + beta_cost * cost_taxi + beta_central_taxi * central_dummy + beta_female_taxi * female_dummy + age_over_15*beta_age_over_15_taxi + university_student * beta_university_student_taxi

end



--availability
--the logic to determine availability is the same with current implementation
local availability = {}
local function computeAvailabilities(params,dbparams)
	availability[1] = {
		dbparams.tme_publicbus_AV,
		dbparams.tme_mrt_AV,
		dbparams.tme_privatebus_AV
	}
	availability[2] = {
		dbparams.tme_drive1_AV,
		dbparams.tme_share2_AV,
		dbparams.tme_share3_AV,
		dbparams.tme_motor_AV
	}
	availability[3] = {
		dbparams.tme_walk_AV,
		dbparams.tme_taxi_AV
	}
end

--scale
local scale={
	{1.51,1.51,1.51},
	{1.45,1.45,1.45,1.45},
	{1,1}
}


function choose_tme(params,dbparams)
	computeUtilities(params,dbparams) 
	computeAvailabilities(params,dbparams)
	local probability = calculate_probability("nl", choice, utility, availability, scale)
	return make_final_choice(probability)
end
