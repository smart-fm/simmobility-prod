--[[
Model - Tour time of day for work tour
Type - MNL
Authors - Siyu Li, Harish Loganathan
]]

-- all require statements do not work with C++. They need to be commented. The order in which lua files are loaded must be explicitly controlled in C++. 
--require "Logit"

--Estimated values for all betas
--Note: the betas that not estimated are fixed to zero.

local beta_DEP_4_3 = 0.829
local beta_DEP_4_2 = 0.551 
local beta_DEP_4_1 = -1.21
local beta_DEP_4_7 = -0.586 
local beta_DEP_4_6 = 1.23 
local beta_DEP_4_5 = -0.14 
local beta_DEP_4_4 = -0.222 
local beta_DEP_4_8 = -0.654 
local beta_ARR_4_3 = 0.48 
local beta_DEP_1_6 = -2.05 
local beta_DEP_1_7 = 0.622 
local beta_DEP_1_4 = -0.0283
local beta_DEP_1_5 = -3.97 
local beta_DEP_1_2 = -2.15 
local beta_DEP_1_3 = -1.02 
local beta_DEP_1_1 = 1.206 
local beta_DEP_1_8 = 1.06 
local beta_ARR_2_8 = 0.754 
local beta_TT2 =  0.0 
local beta_ARR_4_8 = -0.292 
local beta_ARR_2_7 = 0.722 
local beta_ARR_4_1 = -0.0896 
local beta_ARR_3_8 = -0.472 
local beta_ARR_4_2 = 0.676 
local beta_ARR_4_5 = -2.72
local beta_ARR_4_4 = 0.012 
local beta_ARR_4_7 = -1.25 
local beta_ARR_4_6 = -1.53 
local beta_ARR_3_2 = 0.278 
local beta_ARR_3_3 = 0.461 
local beta_ARR_3_1 = -0.522 
local beta_ARR_3_6 = -1.89 
local beta_ARR_3_7 = -1.27 
local beta_ARR_3_4 = 0.38 
local beta_ARR_3_5 = -2.64 
local beta_ARR_2_3 = -0.385 
local beta_ARR_2_2 = -0.745 
local beta_ARR_2_1 = -1.81 
local beta_ARR_1_8 = -0.272 
local beta_ARR_2_6 = 1.25 
local beta_ARR_2_5 = 1.61 
local beta_ARR_2_4 = -0.53 
local beta_ARR_1_4 = 0.13 
local beta_ARR_1_5 = -0.433 
local beta_ARR_1_6 = -0.0139 
local beta_ARR_1_7 = 0.923
local beta_ARR_1_1 = -3.62 
local beta_ARR_1_2 = -2.44 
local beta_ARR_1_3 = -0.612 
local beta_DEP_3_8 = 0.387 
local beta_DUR_2 = 0.0825
local beta_DUR_3 = -0.00652 
local beta_DUR_1 = 0.947 
local beta_DEP_3_1 = -0.85 
local beta_DEP_3_2 = -0.733 
local beta_DEP_3_3 = -0.584 
local beta_DEP_3_4 = 0.109 
local beta_DEP_3_5 = -1.32 
local beta_DEP_3_6 = 0.0607
local beta_DEP_3_7 = 0.376 
local beta_DEP_2_1 = -1.94 
local beta_TT1 = 0.0 
local beta_DEP_2_3 = 0.519 
local beta_DEP_2_2 = -1.05 
local beta_DEP_2_5 = -2.2 
local beta_DEP_2_4 = 0.282 
local beta_DEP_2_7 = 0.67 
local beta_DEP_2_6 = 1.71
local beta_DEP_2_8 = -0.455 
local beta_C = 0.0

local k = 4
local n = 4
local ps = 3
local pi = math.pi

local Begin={}
local End={}
local choiceset={}
local arrmidpoint = {}
local depmidpoint = {}

for i =1,48 do
	Begin[i] = i
	End[i] = i
	arrmidpoint[i] = i * 0.5 + 2.75
	depmidpoint[i] = i * 0.5 + 2.75
end

for i = 1,1176 do
	choiceset[i] = i
end

local comb = {}
local count = 0

for i=1,48 do
	for j=1,48 do
		if j>=i then
			count=count+1
			comb[count]={i,j}
		end
	end
end



local function sarr_1(t)
	return beta_ARR_1_1 * math.sin(2*pi*t/24.) + beta_ARR_1_5 * math.cos(2*pi*t/24.) + beta_ARR_1_2 * math.sin(4*pi*t/24.) + beta_ARR_1_6 * math.cos(4*pi*t/24.) + beta_ARR_1_3 * math.sin(6*pi*t/24.) + beta_ARR_1_7 * math.cos(6*pi*t/24.) + beta_ARR_1_4 * math.sin(8*pi*t/24.) + beta_ARR_1_8 * math.cos(8*pi*t/24.)
end

local function sdep_1(t)
	return beta_DEP_1_1 * math.sin(2*pi*t/24.) + beta_DEP_1_5 * math.cos(2*pi*t/24.) + beta_DEP_1_2 * math.sin(4*pi*t/24.) + beta_DEP_1_6 * math.cos(4*pi*t/24.) + beta_DEP_1_3 * math.sin(6*pi*t/24.) + beta_DEP_1_7 * math.cos(6*pi*t/24.) + beta_DEP_1_4 * math.sin(8*pi*t/24.) + beta_DEP_1_8 * math.cos(8*pi*t/24.)
end

local function sarr_2(t)
	return beta_ARR_2_1 * math.sin(2*pi*t/24.) + beta_ARR_2_5 * math.cos(2*pi*t/24.) + beta_ARR_2_2 * math.sin(4*pi*t/24.) + beta_ARR_2_6 * math.cos(4*pi*t/24.) + beta_ARR_2_3 * math.sin(6*pi*t/24.) + beta_ARR_2_7 * math.cos(6*pi*t/24.) + beta_ARR_2_4 * math.sin(8*pi*t/24.) + beta_ARR_2_8 * math.cos(8*pi*t/24.)
end

local function sdep_2(t)
	return beta_DEP_2_1 * math.sin(2*pi*t/24.) + beta_DEP_2_5 * math.cos(2*pi*t/24.) + beta_DEP_2_2 * math.sin(4*pi*t/24.) + beta_DEP_2_6 * math.cos(4*pi*t/24.) + beta_DEP_2_3 * math.sin(6*pi*t/24.) + beta_DEP_2_7 * math.cos(6*pi*t/24.) + beta_DEP_2_4 * math.sin(8*pi*t/24.) + beta_DEP_2_8 * math.cos(8*pi*t/24.)
end

local function sarr_3(t)
	return beta_ARR_3_1 * math.sin(2*pi*t/24.) + beta_ARR_3_5 * math.cos(2*pi*t/24.) + beta_ARR_3_2 * math.sin(4*pi*t/24.) + beta_ARR_3_6 * math.cos(4*pi*t/24.) + beta_ARR_3_3 * math.sin(6*pi*t/24.) + beta_ARR_3_7 * math.cos(6*pi*t/24.) + beta_ARR_3_4 * math.sin(8*pi*t/24.) + beta_ARR_3_8 * math.cos(8*pi*t/24.)
end

local function sdep_3(t)
	return beta_DEP_3_1 * math.sin(2*pi*t/24.) + beta_DEP_3_5 * math.cos(2*pi*t/24.) + beta_DEP_3_2 * math.sin(4*pi*t/24.) + beta_DEP_3_6 * math.cos(4*pi*t/24.) + beta_DEP_3_3 * math.sin(6*pi*t/24.) + beta_DEP_3_7 * math.cos(6*pi*t/24.) + beta_DEP_3_4 * math.sin(8*pi*t/24.) + beta_DEP_3_8 * math.cos(8*pi*t/24.)
end

local function sarr_4(t)
	return beta_ARR_4_1 * math.sin(2*pi*t/24.) + beta_ARR_4_5 * math.cos(2*pi*t/24.) + beta_ARR_4_2 * math.sin(4*pi*t/24.) + beta_ARR_4_6 * math.cos(4*pi*t/24.) + beta_ARR_4_3 * math.sin(6*pi*t/24.) + beta_ARR_4_7 * math.cos(6*pi*t/24.) + beta_ARR_4_4 * math.sin(8*pi*t/24.) + beta_ARR_4_8 * math.cos(8*pi*t/24.)
end

local function sdep_4(t)
	return beta_DEP_4_1 * math.sin(2*pi*t/24.) + beta_DEP_4_5 * math.cos(2*pi*t/24.) + beta_DEP_4_2 * math.sin(4*pi*t/24.) + beta_DEP_4_6 * math.cos(4*pi*t/24.) + beta_DEP_4_3 * math.sin(6*pi*t/24.) + beta_DEP_4_7 * math.cos(6*pi*t/24.) + beta_DEP_4_4 * math.sin(8*pi*t/24.) + beta_DEP_4_8 * math.cos(8*pi*t/24.)
end


local utility = {}
local function computeUtilities(params,dbparams) 

	local person_type_id = params.person_type_id 
	-- gender in this model is the same as female_dummy
	local gender = params.female_dummy
	-- work time flexibility 1 for fixed hour, 2 for flexible hour
	local worktime = params.fixed_work_hour

	local cost_HT1_am = dbparams.cost_HT1_am
	local cost_HT1_pm = dbparams.cost_HT1_pm
	local cost_HT1_op = dbparams.cost_HT1_op
	local cost_HT2_am = dbparams.cost_HT2_am
	local cost_HT2_pm = dbparams.cost_HT2_pm
	local cost_HT2_op = dbparams.cost_HT2_op

	local pow = math.pow

	for i =1,1176 do
		local arrid = comb[i][1]
		local depid = comb[i][2]
		local arr = arrmidpoint[arrid]
		local dep = depmidpoint[depid]
		local dur = dep - arr

		local arr_am = 0
		local arr_pm = 0
		local arr_op = 0
		local dep_am = 0
		local dep_pm = 0
		local dep_op = 0

		if arr<9.5 and arr>7.5 then
			arr_am, arr_pm, arr_op = 1, 0, 0
		elseif arr < 19.5 and arr > 17.5 then
			arr_am, arr_pm, arr_op = 0, 1, 0
		else
			arr_am, arr_pm, arr_op = 0, 0, 1
		end
		if dep <9.5 and dep >7.5 then 
			dep_am, dep_pm, dep_op = 1, 0, 0
		elseif dep<19.5 and dep > 17.5 then 
			dep_am, dep_pm, dep_op = 0, 1, 0
		else
			dep_am, dep_pm, dep_op = 0, 0, 1
		end
		utility[i] = sarr_1(arr) + sdep_1(dep) + (person_type_id ~= 1 and 1 or 0) * (sarr_2(arr) + sdep_2(dep)) + gender * (sarr_3(arr) + sdep_3(dep)) + (worktime == 2 and 1 or 0) * (sarr_4(arr) + sdep_4(dep)) + beta_DUR_1 * dur + beta_DUR_2 * pow(dur,2) + beta_DUR_3 * pow(dur,3) + beta_TT1 * dbparams:TT_HT1(arrid) + beta_TT2 * dbparams:TT_HT2(depid) + beta_C * (cost_HT1_am * arr_am + cost_HT1_pm * arr_pm + cost_HT1_op * arr_op + cost_HT2_am * dep_am + cost_HT2_pm * dep_pm + cost_HT2_op * dep_op)
	end
end

--availability
--the logic to determine availability is the same with current implementation
local availability = {}
local function computeAvailabilities(params,dbparams)
	local mode = dbparams.mode
	for i = 1, 1176 do 
		availability[i] = params:getTimeWindowAvailabilityTour(i,mode)
	end
end


--scale
local scale = 1 --for all choices

-- function to call from C++ preday simulator
-- params and dbparams tables contain data passed from C++
-- to check variable bindings in params or dbparams, refer PredayLuaModel::mapClasses() function in dev/Basic/medium/behavioral/lua/PredayLuaModel.cpp
function choose_ttdw(params,dbparams)
	computeUtilities(params,dbparams) 
	computeAvailabilities(params,dbparams)
	local probability = calculate_probability("mnl", choiceset, utility, availability, scale)
	return make_final_choice(probability)
end

