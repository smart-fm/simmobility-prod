--[[
Model - Usual/Unusual work place
Type - MNL
Authors - Siyu Li
]]

-- all require statements do not work with C++. They need to be commented. The order in which lua files are loaded must be explicitly controlled in C++. 
--require "Logit"

--Estimated values for all betas
--Note: the betas that not estimated are fixed to zero.

local beta_cons_usual = 1.86
local beta_cons_unusual = 0

local beta_fixedlocation_usual= 0
local beta_fixedlocation_unusual= 0

local beta_fixedtime_usual= 0.153
local beta_fixedtime_unusual= 0

local beta_female_usual= 0.235
local beta_female_unusual= 0

local beta_under3000_usual= 0
local beta_under3000_unusual= 0

local beta_distance_log_usual=-0.074
local beta_distance1_usual= 0
local beta_distance2_usual= 0
local beta_distance3_usual= 0

local beta_distance_log_unusual= 0
local beta_distance1_unusual= 0
local beta_distance2_unusual= 0
local beta_distance3_unusual= 0

local beta_employment_full_usual= 0.0474
local beta_employment_full_unusual= 0

local beta_employment_part_usual= 0.023
local beta_employment_part_unusual= 0

local beta_employment_self_usual= 0.0773
local beta_employment_self_unusual= 0

local beta_work_home_usual=0.806
local beta_work_home_unusual=0

local beta_first_work_usual= -0.663
local beta_first_work_unusual= 0

local beta_sub_work_usual=-0.978
local beta_sub_work_unusual= 0

--choiceset
--1 for usual; 2 for not usual
local choice = {
	1,
	2
}

--utility
local utility = {}
local function computeUtilities(params,dbparams)
	local person_type_id = params.person_type_id
	local income_id = params.income_id
	local fixed_place = params.fixed_place --binary variable
	local fixed_work_hour = params.fixed_work_hour
	local female_dummy = params.female_dummy
	local work_from_home_dummy = params.work_from_home_dummy

	--first of multiple =1 if this work tour is the first work tour
	--of many work tours modeled for an agent, else first of multiple =0
	--subsequent of multiple =1 if this work tour is the subsequent work
	--tour of many work tours modeled for an agent, else first of multiple =0
	local first_of_multiple = params.first_of_multiple
	local subsequent_of_multiple = params.subsequent_of_multiple

	--dbparams.walk_distance1= AM[(origin,destination)]['AM2dis']
	--origin is home mtz, destination is usual work location mtz
	--dbparams.walk_distance2= PM[(destination,origin)]['PM2dis']
	--origin is home mtz, destination is usual work location mtz
	local distance1 = dbparams.distance1
	local distance2 = dbparams.distance2
	local distance = math.max(distance1+distance2,0.1)

	--dbparams.work_op=zone[usual work location mtz]['employment']
	local work_op = dbparams.work_op  

	local low_income=0
	if income_id <= 5 then low_income = 1 end

	local employment = math.log(1+work_op)

	local full_time_dummy,part_time_dummy,self_employed_dummy = 0,0,0
	if person_type_id == 1 then full_time_dummy = 1 end
	if person_type_id == 2 then part_time_dummy = 2 end
	if person_type_id == 3 then self_employed_dummy = 1 end


	utility[1] = beta_cons_usual + beta_fixedlocation_usual * fixed_place + beta_fixedtime_usual * fixed_work_hour + beta_female_usual * female_dummy + beta_under3000_usual * low_income + beta_distance_log_usual * log(distance)+beta_distance1_usual * distance + beta_distance2_usual*math.pow(distance,2) + beta_distance3_usual*math.pow(distance,3) + beta_employment_full_usual * employment * full_time_dummy + beta_employment_part_usual* employment * part_time_dummy + beta_employment_self_usual * employment * self_employed_dummy + beta_work_home_usual * work_from_home_dummy + beta_first_work_usual * first_of_multiple + beta_sub_work_usual * subsequent_of_multiple
	utility[2] = beta_cons_unusual + beta_fixedlocation_unusual * fixed_place + beta_fixedtime_unusual * fixed_work_hour + beta_female_unusual * female_dummy + beta_under3000_unusual * low_income + beta_distance_log_unusual * log(distance)+beta_distance1_unusual * distance + beta_distance2_unusual*math.pow(distance,2) + beta_distance3_unusual*math.pow(distance,3) + beta_employment_full_unusual * employment * full_time_dummy + beta_employment_part_unusual* employment * part_time_dummy + beta_employment_self_unusual * employment * self_employed_dummy + beta_work_home_unusual * work_from_home_dummy + first_work_unusual * first_of_multiple + beta_sub_work_unusual * subsequent_of_multiple
end

--availability
local availability={1,1}

--scale
local scale = {1,1}

function choose_uw(params,dbparams)
	computeUtilities(params,dbparams) 
	local probability = calculate_probability("mnl", choice, utility, availability, scale)
	return make_final_choice(probability)
end

