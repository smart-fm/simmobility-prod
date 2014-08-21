--[[
Model - Tour time of day for other tour
Type - MNL
Authors - Siyu Li, Harish Loganathan, Olga Petrik
]]

-- require statements do not work with C++. They need to be commented. The order in which lua files are loaded must be explicitly controlled in C++. 
-- require "Logit"

--Estimated values for all betas
--Note: the betas that not estimated are fixed to zero.

-- ARR_1_sin2pi:
local beta_ARR_1_1 = 2.20 
-- ARR_1_cos2pi:
local beta_ARR_1_3 = -10.3 
-- ARR_1_sin4pi:
local beta_ARR_1_2 = -0.655 
-- ARR 1_cos4pi:
local beta_ARR_1_4 = 0.735 

local beta_ARR_2_1 = 10.1 
local beta_ARR_2_3 = 3.28
local beta_ARR_2_2 = 2.50 
local beta_ARR_2_4 = -3.32 

local beta_ARR_3_1 = -0.0927 
local beta_ARR_3_3 = 1.35 
local beta_ARR_3_2 = 0.684
local beta_ARR_3_4 = 1.10 

local beta_DEP_1_1 = -7.97 
local beta_DEP_1_3 = 10.2 
local beta_DEP_1_2 = 0.851 
local beta_DEP_1_4 = 5.57 

local beta_DEP_2_1 = -5.03 
local beta_DEP_2_3 = -5.19 
local beta_DEP_2_2 = -2.34 
local beta_DEP_2_4 = -3.39 

local beta_DEP_3_1 = -14.1 
local beta_DEP_3_3 = -7.69 
local beta_DEP_3_2 = -4.51 
local beta_DEP_3_4 = 3.06 

local beta_DUR_1 = -0.718
local beta_DUR_2 = -0.342
local beta_DUR_3 = 0.0207

local pi = math.pi
local pow = math.pow
local sin = math.sin
local cos = math.cos

local choiceset={}
local arrmidpoint = {}
local depmidpoint = {}

for i =1,48 do
	arrmidpoint[i] = i * 0.5 + 2.75
	depmidpoint[i] = i * 0.5 + 2.75
end

for i = 1,1176 do
	choiceset[i] = i
end

local comb = {}
local count = 0

for i=1,48 do
	for j=i,48 do
		count=count+1
		comb[count]={i,j}
	end
end

local function sarr_1(t)
	return beta_ARR_1_1 * sin(2*pi*t/24) + beta_ARR_1_3 * cos(2*pi*t/24)+beta_ARR_1_2 * sin(4*pi*t/24) + beta_ARR_1_4 * cos(4*pi*t/24)
end

local function sdep_1(t)
	return beta_DEP_1_1 * sin(2*pi*t/24) + beta_DEP_1_3 * cos(2*pi*t/24)+beta_DEP_1_2 * sin(4*pi*t/24) + beta_DEP_1_4 * cos(4*pi*t/24)
end

local function sarr_2(t)
	return beta_ARR_2_1 * sin(2*pi*t/24) + beta_ARR_2_3 * cos(2*pi*t/24)+beta_ARR_2_2 * sin(4*pi*t/24) + beta_ARR_2_4 * cos(4*pi*t/24)
end

local function sdep_2(t)
	return beta_DEP_2_1 * sin(2*pi*t/24) + beta_DEP_2_3 * cos(2*pi*t/24)+beta_DEP_2_2 * sin(4*pi*t/24) + beta_DEP_2_4 * cos(4*pi*t/24)
end

local function sarr_3(t)
	return beta_ARR_3_1 * sin(2*pi*t/24) + beta_ARR_3_3 * cos(2*pi*t/24)+beta_ARR_3_2 * sin(4*pi*t/24) + beta_ARR_3_4 * cos(4*pi*t/24)
end

local function sdep_3(t)
	return beta_DEP_3_1 * sin(2*pi*t/24) + beta_DEP_3_3 * cos(2*pi*t/24)+beta_DEP_3_2 * sin(4*pi*t/24) + beta_DEP_3_4 * cos(4*pi*t/24)
end

local utility = {}
local function computeUtilities(params,dbparams)
	local female_dummy = params.female_dummy
	local activity_type = dbparams.activity_type
	for i = 1,1176 do
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
		utility[i] = sarr_1(arr) + sdep_1(dep) + (activity_type==1 and 1 or 0) * (sarr_2(arr) + sdep_2(dep)) + female_dummy * (sarr_3(arr) + sdep_3(dep)) + beta_DUR_1 * dur + beta_DUR_2 * pow(dur,2) + beta_DUR_3 * pow(dur,3)
	end
end

--availability
local availability = {}
local function computeAvailabilities(dbparams)
	for i = 1,1176 do 
		availability[i] = dbparams:time_window_availability(i)
	end
end

--scale
local scale = 1 -- for all choices

function choose_sttd(params,dbparams)
	computeUtilities(params,dbparams) 
	computeAvailabilities(dbparams)
	local probability = calculate_probability("mnl", choiceset, utility, availability, scale)
	return make_final_choice(probability)
end
