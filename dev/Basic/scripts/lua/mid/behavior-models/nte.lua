--[[
Model - Exact number of tour
Type - MNL
Authors - Siyu Li, Harish Loganathan
]]

-- all require statements do not work with C++. They need to be commented. The order in which lua files are loaded must be explicitly controlled in C++. 
--require "Logit"

--Estimated values for all betas
--Note: the betas that not estimated are fixed to zero.

--person type
local beta_parttime_edu_2=0
local beta_selfemployed_edu_2 = 0
local beta_universitystudent_edu_2 = 0
local beta_homemaker_edu_2 = 0
local beta_retired_edu_2 = 0
local beta_unemployed_edu_2 = 0
local beta_nationalservice_edu_2 = 0
local beta_voluntary_edu_2 = 0
local beta_domestic_edu_2 = 0
local beta_otherworker_edu_2 = 0
local beta_student16_edu_2 = 0.605
local beta_student515_edu_2 = 2.490
local beta_child4_edu_2 = 0

--Adult age group

local beta_age2025_edu_2 = 0
local beta_age2635_edu_2 = 0
local beta_age5165_edu_2 = 0

--Adult gender/children

local beta_maleage4_edu_2 = 0
local beta_maleage515_edu_2 = 0
local beta_femalenone_edu_2 = 0
local beta_femaleage4_edu_2 = 0
local beta_femaleage515_edu_2 = 0

--Household composition
local beta_onlyadults_edu_2 = 0
local beta_onlyworkers_edu_2 = 0

--Personal income
local beta_income_edu_2 = 0

--Others
local beta_workathome_edu_2 = 0
local beta_caravail_edu_2 = 0
local beta_motoravail_edu_2 = 0
local beta_logsum_edu_2= 0.246
local beta_cons_edu_2=-6.330


--choiceset
local choice = {
		1,
		2
}


--utility
local utility = {}
local function computeUtilities(params) 
	-- storing data from params table passed into this function locally for use in this function (this is purely for better execution time)
	local person_type_id = params.person_type_id
	local age_id = params.age_id
	local universitystudent = params.universitystudent
	local only_adults = params.only_adults
	local only_workers = params.only_workers 
	local num_underfour = params.num_underfour
	local presence_of_under15 = params.presence_of_under15
	local female_dummy = params.female_dummy
	local income_id = params.income_id
	local income_mid = {500,1250,1750,2250,2750,3500,4500,5500,6500,7500,8500,0,99999,99999}
	local work_at_home_dummy = params. work_at_home_dummy
	local car_own_normal = params.car_own_normal
	local car_own_offpeak = params.car_own_offpeak
	local motor_own = params.motor_own
	local worklogsum = params.worklogsum
	local edulogsum = params.edulogsum
	local shoplogsum = params.shoplogsum
	local otherlogsum = params.otherlogsum

	-- person type related variables
	local fulltime,parttime,selfemployed,universitystudent,homemaker,retired,unemployed,nationalservice,voluntary,domestic,otherworker,student16,student515,child4 = 0,0,0,0,0,0,0,0,0,0,0,0,0,0
	if person_type_id == 1 then
		fulltime = 1
	elseif person_type_id == 2 then 
		parttime = 1
	elseif person_type_id == 3 then
		selfemployed = 1
	elseif person_type_id == 4 and universitystudent == 1 then 
		universitystudent = 1
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
	elseif person_type_id == 12 then
		otherworker = 1
	end 
	if person_type_id == 4 and age_id == 3 then 
		student16 = 1
	elseif person_type_id == 4 and (age_id == 1 or age_id == 2) then
		student515 = 1
	end
	if age_id == 0 then child4 = 1 end
	
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

	-- Household Composition related variables
	local onlyadults,onlyworkers,HH_with_under_4,HH_with_under_15 = 0,0,0,0
	if only_adults == 1 then onlyadults = 1 end
	if only_workers == 1 then onlyworkers = 1 end
	if num_underfour >= 1 then HH_with_under_4 = 1 end
	if presence_of_under15 >= 1 then HH_with_under_15 = 1 end

	-- Adult gender/children related variables
	local maleage4,maleage515,malenone,femalenone,femaleage4,femaleage515 = 0,0,0,0,0,0
	if female_dummy == 0 and HH_with_under_4 == 1 then maleage4 = 1 end
	if female_dummy == 0 and HH_with_under_4 == 0 and HH_with_under_15 == 1 then maleage515 = 1 end
	if female_dummy == 0 and onlyadults == 1 then malenone = 1 end
	if female_dummy == 1 and HH_with_under_4 == 1 then femaleage4 = 1 end
	if female_dummy == 1 and HH_with_under_4 == 0 and HH_with_under_15 == 1 then femaleage515 = 1 end
	if female_dummy == 1 and onlyadults == 1 then femalenone = 1 end
	
	-- income related variables
	local income,missingincome = 0,0
	if income_id>=13 then missingincome = 1 end
	income = income_mid[income_id] * (1 - missingincome)

	-- other variables
	local workathome,caravail,motoravail = 0,0,0
	if work_at_home_dummy == 1 then workathome = 1 end
	if car_own_normal == 1 or car_own_offpeak == 1 then caravail = 1 end
	if motor_own == 1 then motoravail = 1 end
	
	utility[1] = 0
	utility[2] = beta_cons_edu_2+beta_parttime_edu_2*parttime+beta_selfemployed_edu_2*selfemployed+beta_universitystudent_edu_2*universitystudent+beta_homemaker_edu_2*homemaker+beta_retired_edu_2*retired+beta_unemployed_edu_2*unemployed+beta_nationalservice_edu_2*nationalservice+beta_voluntary_edu_2*voluntary+beta_domestic_edu_2*domestic+beta_otherworker_edu_2*otherworker+beta_student16_edu_2*student16+beta_student515_edu_2*student515+beta_child4_edu_2*child4+beta_age2025_edu_2*age2025+beta_age2635_edu_2*age2635+beta_age5165_edu_2*age5165+beta_maleage4_edu_2*maleage4+beta_maleage515_edu_2*maleage515+beta_femalenone_edu_2*femalenone+beta_femaleage4_edu_2*femaleage4+beta_femaleage515_edu_2*femaleage515+beta_onlyadults_edu_2*onlyadults+beta_onlyworkers_edu_2*onlyworkers+beta_income_edu_2*income+beta_workathome_edu_2*workathome+beta_caravail_edu_2*caravail+beta_motoravail_edu_2*motoravail+beta_logsum_edu_2*edulogsum

	
end

--availability
local availability = {1,1}


-- scales
local scale = {1,1}

function choose_nte(params)
	computeUtilities(params) 
	local probability = calculate_probability("mnl", choice, utility, availability, scale)
	return make_final_choice(probability)
end
