--[[
Model - Work-based sub-tour generation
Type - MNL
Authors - Siyu Li, Harish Loganathan
]]

-- all require statements do not work with C++. They need to be commented. The order in which lua files are loaded must be explicitly controlled in C++. 
--require "MNL"

--Estimated values for all betas
--Note: the betas that not estimated are fixed to zero.

local beta_cons_NQ = -4.42
local beta_cons_Q = 0

local beta_first_work = 0
local beta_2plus_work = 0
local beta_not_usual = 0
local beta_car_avail = -0.265

local beta_female_nonquit = -0.502

--choice set
-- 1 for non-quit; 2 for quit
local choice = {1,2}

--utility
-- 1 for work; 2 for education; 3 for shopping; 4 for other; 5 for quit
local utility = {}
local function computeUtilities(params,dbparams)
	local female_dummy = params.female_dummy

	--first of multiple =1 if this work tour is the first work tour
	--of many work tours modeled for an agent, else first of multiple =0
	--subsequent of multiple =1 if this work tour is the subsequent work
	--tour of many work tours modeled for an agent, else first of multiple =0
	local first_of_multiple = dbparams.first_of_multiple
	local subsequent_of_multiple = dbparams.subsequent_of_multiple
	local veh_own_cat = params.vehicle_ownership_category
	
	local zero_car,car_avail = 0,0
	if veh_own_cat == 0  then 
		zero_car = 1 
	end
	
	if veh_own_cat == 2 or veh_own_cat == 3 or veh_own_cat == 4 or veh_own_cat == 5 then 
		car_avail = 1 
	end
	-- if this work tour is made by public transporation
	-- PT_dummy = 1 * (mode_choice == 1 or mode_choice == 2)
	local PT_dummy = 0
	if dbparams.mode_choice==1 or dbparams.mode_choice==2 then
	    PT_dummy = 1
	end

	-- if the the tour is made at usual work location
	local not_usual_dummy = 1 - dbparams.usual_location

	utility[1] = beta_cons_NQ + beta_female_nonquit * female_dummy +beta_car_avail* car_avail
	utility[2] = beta_cons_Q + beta_first_work * first_of_multiple + beta_2plus_work * subsequent_of_multiple + beta_not_usual * not_usual_dummy
end

--availability
--the logic to determine availability is the same with current implementation
local availability = {1,1} -- all choices are available

--scale
local scale= 1 --for all choices

-- function to call from C++ preday simulator
-- params and dbparams tables contain data passed from C++
-- to check variable bindings in params or dbparams, refer PredayLuaModel::mapClasses() function in dev/Basic/medium/behavioral/lua/PredayLuaModel.cpp
function choose_tws(params,dbparams)
	computeUtilities(params,dbparams) 
	local probability = calculate_probability("mnl", choice, utility, availability)
	return make_final_choice(probability)
end
