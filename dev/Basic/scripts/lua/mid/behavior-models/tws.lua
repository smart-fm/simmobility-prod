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
local choice = {
	"nonquit": {1,2,3,4}
	"quit" : {5}
}

--utility
-- 1 for work; 2 for education; 3 for shopping; 4 for other; 5 for quit
local utility = {}
local function computeUtilities(params,dbparams)
	local female_dummy = params.female_dummy

	--first of multiple =1 if this work tour is the first work tour
	--of many work tours modeled for an agent, else first of multiple =0
	--subsequent of multiple =1 if this work tour is the subsequent work
	--tour of many work tours modeled for an agent, else first of multiple =0
	local first_of_multiple = params.first_of_multiple
	local subsequent_of_multiple = params.subsequent_of_multiple

	-- if household has no car available
	local zero_car = params.zero_car

	-- if this work tour is made by public transporation
	-- PT_dummy = 1 * (mode_choice == 1 or mode_choice == 2)
	local PT_dummy = params.PT_dummy

	-- if the the tour is made at usual work location
	local not_usual_dummy = 1- params.go_to_primary_work_location

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
	availability[1] = {
		params.tws_Work_AV,
		params.tws_Education_AV,
		params.tws_Shopping_AV,
		params.tws_Others_AV
		},
	availability[2] = {params.tws_Quit_AV}
}

--scale
local scale={
	{1.81,1.81,1.81,1.81},
	{1}
}

function choose_tws(params,dbparams)
	computeUtilities(params,dbparams) 
	computeAvailabilities(params)
	local probability = calculate_probability("nl", choice, utility, availability, scale)
	return make_final_choice(probability)
end
