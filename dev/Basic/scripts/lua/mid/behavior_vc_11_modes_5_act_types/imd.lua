--[[
Model - Mode/destination choice for work tour to unusual location
Type - logit
Authors - Siyu Li, Harish Loganathan
]]

-- all require statements do not work with C++. They need to be commented. The order in which lua files are loaded must be explicitly controlled in C++. 
--require "Logit"

--Estimated values for all betas
--Note: the betas that not estimated are fixed to zero.

--!! see the documentation on the definition of AM,PM and OP table!!
--!! see gen_modified_mode_destination.py for variable generation !!

--Aug 30, 2014 Now first_bound and second_bound will need to be binded to this file.

local beta_cost_bus_mrt_2 = -0.438
local beta_cost_rail_SMS_2 = -0.438
local beta_cost_private_bus_2 = -0.850
local beta_cost_drive1_2_first = -0.0799
local beta_cost_drive1_2_second = 0.0598
local beta_cost_share2_2 = 0
local beta_cost_share3_2 = 0
local beta_cost_motor_2 = -0.338
local beta_cost_taxi_2 = 0
local beta_cost_SMS_2 = 0


local beta_tt_bus_mrt = -3.75
local beta_tt_rail_SMS = -3.75
local beta_tt_private_bus = -4.22
local beta_tt_drive1_first = -4.77
local beta_tt_drive1_second = -5.18
local beta_tt_share2= 0
local beta_tt_share3= 0
local beta_tt_motor = 0
local beta_tt_walk = 0
local beta_tt_taxi_first = -5.63
local beta_tt_taxi_second = -5.79
local beta_tt_SMS_first = -5.63
local beta_tt_SMS_second = -5.79

local beta_work = 0.567
local beta_shop = 0.979

local beta_central_bus_mrt = 0.172
local beta_central_rail_SMS = 0.172
local beta_central_private_bus = 0.175
local beta_central_drive1 = 0
local beta_central_share2 = -0.0332
local beta_central_share3 = 0.221
local beta_central_motor = 0.180
local beta_central_walk = 0
local beta_central_taxi = 1.07
local beta_central_SMS = 1.07

local beta_distance_bus_mrt = 0.0112
local beta_distance_rail_SMS = 0.0112
local beta_distance_private_bus = 0.0189
local beta_distance_drive1 = 0
local beta_distance_share2 = -0.0229
local beta_distance_share3 = -0.0173
local beta_distance_motor = 0.00259
local beta_distance_walk = 0
local beta_distance_taxi = 0.000622
local beta_distance_SMS = 0.000622

local beta_cons_bus = 4.479
local beta_cons_mrt = 4.383
local beta_cons_rail_SMS = 4.383
local beta_cons_private_bus = 2.172
local beta_cons_drive1 = 0
local beta_cons_share2 = 3.162
local beta_cons_share3 = 2.272
local beta_cons_motor = -2.398
local beta_cons_walk = -101.128
local beta_cons_taxi = -5.228
local beta_cons_SMS = -5.228

local beta_zero_drive1 = 0
local beta_oneplus_drive1 = 3.83
local beta_twoplus_drive1 = 0.0514
local beta_threeplus_drive1 = 0

local beta_zero_share2 = 0
local beta_oneplus_share2 = 2.17
local beta_twoplus_share2 = 0
local beta_threeplus_share2 = 0

local beta_zero_share3 = 0
local beta_oneplus_share3 = 2.26
local beta_twoplus_share3 = 0
local beta_threeplus_share3 = 0

local beta_zero_motor = 0
local beta_oneplus_motor = 4.82
local beta_twoplus_motor = 0
local beta_threeplus_motor = 0

local beta_female_bus = 0.568
local beta_female_mrt = 0.622
local beta_female_rail_SMS = 0.622
local beta_female_private_bus = 1.45
local beta_female_drive1 = 0
local beta_female_share2 = 0.393
local beta_female_share3 = 0.162
local beta_female_motor = 0.283
local beta_female_taxi = 1.40
local beta_female_SMS = 1.40
local beta_female_walk = 0


--choice set
local choice = {}
for i = 1, 24*11 do 
	choice[i] = i
end

--utility
-- 1 for public bus; 2 for MRT/LRT; 3 for private bus; 4 for drive1;
-- 5 for shared2; 6 for shared3+; 7 for motor; 8 for walk; 9 for taxi
local utility = {}
local function computeUtilities(params,dbparams)
	local female_dummy = params.female_dummy
	local income_id = params.income_id
	local income_cat = {500,1250,1750,2250,2750,3500,4500,5500,6500,7500,8500,0,99999,99999}
	local income_mid = income_cat[income_id]
	local missing_income = params.income_id >= 13 and 1 or 0

	local work_stop_dummy = dbparams.stop_type == 1 and 1 or 0
	local edu_stop_dummy = dbparams.stop_type == 2 and 1 or 0
	local shop_stop_dummy = dbparams.stop_type == 3 and 1 or 0
	local other_stop_dummy = dbparams.stop_type == 4 and 1 or 0

	--1 if the current modeled stop is on first half tour, 0 otherwise
	first_bound = dbparams.first_bound
	--1 if the current modeled stop is on second half tour, 0 otherwise
	second_bound = dbparams.second_bound


	--params.car_own_normal is from household table
	--imd use all cars (car_normal + car_offpeak) to calculate zero car...
	local zero_car,one_plus_car,two_plus_car,three_plus_car, zero_motor,one_plus_motor,two_plus_motor,three_plus_motor = 0,0,0,0,0,0,0,0
	local veh_own_cat = params.vehicle_ownership_category
	if veh_own_cat == 0  then 
		zero_car = 1 
		
	end
	if veh_own_cat == 2 or veh_own_cat == 3 or veh_own_cat == 4 or veh_own_cat == 5  then 
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
	


	local cost_public = {}
	local cost_bus = {}
	local cost_mrt = {}
	local cost_rail_SMS = {}
	local cost_Rail_SMS_AE_1 = {}
	local cost_Rail_SMS_AE_2 = {}
	local cost_Rail_SMS_AE_avg = {}
	local cost_private_bus = {}

	local cost_car_OP = {}
	local cost_car_ERP = {}
	local cost_car_parking = {}
	local cost_drive1 = {}
	local cost_share2 = {}
	local cost_share3 = {}
	local cost_motor = {}
	
	local cost_taxi_1 = {}
	local cost_taxi_2 = {}
	local cost_taxi={}
	
	local cost_SMS_1 = {}
	local cost_SMS_2 = {}
	local cost_SMS={}

	local d1={}
	local d2={}
	local central_dummy={}

	local cost_over_income_bus = {}
	local cost_over_income_mrt = {}
	local cost_over_income_rail_SMS = {}
	local cost_over_income_private_bus = {}
	local cost_over_income_drive1 = {}
	local cost_over_income_share2 = {}
	local cost_over_income_share3 = {}
	local cost_over_income_motor = {}
	local cost_over_income_taxi = {}
	local cost_over_income_SMS = {}

	local tt_public_ivt = {}
	local tt_public_out = {}

	local tt_car_ivt = {}

	local tt_bus = {}
	local tt_mrt = {}
	local tt_rail_SMS = {}
	local tt_private_bus = {}
	local tt_drive1 = {}
	local tt_share2 = {}
	local tt_share3 = {}
	local tt_motor = {}
	local tt_walk = {}
	local tt_taxi = {}
	local tt_SMS = {}

	local average_transfer_number = {}

	local employment = {}
	local population = {}
	local area = {}
	local shop = {}
	


	for i =1,24 do
		--dbparams.cost_public(i) = 

		d1[i] = dbparams:walk_distance1(i)
		d2[i] = dbparams:walk_distance2(i)
		
		cost_public[i] = dbparams:cost_public(i)
		cost_bus[i] = cost_public[i]
		cost_mrt[i] = cost_public[i]
		
		cost_private_bus[i] = cost_public[i]

		--dbparams.cost_car_ERP(i) = 
		--dbparams.cost_car_OP(i) = 
		--dbparams.cost_car_parking(i) = 

		cost_drive1[i] = dbparams:cost_car_ERP(i) + dbparams:cost_car_OP(i) + dbparams:cost_car_parking(i)
		cost_share2[i] = (dbparams:cost_car_ERP(i) + dbparams:cost_car_OP(i) + dbparams:cost_car_parking(i))/2
		cost_share3[i] = (dbparams:cost_car_ERP(i) + dbparams:cost_car_OP(i) + dbparams:cost_car_parking(i))/3
		cost_motor[i] = 0.5*(dbparams:cost_car_ERP(i) + dbparams:cost_car_OP(i))+ 0.65*dbparams:cost_car_parking(i)

		--dbparams.walk_distance1(i)= 
		--dbparams.walk_distance2(i)=
		--dbparams.central_dummy(i)=
		
		central_dummy[i] = dbparams:central_dummy(i)

		cost_taxi_1[i] = 3.4+((d1[i]*(d1[i]>10 and 1 or 0)-10*(d1[i]>10 and 1 or 0))/0.35+(d1[i]*(d1[i]<=10 and 1 or 0)+10*(d1[i]>10 and 1 or 0))/0.4)*0.22+ dbparams:cost_car_ERP(i) + central_dummy[i]*3
		cost_taxi_2[i] = 3.4+((d2[i]*(d2[i]>10 and 1 or 0)-10*(d2[i]>10 and 1 or 0))/0.35+(d2[i]*(d2[i]<=10 and 1 or 0)+10*(d2[i]>10 and 1 or 0))/0.4)*0.22+ central_dummy[i]*3
		cost_taxi[i] = (cost_taxi_1[i] + cost_taxi_2[i])/2
		
		cost_SMS_1[i] = 3.4+((d1[i]*(d1[i]>10 and 1 or 0)-10*(d1[i]>10 and 1 or 0))/0.35+(d1[i]*(d1[i]<=10 and 1 or 0)+10*(d1[i]>10 and 1 or 0))/0.4)*0.22+ dbparams:cost_car_ERP(i) + central_dummy[i]*3
		cost_SMS_2[i] = 3.4+((d2[i]*(d2[i]>10 and 1 or 0)-10*(d2[i]>10 and 1 or 0))/0.35+(d2[i]*(d2[i]<=10 and 1 or 0)+10*(d2[i]>10 and 1 or 0))/0.4)*0.22+ central_dummy[i]*3
		cost_SMS[i] = (cost_SMS_1[i] + cost_SMS_2[i])*0.6/2
		
		local aed = 2.0 -- Access egress distance
		cost_Rail_SMS_AE_1[i] = 3.4+((aed*(aed>10 and 1 or 0)-10*(aed>10 and 1 or 0))/0.35+(aed*(aed<=10 and 1 or 0)+10*(aed>10 and 1 or 0))/0.4)*0.22+ dbparams:cost_car_ERP(i) + central_dummy[i]*3
		cost_Rail_SMS_AE_2[i] = 3.4+((aed*(aed>10 and 1 or 0)-10*(aed>10 and 1 or 0))/0.35+(aed*(aed<=10 and 1 or 0)+10*(aed>10 and 1 or 0))/0.4)*0.22+ central_dummy[i]*3
		cost_Rail_SMS_AE_avg[i] = (cost_Rail_SMS_AE_1[i] + cost_Rail_SMS_AE_2[i])/2

		cost_rail_SMS[i] = cost_public[i] + cost_Rail_SMS_AE_avg[i]*0.6

		cost_over_income_bus[i]=30*cost_bus[i]/(0.5+income_mid)
		cost_over_income_mrt[i]=30*cost_mrt[i]/(0.5+income_mid)
		cost_over_income_rail_SMS[i]=30*cost_rail_SMS[i]/(0.5+income_mid)
		cost_over_income_private_bus[i]=30*cost_private_bus[i]/(0.5+income_mid)
		cost_over_income_drive1[i] = 30 * cost_drive1[i]/(0.5+income_mid)
		cost_over_income_share2[i] = 30 * cost_share2[i]/(0.5+income_mid)
		cost_over_income_share3[i] = 30 * cost_share3[i]/(0.5+income_mid)
		cost_over_income_motor[i]=30*cost_motor[i]/(0.5+income_mid)
		cost_over_income_taxi[i]=30*cost_taxi[i]/(0.5+income_mid)
		cost_over_income_SMS[i]=30*cost_SMS[i]/(0.5+income_mid)

		--dbparams.tt_public_ivt(i) =
		--dbparams.tt_public_out(i) =
		tt_public_ivt[i] = dbparams:tt_public_ivt(i)
		tt_public_out[i] = dbparams:tt_public_out(i)

		--dbparams.tt_car_ivt(i) = 
		tt_car_ivt[i] = dbparams:tt_car_ivt(i)

		tt_bus[i] = tt_public_ivt[i]+ tt_public_out[i]
		tt_mrt[i] = tt_public_ivt[i]+ tt_public_out[i]
		tt_rail_SMS[i] = tt_public_ivt[i]+ tt_public_out[i]/6.0
		tt_private_bus[i] = tt_car_ivt[i]
		tt_drive1[i] = tt_car_ivt[i] + 1.0/12
		tt_share2[i] = tt_car_ivt[i] + 1.0/12
		tt_share3[i] = tt_car_ivt[i] + 1.0/12
		tt_motor[i] = tt_car_ivt[i] + 1.0/12
		tt_walk[i] = (d1[i]+d2[i])/5/2
		tt_taxi[i] = tt_car_ivt[i] + 1.0/12
		tt_SMS[i] = tt_car_ivt[i] + 1.0/12


		--dbparams.employment(i) = 
		--dbparams.population(i) = 
		--dbparams.area(i) = 
		--dbparams.shop(i) = 
		employment[i] = dbparams:employment(i)
		population[i] = dbparams:population(i)
		area[i] = dbparams:area(i)
		shop[i] = dbparams:shop(i)
	end

	local V_counter = 0
	local log = math.log

	--utility function for bus 1-24
	for i =1,24 do
		V_counter = V_counter + 1
		utility[V_counter] = beta_cons_bus + cost_bus[i] * beta_cost_bus_mrt_2 + tt_bus[i] * beta_tt_bus_mrt + beta_central_bus_mrt * central_dummy[i] + beta_shop * log(1+shop[i])*shop_stop_dummy+beta_work * log(1+employment[i])*work_stop_dummy + (d1[i]+d2[i]) * beta_distance_bus_mrt + beta_female_bus * female_dummy
	end

	--utility function for mrt 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_mrt + cost_mrt[i] * beta_cost_bus_mrt_2 + tt_mrt[i] * beta_tt_bus_mrt + beta_central_bus_mrt * central_dummy[i] + beta_shop * log(1+shop[i])*shop_stop_dummy+beta_work * log(1+employment[i])*work_stop_dummy + (d1[i]+d2[i]) * beta_distance_bus_mrt + beta_female_mrt * female_dummy
	end

	--utility function for private bus 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_private_bus + cost_private_bus[i] * beta_cost_private_bus_2 + tt_private_bus[i] * beta_tt_private_bus + beta_central_private_bus * central_dummy[i] + beta_shop * log(1+shop[i])*shop_stop_dummy+beta_work * log(1+employment[i])*work_stop_dummy + (d1[i]+d2[i]) * beta_distance_private_bus + beta_female_private_bus * female_dummy
	end

	--utility function for drive1 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_drive1 + first_bound * cost_drive1[i] * beta_cost_drive1_2_first + second_bound * cost_drive1[i] * beta_cost_drive1_2_second + first_bound * tt_drive1[i] * beta_tt_drive1_first + second_bound * tt_drive1[i] * beta_tt_drive1_second + beta_central_drive1 * central_dummy[i] + beta_shop * log(1+shop[i])*shop_stop_dummy+beta_work * log(1+employment[i])*work_stop_dummy + (d1[i]+d2[i]) * beta_distance_drive1 + beta_zero_drive1 *zero_car + beta_oneplus_drive1 * one_plus_car + beta_twoplus_drive1 * two_plus_car + beta_threeplus_drive1 * three_plus_car + beta_female_drive1 * female_dummy
	end

	--utility function for share2 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_share2 + first_bound * cost_share2[i] * beta_cost_drive1_2_first + second_bound * cost_share2[i] * beta_cost_drive1_2_second + first_bound * tt_share2[i] * beta_tt_drive1_first + second_bound * tt_share2[i] * beta_tt_drive1_second + beta_central_share2 * central_dummy[i] + beta_shop * log(1+shop[i])*shop_stop_dummy+beta_work * log(1+employment[i])*work_stop_dummy + (d1[i]+d2[i]) * beta_distance_share2 + beta_zero_share2 *zero_car + beta_oneplus_share2 * one_plus_car + beta_twoplus_share2 * two_plus_car + beta_threeplus_share2 * three_plus_car + beta_female_share2 * female_dummy
	end

	--utility function for share3 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_share3 + first_bound * cost_share3[i] * beta_cost_drive1_2_first + second_bound * cost_share3[i] * beta_cost_drive1_2_second + first_bound * tt_share3[i] * beta_tt_drive1_first + second_bound * tt_share3[i] * beta_tt_drive1_second + beta_central_share3 * central_dummy[i] + beta_shop * log(1+shop[i])*shop_stop_dummy+beta_work * log(1+employment[i])*work_stop_dummy + (d1[i]+d2[i]) * beta_distance_share3 + beta_zero_share3 *zero_car + beta_oneplus_share3 * one_plus_car + beta_twoplus_share3 * two_plus_car + beta_threeplus_share3 * three_plus_car + beta_female_share3 * female_dummy
	end

	--utility function for motor 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_motor + cost_motor[i] * beta_cost_motor_2 + first_bound * tt_motor[i] * beta_tt_drive1_first + second_bound * tt_motor[i] * beta_tt_drive1_second + beta_central_motor * central_dummy[i] + beta_shop * log(1+shop[i])*shop_stop_dummy+beta_work * log(1+employment[i])*work_stop_dummy + (d1[i]+d2[i]) * beta_distance_motor + beta_zero_motor *zero_motor + beta_oneplus_motor * one_plus_motor + beta_twoplus_motor * two_plus_motor + beta_threeplus_motor * three_plus_motor + beta_female_motor * female_dummy
	end

	--utility function for walk 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_walk + tt_walk[i] * beta_tt_walk + beta_central_walk * central_dummy[i] + beta_shop * log(1+shop[i])*shop_stop_dummy+beta_work * log(1+employment[i])*work_stop_dummy + (d1[i]+d2[i]) * beta_distance_walk + beta_female_walk * female_dummy
	end

	--utility function for taxi 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_taxi + first_bound * cost_taxi[i]* beta_cost_drive1_2_first + second_bound * cost_taxi[i]* beta_cost_drive1_2_second + first_bound * tt_taxi[i] * beta_tt_taxi_first + second_bound * tt_taxi[i] * beta_tt_taxi_second + beta_central_taxi * central_dummy[i] + beta_shop * log(1+shop[i])*shop_stop_dummy+beta_work * log(1+employment[i])*work_stop_dummy + (d1[i]+d2[i]) * beta_distance_taxi + beta_female_taxi * female_dummy
	end

	--utility function for SMS 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_SMS + first_bound * cost_SMS[i]* beta_cost_drive1_2_first + second_bound * cost_SMS[i]* beta_cost_drive1_2_second + first_bound * tt_SMS[i] * beta_tt_SMS_first + second_bound * tt_SMS[i] * beta_tt_SMS_second + beta_central_SMS * central_dummy[i] + beta_shop * log(1+shop[i])*shop_stop_dummy+beta_work * log(1+employment[i])*work_stop_dummy + (d1[i]+d2[i]) * beta_distance_SMS + beta_female_SMS * female_dummy
	end
	--utility function for rail_SMS 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_rail_SMS + cost_rail_SMS[i] * beta_cost_rail_SMS_2 + tt_rail_SMS[i] * beta_tt_rail_SMS + beta_central_rail_SMS * central_dummy[i] + beta_shop * log(1+shop[i])*shop_stop_dummy+beta_work * log(1+employment[i])*work_stop_dummy + (d1[i]+d2[i]) * beta_distance_rail_SMS + beta_female_rail_SMS * female_dummy
	end
	
end


--availability
--the logic to determine availability is the same with current implementation
local availability = {}
local function computeAvailabilities(params,dbparams)
	for i = 1, 24*11 do 
		availability[i] = dbparams:availability(i)
	end
end

--scale
local scale={}
for i = 1, 24*11 do
	scale[i]=1
end

-- function to call from C++ preday simulator
-- params and dbparams tables contain data passed from C++
-- to check variable bindings in params or dbparams, refer PredayLuaModel::mapClasses() function in dev/Basic/medium/behavioral/lua/PredayLuaModel.cpp
function choose_imd(params,dbparams)
	computeUtilities(params,dbparams) 
	computeAvailabilities(params,dbparams)
	local probability = calculate_probability("mnl", choice, utility, availability, scale)
	return make_final_choice(probability)
end
