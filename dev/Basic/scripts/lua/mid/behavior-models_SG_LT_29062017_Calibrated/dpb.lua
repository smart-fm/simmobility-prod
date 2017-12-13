--[[
Model - Day Pattern
Type - Binary
Authors - Adnan

UPDATED VERSION - Adnan

]]

-- require statements do not work with C++. They need to be commented. The order in which lua files are loaded must be explicitly controlled in C++. 
-- require "Logit"

--Estimated values for all betas
--Note: the betas that not estimated are fixed to zero.
--travel constants
local cons_travel = 1.85

--Person type
local beta_homemaker = 0.442
local beta_retired = -0.419
local beta_fulltime = 4.1
local beta_parttime = 3.02
local beta_selfemployed = 2.60
local beta_unemployed = -0.329

--female dummy
local beta_female_travel = 0.348


--Adult age group
local beta_age2025_travel = -2.32
local beta_age2635_travel = -2.87
local beta_age3650_travel = -2.68
local beta_age5165_travel = -2.87
local beta_age65_travel = -3.01



--Adult gender/children
local beta_maleage4_travel = -0.0547
local beta_maleage515_travel = -0.0436
local beta_femalenone_travel = -0.0541
local beta_femaleage4_travel = -0.243
local beta_femaleage515_travel = -0.0453

--Household composition
local beta_onlyadults_travel = 0.326


--Personal income
local beta_income_travel = 0.162
local beta_missingincome_travel = -0.427

--Others
local beta_workathome_travel = -1.66


local beta_twoplus_car_travel = 0.421
local beta_one_car_travel = 0.0779

local beta_motoravail_travel = -0.235


--logsums
local beta_dptour_logsum = 0.227


--choiceset
--1 for notravel; 2 for travel
local choice = { 1, 2 }

local activity_types = { ["Work"] = 1, ["Education"] = 2, ["Shop"] = 3, ["Others"] = 4 }

--utility
local utility = {}
local function computeUtilities(params) 
	-- storing data from params table passed into this function locally for use in this function (this is purely for better execution time)
	local pid = params.person_id
	local person_type_id = params.person_type_id 
	local age_id = params.age_id
	local universitystudent = params.universitystudent
	local onlyadults = params.only_adults
	local onlyworkers = params.only_workers
	local HH_with_under_4 = params.num_underfour
	local HH_with_under_15 = params.presence_of_under15
	local female_dummy = params.female_dummy
	local income_id = params.income_id
	local income_mid = {500.5,1250,1749.5,2249.5,2749.5,3499.5,4499.5,5499.5,6499.5,7499.5,8500,0,99999,99999}
	local missing_income = params.missing_income
	local workathome = params.work_at_home_dummy
	local veh_own_cat = params.vehicle_ownership_category
	local worklogsum = params:activity_logsum(activity_types.Work)
	local edulogsum = params:activity_logsum(activity_types.Education)
	local shoplogsum = params:activity_logsum(activity_types.Shop)
	local otherlogsum = params:activity_logsum(activity_types.Others)
	local dptour_logsum = params.dptour_logsum
	local dpstop_logsum = params.dpstop_logsum

	-- person type related variables
	local fulltime,parttime,selfemployed,homemaker,retired,univ_student,unemployed,nationalservice,voluntary,domestic,otherindividual,student16,student515,child4 = 0,0,0,0,0,0,0,0,0,0,0,0,0,0	
	if person_type_id == 1 then
		fulltime = 1
	elseif person_type_id == 2 then 
		parttime = 1
	elseif person_type_id == 3 then
		selfemployed = 1
	elseif person_type_id == 4 and universitystudent == 1 then 
		univ_student = 1
	elseif person_type_id == 5 then
		homemaker = 1
	elseif person_type_id == 6 then
		retired = 1
	elseif person_type_id == 7 then
		unemployed = 1
	elseif person_type_id == 8 then
		nationalservice = 1
	elseif person_type_id == 9 then
		voluntary = 1
	elseif person_type_id == 10 then
		domestic = 1
	elseif person_type_id == 12 then
		otherindividual = 1
	end 
	if person_type_id == 4 and age_id == 3 then 
		student16 = 1
	elseif person_type_id == 4 and (age_id == 1 or age_id == 2) then
		student515 = 1
	end
	if age_id == 0 then 
		child4 = 1 
	end

	-- age group related variables
	local age20,age2025,age2635,age3650,age5165,age65 = 0,0,0,0,0,0
	if age_id < 4 then 
		age20 = 1
	elseif age_id == 4 then 
		age2025 = 1
	elseif age_id == 5 or age_id == 6 then 
		age2635 = 1
	elseif age_id == 7 or age_id == 8 or age_id == 9 then 
		age3650 = 1
	elseif age_id == 10 or age_id == 11 or age_id == 12 then 
		age5165 = 1
	elseif age_id > 12 then 
		age65 = 1
	end

	-- Adult gender/children related variables
	--HH_with_under_4 is the number
	--However, HH_with_under_15 is binary
	local maleage4,maleage515,malenone,femalenone,femaleage4,femaleage515 = 0,0,0,0,0,0
	if female_dummy == 0 then
		if HH_with_under_4 >= 1 then 
			maleage4 = 1 
		end
		if HH_with_under_4 == 0 and HH_with_under_15 == 1 then 
			maleage515 = 1 
		end
		if onlyadults == 1 then 
			malenone = 1 
		end
	elseif female_dummy == 1 then
		if HH_with_under_4 >= 1 then 
			femaleage4 = 1 
		end
		if HH_with_under_4 == 0 and HH_with_under_15 == 1 then 
			femaleage515 = 1 
		end
		if onlyadults == 1 then 
			femalenone = 1 
		end
	end

	-- income related variables
	local income = income_mid[income_id] * (1 - missing_income)/1000 

	-- other variables
	local zero_car,one_car,twoplus_car,motoravail = 0,0,0,0
	if veh_own_cat == 0 or veh_own_cat == 1 or veh_own_cat ==2 then
		zero_car = 1 
	end
	if veh_own_cat == 1 or veh_own_cat == 2 or veh_own_cat == 4 or veh_own_cat == 5  then 
		motoravail = 1 
	end
	if veh_own_cat == 2 or veh_own_cat == 3 or veh_own_cat == 4  then 
		one_car = 1 
	end
	if veh_own_cat == 5  then 
		twoplus_car = 1 
	end
			
	utility[1] = 0.55
			
	if person_type_id == 11 or person_type_id == 99 then 
		--taking care of excluded individuals at dpbinary level (individuals not eligible for hits interview and also not traveling 
		--person_type_id = 11, age_id=0 for HITS population data
		--person_type_id = 99, age_id=0 or 99 for LT population data. NOTE: LT population dataset has no individual with person_type_id = 11.
		utility[2]=-999
	else	
		utility[2] = cons_travel +  
			beta_female_travel*female_dummy+
			beta_age2025_travel * age2025+
			beta_age2635_travel * age2635 +
			beta_age3650_travel * age3650 +
			beta_age5165_travel * age5165 +
			beta_age65_travel * age65 +
			beta_maleage4_travel * maleage4 +
			beta_maleage515_travel * maleage515 +
			beta_femalenone_travel * femalenone +
			beta_femaleage4_travel * femaleage4 +
			beta_femaleage515_travel * femaleage515 +
			beta_onlyadults_travel * onlyadults +
			beta_income_travel * income +
			beta_missingincome_travel * missing_income + 
			beta_workathome_travel * workathome +
			beta_one_car_travel * one_car +
			beta_twoplus_car_travel * twoplus_car +
			beta_motoravail_travel * motoravail +
			beta_dptour_logsum * (dptour_logsum + dpstop_logsum) +
			beta_homemaker * homemaker + beta_retired * retired + beta_fulltime * fulltime + beta_unemployed * unemployed + beta_selfemployed * selfemployed + beta_parttime * parttime
	end
end



--availability
--availability
local availability={1,1}

--scale
local scale = 1 -- for all choices

-- function to call from C++ preday simulator
-- params table contains data passed from C++
-- to check variable bindings in params, refer PredayLuaModel::mapClasses() function in dev/Basic/medium/behavioral/lua/PredayLuaModel.cpp
function choose_dpb(params)
	computeUtilities(params) 
	local probability = calculate_probability("mnl", choice, utility, availability, scale)
	idx = make_final_choice(probability)
	return choice[idx]
end

-- function to call from C++ preday simulator for logsums computation
-- params table contain person data passed from C++
-- to check variable bindings in params, refer PredayLuaModel::mapClasses() function in dev/Basic/medium/behavioral/lua/PredayLuaModel.cpp
function compute_logsum_dpb(params)
	computeUtilities(params) 
	local probability = calculate_probability("mnl", choice, utility, availability, scale)
	local return_table = {}
	return_table[1] = compute_mnl_logsum(utility, availability)
	return_table[2] = probability[2]
	return return_table
end
