--[[
    Lua 5.1 Copyright (C) 1994-2006 Lua.org, PUC-Rio
]]


--[[
    Math helpers
]]

Math = setmetatable({}, {
    __index = function(_, index)
        return math[index]
    end;
    __metatable = "The metatable is locked";
})
 
Math.E = math.exp(1) -- euler's number
Math.PHI = (1 + Math.sqrt(5))/2 -- golden ratio

Math.nan = function(x) -- tests if value is nan
    return (x ~= x)
end

Math.infinite = function(x) -- tests if value is infinite
    return (x == -math.huge or x == math.huge)
end

Math.finite = function(x) -- tests if value is finite
    return (x == x and x > -math.huge and x < math.huge)
end

Math.ln = function(x)
    return math.log(x) / math.log(Math.E)
end

--
 --F'(x) = (f(x + crit) - f(x - crit)) / 2*crit 
 --
function numerical1Derivative(func, x0, p1, p2, p3, p4, crit)
    return ((func((x0 + crit), p1, p2, p3, p4) - func((x0 - crit), p1, p2, p3, p4)) / (2 * crit))
end

--
-- F''(x) = (f(x + crit) - (2 * f(x)) + f(x - crit)) / crit^2
--
-- returns nan or infinite if some error occurs.
--
function numerical2Derivative(func, x0, p1, p2, p3, p4, crit)
    return ((func((x0 + crit), p1, p2, p3, p4) - (2 * func((x0), p1, p2, p3, p4)) + (func((x0 - crit), p1, p2, p3, p4))) / (crit * crit))
end

function findMaxArg(func, x0, p1, p2, p3, p4, crit, maxIterations)
    local x1 = 0
    local delta = 0
    local iters = 0
    repeat
        x1 = x0 - numerical1Derivative(func, x0, p1, p2, p3, p4, crit) / numerical2Derivative(func, x0, p1, p2, p3, p4, crit)
        delta = math.abs(x1 - x0)
        x0 = x1
        iters=iters+1
    until ((delta > crit and iters <= maxIterations) or not Math.finite(delta))
    return Math.finite(delta) and x0 or delta
end
