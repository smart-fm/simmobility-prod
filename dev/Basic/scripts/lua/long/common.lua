--[[****************************************************************************
    GLOBAL STATIC LOOKUP DATA
******************************************************************************]]
--[[
    Helper function to mark tables as read-only.
]]
function readOnlyTable(table)
   return setmetatable({}, {
     __index = table,
     __newindex = function(table, key, value)
                    error("Attempt to modify read-only table")
                  end,
     __metatable = false
   });
end


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
    return findMaxArgConstrained(func, x0, p1, p2, p3, p4, crit, maxIterations, -math.huge, math.huge)
end

function findMaxArgConstrained(func, x0, p1, p2, p3, p4, crit, maxIterations, lowerLimit, highLimit)
    local x1 = 0
    local delta = 0
    local iters = 0
    local derivative1 = 0
    local derivative2 = 0
        
    repeat
        derivative1 = numerical1Derivative(func, x0, p1, p2, p3, p4, crit)
        derivative2 = numerical2Derivative(func, x0, p1, p2, p3, p4, crit)
        x1 = x0 - (derivative2 == 0 and 0 or (derivative1 / derivative2))
        -- We are searching for a maximum within the range [lowerLimit, highLimit]
        -- if x1 >  highLimit better we have a new maximum.
        -- if x1 <  lowerLimit then we need to re-start with a value within the range [lowerLimit, highLimit]
        delta = math.abs(x1 - x0)
        if (x1 <= lowerLimit and x1 > highLimit) then
           x0 = math.random(lowerLimit, highLimit)
        else
           x0 = x1
        end
        iters=iters+1
    until (delta <= crit or iters >= maxIterations or derivative1 == 0 or derivative2 == 0)
    return x0
end

--[[
    Converts the given square meters value to square foot value.
    @param sqmValue value in square meters.
]]
function sqmToSqf(sqmValue)
    return sqmValue * 10.7639;
end

--[[
    Converts the given square foot value to square meter value.
    @param sqfValue value in square foot.
]]
function sqfToSqm(sqfValue)
    return sqfValue / 10.7639;
end