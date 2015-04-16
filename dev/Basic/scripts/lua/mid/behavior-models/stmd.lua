--[[
Model - Mode/destination choice for work-based tour
Type - logit
Authors - Siyu Li, Harish Loganathan, Olga Petrik
]]

-- all require statements do not work with C++. They need to be commented. The order in which lua files are loaded must be explicitly controlled in C++. 
--require "Logit"

--Estimated values for all betas
--Note: the betas that not estimated are fixed to zero.

--!! see the documentation on the definition of AM,PM and OP table!!

local beta_cost_bus_mrt_1= -0.190
local beta_cost_private_bus_1 = -0.696
local beta_cost_drive1_1 = 0
local beta_cost_share2_1= 0
local beta_cost_share3_1= 0
local beta_cost_motor_1 = 0
local beta_cost_taxi_1 = 0

local beta_tt_bus_mrt = -3.78
local beta_tt_private_bus = 0
local beta_tt_drive1 = -4.64
local beta_tt_share2 = -5.45
local beta_tt_share3 = -3.53
local beta_tt_motor = 0
local beta_tt_walk = -0.675
local beta_tt_taxi = 0

local beta_log = 0.775
local beta_area = 0
local beta_population = 0 
local beta_employment = 0

local beta_central_bus_mrt = 0
local beta_central_private_bus = 0
local beta_central_drive1 = 0
local beta_central_share2 = 0
local beta_central_share3 = 0
local beta_central_motor = 0
local beta_central_walk = 0
local beta_central_taxi = 0

local beta_distance_bus_mrt = 0
local beta_distance_private_bus = 0 
local beta_distance_drive1 = 0
local beta_distance_share2 = 0
local beta_distance_share3 = 0
local beta_distance_motor = 0
local beta_distance_walk = 0
local beta_distance_taxi = 0

local beta_cons_bus = 2.82
local beta_cons_mrt = 2.21
local beta_cons_private_bus = 2.44
local beta_cons_drive1 = 0
local beta_cons_share2 = 1.54
local beta_cons_share3 = 0.905
local beta_cons_motor = -5.14
local beta_cons_walk = -1.79
local beta_cons_taxi = -3.39

local beta_female_bus = 0
local beta_female_mrt = 0
local beta_female_private_bus = 0
local beta_female_drive1 = 0
local beta_female_share2 = 0
local beta_female_share3 = 0
local beta_female_motor = 0
local beta_female_taxi = 0
local beta_female_walk = 0

local beta_mode_work_bus = 0
local beta_mode_work_mrt = 0
local beta_mode_work_private_bus = 0
local beta_mode_work_drive1 = 5.67
local beta_mode_work_share2 = 3.39
local beta_mode_work_share3 = 0
local beta_mode_work_motor = 0
local beta_mode_work_walk= 0

--choice set
local choice = {}
for i = 1, 1169*9 do 
	choice[i] = i
end

--utility
-- 1 for public bus; 2 for MRT/LRT; 3 for private bus; 4 for drive1;
-- 5 for shared2; 6 for shared3+; 7 for motor; 8 for walk; 9 for taxi
local utility = {}
local function computeUtilities(params,dbparams)
	local female_dummy = params.female_dummy
	local mode_work_bus = dbparams.mode_to_work == 1 and 1 or 0
	local mode_work_mrt = dbparams.mode_to_work == 2 and 1 or 0
	local mode_work_private_bus = dbparams.mode_to_work == 3 and 1 or 0
	local mode_work_drive1 = dbparams.mode_to_work == 4 and 1 or 0
	local mode_work_share2 = dbparams.mode_to_work == 5 and 1 or 0
	local mode_work_share3 = dbparams.mode_to_work == 6 and 1 or 0
	local mode_work_motor = dbparams.mode_to_work == 7 and 1 or 0
	local mode_work_walk = dbparams.mode_to_work == 8 and 1 or 0

	local cost_bus = {}
	local cost_mrt = {}
	local cost_private_bus = {}
	local cost_drive1 = {}
	local cost_share2 = {}
	local cost_share3 = {}
	local cost_motor = {}
	local cost_taxi={}
	local cost_taxi_1 = {}
	local cost_taxi_2 = {}

	local central_dummy={}

	local tt_bus = {}
	local tt_mrt = {}
	local tt_private_bus = {}
	local tt_drive1 = {}
	local tt_share2 = {}
	local tt_share3 = {}
	local tt_motor = {}
	local tt_walk = {}
	local tt_taxi = {}
	local tt_car_ivt = {}
	local tt_public_ivt = {}
	local tt_public_out = {}

	local employment = {}
	local population = {}
	local area = {}
	local shop = {}

	local d1 = {}
	local d2 = {}

	--for each area
	for i =1,1169 do
		cost_bus[i] = dbparams:cost_public_first(i) + dbparams:cost_public_second(i)
		cost_mrt[i] = cost_bus[i]
		cost_private_bus[i] = cost_bus[i]

		cost_drive1[i] = dbparams:cost_car_ERP_first(i)+dbparams:cost_car_ERP_second(i)+dbparams:cost_car_OP_first(i)+dbparams:cost_car_OP_second(i)+dbparams:cost_car_parking(i)
		cost_share2[i] = dbparams:cost_car_ERP_first(i)+dbparams:cost_car_ERP_second(i)+dbparams:cost_car_OP_first(i)+dbparams:cost_car_OP_second(i)+dbparams:cost_car_parking(i)/2
		cost_share3[i] = dbparams:cost_car_ERP_first(i)+dbparams:cost_car_ERP_second(i)+dbparams:cost_car_OP_first(i)+dbparams:cost_car_OP_second(i)+dbparams:cost_car_parking(i)/3
		cost_motor[i] = 0.5*(dbparams:cost_car_ERP_first(i)+dbparams:cost_car_ERP_second(i)+dbparams:cost_car_OP_first(i)+dbparams:cost_car_OP_second(i))+0.65*dbparams:cost_car_parking(i)
		
		central_dummy[i] = dbparams:central_dummy(i)
		d1[i] = dbparams:walk_distance1(i)
		d2[i] = dbparams:walk_distance2(i)

		cost_taxi_1[i] = 3.4+((d1[i]*(d1[i]>10 and 1 or 0)-10*(d1[i]>10 and 1 or 0))/0.35+(d1[i]*(d1[i]<=10 and 1 or 0)+10*(d1[i]>10 and 1 or 0))/0.4)*0.22+ dbparams:cost_car_ERP_first(i) + central_dummy[i]*3
		cost_taxi_2[i] = 3.4+((d2[i]*(d2[i]>10 and 1 or 0)-10*(d2[i]>10 and 1 or 0))/0.35+(d2[i]*(d2[i]<=10 and 1 or 0)+10*(d2[i]>10 and 1 or 0))/0.4)*0.22+ dbparams:cost_car_ERP_second(i) + central_dummy[i]*3
		cost_taxi[i] = cost_taxi_1[i] + cost_taxi_2[i]

		tt_car_ivt[i] = dbparams:tt_car_ivt_first(i) + dbparams:tt_car_ivt_second(i)
		tt_public_ivt[i] = dbparams:tt_public_ivt_first(i) + dbparams:tt_public_ivt_second(i)
		tt_public_out[i] = dbparams:tt_public_out_first(i) + dbparams:tt_public_out_second(i)

		tt_bus[i] = tt_public_ivt[i]+ tt_public_out[i]
		tt_mrt[i] = tt_public_ivt[i]+ tt_public_out[i]
		tt_private_bus[i] = tt_car_ivt[i]
		tt_drive1[i] = tt_car_ivt[i] + 1.0/6
		tt_share2[i] = tt_car_ivt[i] + 1.0/6
		tt_share3[i] = tt_car_ivt[i] + 1.0/6
		tt_motor[i] = tt_car_ivt[i] + 1.0/6
		tt_walk[i] = (d1[i]+d2[i])/5
		tt_taxi[i] = tt_car_ivt[i] + 1.0/6

		employment[i] = dbparams:employment(i)
		population[i] = dbparams:population(i)
		area[i] = dbparams:area(i)
		shop[i] = dbparams:shop(i)
	end

	local exp = math.exp
	local log = math.log

	local V_counter = 0

	--utility function for bus 1-1169
	for i =1,1169 do
		V_counter = V_counter + 1
		utility[V_counter] = beta_cons_bus + cost_bus[i] * beta_cost_bus_mrt_1 + tt_bus[i] * beta_tt_bus_mrt + beta_central_bus_mrt * central_dummy[i] + beta_log * log(shop[i]+exp(beta_employment)*employment[i]) + (d1[i]+d2[i]) * beta_distance_bus_mrt + beta_female_bus * female_dummy + beta_mode_work_bus * mode_work_bus
	end

	--utility function for mrt 1-1169
	for i=1,1169 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_mrt + cost_mrt[i] * beta_cost_bus_mrt_1 + tt_mrt[i] * beta_tt_bus_mrt + beta_central_bus_mrt * central_dummy[i] + beta_log * log(shop[i]+exp(beta_employment)*employment[i]) + (d1[i]+d2[i]) * beta_distance_bus_mrt + beta_female_mrt * female_dummy + beta_mode_work_bus * mode_work_mrt
	end

	--utility function for private bus 1-1169
	for i=1,1169 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_private_bus + cost_private_bus[i] * beta_cost_private_bus_1 + tt_private_bus[i] * beta_tt_bus_mrt + beta_central_private_bus * central_dummy[i] + beta_log * log(shop[i]+exp(beta_employment)*employment[i]) + (d1[i]+d2[i]) * beta_distance_private_bus + beta_female_private_bus * female_dummy + beta_mode_work_bus * mode_work_private_bus
	end

	--utility function for drive1 1-1169
	for i=1,1169 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_drive1 + cost_drive1[i] * beta_cost_drive1_1 + tt_drive1[i] * beta_tt_drive1 + beta_central_drive1 * central_dummy[i] + beta_log * log(shop[i]+exp(beta_employment)*employment[i]) + (d1[i]+d2[i]) * beta_distance_drive1 + beta_female_drive1 * female_dummy + beta_mode_work_drive1 * mode_work_drive1 
	end

	--utility function for share2 1-1169
	for i=1,1169 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_share2 + cost_share2[i] *beta_cost_drive1_1 + tt_share2[i] * beta_tt_share2 + beta_central_share2 * central_dummy[i] + beta_log * log(shop[i]+exp(beta_employment)*employment[i]) + (d1[i]+d2[i]) * beta_distance_share2 + beta_female_share2 * female_dummy + beta_mode_work_share2 * mode_work_share2
	end

	--utility function for share3 1-1169
	for i=1,1169 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_share3 + cost_share3[i] * beta_cost_drive1_1 + tt_share3[i] * beta_tt_share3 + beta_central_share3 * central_dummy[i] + beta_log * log(shop[i]+exp(beta_employment)*employment[i]) + (d1[i]+d2[i]) * beta_distance_share3 + beta_female_share3 * female_dummy + beta_mode_work_share2 * mode_work_share3
	end

	--utility function for motor 1-1169
	for i=1,1169 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_motor + cost_motor[i] * beta_cost_drive1_1 + tt_motor[i] * beta_tt_motor + beta_central_motor * central_dummy[i] + beta_log * log(shop[i]+exp(beta_employment)*employment[i]) + (d1[i]+d2[i]) * beta_distance_motor + beta_female_motor * female_dummy + beta_mode_work_drive1 * mode_work_motor
	end

	--utility function for walk 1-1169
	for i=1,1169 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_walk + tt_walk[i] * beta_tt_walk + beta_central_walk * central_dummy[i] + beta_log * log(shop[i]+exp(beta_employment)*employment[i]) + (d1[i]+d2[i]) * beta_distance_walk + beta_female_walk * female_dummy + beta_mode_work_walk * mode_work_walk
	end

	--utility function for taxi 1-1169
	for i=1,1169 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_taxi + cost_taxi[i] * beta_cost_drive1_1 + tt_taxi[i] * beta_tt_taxi + beta_central_taxi * central_dummy[i] + beta_log * log(shop[i]+exp(beta_employment)*employment[i]) + (d1[i]+d2[i]) * beta_distance_taxi + beta_female_taxi * female_dummy
	end
end


--availability
--the logic to determine availability is the same with current implementation
local availability = {}
local function computeAvailabilities(params,dbparams)
	for i = 1, 1169*9 do 
		availability[i] = dbparams:availability(i)
	end
end

--scale
local scale = 1 -- for all choices

function choose_stmd(params,dbparams)
	computeUtilities(params,dbparams) 
	computeAvailabilities(params,dbparams)
	local probability = calculate_probability("mnl", choice, utility, availability, scale)
	return make_final_choice(probability)
end
