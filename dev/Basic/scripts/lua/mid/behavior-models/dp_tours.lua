--[[
Model - Day Pattern
Type - MNL
Authors - Siyu Li, Harish Loganathan

UPDATED VERSION - Carlos, Adnan, Harish

]]

-- require statements do not work with C++. They need to be commented. The order in which lua files are loaded must be explicitly controlled in C++. 
-- require "Logit"

--Estimated values for all betas
--Note: the betas that not estimated are fixed to zero.
--Tour constants
local beta_tour_work = -10.2
local beta_tour_edu = 10
local beta_tour_shop = -3.77
local beta_tour_others = -3.93

--Person type
local beta_parttime_work = 0
local beta_parttime_edu = 0
local beta_parttime_shop = 0
local beta_parttime_others = 0
local beta_selfemployed_work = -1.33
local beta_selfemployed_edu = 0
local beta_selfemployed_shop = 1.23
local beta_selfemployed_others = 1.44
local beta_universitystudent_work = 0
local beta_universitystudent_edu = 0
local beta_universitystudent_shop = 0
local beta_universitystudent_others = -1.62
local beta_homemaker_work = 0
local beta_homemaker_edu = 0
local beta_homemaker_shop = 3.40
local beta_homemaker_others = 2.49
local beta_retired_work = 0
local beta_retired_edu = 0
local beta_retired_shop = 3.19
local beta_retired_others = 2.92
local beta_unemployed_work = 0
local beta_unemployed_edu = 0
local beta_unemployed_shop = 3.06
local beta_unemployed_others = 3.25
local beta_nationalservice_work = 0
local beta_nationalservice_edu = 0
local beta_nationalservice_shop = 0
local beta_nationalservice_others = 0
local beta_voluntary_work = 0
local beta_voluntary_edu = 0
local beta_voluntary_shop = 0
local beta_voluntary_others = 0
local beta_domestic_work= 0
local beta_domestic_edu = 0
local beta_domestic_shop = 0
local beta_domestic_others = 0
local beta_otherworker_work = 0
local beta_otherworker_edu = 0
local beta_otherworker_shop = 0
local beta_otherworker_others = 0
local beta_student16_work = 0
local beta_student16_edu = 0
local beta_student16_shop = -1.22
local beta_student16_others = -2.35
local beta_student515_work = 0
local beta_student515_edu = 0
local beta_student515_shop = -2.15
local beta_student515_others = -2.15
local beta_child4_work = 0
local beta_child4_edu = 0
local beta_child4_shop = 0
local beta_child4_others = 0

--Adult age group
local beta_age2025_work = -0.649
local beta_age2025_edu = 0
local beta_age2025_shop = -0.372
local beta_age2025_others = 0.0554
local beta_age2635_work = 0.00587
local beta_age2635_edu = -0.0713
local beta_age2635_shop = 0.0574
local beta_age2635_others = 0
local beta_age5165_work = 0
local beta_age5165_edu = 0
local beta_age5165_shop = 0
local beta_age5165_others = 0

--Adult gender/children
local beta_maleage4_work = 0.666
local beta_maleage4_edu = 0
local beta_maleage4_shop = 0.140
local beta_maleage4_others = 0.196
local beta_maleage515_work = 0.0631
local beta_maleage515_edu = 0
local beta_maleage515_shop = -0.322
local beta_maleage515_others = 0.465
local beta_femalenone_work = -0.788
local beta_femalenone_edu = 0
local beta_femalenone_shop = 0.304
local beta_femalenone_others = -0.259
local beta_femaleage4_work = -1.06
local beta_femaleage4_edu = 0
local beta_femaleage4_shop = 0.387
local beta_femaleage4_others = 0.647
local beta_femaleage515_work = -0.984
local beta_femaleage515_edu = 0
local beta_femaleage515_shop = 0.217
local beta_femaleage515_others = 0.611

--Household composition
local beta_onlyadults_work = -0.0894
local beta_onlyadults_edu = 0
local beta_onlyadults_shop = -0.154
local beta_onlyadults_others = 0.633
local beta_onlyworkers_work = 0.434
local beta_onlyworkers_edu = 0
local beta_onlyworkers_shop = 0.581
local beta_onlyworkers_others = -0.539

--Personal income
local beta_income_work = 0.389
local beta_income_edu = 0
local beta_income_shop = -0.123
local beta_income_others = -0.289

local beta_missingincome_work = 0.867
local beta_missingincome_edu = 0
local beta_missingincome_shop = -0.897
local beta_missingincome_others = -1.20

--Others
local beta_workathome_work = 0
local beta_workathome_edu = 0
local beta_workathome_shop = 0
local beta_workathome_others = 0
local beta_caravail_work = 0.280
local beta_caravail_edu = 0
local beta_caravail_shop = -0.00395
local beta_caravail_others = 0.244
local beta_motoravail_work = 1.34
local beta_motoravail_edu = 0
local beta_motoravail_shop = 0.144
local beta_motoravail_others = -0.478

--Additional constants
local beta_onetour = 0
local beta_twotour = 0
local beta_threetour = 0


local beta_work_logsum = 0.552
local beta_edu_logsum = 0
local beta_shopping_logsum = 0
local beta_other_logsum = 0.136

--Combination constants

local beta_workshop_tt = 0
local beta_workothers_tt = 1.29
local beta_edushop_tt =	0
local beta_eduothers_tt = 0
local beta_shopothers_tt = 0

--choiceset
local choice = {
        {1,0,0,0},
        {0,1,0,0},
        {0,0,1,0},
        {0,0,0,1},
        {1,1,0,0},
        {1,0,1,0},
        {1,0,0,1},
        {0,1,1,0},
        {0,1,0,1},
        {0,0,1,1},
        {1,1,0,1},
        {1,0,1,1},
        {1,1,1,0},
        {0,1,1,1}
}        

  -- WorkTi,EduTi,ShopTi,OthersTi 
local WorkT = {1,0,0,0,1,1,1,0,0,0,1,1,1,0}
local EduT = {0,1,0,0,1,0,0,1,1,0,1,0,1,1}
local ShopT = {0,0,1,0,0,1,0,1,0,1,0,1,1,1}
local OthersT = {0,0,0,1,0,0,1,0,1,1,1,1,0,1}


  -- XXXtour_ . . series
local onetour = {1,1,1,1,0,0,0,0,0,0,0,0,0,0}
local twotour = {0,0,0,0,1,1,1,1,1,1,0,0,0,0}
local threetour = {0,0,0,0,0,0,0,0,0,0,1,1,1,1}


 --XXX_tt_ss_ts .. series
local workedu_tt = {0,0,0,0,1,0,0,0,0,0,1,0,1,0}
local workshop_tt = {0,0,0,0,0,1,0,0,0,0,0,1,1,0}
local workothers_tt = {0,0,0,0,0,0,1,0,0,0,1,1,0,0}
local edushop_tt = {0,0,0,0,0,0,0,1,0,0,0,0,1,1}
local eduothers_tt = {0,0,0,0,0,0,0,0,1,0,1,0,0,1}
local shopothers_tt = {0,0,0,0,0,0,0,0,0,1,0,1,0,1}

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

	-- person type related variables
	local fulltime,parttime,selfemployed,homemaker,retired,univ_student,unemployed,nationalservice,voluntary,domestic,otherworker,student16,student515,child4 = 0,0,0,0,0,0,0,0,0,0,0,0,0,0	
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
		otherworker = 1
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
			
	for i = 1,14 do
		utility[i] = 
			beta_tour_work * WorkT[i] + 
			beta_tour_edu * EduT[i] + 
			beta_tour_shop * ShopT[i] + 
			beta_tour_others * OthersT[i] + 
			beta_onetour * onetour[i] +
			beta_twotour * twotour[i] +
			beta_threetour * threetour[i] +
			beta_parttime_work * (WorkT[i] * parttime) +
			beta_parttime_edu * (EduT[i] * parttime) +
			beta_parttime_shop * (ShopT[i] * parttime) +
			beta_parttime_others * (OthersT[i] * parttime) +
			beta_selfemployed_work * (WorkT[i] * selfemployed) +
			beta_selfemployed_edu * (EduT[i] * selfemployed) +
			beta_selfemployed_shop * (ShopT[i] * selfemployed) +
			beta_selfemployed_others * (OthersT[i] * selfemployed) +
			beta_universitystudent_work * (WorkT[i] * univ_student) +
			beta_universitystudent_edu * (EduT[i] * univ_student) +
			beta_universitystudent_shop * (ShopT[i] * univ_student) +
			beta_universitystudent_others * (OthersT[i] * univ_student) +
			beta_homemaker_work * (WorkT[i] * homemaker) +
			beta_homemaker_edu * (EduT[i] * homemaker) +
			beta_homemaker_shop * (ShopT[i] * homemaker) +
			beta_homemaker_others * (OthersT[i] * homemaker) +
			beta_retired_work * (WorkT[i] * retired) +
			beta_retired_edu * (EduT[i] * retired) +
			beta_retired_shop * (ShopT[i] * retired) +
			beta_retired_others * (OthersT[i] * retired) +
			beta_unemployed_work * (WorkT[i] * unemployed) +
			beta_unemployed_edu * (EduT[i] * unemployed) +
			beta_unemployed_shop * (ShopT[i] * unemployed) +
			beta_unemployed_others * (OthersT[i] * unemployed) +
			beta_nationalservice_work * (WorkTi[i] * nationalservice) +
			beta_nationalservice_edu * (EduT[i] * nationalservice) +
			beta_nationalservice_shop * (ShopT[i] * nationalservice) +
			beta_nationalservice_others * (OthersT[i] * nationalservice) +
			beta_voluntary_work * (WorkT[i] * voluntary) +
			beta_voluntary_edu * (EduT[i] * voluntary) +
			beta_voluntary_shop * (ShopT[i] * voluntary) +
			beta_voluntary_others * (OthersT[i] * voluntary) +
			beta_domestic_work * (WorkT[i] * domestic) +
			beta_domestic_edu * (EduT[i] * domestic) +
			beta_domestic_shop * (ShopT[i] * domestic) +
			beta_domestic_others * (OthersT[i] * domestic) +
			beta_otherworker_work * (WorkT[i] * otherworker) +
			beta_otherworker_edu * (EduT[i] * otherworker) +
			beta_otherworker_shop * (ShopT[i] * otherworker) +
			beta_otherworker_others * (OthersT[i] * otherworker) +
			beta_student16_work * (WorkT[i] * student16) +
			beta_student16_edu * (EduT[i] * student16) +
			beta_student16_shop * (ShopT[i] * student16) +
			beta_student16_others * (OthersT[i] * student16) +
			beta_student515_work * (WorkT[i] * student515) +
			beta_student515_edu * (EduT[i] * student515) +
			beta_student515_shop * (ShopT[i] * student515) +
			beta_student515_others * (OthersT[i] * student515) +
			beta_child4_work * (WorkT[i] * child4) +
			beta_child4_edu * (EduT[i] * child4) +
			beta_child4_shop * (ShopT[i] * child4) +
			beta_child4_others * (OthersT[i] * child4) +
			beta_age2025_work * (WorkT[i] * age2025) +
			beta_age2025_edu * (EduT[i] * age2025) +
			beta_age2025_shop * (ShopT[i] * age2025) +
			beta_age2025_others * (OthersT[i] * age2025) +
			beta_age2635_work * (WorkT[i] * age2635) +
			beta_age2635_edu * (EduT[i] * age2635) +
			beta_age2635_shop * (ShopT[i] * age2635) +
			beta_age2635_others * (OthersT[i] * age2635) +
			beta_age5165_work * (WorkT[i] * age5165) +
			beta_age5165_edu * (EduT[i] * age5165) +
			beta_age5165_shop * (ShopT[i] * age5165) +
			beta_age5165_others * (OthersT[i] * age5165) +
			beta_maleage4_work * (WorkT[i] * maleage4) +
			beta_maleage4_edu * (EduT[i] * maleage4) +
			beta_maleage4_shop * (ShopT[i] * maleage4) +
			beta_maleage4_others * (OthersT[i] * maleage4) +
			beta_maleage515_work * (WorkT[i] * maleage515) +
			beta_maleage515_edu * (EduT[i] * maleage515) +
			beta_maleage515_shop * (ShopT[i] * maleage515) +
			beta_maleage515_others * (OthersT[i] * maleage515) +
			beta_femalenone_work * (WorkT[i] * femalenone) +
			beta_femalenone_edu * (EduT[i] * femalenone) +
			beta_femalenone_shop * (ShopT[i] * femalenone) +
			beta_femalenone_others * (OthersT[i] * femalenone) +
			beta_femaleage4_work * (WorkT[i] * femaleage4) +
			beta_femaleage4_edu * (EduT[i] * femaleage4) +
			beta_femaleage4_shop * (ShopT[i] * femaleage4) +
			beta_femaleage4_others * (OthersT[i] * femaleage4) +
			beta_femaleage515_work * (WorkT[i] * femaleage515) +
			beta_femaleage515_edu * (EduT[i] * femaleage515) +
			beta_femaleage515_shop * (ShopT[i] * femaleage515) +
			beta_femaleage515_others * (OthersT[i] * femaleage515) +
			beta_onlyadults_work * (WorkT[i] * onlyadults) +
			beta_onlyadults_edu * (EduT[i] * onlyadults) +
			beta_onlyadults_shop * (ShopT[i] * onlyadults) +
			beta_onlyadults_others * (OthersT[i] * onlyadults) +
			beta_onlyworkers_work * (WorkT[i] * onlyworkers) +
			beta_onlyworkers_edu * (EduT[i] * onlyworkers) +
			beta_onlyworkers_shop * (ShopT[i] * onlyworkers) +
			beta_onlyworkers_others * (OthersT[i] * onlyworkers) +
			beta_income_work * (WorkT[i] * income) +
			beta_income_edu * (EduT[i] * income) +
			beta_income_shop * (ShopT[i] * income) +
			beta_income_others * (OthersT[i] * income) +
			beta_missingincome_work * (WorkT[i] * missing_income) + 
			beta_missingincome_edu * (EduT[i] * missing_income) + 
			beta_missingincome_shop * (ShopT[i] * missing_income) + 
			beta_missingincome_others * (OthersT[i] * missing_income) + 
			beta_workathome_work * (WorkT[i] * workathome) +
			beta_workathome_edu * (EduT[i] * workathome) +
			beta_workathome_shop * (ShopT[i] * workathome) +
			beta_workathome_others * (OthersT[i] * workathome) +
			beta_caravail_work * (WorkT[i] * caravail) +
			beta_caravail_edu * (EduT[i] * caravail) +
			beta_caravail_shop * (ShopT[i] * caravail) +
			beta_caravail_others * (OthersT[i] * caravail) +
			beta_motoravail_work * (WorkT[i] * motoravail) +
			beta_motoravail_edu * (EduT[i] * motoravail) +
			beta_motoravail_shop * (ShopT[i] * motoravail) +
			beta_motoravail_others * (OthersT[i] * motoravail) +
			beta_work_logsum * WorkT[i] * worklogsum +
			beta_edu_logsum * EduT[i] * edulogsum +
			beta_shopping_logsum * ShopT[i] * shoplogsum +
			beta_other_logsum * OthersT[i] * otherlogsum +
			beta_workshop_tt * workshop_tt[i] +
			beta_workothers_tt * workothers_tt[i] +
			beta_edushop_tt * edushop_tt[i] +
			beta_eduothers_tt * eduothers_tt[i] +
			beta_shopothers_tt * shopothers_tt[i] +
			
	end
end

--availability
local availability = {}
local function computeAvailabilities(params)
	-- storing data from params table passed into this function locally for use in this function (this is purely for better execution time)
	local person_type_id = params.person_type_id

	for i = 1,14 do
		-- For Full time student (person_type_id=4): All alternatives are available.
		-- For other person type: only alternatives with EduT=0 (i.e. choice[i][2] = 0) are available to them
		if person_type_id == 4 then
			availability[i] = 1
		else
			if choice[i][2] == 0 then 
				availability[i] = 1
			else
				availability[i] = 0
			end
		end
	end
end

-- scales
local scale = {1,1,1,1,1,1,1,1,1,1,1,1,1,1} -- 1 for all choices (14 1s)

-- function to call from C++ preday simulator
-- params table contains data passed from C++
-- to check variable bindings in params, refer PredayLuaModel::mapClasses() function in dev/Basic/medium/behavioral/lua/PredayLuaModel.cpp
function choose_dp(params)
	computeUtilities(params) 
	computeAvailabilities(params)
	local probability = calculate_probability("mnl", choice, utility, availability, scale)
	idx = make_final_choice(probability)
	return choice[idx]
end
