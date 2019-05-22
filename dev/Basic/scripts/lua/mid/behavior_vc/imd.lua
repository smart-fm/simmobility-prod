--[[
Model - Mode/destination choice for work tour to unusual location
Type - logit
Authors - Siyu Li, Harish Loganathan
]]

-- all require statements do not work with C++. They need to be commented. The order in which lua files are loaded must be explicitly controlled in C++. 
--require "Logit"

--Estimated values for all betas
--Note: the betas that not estimated are fixed to zero.


-------------------------------------------------
-- The variables having name format as  [ beta_cost_<modeNumber>_2 ]  are coefficients for travel cost
local beta_cost_bus_mrt_2 = -0.438
local beta_cost_Rail_SMS_2 = -0.438
local beta_cost_Rail_SMS_Pool_2 = -0.438
local beta_cost_private_bus_2 = -0.850
local beta_cost_drive1_2_first = -0.0799
local beta_cost_drive1_2_second = 0.0598
local beta_cost_share2_2 = 0
local beta_cost_share3_2 = 0
local beta_cost_motor_2 = -0.338
local beta_cost_taxi_2 = 0
local beta_cost_SMS_2 = 0
local beta_cost_SMS_Pool_2 = 0



-------------------------------------------------
-- The variables having name format as  [ beta_tt_<modeName> ]  are coefficients for travel time 
-- These are multiplied by the travel time for the respective modes
local beta_tt_bus_mrt = -3.75
local beta_tt_Rail_SMS = -3.75
local beta_tt_Rail_SMS_Pool = -3.75
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
local beta_tt_SMS_Pool_first = -5.63
local beta_tt_SMS_Pool_second = -5.79



local beta_work = 0.567                 --Coefficient for derived variable (logarithm of employment in the taz)
local beta_shop = 0.979                 --Coefficient for derived variable (logarithm of shop count in the taz)



-------------------------------------------------
-- The variables having name format as  [ beta_central_<modeName> ]  are coefficients for centralDummy
-- centralDummy is a dummy varible taking values 0 or 1 based on whether the O/D is in the CBD region of the city
local beta_central_bus_mrt = 0.172
local beta_central_Rail_SMS = 0.172
local beta_central_Rail_SMS_Pool = 0.172
local beta_central_private_bus = 0.175
local beta_central_drive1 = 0
local beta_central_share2 = -0.0332
local beta_central_share3 = 0.221
local beta_central_motor = 0.180
local beta_central_walk = 0
local beta_central_taxi = 1.07
local beta_central_SMS = 1.07
local beta_central_SMS_Pool = 1.07



-------------------------------------------------
-- The variables having name format as  [ beta_distance_<modeName> ]  are coefficients for walk distance
-- <More comments will be added here clarifying the usage of walk distance>
local beta_distance_bus_mrt = 0.0112
local beta_distance_Rail_SMS = 0.0112
local beta_distance_Rail_SMS_Pool = 0.0112
local beta_distance_private_bus = 0.0189
local beta_distance_drive1 = 0
local beta_distance_share2 = -0.0229
local beta_distance_share3 = -0.0173
local beta_distance_motor = 0.00259
local beta_distance_walk = 0
local beta_distance_taxi = 0.000622
local beta_distance_SMS = 0.000622
local beta_distance_SMS_Pool = 0.000622



-------------------------------------------------
-- The variables having name format as [ beta_cons_<modeName> ] are used to store the Alternate Specific Constants(also called ASCs)
-- These constants are added into the utility calculation later
-- An increase in the [ beta_cons_<modeName> ] for any mode will result in an increase in the percentage of mode shares being increased for this model
local beta_cons_bus = 4.479
local beta_cons_mrt = 4.383
local beta_cons_Rail_SMS = -9.235
local beta_cons_Rail_SMS_Pool = -14.168
local beta_cons_private_bus = 3.072
local beta_cons_drive1 = 0
local beta_cons_share2 = 2.012
local beta_cons_share3 = 2.272
local beta_cons_motor = 1.698
local beta_cons_walk = -99.128
local beta_cons_taxi = -4.128
local beta_cons_SMS = -4.628
local beta_cons_SMS_Pool = -7.235



-------------------------------------------------
-- The variables having name format as  [ beta_<vehicleOwnerShipCategoryDummy>_<modeName> ]  are coefficients for vehicleOwnershipDummy variables
-- vehicleOwnershipDummy is a dummy varible taking values 0 or 1 based on whether the individual owns has a particular set of vehicles(like oneCar, onePlusCar etc.. )
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



-------------------------------------------------
-- The variables having name format as  [ beta_female_<modeName> ]  are joint coefficients for femaleDummy and mode name variables
-- femaleDummy is a dummy varible taking values 1 or 0 based on whether the individual is a female or not
local beta_female_bus = 0.568
local beta_female_mrt = 0.622
local beta_female_Rail_SMS = 0.622
local beta_female_Rail_SMS_Pool = 0.622
local beta_female_private_bus = 1.45
local beta_female_drive1 = 0
local beta_female_share2 = 0.393
local beta_female_share3 = 0.162
local beta_female_motor = 0.283
local beta_female_taxi = 1.40
local beta_female_SMS = 1.40
local beta_female_SMS_Pool = 1.40
local beta_female_walk = 0



--choice set
-- choice set contains the set of choices(mode,taz combinations) which are available in this model 
-- The serial number of modes in the choice set corresponds the order of modes as listed in the config file data/simulation.xml
-- Number of taz (traffic analysis zones in Virtual city) = 24
-- Number of modes = 13; Thus total number of mode zone combinations = 24 * 13

local choice = {}
for i = 1, 24*13 do
	choice[i] = i
end



--utility is a lua table which will store the computed utilities for various (modes,taz) combinations
-- 1 for public bus; 2 for MRT/LRT; 3 for private bus; 4 for drive1;
-- 5 for shared2; 6 for shared3+; 7 for motor; 8 for walk; 9 for taxi
-- 10 for SMS, 11 for Rail_SMS; 12 for SMS_Pool, 13 for Rail_SMS_Pool
local utility = {}
local function computeUtilities(params,dbparams)
	local female_dummy = params.female_dummy          -- takes value 1 or 0 based on the individual is a female or not
	local income_id = params.income_id
	local income_cat = {500,1250,1750,2250,2750,3500,4500,5500,6500,7500,8500,0,99999,99999}
	local income_mid = income_cat[income_id]
	local missing_income = params.income_id >= 13 and 1 or 0

	local work_stop_dummy = dbparams.stop_type == 1 and 1 or 0
	local edu_stop_dummy = dbparams.stop_type == 2 and 1 or 0
	local shop_stop_dummy = dbparams.stop_type == 3 and 1 or 0
	local other_stop_dummy = dbparams.stop_type == 4 and 1 or 0

	
	first_bound = dbparams.first_bound                 --1 if the current modeled stop is on first half tour, 0 otherwise
	second_bound = dbparams.second_bound               --1 if the current modeled stop is on second half tour, 0 otherwise



    -- Converting vehicle ownership category id to dummy variables (dummy variables can take value 1 or 0)
	local zero_car,one_plus_car,two_plus_car,three_plus_car, zero_motor,one_plus_motor,two_plus_motor,three_plus_motor = 0,0,0,0,0,0,0,0
	local veh_own_cat = params.vehicle_ownership_category
	if veh_own_cat == 0 or veh_own_cat == 1 or veh_own_cat ==2 then
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


    -- Variable initialisations for time and cost calculations
	local cost_public = {}
	local cost_bus = {}
	local cost_mrt = {}
	local cost_Rail_SMS = {}
	local cost_Rail_SMS_AE_1 = {}
	local cost_Rail_SMS_AE_2 = {}
	local cost_Rail_SMS_AE_avg = {}
	local cost_Rail_SMS_Pool = {}
	local cost_Rail_SMS_AE_Pool_1 = {}
	local cost_Rail_SMS_AE_Pool_2 = {}
	local cost_Rail_SMS_AE_Pool_avg = {}
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
	local cost_SMS_Pool_1 = {}
	local cost_SMS_Pool_2 = {}
	local cost_SMS_Pool={}
	local d1={}
	local d2={}
	local central_dummy={}
	local tt_public_ivt = {}
	local tt_public_out = {}
    local tt_car_ivt = {}
	local tt_bus = {}
	local tt_mrt = {}
	local tt_Rail_SMS = {}
	local tt_Rail_SMS_Pool = {}
	local tt_private_bus = {}
	local tt_drive1 = {}
	local tt_share2 = {}
	local tt_share3 = {}
	local tt_motor = {}
	local tt_walk = {}
	local tt_taxi = {}
	local tt_SMS = {}
	local tt_SMS_Pool = {}
	local average_transfer_number = {}
	local employment = {}
	local population = {}
	local area = {}
	local shop = {}


	for i =1,24 do
		d1[i] = dbparams:walk_distance1(i)
		d2[i] = dbparams:walk_distance2(i)
		central_dummy[i] = dbparams:central_dummy(i)    -- takes value 1 if the destination taz is in the central business district (CBD) of the city
		

	    -------------------------------------------------
		-- Expressions for calculating travel costs of various modes
		-- first: first half tour; -- second: second half tour
		cost_public[i] = dbparams:cost_public(i)
		cost_bus[i] = cost_public[i]
		cost_mrt[i] = cost_public[i]
		cost_private_bus[i] = cost_public[i]
		cost_drive1[i] = dbparams:cost_car_ERP(i) + dbparams:cost_car_OP(i) + dbparams:cost_car_parking(i)
		cost_share2[i] = (dbparams:cost_car_ERP(i) + dbparams:cost_car_OP(i) + dbparams:cost_car_parking(i))/2
		cost_share3[i] = (dbparams:cost_car_ERP(i) + dbparams:cost_car_OP(i) + dbparams:cost_car_parking(i))/3
		cost_motor[i] = 0.5*(dbparams:cost_car_ERP(i) + dbparams:cost_car_OP(i))+ 0.65*dbparams:cost_car_parking(i)


        -- Cost of travelling by taxi is computed using three components: an initial flag down cost (3.4), a fixed rate per km, upto 10 kms and another rate per km after 10 kms travelled 
    	cost_taxi_1[i] = 3.4+((d1[i]*(d1[i]>10 and 1 or 0)-10*(d1[i]>10 and 1 or 0))/0.35+(d1[i]*(d1[i]<=10 and 1 or 0)+10*(d1[i]>10 and 1 or 0))/0.4)*0.22+ dbparams:cost_car_ERP(i) + central_dummy[i]*3
		cost_taxi_2[i] = 3.4+((d2[i]*(d2[i]>10 and 1 or 0)-10*(d2[i]>10 and 1 or 0))/0.35+(d2[i]*(d2[i]<=10 and 1 or 0)+10*(d2[i]>10 and 1 or 0))/0.4)*0.22+ central_dummy[i]*3
		cost_taxi[i] = (cost_taxi_1[i] + cost_taxi_2[i])/2
		
		
		
		-- Cost of SMS defined as a percentage of cost of Taxi (72% in the example below)		
		cost_SMS_1[i] = 3.4+((d1[i]*(d1[i]>10 and 1 or 0)-10*(d1[i]>10 and 1 or 0))/0.35+(d1[i]*(d1[i]<=10 and 1 or 0)+10*(d1[i]>10 and 1 or 0))/0.4)*0.22+ dbparams:cost_car_ERP(i) + central_dummy[i]*3
		cost_SMS_2[i] = 3.4+((d2[i]*(d2[i]>10 and 1 or 0)-10*(d2[i]>10 and 1 or 0))/0.35+(d2[i]*(d2[i]<=10 and 1 or 0)+10*(d2[i]>10 and 1 or 0))/0.4)*0.22+ central_dummy[i]*3
		cost_SMS[i] = (cost_SMS_1[i] + cost_SMS_2[i])*0.72/2
		
		
		
	   	-- Cost of SMS_Pool defined as a percentage of cost of SMS (70 % in the example below)		
		cost_SMS_Pool_1[i] = 3.4+((d1[i]*(d1[i]>10 and 1 or 0)-10*(d1[i]>10 and 1 or 0))/0.35+(d1[i]*(d1[i]<=10 and 1 or 0)+10*(d1[i]>10 and 1 or 0))/0.4)*0.22+ dbparams:cost_car_ERP(i) + central_dummy[i]*3
		cost_SMS_Pool_2[i] = 3.4+((d2[i]*(d2[i]>10 and 1 or 0)-10*(d2[i]>10 and 1 or 0))/0.35+(d2[i]*(d2[i]<=10 and 1 or 0)+10*(d2[i]>10 and 1 or 0))/0.4)*0.22+ central_dummy[i]*3
		cost_SMS_Pool[i] = (cost_SMS_Pool_1[i] + cost_SMS_Pool_2[i])*0.72*0.7/2


		local aed = 2.0 -- Access egress distance(AED)
				
		
        -- Cost of Rail_SMS calculated similar to SMS but by using AED in place of walking distance 
		cost_Rail_SMS_AE_1[i] = 3.4+((aed*(aed>10 and 1 or 0)-10*(aed>10 and 1 or 0))/0.35+(aed*(aed<=10 and 1 or 0)+10*(aed>10 and 1 or 0))/0.4)*0.22+ dbparams:cost_car_ERP(i) + central_dummy[i]*3
		cost_Rail_SMS_AE_2[i] = 3.4+((aed*(aed>10 and 1 or 0)-10*(aed>10 and 1 or 0))/0.35+(aed*(aed<=10 and 1 or 0)+10*(aed>10 and 1 or 0))/0.4)*0.22+ central_dummy[i]*3
		cost_Rail_SMS_AE_avg[i] = (cost_Rail_SMS_AE_1[i] + cost_Rail_SMS_AE_2[i])/2
		cost_Rail_SMS[i] = cost_public[i] + cost_Rail_SMS_AE_avg[i]*0.72


    	-- Cost of Rail_SMS_Pool defined as a percentage of cost of Rail_SMS (70 % in the example below)		
		cost_Rail_SMS_AE_Pool_1[i] = 3.4+((aed*(aed>10 and 1 or 0)-10*(aed>10 and 1 or 0))/0.35+(aed*(aed<=10 and 1 or 0)+10*(aed>10 and 1 or 0))/0.4)*0.22+ dbparams:cost_car_ERP(i) + central_dummy[i]*3
		cost_Rail_SMS_AE_Pool_2[i] = 3.4+((aed*(aed>10 and 1 or 0)-10*(aed>10 and 1 or 0))/0.35+(aed*(aed<=10 and 1 or 0)+10*(aed>10 and 1 or 0))/0.4)*0.22+ central_dummy[i]*3
		cost_Rail_SMS_AE_Pool_avg[i] = (cost_Rail_SMS_AE_Pool_1[i] + cost_Rail_SMS_AE_Pool_2[i])/2
		cost_Rail_SMS_Pool[i] = cost_public[i] + cost_Rail_SMS_AE_Pool_avg[i]*0.72*0.7

	

        -- ivt: in-vehicle time;  
        -- first: first half tour; -- second: second half tour
        -- public: name of mode
        -- public_walk : time spent in walking if the public mode chosen      
		tt_public_ivt[i] = dbparams:tt_public_ivt(i)
		tt_public_out[i] = dbparams:tt_public_out(i)
		tt_car_ivt[i] = dbparams:tt_car_ivt(i)
		tt_bus[i] = tt_public_ivt[i]+ tt_public_out[i]
		tt_mrt[i] = tt_public_ivt[i]+ tt_public_out[i]
		tt_Rail_SMS[i] = tt_public_ivt[i]+ tt_public_out[i]/6.0
		tt_Rail_SMS_Pool[i] = tt_public_ivt[i]+ tt_public_out[i]/6.0+(aed+aed)/60/2+1/20
		tt_private_bus[i] = tt_car_ivt[i]
		tt_drive1[i] = tt_car_ivt[i] + 1.0/12
		tt_share2[i] = tt_car_ivt[i] + 1.0/12
		tt_share3[i] = tt_car_ivt[i] + 1.0/12
		tt_motor[i] = tt_car_ivt[i] + 1.0/12
		tt_walk[i] = (d1[i]+d2[i])/5/2
		tt_taxi[i] = tt_car_ivt[i] + 1.0/12
		local tt_SMS_out=dbparams:wtt_sms(i)
		tt_SMS[i] = tt_car_ivt[i] + tt_SMS_out
		local tt_SMS_pool_out=dbparams:wtt_sms_pool(i)
		tt_SMS_Pool[i] = tt_car_ivt[i] + tt_SMS_pool_out + 1/20+(d1[i]+d2[i])/4/60


		
		 -- Variables to store attributes of the destination taz in question (i-th taz)        		
		employment[i] = dbparams:employment(i)  -- number of people working in the i-th taz
		population[i] = dbparams:population(i)  -- number of people living in the i-th taz
		area[i] = dbparams:area(i)              -- area of the i-th taz
		shop[i] = dbparams:shop(i)              -- number of shops in the i-th taz
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
	--utility function for Rail_SMS 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_Rail_SMS + cost_Rail_SMS[i] * beta_cost_Rail_SMS_2 + tt_Rail_SMS[i] * beta_tt_Rail_SMS + beta_central_Rail_SMS * central_dummy[i] + beta_shop * log(1+shop[i])*shop_stop_dummy+beta_work * log(1+employment[i])*work_stop_dummy + (d1[i]+d2[i]) * beta_distance_Rail_SMS + beta_female_Rail_SMS * female_dummy
	end

	--utility function for SMS_Pool 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_SMS_Pool + first_bound * cost_SMS_Pool[i]* beta_cost_drive1_2_first + second_bound * cost_SMS_Pool[i]* beta_cost_drive1_2_second + first_bound * tt_SMS_Pool[i] * beta_tt_SMS_Pool_first + second_bound * tt_SMS_Pool[i] * beta_tt_SMS_Pool_second + beta_central_SMS_Pool * central_dummy[i] + beta_shop * log(1+shop[i])*shop_stop_dummy+beta_work * log(1+employment[i])*work_stop_dummy + (d1[i]+d2[i]) * beta_distance_SMS_Pool + beta_female_SMS_Pool * female_dummy
	end
	--utility function for Rail_SMS_Pool 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_Rail_SMS_Pool + cost_Rail_SMS_Pool[i] * beta_cost_Rail_SMS_Pool_2 + tt_Rail_SMS_Pool[i] * beta_tt_Rail_SMS_Pool + beta_central_Rail_SMS_Pool * central_dummy[i] + beta_shop * log(1+shop[i])*shop_stop_dummy+beta_work * log(1+employment[i])*work_stop_dummy + (d1[i]+d2[i]) * beta_distance_Rail_SMS_Pool + beta_female_Rail_SMS_Pool * female_dummy
	end

end


--availability
--the logic to determine availability is the same with current implementation
local availability = {}
local function computeAvailabilities(params,dbparams)
	for i = 1, 24*13 do
		availability[i] = dbparams:availability(i)
	end
end

--scale
--the logic to determine availability is the same with current implementation
local scale={}
for i = 1, 24*13 do
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
