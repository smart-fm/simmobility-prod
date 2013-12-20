--[[
Model - intermediate stop time of day
Type - MNL
Authors - Siyu Li, Harish Loganathan
]]

-- all require statements do not work with C++. They need to be commented. The order in which lua files are loaded must be explicitly controlled in C++. 
--require "Logit"

--Estimated values for all betas
--Note= the betas that not estimated are fixed to zero.

local beta_C_1 =0.109
local beta_DUR_1_shopping= 0.0286
local beta_DUR_2_shopping= -0.378 
local beta_DUR_1_edu= -0.0418 
local beta_DUR_3_work= 0.00256 
local beta_DUR_2_work= -0.0483 
local beta_DUR_1_other= -1.96 
local beta_DUR_1_work= -0.865 
local beta_TT= -1.97 
local beta_C_2= -0.511 
local beta_DEP_1_1= -5.94 
local beta_DEP_1_3= -0.765 
local beta_DEP_1_2= -1.54 
local beta_DEP_1_5= 3.32 
local beta_DEP_1_4= 0.258 
local beta_DEP_1_7= 1.26 
local beta_DEP_1_6= 1.53 
local beta_DEP_1_8= 1.0 
local beta_DUR_2_other= 0.0774 
local beta_DUR_3_shopping= 0.02 
local beta_DUR_2_edu= -0.147
local beta_DUR_3_edu= 0.00553 
local beta_ARR_1_8= 0.162
local beta_ARR_1_7= 0.933 
local beta_ARR_1_6= -0.744 
local beta_ARR_1_5= -5.53 
local beta_ARR_1_4= 0.506 
local beta_ARR_1_3= -0.105 
local beta_ARR_1_2= -1.85 
local beta_ARR_1_1= 2.14 
local beta_DUR_3_other= -0.00275


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

for i = 1，48 do
	choiceset[i] = i
end

local utility = {}
local function computeUtilities(params,dbparams)

	local work_stop_dummy = dbparams.stop_type == 1 and 1 or 0
	local edu_stop_dummy = dbparams.stop_type == 2 and 1 or 0
	local shop_stop_dummy = dbparams.stop_type == 3 and 1 or 0
	local other_stop_dummy = dbparams.stop_type ==4 and 1 or 0 

	local income_id = params.income_id
	local income_cat = {500,1250,1750,2250,2750,3500,4500,5500,6500,7500,8500,0,99999,99999}
	local income_mid = income_cat[income_id]
	local missing_income = params.missing_income
	--whether stop is on first half tour or second half tour
	local first_bound = dbparams.first_bound
	local second_bound = dbparams.second_bound 

	local high_tod = dbparams.high_tod
	local low_tod = dbparams.low_tod

	local TT = dbparams:TT
	local cost = dbparams:cost

	local function sarr_1(t):
		return first_bound*beta_ARR_1_1 * math.sin(2*pi*t/24.) + first_bound*beta_ARR_1_5 * math.cos(2*pi*t/24.)+first_bound*beta_ARR_1_2 * math.sin(4*pi*t/24.) + first_bound*beta_ARR_1_6 * math.cos(4*pi*t/24.)+first_bound*beta_ARR_1_3 * math.sin(6*pi*t/24.) + first_bound*beta_ARR_1_7 * math.cos(6*pi*t/24.)+first_bound*beta_ARR_1_4 * math.sin(8*pi*t/24.) + first_bound*beta_ARR_1_8 * math.cos(8*pi*t/24.)
	local function sdep_1(t):
		return second_bound*beta_DEP_1_1 * math.sin(2*pi*t/24.) + second_bound*beta_DEP_1_5 * math.cos(2*pi*t/24.)+second_bound*beta_DEP_1_2 * math.sin(4*pi*t/24.) + second_bound*beta_DEP_1_6 * math.cos(4*pi*t/24.)+second_bound*beta_DEP_1_3 * math.sin(6*pi*t/24.) + second_bound*beta_DEP_1_7 * math.cos(6*pi*t/24.)+second_bound*beta_DEP_1_4 * math.sin(8*pi*t/24.) + second_bound*beta_DEP_1_8 * math.cos(8*pi*t/24.)
	local function sarr_2(t):
		return first_bound*beta_ARR_2_1 * math.sin(2*pi*t/24.) + first_bound*beta_ARR_2_5 * math.cos(2*pi*t/24.)+first_bound*beta_ARR_2_2 * math.sin(4*pi*t/24.) + first_bound*beta_ARR_2_6 * math.cos(4*pi*t/24.)+first_bound*beta_ARR_2_3 * math.sin(6*pi*t/24.) + first_bound*beta_ARR_2_7 * math.cos(6*pi*t/24.)+first_bound*beta_ARR_2_4 * math.sin(8*pi*t/24.) + first_bound*beta_ARR_2_8 * math.cos(8*pi*t/24.)
	local function sdep_2(t):
		return second_bound*beta_DEP_2_1 * math.sin(2*pi*t/24.) + second_bound*beta_DEP_2_5 * math.cos(2*pi*t/24.)+second_bound*beta_DEP_2_2 * math.sin(4*pi*t/24.) + second_bound*beta_DEP_2_6 * math.cos(4*pi*t/24.)+second_bound*beta_DEP_2_3 * math.sin(6*pi*t/24.) + second_bound*beta_DEP_2_7 * math.cos(6*pi*t/24.)+second_bound*beta_DEP_2_4 * math.sin(8*pi*t/24.) + second_bound*beta_DEP_2_8 * math.cos(8*pi*t/24.)
	local function sarr_3(t):
		return first_bound*beta_ARR_3_1 * math.sin(2*pi*t/24.) + first_bound*beta_ARR_3_5 * math.cos(2*pi*t/24.)+first_bound*beta_ARR_3_2 * math.sin(4*pi*t/24.) + first_bound*beta_ARR_3_6 * math.cos(4*pi*t/24.)+first_bound*beta_ARR_3_3 * math.sin(6*pi*t/24.) + first_bound*beta_ARR_3_7 * math.cos(6*pi*t/24.)+first_bound*beta_ARR_3_4 * math.sin(8*pi*t/24.) + first_bound*beta_ARR_3_8 * math.cos(8*pi*t/24.)
	local function sdep_3(t):
		return second_bound*beta_DEP_3_1 * math.sin(2*pi*t/24.) + second_bound*beta_DEP_3_5 * math.cos(2*pi*t/24.)+second_bound*beta_DEP_3_2 * math.sin(4*pi*t/24.) + second_bound*beta_DEP_3_6 * math.cos(4*pi*t/24.)+second_bound*beta_DEP_3_3 * math.sin(6*pi*t/24.) + second_bound*beta_DEP_3_7 * math.cos(6*pi*t/24.)+second_bound*beta_DEP_3_4 * math.sin(8*pi*t/24.) + second_bound*beta_DEP_3_8 * math.cos(8*pi*t/24.)
	local function sarr_4(t):
		return first_bound*beta_ARR_4_1 * math.sin(2*pi*t/24.) + first_bound*beta_ARR_4_5 * math.cos(2*pi*t/24.)+first_bound*beta_ARR_4_2 * math.sin(4*pi*t/24.) + first_bound*beta_ARR_4_6 * math.cos(4*pi*t/24.)+first_bound*beta_ARR_4_3 * math.sin(6*pi*t/24.) + first_bound*beta_ARR_4_7 * math.cos(6*pi*t/24.)+first_bound*beta_ARR_4_4 * math.sin(8*pi*t/24.) + first_bound*beta_ARR_4_8 * math.cos(8*pi*t/24.)
	local function sdep_4(t):
		return second_bound*beta_DEP_4_1 * math.sin(2*pi*t/24.) + second_bound*beta_DEP_4_5 * math.cos(2*pi*t/24.)+second_bound*beta_DEP_4_2 * math.sin(4*pi*t/24.) + second_bound*beta_DEP_4_6 * math.cos(4*pi*t/24.)+second_bound*beta_DEP_4_3 * math.sin(6*pi*t/24.) + second_bound*beta_DEP_4_7 * math.cos(6*pi*t/24.)+second_bound*beta_DEP_4_4 * math.sin(8*pi*t/24.) + second_bound*beta_DEP_4_8 * math.cos(8*pi*t/24.)

	for i =1,48 do
		local arr = arrmidpoint[i]
		local dep = depmidpoint[i]
		local dur = first_bound*(high_tod-i+1)+second_bound*(i-low_tod+1)
		dur = 0.25 + (dur-1)/2
		
		utility[i] = sarr_1(arr) + sdep_1(dep) + work_stop_dummy * (beta_DUR_1_work * dur + beta_DUR_2_work * math.pow(dur,2) + beta_DUR_3_work * math.pow(dur,3)) + edu_stop_dummy * (beta_DUR_1_work * dur + beta_DUR_2_work * math.pow(dur,2) + beta_DUR_3_work * math.pow(dur,3)) + shop_stop_dummy * (beta_DUR_1_work * dur + beta_DUR_2_work * math.pow(dur,2) + beta_DUR_3_work * math.pow(dur,3)) + other_stop_dummy * (beta_DUR_1_work * dur + beta_DUR_2_work * math.pow(dur,2) + beta_DUR_3_work * math.pow(dur,3)) + beta_TT * TT(i) + (1-missing_income) * beta_C_1 * cost(i)/(0.5+income_mid) + missing_income * beta_C_2 *cost(i)

	end

end

--availability
--the logic to determine availability is the same with current implementation
local availability = {}
local function computeAvailabilities(params,dbparams)
	for i = 1, 48 do 
		availability[i] = dbparams:availability(i)
	end
end

--scale
local scale={}
for i = 1, 48 do
	scale[i]=1
end

function choose_itd(params,dbparams)
	computeUtilities(params,dbparams) 
	computeAvailabilities(params,dbparams)
	local probability = calculate_probability("mnl", choice, utility, availability, scale)
	return make_final_choice(probability)
end

