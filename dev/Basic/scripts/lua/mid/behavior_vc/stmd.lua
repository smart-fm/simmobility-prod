--[[
Model - Mode/destination choice for work-based tour
Type - logit
Authors - Siyu Li, Harish Loganathan, Olga Petrik
]]

-- all require statements do not work with C++. They need to be commented. The order in which lua files are loaded must be explicitly controlled in C++. 
--require "Logit"

--Estimated values for all betas
--Note: the betas that not estimated are fixed to zero.


-------------------------------------------------
-- The variables having name format as  [ beta_cost_<modeNumber> ]  are coefficients for travel cost
local beta_cost_bus_mrt_1 = -0.19
local beta_cost_Rail_SMS_1 = -0.19
local beta_cost_Rail_SMS_Pool_1 = -0.19
local beta_cost_private_bus_1 = -0.696
local beta_cost_drive1_1 = 0
local beta_cost_share2_1= 0
local beta_cost_share3_1= 0
local beta_cost_motor_1 = 0
local beta_cost_taxi_1 = 0
local beta_cost_SMS_1 = 0
local beta_cost_SMS_Pool_1 = 0



-------------------------------------------------
-- The variables having name format as  [ beta_tt_<modeName> ]  are coefficients for travel time 
-- These are multiplied by the travel time for the respective modes
local beta_tt_bus_mrt = -3.78
local beta_tt_Rail_SMS = -3.78
local beta_tt_Rail_SMS_Pool = -3.78
local beta_tt_private_bus = 0
local beta_tt_drive1 = -4.64
local beta_tt_share2 = -5.45
local beta_tt_share3 = -3.53
local beta_tt_motor = 0
local beta_tt_walk = -0.675
local beta_tt_taxi = 0
local beta_tt_SMS = 0
local beta_tt_SMS_Pool = 0


local beta_log = 0.775              -- coefficient for derived variable log_area
local beta_area = 0                 -- coefficient for area of destination zone
local beta_population = 0           -- coefficient for population of destination zone
local beta_employment = 0           -- coefficient for number of people who work in the destination zone
    


-------------------------------------------------
-- The variables having name format as  [ beta_central_<modeName> ]  are coefficients for centralDummy
-- centralDummy is a dummy varible taking values 0 or 1 based on whether the O/D is in the CBD region of the city
local beta_central_bus_mrt = 0
local beta_central_Rail_SMS = 0
local beta_central_Rail_SMS_Pool = 0
local beta_central_private_bus = 0
local beta_central_drive1 = 0
local beta_central_share2 = 0
local beta_central_share3 = 0
local beta_central_motor = 0
local beta_central_walk = 0
local beta_central_taxi = 0
local beta_central_SMS = 0
local beta_central_SMS_Pool = 0



-------------------------------------------------
-- The variables having name format as  [ beta_distance_<modeName> ]  are coefficients for walk distance
-- <More comments will be added here clarifying the usage of walk distance>
local beta_distance_bus_mrt = 0
local beta_distance_Rail_SMS = 0
local beta_distance_Rail_SMS_Pool = 0
local beta_distance_private_bus = 0 
local beta_distance_drive1 = 0
local beta_distance_share2 = 0
local beta_distance_share3 = 0
local beta_distance_motor = 0
local beta_distance_walk = 0
local beta_distance_taxi = 0
local beta_distance_SMS = 0
local beta_distance_SMS_Pool = 0



-------------------------------------------------
-- The variables having name format as [ beta_cons_<modeName> ] are used to store the Alternate Specific Constants(also called ASCs)
-- These constants are added into the utility calculation later
-- An increase in the [ beta_cons_<modeName> ] for any mode will result in an increase in the percentage of mode shares being increased for this model
local beta_cons_bus = -2.892
local beta_cons_mrt = -3.358
local beta_cons_Rail_SMS = -9.768
local beta_cons_Rail_SMS_Pool = -20.335
local beta_cons_private_bus = -3.162
local beta_cons_drive1 = 0
local beta_cons_share2 = -4.294
local beta_cons_share3 = -5.575
local beta_cons_motor = -7.308
local beta_cons_walk = -7.924
local beta_cons_taxi = -9.075
local beta_cons_SMS = -8.575
local beta_cons_SMS_Pool = -12.168



-------------------------------------------------
-- The variables having name format as  [ beta_female_<modeName> ]  are joint coefficients for femaleDummy and mode name variables
-- femaleDummy is a dummy varible taking values 0 or 1 based on whether the individual is a female or not
local beta_female_bus = 0
local beta_female_mrt = 0
local beta_female_Rail_SMS = 0
local beta_female_Rail_SMS_Pool = 0
local beta_female_private_bus = 0
local beta_female_drive1 = 0
local beta_female_share2 = 0
local beta_female_share3 = 0
local beta_female_motor = 0
local beta_female_taxi = 0
local beta_female_SMS = 0
local beta_female_SMS_Pool = 0
local beta_female_walk = 0



-------------------------------------------------
-- The variables having name format as  [ beta_mode_<modeName> ]  are coefficients for [ mode_work_<modeName> ] variables
-- [ mode_work_<modeName> ] are dummy varibles taking values 0 or 1 based on whether the individual is took the <modeName> as the main mode while going to work
local beta_mode_work_bus = 0
local beta_mode_work_mrt = 0
local beta_mode_work_Rail_SMS = 0
local beta_mode_work_Rail_SMS_Pool = 0
local beta_mode_work_private_bus = 0
local beta_mode_work_drive1 = 5.67
local beta_mode_work_share2 = 3.39
local beta_mode_work_share3 = 0
local beta_mode_work_motor = 0
local beta_mode_work_walk= 0



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
	local female_dummy = params.female_dummy                 -- takes value 1 or 0 based on the individual is a female or not
	
	
	-- [ mode_work_<modeName> ] are dummy varibles taking values 0 or 1 based on whether the individual is took the <modeName> as the main mode while going to work
	local mode_work_bus = dbparams.mode_to_work == 1 and 1 or 0
	local mode_work_mrt = dbparams.mode_to_work == 2 and 1 or 0
	local mode_work_Rail_SMS = dbparams.mode_to_work == 2 and 1 or 0
	local mode_work_Rail_SMS_Pool = dbparams.mode_to_work == 2 and 1 or 0
	local mode_work_private_bus = dbparams.mode_to_work == 3 and 1 or 0
	local mode_work_drive1 = dbparams.mode_to_work == 4 and 1 or 0
	local mode_work_share2 = dbparams.mode_to_work == 5 and 1 or 0
	local mode_work_share3 = dbparams.mode_to_work == 6 and 1 or 0
	local mode_work_motor = dbparams.mode_to_work == 7 and 1 or 0
	local mode_work_walk = dbparams.mode_to_work == 8 and 1 or 0

	

    -- Variable initialisations for time and cost calculations
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
	local cost_drive1 = {}
	local cost_share2 = {}
	local cost_share3 = {}
	local cost_motor = {}
	local cost_taxi={}
	local cost_taxi_1 = {}
	local cost_taxi_2 = {}
	local cost_SMS={}
	local cost_SMS_1 = {}
	local cost_SMS_2 = {}
	local cost_SMS_Pool={}
	local cost_SMS_Pool_1 = {}
	local cost_SMS_Pool_2 = {}
	local central_dummy={}
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
	local tt_car_ivt = {}
	local tt_public_ivt = {}
	local tt_public_out = {}
	local employment = {}
	local population = {}
	local area = {}
	local shop = {}
	local d1 = {}
	local d2 = {}
	

	
    -- Iterating over the taz's (traffic analysis zones)
	for i =1,24 do
		d1[i] = dbparams:walk_distance1(i)
		d2[i] = dbparams:walk_distance2(i)
		central_dummy[i] = dbparams:central_dummy(i)	-- takes value 1 if the destination taz is in the central business district (CBD) of the city	

        -------------------------------------------------
		-- Expressions for calculating travel costs of various modes
		-- first: first half tour; -- second: second half tour
		cost_bus[i] = dbparams:cost_public_first(i) + dbparams:cost_public_second(i)
		cost_mrt[i] = cost_bus[i]
       	cost_private_bus[i] = cost_bus[i]
		cost_drive1[i] = dbparams:cost_car_ERP_first(i)+dbparams:cost_car_ERP_second(i)+dbparams:cost_car_OP_first(i)+dbparams:cost_car_OP_second(i)+dbparams:cost_car_parking(i)
		cost_share2[i] = dbparams:cost_car_ERP_first(i)+dbparams:cost_car_ERP_second(i)+dbparams:cost_car_OP_first(i)+dbparams:cost_car_OP_second(i)+dbparams:cost_car_parking(i)/2
		cost_share3[i] = dbparams:cost_car_ERP_first(i)+dbparams:cost_car_ERP_second(i)+dbparams:cost_car_OP_first(i)+dbparams:cost_car_OP_second(i)+dbparams:cost_car_parking(i)/3
		cost_motor[i] = 0.5*(dbparams:cost_car_ERP_first(i)+dbparams:cost_car_ERP_second(i)+dbparams:cost_car_OP_first(i)+dbparams:cost_car_OP_second(i))+0.65*dbparams:cost_car_parking(i)
		
		
		
        -- Cost of travelling by taxi is computed using three components: an initial flag down cost (3.4), a fixed rate per km, upto 10 kms and another rate per km after 10 kms travelled 
		cost_taxi_1[i] = 3.4+((d1[i]*(d1[i]>10 and 1 or 0)-10*(d1[i]>10 and 1 or 0))/0.35+(d1[i]*(d1[i]<=10 and 1 or 0)+10*(d1[i]>10 and 1 or 0))/0.4)*0.22+ dbparams:cost_car_ERP_first(i) + central_dummy[i]*3
		cost_taxi_2[i] = 3.4+((d2[i]*(d2[i]>10 and 1 or 0)-10*(d2[i]>10 and 1 or 0))/0.35+(d2[i]*(d2[i]<=10 and 1 or 0)+10*(d2[i]>10 and 1 or 0))/0.4)*0.22+ dbparams:cost_car_ERP_second(i) + central_dummy[i]*3
		cost_taxi[i] = cost_taxi_1[i] + cost_taxi_2[i]
		
		
		
		-- Cost of SMS defined as a percentage of cost of Taxi (72% in the example below)
		cost_SMS_1[i] = 3.4+((d1[i]*(d1[i]>10 and 1 or 0)-10*(d1[i]>10 and 1 or 0))/0.35+(d1[i]*(d1[i]<=10 and 1 or 0)+10*(d1[i]>10 and 1 or 0))/0.4)*0.22+ dbparams:cost_car_ERP_first(i) + central_dummy[i]*3
		cost_SMS_2[i] = 3.4+((d2[i]*(d2[i]>10 and 1 or 0)-10*(d2[i]>10 and 1 or 0))/0.35+(d2[i]*(d2[i]<=10 and 1 or 0)+10*(d2[i]>10 and 1 or 0))/0.4)*0.22+ dbparams:cost_car_ERP_second(i) + central_dummy[i]*3
		cost_SMS[i] = (cost_SMS_1[i] + cost_SMS_2[i])*0.72
		
		
	   	-- Cost of SMS_Pool defined as a percentage of cost of SMS (70 % in the example below)		
		cost_SMS_Pool_1[i] = 3.4+((d1[i]*(d1[i]>10 and 1 or 0)-10*(d1[i]>10 and 1 or 0))/0.35+(d1[i]*(d1[i]<=10 and 1 or 0)+10*(d1[i]>10 and 1 or 0))/0.4)*0.22+ dbparams:cost_car_ERP_first(i) + central_dummy[i]*3
		cost_SMS_Pool_2[i] = 3.4+((d2[i]*(d2[i]>10 and 1 or 0)-10*(d2[i]>10 and 1 or 0))/0.35+(d2[i]*(d2[i]<=10 and 1 or 0)+10*(d2[i]>10 and 1 or 0))/0.4)*0.22+ dbparams:cost_car_ERP_second(i) + central_dummy[i]*3
		cost_SMS_Pool[i] = (cost_SMS_Pool_1[i] + cost_SMS_Pool_2[i])*0.72*0.7
		
		
		local aed = 2.0 -- Access egress distance (AED)
		
		
        -- Cost of Rail_SMS calculated similar to SMS but by using AED in place of walking distance 		
		cost_Rail_SMS_AE_1[i] = 3.4+((aed*(aed>10 and 1 or 0)-10*(aed>10 and 1 or 0))/0.35+(aed*(aed<=10 and 1 or 0)+10*(aed>10 and 1 or 0))/0.4)*0.22+ dbparams:cost_car_ERP_first(i) + central_dummy[i]*3
		cost_Rail_SMS_AE_2[i] = 3.4+((aed*(aed>10 and 1 or 0)-10*(aed>10 and 1 or 0))/0.35+(aed*(aed<=10 and 1 or 0)+10*(aed>10 and 1 or 0))/0.4)*0.22+ dbparams:cost_car_ERP_second(i) + central_dummy[i]*3
		cost_Rail_SMS_AE_avg[i] = (cost_Rail_SMS_AE_1[i] + cost_Rail_SMS_AE_2[i])/2
		cost_Rail_SMS[i] = cost_mrt[i] + cost_Rail_SMS_AE_avg[i]*0.72


    	-- Cost of Rail_SMS_Pool defined as a percentage of cost of Rail_SMS (70 % in the example below)				
		cost_Rail_SMS_AE_Pool_1[i] = 3.4+((aed*(aed>10 and 1 or 0)-10*(aed>10 and 1 or 0))/0.35+(aed*(aed<=10 and 1 or 0)+10*(aed>10 and 1 or 0))/0.4)*0.22+ dbparams:cost_car_ERP_first(i) + central_dummy[i]*3
		cost_Rail_SMS_AE_Pool_2[i] = 3.4+((aed*(aed>10 and 1 or 0)-10*(aed>10 and 1 or 0))/0.35+(aed*(aed<=10 and 1 or 0)+10*(aed>10 and 1 or 0))/0.4)*0.22+ dbparams:cost_car_ERP_second(i) + central_dummy[i]*3
		cost_Rail_SMS_AE_Pool_avg[i] = (cost_Rail_SMS_AE_Pool_1[i] + cost_Rail_SMS_AE_Pool_2[i])/2
		cost_Rail_SMS_Pool[i] = cost_mrt[i] + cost_Rail_SMS_AE_Pool_avg[i]*0.72*0.7



        -- ivt: in-vehicle time;  
        -- first: first half tour; -- second: second half tour
        -- public: name of mode
        -- public_walk : time spent in walking if the public mode chosen   
		tt_car_ivt[i] = dbparams:tt_car_ivt_first(i) + dbparams:tt_car_ivt_second(i)
		tt_public_ivt[i] = dbparams:tt_public_ivt_first(i) + dbparams:tt_public_ivt_second(i)
		tt_public_out[i] = dbparams:tt_public_out_first(i) + dbparams:tt_public_out_second(i)
		tt_bus[i] = tt_public_ivt[i]+ tt_public_out[i]
		tt_mrt[i] = tt_public_ivt[i]+ tt_public_out[i]
		tt_Rail_SMS[i] = tt_public_ivt[i]+ tt_public_out[i]/6.0
		tt_Rail_SMS_Pool[i] = tt_public_ivt[i]+ tt_public_out[i]/6.0+(aed+aed)/60+1/10
		tt_private_bus[i] = tt_car_ivt[i]
		tt_drive1[i] = tt_car_ivt[i] + 1.0/6
		tt_share2[i] = tt_car_ivt[i] + 1.0/6
		tt_share3[i] = tt_car_ivt[i] + 1.0/6
		tt_motor[i] = tt_car_ivt[i] + 1.0/6
		tt_walk[i] = (d1[i]+d2[i])/5
		tt_taxi[i] = tt_car_ivt[i] + 1.0/6
		tt_SMS[i] = tt_car_ivt[i] + 1.0/6
		tt_SMS_Pool[i] = tt_car_ivt[i] + 1.0/6+ 1/10+(d1[i]+d2[i])/2/60 + 1.0/6


        -- Variables to store attributes of the destination taz in question (i-th taz)        		
		employment[i] = dbparams:employment(i)  -- number of people working in the i-th taz
		population[i] = dbparams:population(i)  -- number of people living in the i-th taz
		area[i] = dbparams:area(i)              -- area of the i-th taz
		shop[i] = dbparams:shop(i)              -- number of shops in the i-th taz

	end

	local exp = math.exp
	local log = math.log

	local V_counter = 0

	--utility function for bus 1-24
	for i =1,24 do
		V_counter = V_counter + 1
		utility[V_counter] = beta_cons_bus + cost_bus[i] * beta_cost_bus_mrt_1 + tt_bus[i] * beta_tt_bus_mrt + beta_central_bus_mrt * central_dummy[i] + beta_log * log(shop[i]+exp(beta_employment)*employment[i]) + (d1[i]+d2[i]) * beta_distance_bus_mrt + beta_female_bus * female_dummy + beta_mode_work_bus * mode_work_bus
	end

	--utility function for mrt 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_mrt + cost_mrt[i] * beta_cost_bus_mrt_1 + tt_mrt[i] * beta_tt_bus_mrt + beta_central_bus_mrt * central_dummy[i] + beta_log * log(shop[i]+exp(beta_employment)*employment[i]) + (d1[i]+d2[i]) * beta_distance_bus_mrt + beta_female_mrt * female_dummy + beta_mode_work_bus * mode_work_mrt
	end

	--utility function for private bus 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_private_bus + cost_private_bus[i] * beta_cost_private_bus_1 + tt_private_bus[i] * beta_tt_bus_mrt + beta_central_private_bus * central_dummy[i] + beta_log * log(shop[i]+exp(beta_employment)*employment[i]) + (d1[i]+d2[i]) * beta_distance_private_bus + beta_female_private_bus * female_dummy + beta_mode_work_bus * mode_work_private_bus
	end

	--utility function for drive1 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_drive1 + cost_drive1[i] * beta_cost_drive1_1 + tt_drive1[i] * beta_tt_drive1 + beta_central_drive1 * central_dummy[i] + beta_log * log(shop[i]+exp(beta_employment)*employment[i]) + (d1[i]+d2[i]) * beta_distance_drive1 + beta_female_drive1 * female_dummy + beta_mode_work_drive1 * mode_work_drive1 
	end

	--utility function for share2 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_share2 + cost_share2[i] *beta_cost_drive1_1 + tt_share2[i] * beta_tt_share2 + beta_central_share2 * central_dummy[i] + beta_log * log(shop[i]+exp(beta_employment)*employment[i]) + (d1[i]+d2[i]) * beta_distance_share2 + beta_female_share2 * female_dummy + beta_mode_work_share2 * mode_work_share2
	end

	--utility function for share3 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_share3 + cost_share3[i] * beta_cost_drive1_1 + tt_share3[i] * beta_tt_share3 + beta_central_share3 * central_dummy[i] + beta_log * log(shop[i]+exp(beta_employment)*employment[i]) + (d1[i]+d2[i]) * beta_distance_share3 + beta_female_share3 * female_dummy + beta_mode_work_share2 * mode_work_share3
	end

	--utility function for motor 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_motor + cost_motor[i] * beta_cost_drive1_1 + tt_motor[i] * beta_tt_motor + beta_central_motor * central_dummy[i] + beta_log * log(shop[i]+exp(beta_employment)*employment[i]) + (d1[i]+d2[i]) * beta_distance_motor + beta_female_motor * female_dummy + beta_mode_work_drive1 * mode_work_motor
	end

	--utility function for walk 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_walk + tt_walk[i] * beta_tt_walk + beta_central_walk * central_dummy[i] + beta_log * log(shop[i]+exp(beta_employment)*employment[i]) + (d1[i]+d2[i]) * beta_distance_walk + beta_female_walk * female_dummy + beta_mode_work_walk * mode_work_walk
	end

	--utility function for taxi 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_taxi + cost_taxi[i] * beta_cost_drive1_1 + tt_taxi[i] * beta_tt_taxi + beta_central_taxi * central_dummy[i] + beta_log * log(shop[i]+exp(beta_employment)*employment[i]) + (d1[i]+d2[i]) * beta_distance_taxi + beta_female_taxi * female_dummy
	end
	
	--utility function for SMS 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_SMS + cost_SMS[i] * beta_cost_drive1_1 + tt_SMS[i] * beta_tt_SMS + beta_central_SMS * central_dummy[i] + beta_log * log(shop[i]+exp(beta_employment)*employment[i]) + (d1[i]+d2[i]) * beta_distance_SMS + beta_female_SMS * female_dummy
	end
	
	--utility function for Rail_SMS 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_Rail_SMS + cost_Rail_SMS[i] * beta_cost_Rail_SMS_1 + tt_Rail_SMS[i] * beta_tt_Rail_SMS + beta_central_Rail_SMS * central_dummy[i] + beta_log * log(shop[i]+exp(beta_employment)*employment[i]) + (d1[i]+d2[i]) * beta_distance_Rail_SMS + beta_female_Rail_SMS * female_dummy + beta_mode_work_bus * mode_work_Rail_SMS
	end
		
	--utility function for SMS_Pool 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_SMS_Pool + cost_SMS_Pool[i] * beta_cost_drive1_1 + tt_SMS_Pool[i] * beta_tt_SMS_Pool + beta_central_SMS_Pool * central_dummy[i] + beta_log * log(shop[i]+exp(beta_employment)*employment[i]) + (d1[i]+d2[i]) * beta_distance_SMS_Pool + beta_female_SMS_Pool * female_dummy
	end
	
	--utility function for Rail_SMS_Pool 1-24
	for i=1,24 do
		V_counter = V_counter +1
		utility[V_counter] = beta_cons_Rail_SMS_Pool + cost_Rail_SMS_Pool[i] * beta_cost_Rail_SMS_Pool_1 + tt_Rail_SMS_Pool[i] * beta_tt_Rail_SMS_Pool + beta_central_Rail_SMS_Pool * central_dummy[i] + beta_log * log(shop[i]+exp(beta_employment)*employment[i]) + (d1[i]+d2[i]) * beta_distance_Rail_SMS_Pool + beta_female_Rail_SMS_Pool * female_dummy + beta_mode_work_bus * mode_work_Rail_SMS_Pool
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
--scale can be used to control the variance of selection of choices
local scale = 1 -- for all choices

function choose_stmd(params,dbparams)
	computeUtilities(params,dbparams) 
	computeAvailabilities(params,dbparams)
	local probability = calculate_probability("mnl", choice, utility, availability, scale)
	return make_final_choice(probability)
end
