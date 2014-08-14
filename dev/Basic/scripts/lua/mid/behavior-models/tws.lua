--[[
Model - Work-based sub-tour generation
Type - NL
Authors - Siyu Li, Harish Loganathan
]]

-- all require statements do not work with C++. They need to be commented. The order in which lua files are loaded must be explicitly controlled in C++. 
--require "NLogit"

--Estimated values for all betas
--Note: the betas that not estimated are fixed to zero.

local beta_cons_work= -4.1
local beta_cons_edu = -7.13
local beta_cons_shopping = -6.36
local beta_cons_other= -3.78
local beta_cons_Q = 0

local beta_first_work = -0.0101
local beta_2plus_work = -1.36
local beta_not_usual = 0.997
local beta_no_car= -0.523

local beta_female_work = -1.04
local beta_female_edu = 0
local beta_female_shopping = 0
local beta_female_other = -0.755

local beta_PT_shopping = 0

--choice set
-- 1 for work; 2 for education; 3 for shopping; 4 for other; 5 for quit
local choice = {}
choice["nonquit"] = {1,2,3,4}
choice["quit"] = {5}

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

	-- if household has no car available
	local zero_car = 1 - params.car_own

	-- if this work tour is made by public transporation
	-- PT_dummy = 1 * (mode_choice == 1 or mode_choice == 2)
	local PT_dummy = 0
	if dbparams.mode_choice==1 or dbparams.mode_choice==2 then
	    PT_dummy = 1
	end

	-- if the the tour is made at usual work location
	local not_usual_dummy = 1 - dbparams.usual_location

	utility[1] = beta_cons_work + beta_female_work * female_dummy
	utility[2] = beta_cons_edu + beta_female_edu * female_dummy
	utility[3] = beta_cons_shopping + beta_female_shopping * female_dummy + beta_PT_shopping * PT_dummy
	utility[4] = beta_cons_other + beta_female_other * female_dummy
	utility[5] = beta_cons_Q + beta_first_work * first_of_multiple + beta_2plus_work * subsequent_of_multiple + beta_not_usual * not_usual_dummy + beta_no_car * zero_car
end

--availability
--the logic to determine availability is the same with current implementation
local availability = {}
local function computeAvailabilities(params)
	availability = {
		params.tws_Work_AV,
		params.tws_Education_AV,
		params.tws_Shopping_AV,
		params.tws_Others_AV,
		params.tws_Quit_AV
	}
end

--scale
local scale={}
scale["nonquit"] = 1.81
scale["quit"] = 1

-- function to call from C++ preday simulator
-- params and dbparams tables contain data passed from C++
-- to check variable bindings in params or dbparams, refer PredayLuaModel::mapClasses() function in dev/Basic/medium/behavioral/lua/PredayLuaModel.cpp
function choose_tws(params,dbparams)
	computeUtilities(params,dbparams) 
	computeAvailabilities(params)
	local probability = calculate_probability("nl", choice, utility, availability, scale)
	return make_final_choice(probability)
end
