--[[
Model - Private Traffic Route Choice	
Type - MNL
Authors - Rui Tan	
]]

--Estimated values for all betas
--Note: the betas that not estimated are fixed to zero.

local beta_bTTVOT = -0.01373
local beta_bCommonFactor = 1
local beta_bLength = -0.00103
local beta_bHighway= 0.00052
local beta_bCost = 0.0
local beta_bSigInter = -0.13
local beta_bLeftTurns = 0.0
local beta_bWork = 0.0
local beta_bLeisure = 0.0
local beta_highwayBias = 0.5
local beta_minTravelTimeParam = 0.879
local beta_minDistanceParam = 0.325
local beta_minSignalParam = 0.256
local beta_maxHighwayParam = 0.422

--utility
--utility[i] for choice[i]
local utility = {}
local choice = {}
local availability = {}

local function computeUtilities(params, N_choice)
    utility = {}
    choice = {}
    availability = {}

	for i = 1,N_choice do 
		choice[i] = i
		availability[i] = 1
	end
	for i = 1,N_choice do
		local pUtility = 0.0
		if params:travel_time(i) <= 0 then io.write("generateUtility: invalid single path travelTime\n") end
		if params:partial_utility(i) > 0.0 then pUtility = params:partial_utility(i)
		else
			pUtility = pUtility + (params:path_size(i))*beta_bCommonFactor
			pUtility = pUtility + (params:length(i))*beta_bLength
			pUtility = pUtility + (params:highway_distance(i))*beta_bHighway
			if params:highway_distance(i) > 0 then pUtility = pUtility + beta_highwayBias end
			pUtility = pUtility+(params:signal_number(i))*beta_bSigInter
			pUtility = pUtility+(params:right_turn_number(i))*beta_bLeftTurns
			if params:is_min_distance(i) == 1 then pUtility = pUtility+beta_minDistanceParam end
			if params:is_min_signal(i) == 1 then pUtility = pUtility+beta_minSignalParam end
			if params:is_max_highway_usage(i) == 1 then pUtility = pUtility+beta_maxHighwayParam end
			if params:purpose(i) == 1 then pUtility = pUtility+params:purpose(i)*beta_bWork end
			if params:purpose(i) == 2 then pUtility = pUtility+params:purpose(i)*beta_bLeisure end
		
		end	
		utility[i] = pUtility
		utility[i] = utility[i] + params:travel_time(i) * beta_bTTVOT
		utility[i] = utility[i]+params:travel_cost(i) * beta_bCost
	end
end

--scale
local scale= 1 --for all choices

-- function to call from C++ preday simulator
-- params and dbparams tables contain data passed from C++
-- to check variable bindings in params or dbparams, refer PredayLuaModel::mapClasses() function in dev/Basic/medium/behavioral/lua/PredayLuaModel.cpp
function choose_PVT_path(params, N_choice)	
	computeUtilities(params, N_choice) 
	local probability = calculate_probability("mnl", choice, utility, availability, scale)
	return make_final_choice(probability)
end

