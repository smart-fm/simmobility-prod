
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


function sellerExpectationFunction(price, v, theta, alpha)
    local E = math.exp(1)
    --Calculates the bids distribution using F(X) = X/Price where F(V(t+1)) = V(t+1)/Price
    local bidsDistribution = (v / price)
    --Calculates the probability of not having any bid greater than v.
    local priceProb = math.pow(E, -((theta / math.pow(price, alpha)) * (1 - bidsDistribution)))
    --// Calculates expected maximum bid.
    local p1 = math.pow(price, 2 * alpha + 1)
    local p2 = (price * (theta * math.pow(price, -alpha) - 1))
    local p3 = math.pow(E, (theta * math.pow(price, -alpha)* (bidsDistribution - 1)))
    local p4 = (price - theta * v * math.pow(price, -alpha))
    local expectedMaxBid = (p1 * (p2 + p3 * p4)) / (theta * theta)
    return (v * priceProb + (1 - priceProb) * expectedMaxBid) - (0.01 * price)
end

function arrayTest()
    entry1 = ExpectationEntry()
    entry1.price = 13
    entry1.expectation = 19
    entry2 = ExpectationEntry()
    entry2.price = 14
    entry2.expectation = 18
    local foo = {entry1, entry2}
    return foo
end

print (findMaxArg(sellerExpectationFunction, 20, 4, 1.0, 2.0, nil, 0.001, 100000))

print(arrayTest()[1].price)
print(arrayTest()[2].price)
