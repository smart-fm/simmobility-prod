--[[
Model - Public Transport Route Choice	
Type - MNL
Authors - Rui Tan	
]]

--Estimated values for all betas
--Note: the betas that not estimated are fixed to zero.
local beta_in_vehicle = -0.315324638
local beta_walk = -0.6152576346
local beta_wait = -0.4252576346
local beta_no_txf = -4.2752576346
local beta_cost = -0.1254734748
local beta_path_size = 0.8347423654
local beta_bus = 0
local beta_train = 3
local beta_both = 0

--utility
-- utility[i] for choice[i]
local utility = {}
local choice = {}
local availability = {}

local function computeUtilities(params, N_choice)
    local log = math.log

    utility = {}
    choice = {}
    availability = {}
	for i = 1,N_choice do 
		choice[i] = i
		availability[i] = 1
	end
	
	for i = 1,N_choice do
		utility[i] = beta_in_vehicle * params:total_in_vehicle_time(i) / 60
				+ beta_walk * params:total_walk_time(i) / 60
				+ beta_wait * params:total_wait_time(i) / 60
				+ beta_no_txf * params:total_no_txf(i) 
				+ beta_path_size * log(params:total_path_size(i)) 
				+ beta_cost * params:total_cost(i)
	end
end

--scale
local scale= 1 --for all choices

-- function to call from C++ preday simulator
-- params and dbparams tables contain data passed from C++
-- to check variable bindings in params or dbparams, refer PredayLuaModel::mapClasses() function in dev/Basic/medium/behavioral/lua/PredayLuaModel.cpp
function choose_PT_path(params, N_choice)	
	computeUtilities(params, N_choice) 
	local probability = calculate_probability("mnl", choice, utility, availability, scale)
	return make_final_choice(probability)
end
