--[[
Model - Private Traffic Route Choice	
Type - MNL
Authors - Adnan, Shahita	
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
		local travel_time = params:travel_time(i)
		local travel_cost = params:travel_cost(i)
		local partial_utility = params:partial_utility(i)
		local path_size = params:path_size(i)
		local length = params:length(i)
		local highway_distance = params:highway_distance(i)
		local signal_number = params:signal_number(i)
		local right_turn_number = params:right_turn_number(i)
		local is_min_distance = params:is_min_distance(i)
		local is_min_signal = params:is_min_signal(i)
		local is_max_highway_usage = params:is_max_highway_usage(i)
		local purpose = params:purpose(i)
		local pUtility = 0.0

		if partial_utility > 0.0 then pUtility = partial_utility
		else
			pUtility = pUtility + (path_size)*beta_bCommonFactor
			pUtility = pUtility + (length)*beta_bLength
			pUtility = pUtility + (highway_distance)*beta_bHighway
			if highway_distance > 0 then pUtility = pUtility + beta_highwayBias end
			pUtility = pUtility+(signal_number)*beta_bSigInter
			pUtility = pUtility+(right_turn_number)*beta_bLeftTurns
			if is_min_distance == 1 then pUtility = pUtility+beta_minDistanceParam end
			if is_min_signal == 1 then pUtility = pUtility+beta_minSignalParam end
			if is_max_highway_usage == 1 then pUtility = pUtility+beta_maxHighwayParam end
			if purpose == 1 then pUtility = pUtility+purpose*beta_bWork end
			if purpose == 2 then pUtility = pUtility+purpose*beta_bLeisure end
		
		end	
		utility[i] = pUtility
		utility[i] = utility[i] + travel_time * beta_bTTVOT
		utility[i] = utility[i]+ travel_cost * beta_bCost
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

