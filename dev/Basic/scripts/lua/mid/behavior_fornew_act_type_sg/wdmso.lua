--[[
Model - Mode/destination choice for shopping tour
Type - logit
Authors - Siyu Li, Harish Loganathan
]]

-- all require statements do not work with C++. They need to be commented. The order in which lua files are loaded must be explicitly controlled in C++. 
--require "Logit"

--Estimated values for all betas
--Note: the betas that not estimated are fixed to zero.

--!! see the documentation on the definition of AM,PM and OP table!!

local beta_cost_bus_mrt_1= 0
local beta_cost_private_bus_1 = 0
local beta_cost_drive1_1 = 0
local beta_cost_share2_1= 0
local beta_cost_share3_1= 0
local beta_cost_motor_1 = 0
local beta_cost_taxi_1 = 0

local beta_cost_bus_mrt_2 = -0.294
local beta_cost_private_bus_2 = -1.29
local beta_cost_drive1_2 = -0.371
local beta_cost_share2_2 = -0.233
local beta_cost_share3_2 = 0
local beta_cost_motor_2 = -0.150
local beta_cost_taxi_2 = -0.113

local beta_tt_bus_mrt = -2.78
local beta_tt_private_bus = 0 
local beta_tt_drive1 = -4.65
local beta_tt_share2= -3.59
local beta_tt_share3= -2.14
local beta_tt_motor = 0
local beta_tt_walk = -5.05
local beta_tt_taxi = -3.33

local beta_log = 1.14
local beta_area = 3.41
local beta_population = -7.24

local beta_central_bus_mrt = -0.178
local beta_central_private_bus= -1.13
local beta_central_drive1 = 0 
local beta_central_share2 = 0.325
local beta_central_share3 = 0.103
local beta_central_motor = 0.0738
local beta_central_walk = 1.12
local beta_central_taxi = 1.92

local beta_distance_bus_mrt = 0.00911
local beta_distance_private_bus = 0.0808
local beta_distance_drive1 = 0
local beta_distance_share2 = -0.0413
local beta_distance_share3 = -0.0220
local beta_distance_motor = -0.0635
local beta_distance_walk = 0
local beta_distance_taxi = 0


local beta_cons_bus = -1.204
local beta_cons_mrt = -2.146
local beta_cons_private_bus = -3.649
local beta_cons_drive1 = 0
local beta_cons_share2 = -3.28
local beta_cons_share3 = -4.28
local beta_cons_motor = -2.538
local beta_cons_walk = -6.77
local beta_cons_taxi = -5.16

local beta_zero_bus = 0
local beta_oneplus_bus = 0
local beta_twoplus_bus = -0.512
local beta_threeplus_bus = 0

local beta_zero_mrt= 0
local beta_oneplus_mrt = 0.648
local beta_twoplus_mrt = -2.55
local beta_threeplus_mrt = 0

local beta_zero_privatebus = 0
local beta_oneplus_privatebus = 0.0862
local beta_twoplus_privatebus = 0
local beta_threeplus_privatebus = 0

local beta_zero_drive1 = 0
local beta_oneplus_drive1 = 0
local beta_twoplus_drive1 = 0.476
local beta_threeplus_drive1 = 0

local beta_zero_share2 = 0
local beta_oneplus_share2 = 3.65
local beta_twoplus_share2 = 0
local beta_threeplus_share2 = 0

local beta_zero_share3 = 0
local beta_oneplus_share3 = 2.82
local beta_twoplus_share3 = -0.735
local beta_threeplus_share3 = 0

local beta_zero_car_motor = 0
local beta_oneplus_car_motor = 0
local beta_twoplus_car_motor = 0
local beta_threeplus_car_motor = 0

local beta_zero_walk = 0
local beta_oneplus_walk = -0.0563
local beta_twoplus_walk = 0
local beta_threeplus_walk = 0

local beta_zero_taxi = 0
local beta_oneplus_taxi = -0.0655
local beta_twoplus_taxi = 0
local beta_threeplus_taxi = 0

local beta_zero_motor = 0
local beta_oneplus_motor = 0
local beta_twoplus_motor= 4.89
local beta_threeplus_motor= 0 

local beta_female_bus = 0.0894
local beta_female_mrt = 0
local beta_female_private_bus = -0.437
local beta_female_drive1 = 0
local beta_female_share2 = -0.669
local beta_female_share3 = -0.690
local beta_female_motor = -2.60
local beta_female_taxi = 0.404
local beta_female_walk = 1.83


--choice set
local choice = {}
for i = 1, 9 do 
	choice[i] = i
end

--utility
-- 1 for public bus; 2 for MRT/LRT; 3 for private bus; 4 for drive1;
-- 5 for shared2; 6 for shared3+; 7 for motor; 8 for walk; 9 for taxi

local utility = {}
local function computeUtilities(pparams,mparams)
	local cost_increase = 0
	local female_dummy = pparams.female_dummy
	local income_id = pparams.income_id
	local income_cat = {500,1250,1750,2250,2750,3500,4500,5500,6500,7500,8500,0,99999,99999}
	local income_mid = income_cat[income_id]
	local missing_income = (pparams.income_id >= 13) and 1 or 0

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

	local distance_remained = mparams.distance_remaining	
	local cost_public = 0.77 + distance_remained * 0

	local cost_bus=cost_public
	local cost_mrt=cost_public
	local cost_privatebus=cost_public
	
	local cost_car_voc = distance_remained * 0.147 
	
	local cost_car_parking = 8 * mparams.parking_rate  -- for final destination of the trip

	local cost_cardriver= cost_car_voc+cost_car_parking
	local cost_carpassenger= cost_car_voc+cost_car_parking
	local cost_motor=0.5*(cost_car_voc)+0.65*cost_car_parking

	local d1 = distance_remained

	local central_dummy = mparams.central_dummy

	local cost_taxi=3.4+((d1*(d1>10 and 1 or 0)-10*(d1>10 and 1 or 0))/0.35+(d1*(d1<=10 and 1 or 0)+10*(d1>10 and 1 or 0))/0.4)*0.22+ central_dummy*3

	local cost_over_income_bus=30*cost_bus/(0.5+income_mid)
	local cost_over_income_mrt=30*cost_mrt/(0.5+income_mid)
	local cost_over_income_privatebus=30*cost_privatebus/(0.5+income_mid)
	local cost_over_income_cardriver=30*cost_cardriver/(0.5+income_mid)
	local cost_over_income_carpassenger=30*cost_carpassenger/(0.5+income_mid)
	local cost_over_income_motor=30*cost_motor/(0.5+income_mid)
	local cost_over_income_taxi=30*cost_taxi/(0.5+income_mid)

	local tt_public_ivt = mparams.tt_public_ivt / 3600.0
	local tt_public_waiting = mparams.tt_public_waiting / 3600.0
	local tt_public_walk = mparams.tt_public_walk / 3600.0
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


	local average_transfer_number = mparams.average_transfer_number

	local population = mparams.population
	local area = mparams.destination_area
	local shop = mparams.shop


	utility[1] = beta_cons_bus + cost_over_income_bus * (1- missing_income) * beta_cost_bus_mrt_1 + cost_bus * beta_cost_bus_mrt_2 + tt_bus_all * beta_tt_bus_mrt + beta_central_bus_mrt * central_dummy + beta_log * math.log(shop + math.exp(beta_area)*area + math.exp(beta_population)*population) + d1 * beta_distance_bus_mrt + beta_female_bus * female_dummy + beta_zero_bus* zero_car + beta_oneplus_bus* one_plus_car+ beta_twoplus_bus* two_plus_car

	utility[2] = beta_cons_mrt + cost_over_income_mrt * (1- missing_income) * beta_cost_bus_mrt_1 + cost_mrt * beta_cost_bus_mrt_2 + tt_mrt_all * beta_tt_bus_mrt + beta_central_bus_mrt * central_dummy + beta_log * math.log(shop + math.exp(beta_area)*area + math.exp(beta_population)*population) + d1 * beta_distance_bus_mrt + beta_female_mrt * female_dummy + beta_zero_mrt*zero_car+ beta_oneplus_mrt*one_plus_car+beta_twoplus_mrt*two_plus_car

	utility[3] = beta_cons_private_bus + cost_over_income_privatebus * (1- missing_income) * beta_cost_private_bus_2 + cost_privatebus * beta_cost_private_bus_2 + tt_privatebus_all * beta_tt_bus_mrt + beta_central_private_bus * central_dummy + beta_log * math.log(shop +math.exp(beta_area)*area + math.exp(beta_population)*population) + d1 * beta_distance_private_bus + beta_female_private_bus * female_dummy + beta_zero_privatebus+beta_oneplus_privatebus*one_plus_car+beta_twoplus_privatebus*two_plus_car

	utility[4] = beta_cons_drive1 + cost_over_income_cardriver * (1 - missing_income) * beta_cost_drive1_1 + cost_cardriver * beta_cost_drive1_2 + tt_cardriver_all * beta_tt_drive1 + beta_central_drive1 * central_dummy + beta_log * math.log(shop+math.exp(beta_area)*area+math.exp(beta_population)*population) + (d1) * beta_distance_drive1 + beta_zero_drive1 *zero_car + beta_oneplus_drive1 * one_plus_car + beta_twoplus_drive1 * two_plus_car + beta_threeplus_drive1 * three_plus_car + beta_female_drive1 * female_dummy
	
	utility[5] = beta_cons_share2 + cost_over_income_carpassenger * (1 - missing_income) * beta_cost_share2_1 + cost_carpassenger * beta_cost_share2_2 + tt_carpassenger_all * beta_tt_share2 + beta_central_share2 * central_dummy + beta_log * math.log(shop+math.exp(beta_area)*area+math.exp(beta_population)*population) + (d1) * beta_distance_share2 + beta_zero_share2 *zero_car + beta_oneplus_share2 * one_plus_car + beta_twoplus_share2 * two_plus_car + beta_threeplus_share2 * three_plus_car + beta_female_share2 * female_dummy

	utility[6] = beta_cons_share3 + cost_over_income_carpassenger * (1 - missing_income) * beta_cost_share3_1 + cost_carpassenger * beta_cost_share2_2 + tt_carpassenger_all * beta_tt_share3 + beta_central_share3 * central_dummy + beta_log * math.log(shop+math.exp(beta_area)*area+math.exp(beta_population)*population) + (d1) * beta_distance_share3 + beta_zero_share3 *zero_car + beta_oneplus_share3 * one_plus_car + beta_twoplus_share3 * two_plus_car + beta_threeplus_share3 * three_plus_car + beta_female_share3 * female_dummy

	utility[7] = beta_cons_motor + cost_over_income_motor * (1 - missing_income) * beta_cost_motor_1 + cost_motor * beta_cost_motor_2 + tt_motor_all * beta_tt_drive1 + beta_central_motor * central_dummy + beta_log * math.log(shop+math.exp(beta_area)*area+math.exp(beta_population)*population) + (d1) * beta_distance_motor + beta_zero_motor *zero_motor + beta_oneplus_motor * one_plus_motor + beta_twoplus_motor * two_plus_motor + beta_threeplus_motor * three_plus_motor + beta_female_motor * female_dummy + beta_zero_car_motor*zero_car+beta_oneplus_car_motor*one_plus_car+ beta_twoplus_car_motor*two_plus_car

	utility[8] = beta_cons_walk + tt_walk * beta_tt_walk + beta_central_walk * central_dummy + beta_log * math.log(shop+math.exp(beta_area)*area+math.exp(beta_population)*population + 1) + (d1) * beta_distance_walk + beta_female_walk * female_dummy + beta_zero_walk*zero_car + beta_oneplus_walk*one_plus_car+beta_twoplus_walk*two_plus_car

	utility[9] = beta_cons_taxi + cost_over_income_taxi * (1-missing_income)* beta_cost_taxi_1 + cost_taxi* beta_cost_taxi_2 + tt_taxi_all * beta_tt_taxi + beta_central_taxi * central_dummy + beta_log * math.log(shop+math.exp(beta_area)*area+math.exp(beta_population)*population) + (d1) * beta_distance_taxi + beta_female_taxi * female_dummy + beta_zero_taxi*zero_car+beta_oneplus_taxi*one_plus_car+beta_twoplus_taxi*two_plus_car
	

	end

	
--availability
--the logic to determine availability is the same with current implementation
local availability = {}
local function computeAvailabilities(params,mparams)
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
local scale = 1 --for all choices

-- function to call from C++ preday simulator
-- params and dbparams tables contain data passed from C++
-- to check variable bindings in params or dbparams, refer PredayLuaModel::mapClasses() function in dev/Basic/medium/behavioral/lua/PredayLuaModel.cpp
function choose_wdmso(pparams,mparams)
	computeUtilities(pparams,mparams) 
	computeAvailabilities(pparams,mparams)
	local probability = calculate_probability("mnl", choice, utility, availability, scale)
	return make_final_choice(probability)
end

