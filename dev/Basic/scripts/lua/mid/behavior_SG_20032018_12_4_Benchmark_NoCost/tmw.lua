--[[
Model - Mode choice for work tour to usual location
Type - MNL
Authors - Siyu Li, Harish Loganathan
]]

-- all require statements do not work with C++. They need to be commented. The order in which lua files are loaded must be explicitly controlled in C++.
--require "NLogit"

--Estimated values for all betas
--Note: the betas that not estimated are fixed to zero.

--!! see the documentation on the definition of AM,PM and OP table!!
local bundled_variables = {}
bundled_variables.beta_cons_bus = -2.027
bundled_variables.beta_cons_mrt = -2.138
bundled_variables.beta_cons_privatebus = -4.61
bundled_variables.beta_cons_drive1 = 0
bundled_variables.beta_cons_share2 = -7.781
bundled_variables.beta_cons_share3 = -6.132
bundled_variables.beta_cons_motor = -0.042
bundled_variables.beta_cons_walk = 4.696
bundled_variables.beta_cons_taxi = -4.817
bundled_variables.beta_cons_SMS =  -4.317
bundled_variables.beta_cons_Rail_SMS =  -4.446
bundled_variables.beta_cons_SMS_Pool =  -6.846
bundled_variables.beta_cons_Rail_SMS_Pool =  -13.682

local beta1_1_tt = -0.717
local beta1_2_tt = -1.37
local beta1_3_tt = -3.53

bundled_variables.beta_private_1_tt = -0.466

local beta2_tt_drive1 = -0.980
local beta2_tt_share2 = -1.46
local beta2_tt_share3 = -1.47
local beta2_tt_motor = -0.897

bundled_variables.beta_tt_walk = -2.21
bundled_variables.beta_tt_taxi = -1.17
bundled_variables.beta_tt_SMS = -1.17
bundled_variables.beta_tt_SMS_Pool = -1.17

local beta4_1_cost = 0
local beta4_2_cost = 0
local beta5_1_cost = 0
local beta5_2_cost = 0
local beta6_1_cost = 0
local beta6_2_cost = 0
local beta7_1_cost = 0
local beta7_2_cost = 0
local beta8_1_cost = 0
local beta8_2_cost = 0
local beta9_1_cost = 0
local beta9_2_cost = 0
local beta11_1_cost = 0
local beta11_2_cost = 0
local beta10_1_cost = 0
local beta10_2_cost = 0



bundled_variables.beta_cost_erp = 0
bundled_variables.beta_cost_parking = 0

bundled_variables.beta_central_bus = 0.123
bundled_variables.beta_central_mrt = 1.13
bundled_variables.beta_central_privatebus = -0.685
bundled_variables.beta_central_share2 = 0.415
bundled_variables.beta_central_share3 = -0.165
bundled_variables.beta_central_motor = 0.300
bundled_variables.beta_central_taxi = 1.11
bundled_variables.beta_central_walk = 0.766
bundled_variables.beta_central_SMS = 1.11
bundled_variables.beta_central_Rail_SMS = 1.13
bundled_variables.beta_central_SMS_Pool = 1.11
bundled_variables.beta_central_Rail_SMS_Pool = 1.13

bundled_variables.beta_female_oneplus_bus = 1.73
bundled_variables.beta_female_twoplus_bus = -0.977

bundled_variables.beta_female_oneplus_mrt = 1.73
bundled_variables.beta_female_twoplus_mrt = -1.58

bundled_variables.beta_female_oneplus_Rail_SMS = 1.73
bundled_variables.beta_female_twoplus_Rail_SMS = -1.58

bundled_variables.beta_female_oneplus_Rail_SMS_Pool = 1.73
bundled_variables.beta_female_twoplus_Rail_SMS_Pool = -1.58

bundled_variables.beta_female_oneplus_privatebus = 1.77
bundled_variables.beta_female_twoplus_privatebus = -1.05

bundled_variables.beta_female_oneplus_drive1 = 0
bundled_variables.beta_female_twoplus_drive1 = -0.708

bundled_variables.beta_female_oneplus_share2 = 1.57
bundled_variables.beta_female_twoplus_share2 = -1.54

bundled_variables.beta_female_oneplus_share3 = 1.38
bundled_variables.beta_female_twoplus_share3 = -0.560

bundled_variables.beta_female_oneplus_motor = -3.33
bundled_variables.beta_female_twoplus_motor = 0

bundled_variables.beta_female_oneplus_taxi = 0.826
bundled_variables.beta_female_twoplus_taxi = 0

bundled_variables.beta_female_oneplus_SMS = 0.826
bundled_variables.beta_female_twoplus_SMS = 0

bundled_variables.beta_female_oneplus_SMS_Pool = 0.826
bundled_variables.beta_female_twoplus_SMS_Pool = 0

bundled_variables.beta_female_oneplus_walk = 1.36
bundled_variables.beta_female_twoplus_walk = 0

bundled_variables.beta_zero_bus = 0
bundled_variables.beta_oneplus_bus = -1.61
bundled_variables.beta_twoplus_bus = 0.384
bundled_variables.beta_threeplus_bus = 0

bundled_variables.beta_zero_mrt = 0
bundled_variables.beta_oneplus_mrt = -1.43
bundled_variables.beta_twoplus_mrt = 0.525
bundled_variables.beta_threeplus_mrt = 0

bundled_variables.beta_zero_Rail_SMS = 0
bundled_variables.beta_oneplus_Rail_SMS = -1.43
bundled_variables.beta_twoplus_Rail_SMS = 0.525
bundled_variables.beta_threeplus_Rail_SMS = 0

bundled_variables.beta_zero_Rail_SMS_Pool = 0
bundled_variables.beta_oneplus_Rail_SMS_Pool = -1.43
bundled_variables.beta_twoplus_Rail_SMS_Pool = 0.525
bundled_variables.beta_threeplus_Rail_SMS_Pool = 0

bundled_variables.beta_zero_privatebus = 0
bundled_variables.beta_oneplus_privatebus= -1.57
bundled_variables.beta_twoplus_privatebus = 0
bundled_variables.beta_threeplus_privatebus = 0

bundled_variables.beta_zero_drive1 = 0
bundled_variables.beta_oneplus_drive1 = 0
bundled_variables.beta_twoplus_drive1 = 0
bundled_variables.beta_threeplus_drive1 = 0

bundled_variables.beta_zero_share2 = 0
bundled_variables.beta_oneplus_share2 = 1.31
bundled_variables.beta_twoplus_share2 = 1.20
bundled_variables.beta_threeplus_share2 = 0

bundled_variables.beta_zero_share3 = 0
bundled_variables.beta_oneplus_share3 = 0.562
bundled_variables.beta_twoplus_share3 = 0
bundled_variables.beta_threeplus_share3 = 0

bundled_variables.beta_zero_motor_car = 0
bundled_variables.beta_oneplus_motor_car = -0.550
bundled_variables.beta_twoplus_motor_car = -0.0800
bundled_variables.beta_threeplus_motor_car= 0

bundled_variables.beta_zero_walk = 0
bundled_variables.beta_oneplus_walk = -1.28
bundled_variables.beta_twoplus_walk = 0
bundled_variables.beta_threeplus_walk = 0

bundled_variables.beta_zero_taxi = 0
bundled_variables.beta_oneplus_taxi = 0
bundled_variables.beta_twoplus_taxi = 0
bundled_variables.beta_threeplus_taxi = 0

bundled_variables.beta_zero_SMS = 0
bundled_variables.beta_oneplus_SMS = 0
bundled_variables.beta_twoplus_SMS = 0
bundled_variables.beta_threeplus_SMS = 0

bundled_variables.beta_zero_SMS_Pool = 0
bundled_variables.beta_oneplus_SMS_Pool = 0
bundled_variables.beta_twoplus_SMS_Pool = 0
bundled_variables.beta_threeplus_SMS_Pool = 0

bundled_variables.beta_zero_motor = 0
bundled_variables.beta_oneplus_motor = 8.2
bundled_variables.beta_twoplus_motor = 0.238
bundled_variables.beta_threeplus_motor = 0.0613

bundled_variables.beta_transfer = -0.0757

bundled_variables.beta_distance = 0
bundled_variables.beta_residence = 0.0532
bundled_variables.beta_residence_2 = 0
bundled_variables.beta_attraction = -0.0598
bundled_variables.beta_attraction_2 = 0


bundled_variables.beta_age2025_zero_car_bus = -1.06
bundled_variables.beta_age2635_zero_car_bus = 1.41
bundled_variables.beta_age5165_zero_car_bus = 0.602
bundled_variables.beta_age65_zero_car_bus = 1.0

bundled_variables.beta_age2025_zero_car_mrt = -1.04
bundled_variables.beta_age2635_zero_car_mrt = 1.73

bundled_variables.beta_age2025_zero_car_Rail_SMS = -1.04
bundled_variables.beta_age2635_zero_car_Rail_SMS = 1.73

bundled_variables.beta_age2025_zero_car_Rail_SMS_Pool = -1.04
bundled_variables.beta_age2635_zero_car_Rail_SMS_Pool = 1.73

bundled_variables.beta_age2025_zero_car_privatebus = -1.22
bundled_variables.beta_age2635_zero_car_privatebus = 1.37
bundled_variables.beta_age3650_zero_car_privatebus = -0.230
bundled_variables.beta_age5165_zero_car_privatebus = 0.247
bundled_variables.beta_age65_zero_car_privatebus = -0.528
bundled_variables.beta_age65_one_plus_car_privatebus = 0.454

bundled_variables.beta_age2025_zero_car_share2 = -1.23
bundled_variables.beta_age2635_zero_car_share2 = 1.48
bundled_variables.beta_age3650_zero_car_share2 = 0
bundled_variables.beta_age3650_one_plus_car_share2 = 0.297
bundled_variables.beta_age5165_zero_car_share2 = 0.550
bundled_variables.beta_age65_zero_car_share2 = 0.868
bundled_variables.beta_age65_one_plus_car_share2 = 0.998

bundled_variables.beta_age2025_zero_car_share3 = -1.94
bundled_variables.beta_age2635_zero_car_share3 = 1.44
bundled_variables.beta_age3650_zero_car_share3 = 0
bundled_variables.beta_age3650_one_plus_car_share3 = -0.0372
bundled_variables.beta_age5165_zero_car_share3 = -0.0415


bundled_variables.beta_age2025_zero_car_motor = -2.13
bundled_variables.beta_age2635_zero_car_motor = 0.884
bundled_variables.beta_age3650_zero_car_motor = -0.0532
bundled_variables.beta_age5165_zero_car_motor = 0.337
bundled_variables.beta_age65_zero_car_motor = 0.844
bundled_variables.beta_age65_one_plus_car_motor = -1.24

bundled_variables.beta_age2025_zero_car_walk=-0.89
bundled_variables.beta_age2635_zero_car_walk=1.78
bundled_variables.beta_age3650_one_plus_car_walk=0.445
bundled_variables.beta_age5165_zero_car_walk=0.342
bundled_variables.beta_age65_zero_car_walk=-0.265
bundled_variables.beta_age65_one_plus_car_walk=-1.33

bundled_variables.beta_age2635_zero_car_taxi=2.34
bundled_variables.beta_age2635_one_plus_car_taxi=0.271
bundled_variables.beta_age3650_one_plus_car_taxi=-0.428
bundled_variables.beta_age5165_zero_car_taxi=0.673
bundled_variables.beta_age65_zero_car_taxi=2.33

bundled_variables.beta_age2635_zero_car_SMS=2.34
bundled_variables.beta_age2635_one_plus_car_SMS=0.271
bundled_variables.beta_age3650_one_plus_car_SMS=-0.428
bundled_variables.beta_age5165_zero_car_SMS=0.673
bundled_variables.beta_age65_zero_car_SMS=2.33

bundled_variables.beta_age2635_zero_car_SMS_Pool=2.34
bundled_variables.beta_age2635_one_plus_car_SMS_Pool=0.271
bundled_variables.beta_age3650_one_plus_car_SMS_Pool=-0.428
bundled_variables.beta_age5165_zero_car_SMS_Pool=0.673
bundled_variables.beta_age65_zero_car_SMS_Pool=2.33



--choice set
-- 1 for public bus; 2 for MRT/LRT; 3 for private bus; 4 for drive1;
-- 5 for shared2; 6 for shared3+; 7 for motor; 8 for walk; 9 for taxi
	--choiceset
local choice = {
		1,
		2,
		3,
		4,
		5,
		6,
		7,
		8,
		9,
		10,
		11,
		12,
		13
		}


local modes = {['BusTravel'] = 1 , ['MRT'] =2 , ['PrivateBus'] =3 ,  ['Car'] = 4,  ['Car_Sharing_2'] = 5,
	['Car_Sharing_3'] = 6, ['Motorcycle'] = 7,['Walk'] = 8, ['Taxi'] = 9 , ['SMS'] = 10,
	['Rail_SMS'] = 11, ['SMS_Pool'] = 12, ['Rail_SMS_Pool'] = 13 }



--choice["PT"] = {1,2,3}
--choice["non-PT"] = {4,5,6,7,8,9,10,11,12}


--utility
-- 1 for public bus; 2 for MRT/LRT; 3 for private bus; 4 for drive1;
-- 5 for shared2; 6 for shared3+; 7 for motor; 8 for walk; 9 for taxi
local utility = {}

local function computeUtilities(params,dbparams)
	local cost_increase = dbparams.cost_increase
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
	local d1 = dbparams.walk_distance1
	local d2 = dbparams.walk_distance2

	local age_id = params.age_id
	-- age group related variables
	local age20,age2025,age2635,age3650,age5165,age65 = 0,0,0,0,0,0
	if age_id < 4 then
		age20 = 1
	elseif age_id == 4 then
		age2025 = 1
	elseif age_id == 5 or age_id == 6 then
		age2635 = 1
	elseif age_id == 7 or age_id == 8 or age_id == 9 then
		age3650 = 1
	elseif age_id == 10 or age_id == 11 or age_id == 12 then
		age5165 = 1
	elseif age_id > 12 then
		age65 = 1
	end


	--dbparams.cost_public_first = AM[(origin,destination)]['pub_cost']
	--origin is home, destination is tour destination
	--0 if origin == destination
	local cost_public_first = dbparams.cost_public_first

	--dbparams.cost_public_second = PM[(destination,origin)]['pub_cost']
	--origin is home, destination is tour destination
	--0 if origin == destination
	local cost_public_second = dbparams.cost_public_second

	local cost_bus=cost_public_first+cost_public_second + cost_increase
	local cost_mrt=cost_public_first+cost_public_second + cost_increase

	local cost_privatebus=cost_public_first+cost_public_second + cost_increase

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

	local cost_cardriver=cost_car_ERP_first+cost_car_ERP_second+cost_car_OP_first+cost_car_OP_second+cost_car_parking + cost_increase
	local cost_carpassenger=cost_car_ERP_first+cost_car_ERP_second+cost_car_OP_first+cost_car_OP_second+cost_car_parking + cost_increase
	local cost_motor=0.5*(cost_car_ERP_first+cost_car_ERP_second+cost_car_OP_first+cost_car_OP_second)+0.65*cost_car_parking + cost_increase

	--dbparams.walk_distance1= AM[(origin,destination)]['AM2dis']
	--origin is home mtz, destination is usual work location mtz
	--0 if origin == destination
	--dbparams.walk_distance2= PM[(destination,origin)]['PM2dis']
	--origin is home mtz, destination is usual work location mtz
	--0 if origin == destination


	--dbparams.central_dummy=ZONE[destination]['central_dummy']
	--destination is tour destination
	local central_dummy = dbparams.central_dummy

	local female_dummy = params.female_dummy
	local income_id = params.income_id
	local income_cat = {500,1250,1750,2250,2750,3500,4500,5500,6500,7500,8500,0,99999,99999}
	local income_mid = income_cat[income_id]
	local missing_income = (params.income_id >= 13) and 1 or 0    -- Vishnu 14th April 2016- Changed from the previous value of 12

	local cost_taxi_1=3.4+((d1*(d1>10 and 1 or 0)-10*(d1>10 and 1 or 0))/0.35+(d1*(d1<=10 and 1 or 0)+10*(d1>10 and 1 or 0))/0.4)*0.22+ cost_car_ERP_first + central_dummy*3
	local cost_taxi_2=3.4+((d2*(d2>10 and 1 or 0)-10*(d2>10 and 1 or 0))/0.35+(d2*(d2<=10 and 1 or 0)+10*(d2>10 and 1 or 0))/0.4)*0.22+ cost_car_ERP_second + central_dummy*3
	local cost_taxi=cost_taxi_1+cost_taxi_2 + cost_increase

	local cost_SMS_1=3.4+((d1*(d1>10 and 1 or 0)-10*(d1>10 and 1 or 0))/0.35+(d1*(d1<=10 and 1 or 0)+10*(d1>10 and 1 or 0))/0.4)*0.22+ cost_car_ERP_first + central_dummy*3
	local cost_SMS_2=3.4+((d2*(d2>10 and 1 or 0)-10*(d2>10 and 1 or 0))/0.35+(d2*(d2<=10 and 1 or 0)+10*(d2>10 and 1 or 0))/0.4)*0.22+ cost_car_ERP_second + central_dummy*3
	local cost_SMS=(cost_SMS_1+cost_SMS_2)*0.72 + cost_increase

	local cost_SMS_Pool_1=3.4+((d1*(d1>10 and 1 or 0)-10*(d1>10 and 1 or 0))/0.35+(d1*(d1<=10 and 1 or 0)+10*(d1>10 and 1 or 0))/0.4)*0.22+ cost_car_ERP_first + central_dummy*3
	local cost_SMS_Pool_2=3.4+((d2*(d2>10 and 1 or 0)-10*(d2>10 and 1 or 0))/0.35+(d2*(d2<=10 and 1 or 0)+10*(d2>10 and 1 or 0))/0.4)*0.22+ cost_car_ERP_second + central_dummy*3
	local cost_SMS_Pool=(cost_SMS_Pool_1+cost_SMS_Pool_2)*0.72*0.7 + cost_increase

	local aed_1 = (5*tt_public_walk_first) -- Access egress distance
	local aed_2 = (5*tt_public_walk_second) -- Access egress distance

	local cost_Rail_SMS_AE_Pool_1 = 3.4+((aed_1*(aed_1>10 and 1 or 0)-10*(aed_1>10 and 1 or 0))/0.35+(aed_1*(aed_1<=10 and 1 or 0)+10*(aed_1>10 and 1 or 0))/0.4)*0.22+ cost_car_ERP_first + central_dummy*3
	local cost_Rail_SMS_AE_Pool_2 = 3.4+((aed_2*(aed_2>10 and 1 or 0)-10*(aed_2>10 and 1 or 0))/0.35+(aed_2*(aed_2<=10 and 1 or 0)+10*(aed_2>10 and 1 or 0))/0.4)*0.22+ cost_car_ERP_second + central_dummy*3

	local cost_Rail_SMS_Pool = cost_public_first + cost_public_second + cost_increase + (cost_Rail_SMS_AE_Pool_1 + cost_Rail_SMS_AE_Pool_2)*0.72*0.7

	local cost_Rail_SMS_AE_1 = 3.4+((aed_1*(aed_1>10 and 1 or 0)-10*(aed_1>10 and 1 or 0))/0.35+(aed_1*(aed_1<=10 and 1 or 0)+10*(aed_1>10 and 1 or 0))/0.4)*0.22+ cost_car_ERP_first + central_dummy*3
	local cost_Rail_SMS_AE_2 = 3.4+((aed_2*(aed_2>10 and 1 or 0)-10*(aed_2>10 and 1 or 0))/0.35+(aed_2*(aed_2<=10 and 1 or 0)+10*(aed_2>10 and 1 or 0))/0.4)*0.22+ cost_car_ERP_second + central_dummy*3

	local cost_Rail_SMS = cost_public_first + cost_public_second + cost_increase + (cost_Rail_SMS_AE_1 + cost_Rail_SMS_AE_2) * 0.72

	local cost_over_income_bus=30*cost_bus/(0.5+income_mid)
	local cost_over_income_mrt=30*cost_mrt/(0.5+income_mid)

	local cost_over_income_privatebus=30*cost_privatebus/(0.5+income_mid)
	local cost_over_income_cardriver=30*cost_cardriver/(0.5+income_mid)
	local cost_over_income_carpassenger=30*cost_carpassenger/(0.5+income_mid)
	local cost_over_income_motor=30*cost_motor/(0.5+income_mid)
	local cost_over_income_taxi=30*cost_taxi/(0.5+income_mid)
	local cost_over_income_SMS=30*cost_SMS/(0.5+income_mid)
	local cost_over_income_Rail_SMS=30*cost_Rail_SMS/(0.5+income_mid)
	local cost_over_income_SMS_Pool=30*cost_SMS/(0.5+income_mid)
	local cost_over_income_Rail_SMS_Pool=30*cost_Rail_SMS/(0.5+income_mid)

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

	local tt_Rail_SMS_ivt=tt_public_ivt_first+tt_public_ivt_second
	local tt_Rail_SMS_wait=tt_public_waiting_first+tt_public_waiting_second+1/6.0+1/6.0
	local tt_Rail_SMS_walk=(tt_public_walk_first+tt_public_walk_second)/8.0
	local tt_Rail_SMS_all=tt_mrt_ivt+tt_mrt_wait+tt_mrt_walk

	local tt_Rail_SMS_Pool_ivt=tt_public_ivt_first+tt_public_ivt_second+(aed_1+aed_2)/60
	local tt_Rail_SMS_Pool_wait=tt_public_waiting_first+tt_public_waiting_second+1/6.0+1/6.0+1/10
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
	local tt_SMS_out=1.0/6
	local tt_SMS_all=tt_cardriver_ivt+tt_cardriver_out

	local tt_SMS_Pool_ivt=tt_ivt_car_first+tt_ivt_car_second+(d1+d2)/2/60
	local tt_SMS_Pool_out=1.0/6+1/10
	local tt_SMS_Pool_all=tt_cardriver_ivt+tt_cardriver_out

	--dbparams.average_transfer_number = (AM[(origin,destination)]['avg_transfer'] + PM[(destination,origin)]['avg_transfer'])/2
	--origin is home, destination is tour destination
	-- 0 if origin == destination
	local average_transfer_number = dbparams.average_transfer_number

	local zero_car,one_plus_car,two_plus_car,three_plus_car, zero_motor,one_plus_motor,two_plus_motor,three_plus_motor = 0,0,0,0,0,0,0,0
	local veh_own_cat = params.vehicle_ownership_category
	if veh_own_cat == 0 or veh_own_cat == 1 or veh_own_cat == 2 then
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


	--dbparams.resident_size = ZONE[origin]['resident workers']
	--dbparams.work_op = ZONE[destination]['employment'] --total employment
	--dbparams.origin_area= ZONE[origin]['area'] -- in square km
	--dbparams.destination_area = ZONE[destination]['area'] -- in square km
	--origin is home, destination is tour destination
	local resident_size = dbparams.resident_size
	local work_op = dbparams.work_op
	local origin_area = dbparams.origin_area
	local destination_area = dbparams.destination_area

	local residential_size=resident_size/origin_area/10000.0
	local work_attraction=work_op/destination_area/10000.0
	utility[1] = bundled_variables.beta_cons_bus + beta1_1_tt * tt_bus_ivt + beta1_2_tt * tt_bus_walk + beta1_3_tt * tt_bus_wait + beta4_1_cost * cost_over_income_bus * (1-missing_income) + beta4_2_cost * cost_bus * missing_income + bundled_variables.beta_central_bus * central_dummy + bundled_variables.beta_transfer * average_transfer_number + bundled_variables.beta_female_oneplus_bus * one_plus_car* female_dummy + bundled_variables.beta_female_twoplus_bus * female_dummy * two_plus_car + bundled_variables.beta_zero_bus*zero_car + bundled_variables.beta_oneplus_bus*one_plus_car + bundled_variables.beta_twoplus_bus*two_plus_car +bundled_variables.beta_threeplus_bus*three_plus_car + bundled_variables.beta_age2025_zero_car_bus * zero_car * age2025 + bundled_variables.beta_age2635_zero_car_bus * zero_car * age2635 + bundled_variables.beta_age5165_zero_car_bus * zero_car * age5165 + bundled_variables.beta_age65_zero_car_bus * zero_car * age65
	utility[2] = bundled_variables.beta_cons_mrt + beta1_1_tt * tt_mrt_ivt + beta1_2_tt * tt_mrt_walk + beta1_3_tt * tt_mrt_wait + beta4_1_cost * cost_over_income_mrt * (1-missing_income) + beta4_2_cost * cost_mrt * missing_income + bundled_variables.beta_central_mrt * central_dummy + bundled_variables.beta_transfer * average_transfer_number + bundled_variables.beta_female_oneplus_mrt * female_dummy * one_plus_car + bundled_variables.beta_female_twoplus_mrt * female_dummy * two_plus_car + bundled_variables.beta_zero_mrt * zero_car + bundled_variables.beta_oneplus_mrt * one_plus_car + bundled_variables.beta_twoplus_mrt * two_plus_car + bundled_variables.beta_threeplus_mrt * three_plus_car + bundled_variables.beta_age2025_zero_car_mrt * zero_car * age2025 + bundled_variables.beta_age2635_zero_car_mrt * zero_car * age2635
	utility[3] = bundled_variables.beta_cons_privatebus + bundled_variables.beta_private_1_tt * tt_privatebus_ivt + beta5_1_cost * cost_over_income_privatebus * (1-missing_income) + beta5_2_cost * cost_privatebus * missing_income + bundled_variables.beta_central_privatebus * central_dummy + bundled_variables.beta_distance*(d1+d2) + bundled_variables.beta_residence * residential_size + bundled_variables.beta_attraction * work_attraction + bundled_variables.beta_residence_2*math.pow(residential_size,2)+bundled_variables.beta_attraction_2*math.pow(work_attraction,2)+bundled_variables.beta_female_oneplus_privatebus* female_dummy * one_plus_car + bundled_variables.beta_female_twoplus_privatebus * female_dummy * two_plus_car + bundled_variables.beta_zero_privatebus * zero_car + bundled_variables.beta_oneplus_privatebus * one_plus_car + bundled_variables.beta_twoplus_privatebus * two_plus_car + bundled_variables.beta_threeplus_privatebus * three_plus_car + bundled_variables.beta_age2025_zero_car_privatebus * zero_car * age2025 + bundled_variables.beta_age2635_zero_car_privatebus * zero_car * age2635 + bundled_variables.beta_age3650_zero_car_privatebus * zero_car * age3650 + bundled_variables.beta_age5165_zero_car_privatebus * zero_car * age5165 + bundled_variables.beta_age65_zero_car_privatebus * zero_car * age65 + bundled_variables.beta_age65_one_plus_car_privatebus * one_plus_car * age65
	utility[4] = bundled_variables.beta_cons_drive1 + beta2_tt_drive1 * tt_cardriver_all + beta6_1_cost * cost_over_income_cardriver * (1-missing_income) + beta6_2_cost * cost_cardriver * missing_income + bundled_variables.beta_female_oneplus_drive1 * female_dummy * one_plus_car + bundled_variables.beta_female_twoplus_drive1* female_dummy * two_plus_car + bundled_variables.beta_zero_drive1 * zero_car + bundled_variables.beta_oneplus_drive1 * one_plus_car + bundled_variables.beta_twoplus_drive1 * two_plus_car + bundled_variables.beta_threeplus_drive1 * three_plus_car
	utility[5] = bundled_variables.beta_cons_share2 + beta2_tt_share2 * tt_carpassenger_all + beta7_1_cost * cost_over_income_carpassenger/2 * (1-missing_income) + beta7_2_cost * cost_carpassenger/2 * missing_income  + bundled_variables.beta_central_share2 * central_dummy + bundled_variables.beta_female_oneplus_share2 * female_dummy * one_plus_car + bundled_variables.beta_female_twoplus_share2 * female_dummy * two_plus_car + bundled_variables.beta_zero_share2 * zero_car + bundled_variables.beta_oneplus_share2 * one_plus_car + bundled_variables.beta_twoplus_share2 * two_plus_car + bundled_variables.beta_threeplus_share2 * three_plus_car + bundled_variables.beta_age2025_zero_car_share2 * zero_car * age2025 + bundled_variables.beta_age2635_zero_car_share2 * zero_car * age2635 + bundled_variables.beta_age3650_zero_car_share2 * zero_car * age3650 + bundled_variables.beta_age3650_one_plus_car_share2 * one_plus_car * age3650 + bundled_variables.beta_age5165_zero_car_share2 * zero_car * age5165 + bundled_variables.beta_age65_zero_car_share2 * zero_car * age65 + bundled_variables.beta_age65_one_plus_car_share2 * one_plus_car * age65
	utility[6] = bundled_variables.beta_cons_share3 + beta2_tt_share3 * tt_carpassenger_all + beta8_1_cost * cost_over_income_carpassenger/3 * (1-missing_income) + beta8_2_cost * cost_carpassenger/3 * missing_income  + bundled_variables.beta_central_share3 * central_dummy + bundled_variables.beta_female_oneplus_share3 * female_dummy * one_plus_car + bundled_variables.beta_female_twoplus_share3 * female_dummy * two_plus_car + bundled_variables.beta_zero_share3 * zero_car + bundled_variables.beta_oneplus_share3 * one_plus_car + bundled_variables.beta_twoplus_share3 * two_plus_car + bundled_variables.beta_threeplus_share3 * three_plus_car + bundled_variables.beta_age2025_zero_car_share3 * zero_car * age2025 + bundled_variables.beta_age2635_zero_car_share3 * zero_car * age2635 + bundled_variables.beta_age3650_zero_car_share3 * zero_car * age3650 + bundled_variables.beta_age3650_one_plus_car_share3 * one_plus_car * age3650 + bundled_variables.beta_age5165_zero_car_share3 * zero_car * age5165
	utility[7] = bundled_variables.beta_cons_motor + beta2_tt_motor * tt_motor_all + beta9_1_cost * cost_over_income_motor * (1-missing_income) + beta9_2_cost * cost_motor * missing_income  + bundled_variables.beta_central_motor * central_dummy + bundled_variables.beta_zero_motor * zero_motor + bundled_variables.beta_oneplus_motor * one_plus_motor + bundled_variables.beta_twoplus_motor * two_plus_motor + bundled_variables.beta_threeplus_motor * three_plus_motor + bundled_variables.beta_female_oneplus_motor * female_dummy *one_plus_car + bundled_variables.beta_female_twoplus_motor * female_dummy * two_plus_car + bundled_variables.beta_zero_motor_car * zero_car + bundled_variables.beta_oneplus_motor_car * one_plus_car + bundled_variables.beta_twoplus_motor_car * two_plus_car + bundled_variables.beta_threeplus_motor_car * three_plus_car + bundled_variables.beta_age2025_zero_car_motor *age2025 * zero_car + bundled_variables.beta_age2635_zero_car_motor * zero_car * age2635 + bundled_variables.beta_age3650_zero_car_motor * zero_car * age3650 + bundled_variables.beta_age5165_zero_car_motor * zero_car * age5165 + bundled_variables.beta_age65_zero_car_motor * zero_car * age65 + bundled_variables.beta_age65_one_plus_car_motor * one_plus_car * age65
	utility[8] = bundled_variables.beta_cons_walk  + bundled_variables.beta_tt_walk * tt_walk + bundled_variables.beta_central_walk * central_dummy+ bundled_variables.beta_female_oneplus_walk * female_dummy * one_plus_car + bundled_variables.beta_female_twoplus_walk * female_dummy * two_plus_car + bundled_variables.beta_zero_walk * zero_car + bundled_variables.beta_oneplus_walk * one_plus_car + bundled_variables.beta_twoplus_walk * two_plus_car + bundled_variables.beta_threeplus_walk * three_plus_car + bundled_variables.beta_age2025_zero_car_walk * zero_car * age2025 + bundled_variables.beta_age2635_zero_car_walk * zero_car * age2635 + bundled_variables.beta_age3650_one_plus_car_walk * one_plus_car * age3650 + bundled_variables.beta_age5165_zero_car_walk * zero_car * age5165 + bundled_variables.beta_age65_zero_car_walk * zero_car * age65 + bundled_variables.beta_age65_one_plus_car_walk * one_plus_car * age65
	utility[9] = bundled_variables.beta_cons_taxi + bundled_variables.beta_tt_taxi * tt_taxi_all + beta10_1_cost * cost_over_income_taxi * (1-missing_income) + beta10_2_cost * cost_taxi * missing_income + bundled_variables.beta_central_taxi * central_dummy + bundled_variables.beta_female_oneplus_taxi * female_dummy * one_plus_car + bundled_variables.beta_female_twoplus_taxi * female_dummy * two_plus_car + bundled_variables.beta_zero_taxi * zero_car + bundled_variables.beta_oneplus_taxi * one_plus_car + bundled_variables.beta_twoplus_taxi * two_plus_car + bundled_variables.beta_threeplus_taxi * three_plus_car + bundled_variables.beta_age2635_zero_car_taxi * age2635* zero_car + bundled_variables.beta_age2635_one_plus_car_taxi * one_plus_car * age2635 + bundled_variables.beta_age3650_one_plus_car_taxi * one_plus_car * age3650 + bundled_variables.beta_age5165_zero_car_taxi * zero_car * age5165 + bundled_variables.beta_age65_zero_car_taxi * zero_car * age65
	utility[10] = bundled_variables.beta_cons_SMS + bundled_variables.beta_tt_SMS * tt_SMS_all + beta11_1_cost * cost_over_income_SMS * (1-missing_income) + beta11_2_cost * cost_SMS * missing_income + bundled_variables.beta_central_SMS * central_dummy + bundled_variables.beta_female_oneplus_SMS * female_dummy * one_plus_car + bundled_variables.beta_female_twoplus_SMS * female_dummy * two_plus_car + bundled_variables.beta_zero_SMS * zero_car + bundled_variables.beta_oneplus_SMS * one_plus_car + bundled_variables.beta_twoplus_SMS * two_plus_car + bundled_variables.beta_threeplus_SMS * three_plus_car + bundled_variables.beta_age2635_zero_car_SMS * age2635* zero_car + bundled_variables.beta_age2635_one_plus_car_SMS * one_plus_car * age2635 + bundled_variables.beta_age3650_one_plus_car_SMS * one_plus_car * age3650 + bundled_variables.beta_age5165_zero_car_SMS * zero_car * age5165 + bundled_variables.beta_age65_zero_car_SMS * zero_car * age65
	utility[11] = bundled_variables.beta_cons_Rail_SMS + beta1_1_tt * tt_Rail_SMS_ivt + beta1_2_tt * tt_Rail_SMS_walk + beta1_3_tt * tt_Rail_SMS_wait + beta4_1_cost * cost_over_income_Rail_SMS * (1-missing_income) + beta4_2_cost * cost_Rail_SMS * missing_income + bundled_variables.beta_central_Rail_SMS * central_dummy + bundled_variables.beta_transfer * average_transfer_number + bundled_variables.beta_female_oneplus_Rail_SMS * female_dummy * one_plus_car + bundled_variables.beta_female_twoplus_Rail_SMS * female_dummy * two_plus_car + bundled_variables.beta_zero_Rail_SMS * zero_car + bundled_variables.beta_oneplus_Rail_SMS * one_plus_car + bundled_variables.beta_twoplus_Rail_SMS * two_plus_car + bundled_variables.beta_threeplus_Rail_SMS * three_plus_car + bundled_variables.beta_age2025_zero_car_Rail_SMS * zero_car * age2025 + bundled_variables.beta_age2635_zero_car_Rail_SMS * zero_car * age2635
	utility[12] = bundled_variables.beta_cons_SMS_Pool + bundled_variables.beta_tt_SMS_Pool * tt_SMS_Pool_all + beta11_1_cost * cost_over_income_SMS_Pool * (1-missing_income) + beta11_2_cost * cost_SMS_Pool * missing_income + bundled_variables.beta_central_SMS_Pool * central_dummy + bundled_variables.beta_female_oneplus_SMS_Pool * female_dummy * one_plus_car + bundled_variables.beta_female_twoplus_SMS_Pool * female_dummy * two_plus_car + bundled_variables.beta_zero_SMS_Pool * zero_car + bundled_variables.beta_oneplus_SMS_Pool * one_plus_car + bundled_variables.beta_twoplus_SMS_Pool * two_plus_car + bundled_variables.beta_threeplus_SMS_Pool * three_plus_car + bundled_variables.beta_age2635_zero_car_SMS_Pool * age2635* zero_car + bundled_variables.beta_age2635_one_plus_car_SMS_Pool * one_plus_car * age2635 + bundled_variables.beta_age3650_one_plus_car_SMS_Pool * one_plus_car * age3650 + bundled_variables.beta_age5165_zero_car_SMS_Pool * zero_car * age5165 + bundled_variables.beta_age65_zero_car_SMS_Pool * zero_car * age65
	utility[13] = bundled_variables.beta_cons_Rail_SMS_Pool + beta1_1_tt * tt_Rail_SMS_Pool_ivt + beta1_2_tt * tt_Rail_SMS_Pool_walk + beta1_3_tt * tt_Rail_SMS_Pool_wait + beta4_1_cost * cost_over_income_Rail_SMS_Pool * (1-missing_income) + beta4_2_cost * cost_Rail_SMS_Pool * missing_income + bundled_variables.beta_central_Rail_SMS_Pool * central_dummy + bundled_variables.beta_transfer * average_transfer_number + bundled_variables.beta_female_oneplus_Rail_SMS_Pool * female_dummy * one_plus_car + bundled_variables.beta_female_twoplus_Rail_SMS_Pool * female_dummy * two_plus_car + bundled_variables.beta_zero_Rail_SMS_Pool * zero_car + bundled_variables.beta_oneplus_Rail_SMS_Pool * one_plus_car + bundled_variables.beta_twoplus_Rail_SMS_Pool * two_plus_car + bundled_variables.beta_threeplus_Rail_SMS_Pool * three_plus_car + bundled_variables.beta_age2025_zero_car_Rail_SMS_Pool * zero_car * age2025 + bundled_variables.beta_age2635_zero_car_Rail_SMS_Pool * zero_car * age2635

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
local scale = 1
--scale["PT"] = 1
--scale["non-PT"] = 1
-- function to call from C++ preday simulator
-- params and dbparams tables contain data passed from C++
-- to check variable bindings in params or dbparams, refer PredayLuaModel::mapClasses() function in dev/Basic/medium/behavioral/lua/PredayLuaModel.cpp
function choose_tmw(params,dbparams)
	computeUtilities(params,dbparams)
	computeAvailabilities(params,dbparams)


	local probability = calculate_probability("mnl", choice, utility, availability, scale)
	return make_final_choice(probability)
end

-- function to call from C++ preday simulator for logsums computation
-- params and dbparams tables contain data passed from C++
-- to check variable bindings in params or dbparams, refer PredayLuaModel::mapClasses() function in dev/Basic/medium/behavioral/lua/PredayLuaModel.cpp
function compute_logsum_tmw(params,dbparams)
	computeUtilities(params,dbparams) 
	computeAvailabilities(params,dbparams)
	return compute_mnl_logsum(utility, availability)
end
