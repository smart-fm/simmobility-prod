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
--stop constants
local beta_stop_work = -9.229
local beta_stop_edu = -4.829
local beta_stop_shop = -5.566
local beta_stop_others = -3.96

--Person type
local beta_parttime_work = 0
local beta_parttime_edu = 0
local beta_parttime_shop = 0
local beta_parttime_others = 0
local beta_selfemployed_work = 0.658
local beta_selfemployed_edu = 0
local beta_selfemployed_shop = 0.299
local beta_selfemployed_others = -0.333
local beta_universitystudent_work = 0
local beta_universitystudent_edu = 0
local beta_universitystudent_shop = 0
local beta_universitystudent_others = 0.144
local beta_homemaker_work = 0
local beta_homemaker_edu = 0
local beta_homemaker_shop = 1.12
local beta_homemaker_others = 0.416
local beta_retired_work = 0
local beta_retired_edu = 0
local beta_retired_shop = 1.0
local beta_retired_others = 0.236
local beta_unemployed_work = 0
local beta_unemployed_edu = 0
local beta_unemployed_shop = 1.14
local beta_unemployed_others = 1.06
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
local beta_student16_shop = 0.236
local beta_student16_others = -0.734
local beta_student515_work = 0
local beta_student515_edu = 0
local beta_student515_shop = -1.51
local beta_student515_others = -1.07
local beta_child4_work = 0
local beta_child4_edu = 0
local beta_child4_shop = 0
local beta_child4_others = 0

--Adult age group
local beta_age2025_work = 0.541
local beta_age2025_edu = 0
local beta_age2025_shop = 0.533
local beta_age2025_others = -0.654
local beta_age2635_work = -0.0821
local beta_age2635_edu = 0
local beta_age2635_shop = 0.295
local beta_age2635_others = 0
local beta_age5165_work = 0
local beta_age5165_edu = 0
local beta_age5165_shop = 0
local beta_age5165_others = 0

--Adult gender/children
local beta_maleage4_work = 0.0552
local beta_maleage4_edu = 0
local beta_maleage4_shop = -1.05
local beta_maleage4_others = 0.268
local beta_maleage515_work = 0.119
local beta_maleage515_edu = 0
local beta_maleage515_shop = -0.871
local beta_maleage515_others = 0.482
local beta_femalenone_work = -0.528
local beta_femalenone_edu = 0
local beta_femalenone_shop = 0.618
local beta_femalenone_others = -0.318
local beta_femaleage4_work = 0.382
local beta_femaleage4_edu = 0
local beta_femaleage4_shop = 0.132
local beta_femaleage4_others = 0.242
local beta_femaleage515_work = -0.343
local beta_femaleage515_edu = 0
local beta_femaleage515_shop = 0.391
local beta_femaleage515_others = 0.285

--Household composition
local beta_onlyadults_work = 0.372
local beta_onlyadults_edu = 0
local beta_onlyadults_shop = -0.173
local beta_onlyadults_others = -0.158
local beta_onlyworkers_work = -0.235
local beta_onlyworkers_edu = 0
local beta_onlyworkers_shop = 0.0257
local beta_onlyworkers_others = 0.408

--Personal income
local beta_income_work = 0.413
local beta_income_edu = 0
local beta_income_shop = 0.147
local beta_income_others = 0.104

local beta_missingincome_work = 0
local beta_missingincome_edu = 0
local beta_missingincome_shop = 0.459
local beta_missingincome_others = 0.544

--Others
local beta_workathome_work = 0
local beta_workathome_edu = 0
local beta_workathome_shop = 0
local beta_workathome_others = 0

local beta_zero_car_work = 0
local beta_zero_car_edu = 0
local beta_zero_car_shop = 0
local beta_zero_car_others = 0

local beta_one_car_work = -0.913
local beta_one_car_edu = 0
local beta_one_car_shop = 0.429
local beta_one_car_others = 1.49

local beta_twoplus_car_work = -1.53
local beta_twoplus_car_edu = 0
local beta_twoplus_car_shop = 0.262
local beta_twoplus_car_others = 1.48

local beta_motoravail_work = -0.485
local beta_motoravail_edu = 0
local beta_motoravail_shop = -0.0377
local beta_motoravail_others = -0.0091

--Additional constants
local beta_onestop = 0
local beta_twostop = 0



local beta_work_logsum = 0.2
local beta_edu_logsum = 0
local beta_shopping_logsum = 0
local beta_other_logsum = 0

--Combination constants

local beta_workedu_ss = 0
local beta_workshop_ss = 0
local beta_workothers_ss = 4.08
local beta_edushop_ss = 0
local beta_eduothers_ss = 0.986
local beta_shopothers_ss = 0

--choiceset
local choice = {
        {0,0,0,0},
        {1,0,0,0},
        {0,1,0,0},
        {0,0,1,0},
        {0,0,0,1},
        {1,0,1,0},
        {1,0,0,1},
        {0,1,1,0},
        {0,1,0,1},
        {0,0,1,1},
        
}        

  -- WorkTi,EduTi,ShopTi,OthersTi 
local WorkI = {0,1,0,0,0,1,1,0,0,0}
local EduI = {0,0,1,0,0,0,0,1,1,0}
local ShopI = {0,0,0,1,0,1,0,1,0,1}
local OthersI = {0,0,0,0,1,0,1,0,1,1}


  -- XXXstop_ . . series
local onestop = {0,1,1,1,1,0,0,0,0,0}
local twostop = {0,0,0,0,0,1,1,1,1,1}


 --XXX_tt_ss_ts .. series
local workedu_ss = {0,0,0,0,0,0,0,0,0,0}
local workshop_ss = {0,0,0,0,0,1,0,0,0,0}
local workothers_ss = {0,0,0,0,0,0,1,0,0,0}
local edushop_ss = {0,0,0,0,0,0,0,1,0,0}
local eduothers_ss = {0,0,0,0,0,0,0,0,1,0}
local shopothers_ss = {0,0,0,0,0,0,0,0,0,1}

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
	local zero_car,one_car,twoplus_car,motoravail = 0,0,0,0
	if veh_own_cat == 1  then 
		zero_car = 1 
	end
	if veh_own_cat == 2 or 3 or 5 or 6  then 
		motoravail = 1 
	end
	if veh_own_cat == 4 or 5  then 
		one_car = 1 
	end
	if veh_own_cat == 6  then 
		twoplus_car = 1 
	end
			
	for i = 1,10 do
		utility[i] = 
			beta_stop_work * WorkI[i] + 
			beta_stop_edu * EduI[i] + 
			beta_stop_shop * ShopI[i] + 
			beta_stop_others * OthersI[i] + 
			beta_onestop * onestop[i] +
			beta_twostop * twostop[i] +
			beta_parttime_work * (WorkI[i] * parttime) +
			beta_parttime_edu * (EduI[i] * parttime) +
			beta_parttime_shop * (ShopI[i] * parttime) +
			beta_parttime_others * (OthersI[i] * parttime) +
			beta_selfemployed_work * (WorkI[i] * selfemployed) +
			beta_selfemployed_edu * (EduI[i] * selfemployed) +
			beta_selfemployed_shop * (ShopI[i] * selfemployed) +
			beta_selfemployed_others * (OthersI[i] * selfemployed) +
			beta_universitystudent_work * (WorkI[i] * univ_student) +
			beta_universitystudent_edu * (EduI[i] * univ_student) +
			beta_universitystudent_shop * (ShopI[i] * univ_student) +
			beta_universitystudent_others * (OthersI[i] * univ_student) +
			beta_homemaker_work * (WorkI[i] * homemaker) +
			beta_homemaker_edu * (EduI[i] * homemaker) +
			beta_homemaker_shop * (ShopI[i] * homemaker) +
			beta_homemaker_others * (OthersI[i] * homemaker) +
			beta_retired_work * (WorkI[i] * retired) +
			beta_retired_edu * (EduI[i] * retired) +
			beta_retired_shop * (ShopI[i] * retired) +
			beta_retired_others * (OthersI[i] * retired) +
			beta_unemployed_work * (WorkI[i] * unemployed) +
			beta_unemployed_edu * (EduI[i] * unemployed) +
			beta_unemployed_shop * (ShopI[i] * unemployed) +
			beta_unemployed_others * (OthersI[i] * unemployed) +
			beta_nationalservice_work * (WorkI[i] * nationalservice) +
			beta_nationalservice_edu * (EduI[i] * nationalservice) +
			beta_nationalservice_shop * (ShopI[i] * nationalservice) +
			beta_nationalservice_others * (OthersI[i] * nationalservice) +
			beta_voluntary_work * (WorkI[i] * voluntary) +
			beta_voluntary_edu * (EduI[i] * voluntary) +
			beta_voluntary_shop * (ShopI[i] * voluntary) +
			beta_voluntary_others * (OthersI[i] * voluntary) +
			beta_domestic_work * (WorkI[i] * domestic) +
			beta_domestic_edu * (EduI[i] * domestic) +
			beta_domestic_shop * (ShopI[i] * domestic) +
			beta_domestic_others * (OthersI[i] * domestic) +
			beta_otherworker_work * (WorkI[i] * otherworker) +
			beta_otherworker_edu * (EduI[i] * otherworker) +
			beta_otherworker_shop * (ShopI[i] * otherworker) +
			beta_otherworker_others * (OthersI[i] * otherworker) +
			beta_student16_work * (WorkI[i] * student16) +
			beta_student16_edu * (EduI[i] * student16) +
			beta_student16_shop * (ShopI[i] * student16) +
			beta_student16_others * (OthersI[i] * student16) +
			beta_student515_work * (WorkI[i] * student515) +
			beta_student515_edu * (EduI[i] * student515) +
			beta_student515_shop * (ShopI[i] * student515) +
			beta_student515_others * (OthersI[i] * student515) +
			beta_child4_work * (WorkI[i] * child4) +
			beta_child4_edu * (EduI[i] * child4) +
			beta_child4_shop * (ShopI[i] * child4) +
			beta_child4_others * (OthersI[i] * child4) +
			beta_age2025_work * (WorkI[i] * age2025) +
			beta_age2025_edu * (EduI[i] * age2025) +
			beta_age2025_shop * (ShopI[i] * age2025) +
			beta_age2025_others * (OthersI[i] * age2025) +
			beta_age2635_work * (WorkI[i] * age2635) +
			beta_age2635_edu * (EduI[i] * age2635) +
			beta_age2635_shop * (ShopI[i] * age2635) +
			beta_age2635_others * (OthersI[i] * age2635) +
			beta_age5165_work * (WorkI[i] * age5165) +
			beta_age5165_edu * (EduI[i] * age5165) +
			beta_age5165_shop * (ShopI[i] * age5165) +
			beta_age5165_others * (OthersI[i] * age5165) +
			beta_maleage4_work * (WorkI[i] * maleage4) +
			beta_maleage4_edu * (EduI[i] * maleage4) +
			beta_maleage4_shop * (ShopI[i] * maleage4) +
			beta_maleage4_others * (OthersI[i] * maleage4) +
			beta_maleage515_work * (WorkI[i] * maleage515) +
			beta_maleage515_edu * (EduI[i] * maleage515) +
			beta_maleage515_shop * (ShopI[i] * maleage515) +
			beta_maleage515_others * (OthersI[i] * maleage515) +
			beta_femalenone_work * (WorkI[i] * femalenone) +
			beta_femalenone_edu * (EduI[i] * femalenone) +
			beta_femalenone_shop * (ShopI[i] * femalenone) +
			beta_femalenone_others * (OthersI[i] * femalenone) +
			beta_femaleage4_work * (WorkI[i] * femaleage4) +
			beta_femaleage4_edu * (EduI[i] * femaleage4) +
			beta_femaleage4_shop * (ShopI[i] * femaleage4) +
			beta_femaleage4_others * (OthersI[i] * femaleage4) +
			beta_femaleage515_work * (WorkI[i] * femaleage515) +
			beta_femaleage515_edu * (EduI[i] * femaleage515) +
			beta_femaleage515_shop * (ShopI[i] * femaleage515) +
			beta_femaleage515_others * (OthersI[i] * femaleage515) +
			beta_onlyadults_work * (WorkI[i] * onlyadults) +
			beta_onlyadults_edu * (EduI[i] * onlyadults) +
			beta_onlyadults_shop * (ShopI[i] * onlyadults) +
			beta_onlyadults_others * (OthersI[i] * onlyadults) +
			beta_onlyworkers_work * (WorkI[i] * onlyworkers) +
			beta_onlyworkers_edu * (EduI[i] * onlyworkers) +
			beta_onlyworkers_shop * (ShopI[i] * onlyworkers) +
			beta_onlyworkers_others * (OthersI[i] * onlyworkers) +
			beta_income_work * (WorkI[i] * income) +
			beta_income_edu * (EduI[i] * income) +
			beta_income_shop * (ShopI[i] * income) +
			beta_income_others * (OthersI[i] * income) +
			beta_missingincome_work * (WorkI[i] * missing_income) + 
			beta_missingincome_edu * (EduI[i] * missing_income) + 
			beta_missingincome_shop * (ShopI[i] * missing_income) + 
			beta_missingincome_others * (OthersI[i] * missing_income) + 
			beta_workathome_work * (WorkI[i] * workathome) +
			beta_workathome_edu * (EduI[i] * workathome) +
			beta_workathome_shop * (ShopI[i] * workathome) +
			beta_workathome_others * (OthersI[i] * workathome) +
			beta_zero_car_work * (WorkI[i] * zero_car) +
			beta_zero_car_edu * (EduI[i] * zero_car) +
			beta_zero_car_shop * (ShopI[i] * zero_car) +
			beta_zero_car_others * (OthersI[i] * zero_car) +
			beta_one_car_work * (WorkI[i] * one_car) +
			beta_one_car_edu * (EduI[i] * one_car) +
			beta_one_car_shop * (ShopI[i] * one_car) +
			beta_one_car_others * (OthersI[i] * one_car) +
			beta_twoplus_car_work * (WorkI[i] * twoplus_car) +
			beta_twoplus_car_edu * (EduI[i] * twoplus_car) +
			beta_twoplus_car_shop * (ShopI[i] * twoplus_car) +
			beta_twoplus_car_others * (OthersI[i] * twoplus_car) +
			beta_motoravail_work * (WorkI[i] * motoravail) +
			beta_motoravail_edu * (EduI[i] * motoravail) +
			beta_motoravail_shop * (ShopI[i] * motoravail) +
			beta_motoravail_others * (OthersI[i] * motoravail) +
			beta_work_logsum * WorkI[i] * worklogsum +
			beta_edu_logsum * EduI[i] * edulogsum +
			beta_shopping_logsum * ShopI[i] * shoplogsum +
			beta_other_logsum * OthersI[i] * otherlogsum +
			beta_workedu_ss * workedu_ss [i]+
			beta_workshop_ss * workshop_ss[i]+
			beta_workothers_ss * workothers_ss[i]+
			beta_edushop_ss * edushop_ss[i] +
			beta_eduothers_ss * eduothers_ss[i] +
			beta_shopothers_ss * shopothers_ss[i]
	end
end

--availability
local availability = {}
local function computeAvailabilities(params)
	-- storing data from params table passed into this function locally for use in this function (this is purely for better execution time)
	local person_type_id = params.person_type_id

	for i = 1,10 do
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
local scale = 1 -- for all choices

-- function to call from C++ preday simulator
-- params table contains data passed from C++
-- to check variable bindings in params, refer PredayLuaModel::mapClasses() function in dev/Basic/medium/behavioral/lua/PredayLuaModel.cpp
function choose_dps(params)
	computeUtilities(params) 
	computeAvailabilities(params)
	local probability = calculate_probability("mnl", choice, utility, availability, scale)
	idx = make_final_choice(probability)
	return choice[idx]
end

-- function to call from C++ preday simulator for logsums computation
-- params table contain person data passed from C++
-- to check variable bindings in params, refer PredayLuaModel::mapClasses() function in dev/Basic/medium/behavioral/lua/PredayLuaModel.cpp
function compute_logsum_dps(params)
	computeUtilities(params) 
	computeAvailabilities(params)
	return compute_mnl_logsum(utility, availability)
end
