--[[
Model - Address choice within a zone
Type - logit
Authors - Mohammad Adnan, Vishnu Baburajan
]]

-- all require statements do not work with C++. They need to be commented. The order in which lua files are loaded must be explicitly controlled in C++. 
--require "Logit"

--Estimated values for all betas
--Note: the betas that not estimated are fixed to zero.

local beta_size = -0.99053
local beta_MRTd = 1.6

local choice = {}
local utility = {}
local availability = {}

--utility
local function computeUtilities(dbparams)
	choice = {}
	utility = {}
	availability = {}
	
	local num_addresses = dbparams.num_addresses
	for i = 1, num_addresses do 
		choice[i] = i
		availability[i] = 1
	end

	local distance_mrt = {}
	local distance_bus = {}
	local log = math.log

	for i =1, num_addresses do
		distance_mrt[i] = dbparams:distance_mrt(i)
		distance_bus[i] = dbparams:distance_bus(i)
	end

	--utility function
	for i =1, num_addresses do
		utility[i] = beta_size * log(beta_MRTd*distance_mrt[i] + distance_bus[i])
	end
end

--scale
local scale = 1 --for all choices

-- function to call from C++ preday model
-- params and dbparams tables contain data passed from C++
-- to check variable bindings in params or dbparams, refer PredayLuaModel::mapClasses() function in dev/Basic/medium/behavioral/lua/PredayLuaModel.cpp 
function choose_address(dbparams)
	computeUtilities(dbparams) 
	local probability = calculate_probability("mnl", choice, utility, availability, scale)
	return make_final_choice(probability)
end
