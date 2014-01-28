--[[
Description: Probability computation functions for multinomial and nested logit models
Author: Harish Loganathan
]]

local function calculate_multinomial_logit_probability(choices, utility, availables)
	local probability = {}
	local evsum = 0
	local exp = math.exp
	for k,c in ipairs(choices) do
		--if utility is not a number, then availability is 0
		if utility[k] ~= utility[k] then 
			utility[k] = 0
			availables[k] = 0 
		end 
		probability[k] = availables[k] * exp(utility[k])
		evsum = evsum + probability[k]	
	end
	for c in pairs(probability) do
		if (probability[c] ~= 0) then
			probability[c] = probability[c]/evsum
		end
	end
	return probability
end

local function calculate_nested_logit_probability(choiceset, utility, availables, scales)
	local evmu = {}
	local evsum = {}
	local probability = {}
	local exp = math.exp
	local pow = math.pow
	for nest,choices in pairs(choiceset) do
		local mu = scales[nest][1]
		local nest_evsum = 0
		for i,c in ipairs(choices) do
			if utility[c] ~= utility[c] then 
				utility[c] = 0
				availables[c] = 0
			end
			local evmuc = availables[c] * exp(mu*utility[c])
			evmu[c] = evmuc
			nest_evsum = nest_evsum + evmuc
		end
		evsum[nest] = nest_evsum
	end
		
	sum_evsum_pow_muinv = 0
	for nest,val in pairs(evsum) do
		local mu = scales[nest][1]
		sum_evsum_pow_muinv = sum_evsum_pow_muinv + pow(evsum[nest], (1/mu))
	end

	for nest,choices in pairs(choiceset) do
		local mu = scales[nest][1]
		for i,c in ipairs(choices) do
			if evsum[nest] ~= 0 then
				probability[c] = evmu[c] * pow(evsum[nest], (1/mu - 1))/sum_evsum_pow_muinv
			else
				probability[c] = 0
			end
		end
	end
	return probability
end

local function binary_search(a, x)
	local lo = 1
	local hi = #a
	local floor = math.floor
	while lo ~= hi do
		local mid = floor((lo+hi)/2)
		local midval = a[mid]
		if midval > x then 
			hi = mid
		elseif midval < x then
			lo = mid+1
		end
	end
	return hi --or lo since hi == lo is true
end

function calculate_probability(mtype, choiceset, utility, availables, scales)
	local probability = {}
	if mtype == "mnl" then 
		probability = calculate_multinomial_logit_probability(choiceset, utility, availables)
	elseif mtype == "nl" then
		probability = calculate_nested_logit_probability(choiceset,utility,availables,scales)
	else
		error("unknown model type:" .. mtype .. ". Only 'mnl' and 'nl' are currently supported")
	end
	return probability
end

function make_final_choice(probability)	
	local choices = {}
	local choices_prob = {}
	cum_prob = 0
	for c,p in pairs(probability) do
		table.insert(choices, c)
		if(p~=p) then 
			p = 0
		end
		cum_prob = cum_prob + p
		table.insert(choices_prob, cum_prob)
	end
	math.randomseed( os.time() )
	idx = binary_search(choices_prob, math.random()) 
	return choices[idx]
end
