--[[
Model - Mode/destination choice for other tour
Type - logit
Authors - Siyu Li, Harish Loganathan
]]

-- all require statements do not work with C++. They need to be commented. The order in which lua files are loaded must be explicitly controlled in C++. 
--require "Logit"

--Estimated values for all betas
--Note: the betas that not estimated are fixed to zero.

--!! see the documentation on the definition of AM,PM and OP table!!

local beta_cost_bus_mrt_1 = - 0.00155
local beta_cost_private_bus_1 = -0.00291
local beta_cost_drive1_1 = -0.00164
local beta_cost_share2_1 = -0.00225
local beta_cost_share3_1 = -0.00267
local beta_cost_motor_1 = -0.00301
local beta_cost_taxi_1 = -0.0000912

local beta_cost_bus_mrt_2 = 0
local beta_cost_private_bus_2 = 0
local beta_cost_drive1_2 = 0
local beta_cost_share2_2 = 0
local beta_cost_share3_2 = 0
local beta_cost_motor_2 = 0
local beta_cost_taxi_2 = 0

local beta_tt_bus_mrt = -2.58
local beta_tt_private_bus =  -0.0219
local beta_tt_drive1 =  -2.09
local beta_tt_share2 = -1.68 
local beta_tt_share3 = -1.96
local beta_tt_motor = -0.317
local beta_tt_walk = -3.56
local beta_tt_taxi = -1.63

local beta_log = 0.212
local beta_area = 0
local beta_population = -5.87

local beta_central_bus_mrt = 0.844
local beta_central_private_bus = 0.759
local beta_central_drive1 = 0
local beta_central_share2 = 0.947
local beta_central_share3 = 0.756
local beta_central_motor = 0.682
local beta_central_walk = 1.24
local beta_central_taxi = 1.45

local beta_distance_bus_mrt = 0.0424
local beta_distance_private_bus = -0.0287
local beta_distance_drive1 = 0
local beta_distance_share2 = -0.0406
local beta_distance_share3 = -0.0198
local beta_distance_motor = -0.0322
local beta_distance_walk = 0
local beta_distance_taxi = -0.0334


local beta_cons_bus = -1.05
local beta_cons_mrt = -1.29
local beta_cons_private_bus = -5.4
local beta_cons_drive1 = 0
local beta_cons_share2 = -3.17
local beta_cons_share3 = -3.34
local beta_cons_motor = -4.25
local beta_cons_walk = -1.13
local beta_cons_taxi = -3.88

local beta_zero_drive1 = 0
local beta_oneplus_drive1 = 0
local beta_twoplus_drive1 = 1.3
local beta_threeplus_drive1 = -0.477

local beta_zero_share2 = 0
local beta_oneplus_share2 = 2.65
local beta_twoplus_share2 = 0.967
local beta_threeplus_share2 = -0.505

local beta_zero_share3 = 0
local beta_oneplus_share3 = 2.5
local beta_twoplus_share3 = 0.604
local beta_threeplus_share3 = -0.217


local beta_zero_motor = 0
local beta_oneplus_motor = 0
local beta_twoplus_motor = 2.91
local beta_threeplus_motor = 1.75

local beta_female_bus = 0.566
local beta_female_mrt = 0.578
local beta_female_private_bus = 0.754
local beta_female_drive1 = 0
local beta_female_share2 = 0.187
local beta_female_share3 = 0.386
local beta_female_motor = -3.81
local beta_female_taxi = 0.874
local beta_female_walk = 1.71


--choice set
local choice = {}
for i = 1, 1092*9 do 
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
	local income_mode = income_cat[income_id]
	local missing_income = (params.income_id >= 12) and 1 or 0

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


	local cost_public_first = {}
	local cost_public_second = {}
	local cost_bus = {}
	local cost_mrt = {}
	local cost_private_bus = {}

	local cost_car_OP_first = {}
	local cost_car_OP_second = {}
	local cost_car_ERP_first = {}
	local cost_car_ERP_second = {}
	local cost_car_parking = {}
	local cost_drive1 = {}
	local cost_share2 = {}
	local cost_share3 = {}
	local cost_motor = {}
	
	local cost_taxi_1 = {}
	local cost_taxi_2 = {}
	local cost_taxi={}

	local d1={}
	local d2={}
	local central_dummy={}

	local cost_over_income_bus = {}
	local cost_over_income_mrt = {}
	local cost_over_income_private_bus = {}
	local cost_over_income_drive1 = {}
	local cost_over_income_share2 = {}
	local cost_over_income_share3 = {}
	local cost_over_income_motor = {}
	local cost_over_income_taxi = {}

	local tt_public_ivt_first = {}
	local tt_public_ivt_second = {}
	local tt_public_out_first = {}
	local tt_public_out_second = {}
	local tt_car_ivt_first = {}
	local tt_car_ivt_second = {}

	local tt_bus = {}
	local tt_mrt = {}
	local tt_private_bus = {}
	local tt_drive1 = {}
	local tt_share2 = {}
	local tt_share3 = {}
	local tt_motor = {}
	local tt_walk = {}
	local tt_taxi = {}

	local average_transfer_number = {}

	local employment = {}
	local population = {}
	local area = {}
	local shop = {}

	for i =1,1092 do
		--dbparams.cost_public_first[i] = AM[(origin,destination[i])]['pub_cost']
		--dbparams.cost_public_second[i] = PM[(destination[i],origin)]['pub_cost']
		--origin is home, destination[i] is zone from 1 to 1092
		--0 if origin == destination
		cost_public_first[i] = dbparams.cost_public_first[i]
		cost_public_second[i] = dbparams.cost_public_second[i]
		cost_bus[i] = cost_public_first[i] + cost_public_second[i]
		cost_mrt[i] = cost_public_first[i] + cost_public_second[i]
		cost_private_bus[i] = cost_public_first[i] + cost_public_second[i]

		--dbparams.cost_car_ERP_first[i] = AM[(origin,destination[i])]['car_cost_erp']
		--dbparams.cost_car_ERP_second[i] = PM[(destination[i],origin)]['car_cost_erp']
		--dbparams.cost_car_OP_first[i] = AM[(origin,destination[i])]['distance']*0.147
		--dbparams.cost_car_OP_second[i] = PM[(destination[i],origin)]['distance']*0.147
		--dbparams.cost_car_parking[i] = 8 * ZONE[destination[i]]['parking_rate']
		--for the above 5 variables, origin is home, destination[i] is tour destination from 1 to 1092
		--0 if origin == destination
		cost_drive1[i] = cost_car_ERP_first[i]+cost_car_ERP_second[i]+cost_car_OP_first[i]+cost_car_OP_second[i]+cost_car_parking[i]
		cost_share2[i] = cost_car_ERP_first[i]+cost_car_ERP_second[i]+cost_car_OP_first[i]+cost_car_OP_second[i]+cost_car_parking[i]/2
		cost_share3[i] = cost_car_ERP_first[i]+cost_car_ERP_second[i]+cost_car_OP_first[i]+cost_car_OP_second[i]+cost_car_parking[i]/3
		cost_motor[i] = 0.5*(cost_car_ERP_first[i]+cost_car_ERP_second[i]+cost_car_OP_first[i]+cost_car_OP_second[i])+0.65*cost_car_parking[i]

		--dbparams.walk_distance1[i]= AM[(origin,destination[i])]['AM2dis']
		--dbparams.walk_distance2= PM[(destination[i],origin)]['PM2dis']
		--dbparams.central_dummy[i]=ZONE[destination[i]]['central_dummy']
		--origin is home mtz, destination[i] is zone from 1 to 1092
		--0 if origin == destination
		d1[i] = dbparams.walk_distance1[i]
		d2[i] = dbparams.walk_distance2[i]
		central_dummy[i] = dbparams.central_dummy[i]

		cost_taxi_1[i] = 3.4+((d1[i]*(d1[i]>10 and 1 or 0)-10*(d1[i]>10 and 1 or 0))/0.35+(d1[i]*(d1[i]<=10 and 1 or 0)+10*(d1[i]>10 and 1 or 0))/0.4)*0.22+ cost_car_ERP_first[i] + central_dummy[i]*3
		cost_taxi_2[i] = 3.4+((d2[i]*(d2[i]>10 and 1 or 0)-10*(d2[i]>10 and 1 or 0))/0.35+(d2[i]*(d2[i]<=10 and 1 or 0)+10*(d2[i]>10 and 1 or 0))/0.4)*0.22+ cost_car_ERP_second[i] + central_dummy[i]*3
		cost_taxi[i] = cost_taxi_1[i] + cost_taxi_2[i]

		cost_over_income_bus[i]=30*cost_bus[i]/(0.5+income_mid)
		cost_over_income_mrt[i]=30*cost_mrt[i]/(0.5+income_mid)
		cost_over_income_private_bus[i]=30*cost_private_bus[i]/(0.5+income_mid)
		cost_over_income_drive1[i] = 30 * cost_drive1[i]/(0.5+income_mid)
		cost_over_income_share2[i] = 30 * cost_share2[i]/(0.5+income_mid)
		cost_over_income_share3[i] = 30 * cost_share3[i]/(0.5+income_mid)
		cost_over_income_motor[i]=30*cost_motor[i]/(0.5+income_mid)
		cost_over_income_taxi[i]=30*cost_taxi[i]/(0.5+income_mid)

		--dbparams.tt_public_ivt_first[i] = AM[(origin,destination[i])]['pub_ivt']
		--dbparams.tt_public_ivt_second[i] = PM[(destination[i],origin)]['pub_ivt']
		--dbparams.tt_public_out_first[i] = AM[(origin,destination[i])]['pub_out']
		--dbparams.tt_public_out_second[i] = PM[(destination[i],origin)]['pub_out']
		--for the above 4 variables, origin is home, destination[i] is zone from 1 to 1092
		--0 if origin == destination
		tt_public_ivt_first[i] = dbparams.tt_public_ivt_first[i]
		tt_public_ivt_second[i] = dbparams.tt_public_ivt_second[i]
		tt_public_out_first[i] = dbparams.tt_public_out_first[i]
		tt_public_out_second[i] = dbparams.tt_public_out_second[i]

		--dbparams.tt_car_ivt_first[i] = AM[(origin,destination[i])]['car_ivt']
		--dbparams.tt_car_ivt_second[i] = PM[(destination[i],origin)]['car_ivt']
		tt_car_ivt_first[i] = dbparams.tt_car_ivt_first[i]
		tt_car_ivt_second[i] = dpparams.tt_car_ivt_second[i]

		tt_bus[i] = tt_public_ivt_first[i]+ tt_public_ivt_second[i]+tt_public_out_first[i]+tt_public_out_second[i]
		tt_mrt[i] = tt_public_ivt_first[i]+ tt_public_ivt_second[i]+tt_public_out_first[i]+tt_public_out_second[i]
		tt_private_bus[i] = tt_car_ivt_first[i] + tt_car_ivt_second[i]
		tt_drive1[i] = tt_car_ivt_first[i] + tt_car_ivt_second[i] + 1.0/6
		tt_share2[i] = tt_car_ivt_first[i] + tt_car_ivt_second[i] + 1.0/6
		tt_share3[i] = tt_car_ivt_first[i] + tt_car_ivt_second[i] + 1.0/6
		tt_motor[i] = tt_car_ivt_first[i] + tt_car_ivt_second[i] + 1.0/6
		tt_walk[i] = (d1[i]+d2[i])/5
		tt_taxi[i] = tt_car_ivt_first[i] + tt_car_ivt_second[i] + 1.0/6

		--dbparams.average_transfer_number[i] = (AM[(origin,destination[i])]['avg_transfer'] + PM[(destination[i],origin)]['avg_transfer'])/2
		--origin is home, destination[i] is zone from 1 to 1092
		-- 0 if origin == destination
		average_transfer_number[i] = dpparams.average_transfer_number[i]

		--dbparams.employment[i] = ZONE[i]['employment']
		--dbparams.population[i] = ZONE[i]['population']
		--dbparams.area[i] = ZONE[i]['area']
		--dbparams.shop[i] = ZONE[i]['shop']
		employment[i] = dbparams.employment[i]
		population[i] = dbparams.population[i]
		area[i] = dbparams.area[i]
		shop[i] = dbparams.shop[i]
	end

	local V_counter = 0

	--utility function for bus 1-1092
	for i =1,1092 do
		V_counter = V_counter + 1
		utility[V_counter] = beta_cons_bus + cost_over_income_bus[i] * (1- missing_income) * beta_cost_bus_mrt_1 + cost_bus[i] * missing_income * beta_cost_bus_mrt_2 + tt_bus[i] * beta_tt_bus_mrt + beta_central_bus_mrt * central_dummy[i] + beta_log * math.log(math.exp(beta_area)*area[i]+math.exp(beta_population)*population[i]) + (d1[i]+d2[i]) * beta_distance_bus_mrt + beta_female_bus * female_dummy
	end

	--utility function for mrt 1-1092
	for i=1,1092 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_mrt + cost_over_income_mrt[i] * (1- missing_income) * beta_cost_bus_mrt_1 + cost_mrt[i] * missing_income * beta_cost_bus_mrt_2 + tt_mrt[i] * beta_tt_bus_mrt + beta_central_bus_mrt * central_dummy[i] + beta_log * math.log(math.exp(beta_area)*area[i]+math.exp(beta_population)*population[i]) + (d1[i]+d2[i]) * beta_distance_bus_mrt + beta_female_mrt * female_dummy
	end

	--utility function for private bus 1-1092
	for i=1,1092 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_private_bus + cost_over_income_private_bus[i] * (1- missing_income) * beta_cost_private_bus_1 + cost_private_bus[i] * missing_income * beta_cost_private_bus_2 + tt_private_bus[i] * beta_tt_private_bus + beta_central_private_bus * central_dummy[i] + beta_log * math.log(math.exp(beta_area)*area[i]+math.exp(beta_population)*population[i]) + (d1[i]+d2[i]) * beta_distance_private_bus + beta_female_private_bus * female_dummy
	end

	--utility function for drive1 1-1092
	for i=1,1092 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_drive1 + cost_over_income_drive1[i] * (1 - missing_income) * beta_cost_drive1_1 + cost_drive1[i] * missing_income * beta_cost_drive1_2 + tt_drive1[i] * beta_tt_drive1 + beta_central_drive1 * central_dummy[i] + beta_log * math.log(math.exp(beta_area)*area[i]+math.exp(beta_population)*population[i]) + (d1[i]+d2[i]) * beta_distance_drive1 + beta_zero_drive1 *zero_car + beta_oneplus_drive1 * one_plus_car + beta_twoplus_drive1 * two_plus_car + beta_threeplus_drive1 * three_plus_car + beta_female_drive1 * female_dummy
	end

	--utility function for share2 1-1092
	for i=1,1092 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_share2 + cost_over_income_share2[i] * (1 - missing_income) * beta_cost_share2_1 + cost_share2[i] * missing_income * beta_cost_share2_2 + tt_share2[i] * beta_tt_share2 + beta_central_share2 * central_dummy[i] + beta_log * math.log(math.exp(beta_area)*area[i]+math.exp(beta_population)*population[i]) + (d1[i]+d2[i]) * beta_distance_share2 + beta_zero_share2 *zero_car + beta_oneplus_share2 * one_plus_car + beta_twoplus_share2 * two_plus_car + beta_threeplus_share2 * three_plus_car + beta_female_share2 * female_dummy
	end

	--utility function for share3 1-1092
	for i=1,1092 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_share3 + cost_over_income_share3[i] * (1 - missing_income) * beta_cost_share3_1 + cost_share3[i] * missing_income * beta_cost_share3_2 + tt_share3[i] * beta_tt_share3 + beta_central_share3 * central_dummy[i] + beta_log * math.log(math.exp(beta_area)*area[i]+math.exp(beta_population)*population[i]) + (d1[i]+d2[i]) * beta_distance_share3 + beta_zero_share3 *zero_car + beta_oneplus_share3 * one_plus_car + beta_twoplus_share3 * two_plus_car + beta_threeplus_share3 * three_plus_car + beta_female_share3 * female_dummy
	end

	--utility function for motor 1-1092
	for i=1,1092 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_motor + cost_over_income_motor[i] * (1 - missing_income) * beta_cost_motor_1 + cost_motor[i] * missing_income * beta_cost_motor_2 + tt_motor[i] * beta_tt_motor + beta_central_motor * central_dummy[i] + beta_log * math.log(math.exp(beta_area)*area[i]+math.exp(beta_population)*population[i]) + (d1[i]+d2[i]) * beta_distance_motor + beta_zero_motor *zero_motor + beta_oneplus_motor * one_plus_motor + beta_twoplus_motor * two_plus_motor + beta_threeplus_motor * three_plus_motor + beta_female_motor * female_dummy
	end

	--utility function for walk 1-1092
	for i=1,1092 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_walk + tt_walk[i] * beta_tt_walk + beta_central_walk * central_dummy[i] + beta_log * math.log(math.exp(beta_area)*area[i]+math.exp(beta_population)*population[i]) + (d1[i]+d2[i]) * beta_distance_walk + beta_female_walk * female_dummy
	end

	--utility function for taxi 1-1092
	for i=1,1092 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_taxi + cost_over_income_taxi[i] * (1-missing_income)* beta_cost_taxi_1 + cost_taxi[i]*missing_income * beta_cost_taxi_2 + tt_taxi[i] * beta_tt_taxi + beta_central_taxi * central_dummy[i] + beta_log * math.log(math.exp(beta_area)*area[i]+math.exp(beta_population)*population[i]) + (d1[i]+d2[i]) * beta_distance_taxi + beta_female_taxi * female_dummy
	end

end


--availability
--the logic to determine availability is the same with current implementation
local availability = {}
local function computeAvailabilities(params,dbparams)
	for i = 1, 1092*9 do 
	availability[i] = dbparams.availability[i]
end
}
end

--scale
local scale={}
for i = 1, 1092*9 do
	scale[i]=1
end

function choose_tmdo(params,dbparams)
	computeUtilities(params,dbparams) 
	computeAvailabilities(params,dbparams)
	local probability = calculate_probability("mnl", choice, utility, availability, scale)
	return make_final_choice(probability)
end
