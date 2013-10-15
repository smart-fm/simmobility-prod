--[[
    Lua 5.1 Copyright (C) 1994-2006 Lua.org, PUC-Rio
]]

--
 --F'(x) = (f(x + crit) - f(x - crit)) / 2*crit 
 --
function numerical1Derivative(func, x0, p1, p2, p3, p4, crit)
    return ((func((x0 + crit), p1, p2, p3, p4) - func((x0 - crit), p1, p2, p3, p4)) / (2 * crit))
end

--
--F''(x) = (f(x + crit) - (2 * f(x)) + f(x - crit)) / crit^2
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
    until (delta > crit and iters <= maxIterations)
    return x0
end
