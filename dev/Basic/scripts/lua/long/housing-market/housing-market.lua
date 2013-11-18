--ATTENTION requies cant be used with c++ (for now)

--package.path = package.path .. ";../?.lua"
--require "common"
--require "tests.classes"

--[[****************************************************************************
    OBJECTS INFORMATION
******************************************************************************]]

--[[
    Household fields:
        - id (long integer)                : Household identifier.
        - lifestyleId (long integer)       : Lifestyle identifier.
        - unitId (long integer)            : Unit identifier.
        - ethnicityId (long integer)       : Ethnicity identifier.
        - vehicleCategoryId (long integer) : Vehicle Category identifier.
        - size (integer)                   : Number of individuals.
        - children (integer)               : Number of children.
        - income (real)                    : Montly income value.
        - housingDuration (integer)        : Number of days living in the unit.
        - workers (integer)                : Number of workers.
        - ageOfHead (integer)              : Age of the hh head individual.

    Unit fields:
        - id (long integer)                : Household identifier.
        - buildingId (long integer)        : Building identifier.
        - typeId (long integer)            : Unit type identifier.
        - postcodeId (long integer)        : Postcode identifier.
        - tazId (long integer)             : TAZ identifier.
        - floorArea (real)                 : Floor area.
        - storey (integer)                 : Number of stories.
        - rent (real)                      : Montly rent.
        - location (Location)              : Location object with location
            - latitude (real)              : Latitude value.
            - longitude (real)             : Longitude value.

     Entry fields:
        - unit (Unit)                      : Unit object.
        - hedonicPrice (real)              : Unit hedonic price.
        - askingPrice (real)               : Unit asking price.
        - unitId (long integer)            : Unit identifier.
]]


--[[****************************************************************************
    SELLER FUNCTIONS
******************************************************************************]]

--[[
    Calculates the hedonic price for the given Unit.
    
    @param unit to get the hedonic price.
    @return hedonic price value.
]]
function calculateHedonicPrice(unit)
    return unit.rent +
           (unit.rent * 1.0 +
            unit.typeId * 1.0 +
            unit.storey * 1.0 +
            unit.floorArea * 1.0)
end

--[[
    Calculates a single expectation based on given params.
    
    @param price of the unit.
    @param v is the last expectation.
    @param theta is the ratio of events expected by the seller.
    @param alpha is the importance of the price for seller.
    @return expectation value.
]]
function calculateExpectation(price, v, theta, alpha)
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


--[[
    Calculates seller expectations for given unit based on timeOnMarket
    that the seller is able to wait until sell the unit.

    @param unit to sell.
    @param timeOnMarket number of expectations which are necessary to calculate.
    @return array of ExpectationEntry's with N expectations (N should be equal to timeOnMarket).
]]
function calulateUnitExpectations (unit, timeOnMarket)
    local expectations = {}
    local price = 20
    local expectation = 4
    local theta = 1.0 -- ratio of events expected by the seller
    local alpha = 2.0 -- Importance of the price for seller.
    for i=1,timeOnMarket do
        entry = ExpectationEntry()
        entry.price = findMaxArg(calculateExpectation,
                price, expectation, theta, alpha, 0, 0.001, 100000)
        entry.expectation = calculateExpectation(entry.price, expectation, theta, alpha);
        expectation = entry.expectation;
        expectations[i] = entry
    end
    return expectations
end


--[[****************************************************************************
    BIDDER FUNCTIONS
******************************************************************************]]

--[[
    Calculates the surplus for the given unit.
    
    surplus = pow(askingPrice, alpha + 1)/ (n_bids * zeta)

    Where:
         n_bids: Represents the number of attempts(bids) that the bidder already did to the specific unit. 
         alpha: Represents the urgency of the household to get the unit. (Household parameter)
         zeta: Represents the relation between quality and price of the unit. (Unit parameter)
    @param entry market entry.
    @param unitBids number of bids (attempts) to this unit.
    @return the surplus for the given unit.
]]
function calculateSurplus (entry, unitBids)
    local urgencyToBuy = 1.0 -- urgency that the bidder has to buy the home.
    local priceQuality = 1.0 -- Importance of the price for the bidder.
    return math.pow(entry.askingPrice, (urgencyToBuy + 1)) / 
           (unitBids * priceQuality)
end

--[[
    Calculates the willingness to pay based on Household attributes 
    (and importance) and unit attributes.

    This method calculates the willingness to pay following this formula:

    wp = (HHIncomeWeight * HHIncome) + (UnitAttrWeight1 * UnitAttr1) + ... 
         (UnitAttrWeightN * UnitAttrN)

    @param household.
    @param unit to calculate the wp.
    @return value of the willingness to pay of the given household.
]]
function calculateWP (household, unit)
    return ((household.income * 1.0)
            + (unit.floorArea * 1.0)
            + (unit.typeId * 1.0)
            + (unit.rent * 1.0)
            + (unit.storey * 1.0));
end

--print (findMaxArg(calculateExpectation,20, 4, 1, 2, nil, 0.001, 100000))
--print (calulateUnitExpectations(nil, 7)[1].price)