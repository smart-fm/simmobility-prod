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
local cons_travel = 2.08
local cons_notravel = 0

--Person type
local beta_parttime_travel = -0.768
local beta_parttime_notravel=0

local beta_selfemployed_travel = -1.74
local beta_selfemployed_notravel = 0

local beta_universitystudent_travel = 0
local beta_universitystudent_notravel = 0

local beta_homemaker_travel = -4.18
local beta_homemaker_notravel = 0

local beta_retired_travel = -3.84
local beta_retired_notravel = 0

local beta_unemployed_travel = -4.06
local beta_unemployed_notravel = 0

local beta_nationalservice_travel= 0
local beta_nationalservice_notravel= 0

local beta_voluntary_travel = -3.30
local beta_voluntary_notravel = 0

local beta_domestic_travel= -4.33
local beta_domestic_notravel= 0

local beta_otherindividual_travel = -2.37
local beta_otherindividual_notravel = 0

local beta_student16_travel = 0
local beta_student16_notravel = 0

local beta_student515_travel = 0
local beta_student515_notravel = 0

local beta_child4_travel = 0
local beta_child4_notravel = 0


--Adult age group
local beta_age2025_travel = 0.0705
local beta_age2025_notravel = 0

local beta_age2635_travel = 0.160
local beta_age2635_notravel = 0

local beta_age5165_travel = -0.0196
local beta_age5165_notravel = 0

--Adult gender/children
local beta_maleage4_travel = -0.0609
local beta_maleage4_notravel = 0

local beta_maleage515_travel = 0
local beta_maleage515_notravel = 0

local beta_femalenone_travel = 0.331
local beta_femalenone_notravel = 0

local beta_femaleage4_travel = 0.00102
local beta_femaleage4_notravel = 0

local beta_femaleage515_travel = 0.320
local beta_femaleage515_notravel = 0

--Household composition
local beta_onlyadults_travel = 0.213
local beta_onlyadults_notravel = 0

local beta_onlyworkers_travel = 0
local beta_onlyworkers_notravel = 0

--Personal income
local beta_income_travel = 0.112
local beta_income_notravel = 0

local beta_missingincome_travel = 0.725
local beta_missingincome_notravel = 0

--Others
local beta_workathome_travel = -1.75
local beta_workathome_notravel = 0

local beta_caravail_travel = 0.100
local beta_caravail_notravel = 0

local beta_motoravail_travel = 0
local beta_motoravail_notravel = 0

--logsums
local beta_dptour_logsum = 0.413
local beta_dpstop_logsum = 0.413

local beta_work_logsum = 0
local beta_edu_logsum = 0
local beta_shopping_logsum = 0
local beta_other_logsum = 0


--choiceset
--1 for notravel; 2 for travel
local choice = { 1, 2 }

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
	local car_own = params.car_own
	local car_own_normal = params.car_own_normal
	local car_own_offpeak = params.car_own_offpeak
	local motor_own = params.motor_own
	local worklogsum = params.worklogsum
	local edulogsum = params.edulogsum
	local shoplogsum = params.shoplogsum
	local otherlogsum = params.otherlogsum
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
	local caravail,motoravail = 0,0
	if car_own >= 1  then 
		caravail = 1 
	end
	if motor_own >= 1 then 
		motoravail = 1 
	end
			
	utility[1] = cons_notravel +  
			beta_parttime_notravel * parttime +
			beta_selfemployed_notravel * selfemployed +
			beta_universitystudent_notravel * univ_student +
			beta_homemaker_notravel * homemaker +
			beta_retired_notravel * retired +
			beta_unemployed_notravel * unemployed +
			beta_nationalservice_notravel * nationalservice +
			beta_voluntary_notravel * voluntary +
			beta_domestic_notravel * domestic +
			beta_otherindividual_notravel * otherindividual +
			beta_student16_notravel * student16 +
			beta_student515_notravel * student515 +
			beta_child4_notravel * child4 +
			beta_age2025_notravel * age2025 +
			beta_age2635_notravel * age2635 +
			beta_age5165_notravel * age5165 +
			beta_maleage4_notravel * maleage4 +
			beta_maleage515_notravel * maleage515 +
			beta_femalenone_notravel * femalenone +
			beta_femaleage4_notravel * femaleage4 +
			beta_femaleage515_notravel * femaleage515 +
			beta_onlyadults_notravel * onlyadults +
			beta_onlyworkers_notravel * onlyworkers +
			beta_income_notravel * income +
			beta_missingincome_notravel * missing_income + 
			beta_workathome_notravel * workathome +
			beta_caravail_notravel * caravail +
			beta_motoravail_notravel * motoravail
			
	if person_type_id == 11 then --taking care of excluded individuals at dpbinary level (individuals not eligible for hits interview and also not traveling (persontype_id =1, age_id=0))

		utility[2]=-99
	else	
		utility[2] = cons_travel +  
			beta_parttime_travel * parttime +
			beta_selfemployed_travel * selfemployed +
			beta_universitystudent_travel * univ_student +
			beta_homemaker_travel * homemaker +
			beta_retired_travel * retired +
			beta_unemployed_travel * unemployed +
			beta_nationalservice_travel * nationalservice +
			beta_voluntary_travel * voluntary +
			beta_domestic_travel * domestic +
			beta_otherindividual_travel * otherindividual +
			beta_student16_travel * student16 +
			beta_student515_travel * student515 +
			beta_child4_travel * child4 +
			beta_age2025_travel * age2025 +
			beta_age2635_travel * age2635 +
			beta_age5165_travel * age5165 +
			beta_maleage4_travel * maleage4 +
			beta_maleage515_travel * maleage515 +
			beta_femalenone_travel * femalenone +
			beta_femaleage4_travel * femaleage4 +
			beta_femaleage515_travel * femaleage515 +
			beta_onlyadults_travel * onlyadults +
			beta_onlyworkers_travel * onlyworkers +
			beta_income_travel * income +
			beta_missingincome_travel * missing_income + 
			beta_workathome_travel * workathome +
			beta_caravail_travel * caravail +
			beta_motoravail_travel * motoravail +
			beta_work_logsum * worklogsum +
			beta_edu_logsum * edulogsum +
			beta_shopping_logsum * shoplogsum +
			beta_other_logsum * otherlogsum +
			beta_dptour_logsum * dptour_logsum +
			beta_dpstop_logsum * dpstop_logsum		
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

