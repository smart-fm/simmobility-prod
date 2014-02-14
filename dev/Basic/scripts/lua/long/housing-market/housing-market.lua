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
        - floorArea (real)                 : Floor area.
        - storey (integer)                 : Number of storeys.
        - rent (real)                      : Montly rent.

     Entry fields:
        - unit (Unit)                      : Unit object.
        - hedonicPrice (real)              : Unit hedonic price.
        - askingPrice (real)               : Unit asking price.
        - unitId (long integer)            : Unit identifier.

    Building fields:
        - id (long integer)                : Building identifier.
        - builtYear (integer)              : Year when the building was built.
        - landedArea (real)                : Building area.
        - parcelId (long integer)          : Parcel identifier.
        - parkingSpaces (long integer)     : Number of parking spaces available.
        - tenureId (long integer)          : Tenure identifier.
        - typeId (long integer)            : Type identifier.

    Postcode fields:
        - id (long integer)                : Postcode internal identifier.
        - code (string)                    : Real postcode
        - location (Location)              : Location object with location
            - latitude (real)              : Latitude value.
            - longitude (real)             : Longitude value.
        - tazId (long integer)             : Taz id.

    PostcodeAmenities fields:
        - postcode (string)                : Real postcode
        - buildingName (string)            : Building name associated to the postcode.
        - unitBlock (string)               : Block associated to the postcode.
        - roadName (string)                : Road name where the postcode is located.
        - mtzNumber (string)               : MTZ number.
        - mrtStation (string)              : MRT station.
        - distanceToMRT (real)             : Distance to the nearest MRT.        
        - distanceToBus (real)             : Distance to the nearest Bus Stop.        
        - distanceToExpress (real)         : Distance to the nearest Highway.        
        - distanceToPMS30 (real)           : Distance to the nearest Primary school.        
        - distanceToCBD (real)             : Distance to the nearest CBD.        
        - distanceToMall (real)            : Distance to the nearest Mall.        
        - distanceToJob (real)             : Distance to JOB by car.        
        - mrt_200m (boolean)               : Tells if postcode has MRT within 200 meters
        - mrt_400m (boolean)               : Tells if postcode has MRT within 400 meters
        - express_200m (boolean)           : Tells if postcode has Highway within 200 meters
        - bus_200m (boolean)               : Tells if postcode has Bus Stop within 200 meters
        - bus_400m (boolean)               : Tells if postcode has Bus Stop within 400 meters
        - pms_1km (boolean)                : Tells if postcode has Primary School within 1 KM
        - apartment (boolean)              : Tells if postcode is an apartament.
        - condo (boolean)                  : Tells if postcode is a condominium.
        - terrace (boolean)                : Tells if postcode is/has a terrace.
        - semi (boolean)                   : Tells if postcode is a semi
        - ec (boolean)                     : Tells if postcode is a ec
        - private (boolean)                : Tells if postcode is a private unit
        - hdb (boolean)                    : Tells if postcode is a HDB unit
]]

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

--MATH constants.
MATH = readOnlyTable {E = math.exp(1)}

--Simulation constants.
CONSTANTS = readOnlyTable {
  SIMULATION_YEAR   = 2008,
}

CAR_CATEGORIES = readOnlyTable {[1]=true, [6]=true, [7]=true, [8]=true, [9]=true, [16]=true,
                                [17]=true, [18]=true, [22]=true, [23]=true, [24]=true, 
                                [25]=true, [27]=true }
--[[****************************************************************************
    SELLER FUNCTIONS
******************************************************************************]]
function getStoreyEstimation(storey)
    if storey >= 0 and storey <= 5 then return storey
    elseif storey >= 6 and storey <= 10 then return 177.50
    elseif storey >= 11 and storey <= 15 then return 299.00
    elseif storey >= 16 and storey <= 20 then return 581.90
    elseif storey >= 21 and storey <= 25 then return 921.30
    elseif storey >= 26 and storey <= 30 then return 1109.00
    elseif storey >= 31 and storey <= 35 then return 1800.00
    elseif storey >= 36 and storey <= 41 then return 1936.00
    else return 1936.00
    end
end

--[[
    Calculates the hedonic price for the given HDB Unit.
    
    @param unit to calculate the hedonic price.
    @param building where the unit belongs
    @param postcode of the unit.
    @param amenities close to the unit.
    @return hedonic price value.
]]
function calculateHDB_HedonicPrice(unit, building, postcode, amenities)
 local simulationYear = CONSTANTS.SIMULATION_YEAR;
 local hedonicPrice = getStoreyEstimation(unit.storey) + 
                      ((building ~= nil) and 
                            ((simulationYear - building.builtYear)* -17.64) or 0)

 if amenities ~= nil then
    hedonicPrice =  hedonicPrice +
                    (amenities.distanceToCBD * (-76.01) +
                    amenities.distanceToJob * (1.69) +
                    (amenities.pms_1km and 26.78 or 0) +
                    amenities.distanceToMall * (-56.17) +
                    (amenities.mrt_200m and 443.60 or 0) +
                    (amenities.mrt_400m and 273.50 or 0) +
                    (amenities.express_200m and -121.00 or 0) +                    
                    (amenities.bus_200m and 37.30 or 0))
 end
    return hedonicPrice;
end

--[[
    Calculates the hedonic price for the given private Unit.
    
    @param unit to calculate the hedonic price.
    @param building where the unit belongs
    @param postcode of the unit.
    @param amenities close to the unit.
    @return hedonic price value.
]]
function calculatePrivate_HedonicPrice(unit, building, postcode, amenities)
 local hedonicPrice = 0
 if amenities ~= nil then
    hedonicPrice = hedonicPrice +
                    9575.00 + -- intercept
                    (-1239.00) + -- Resale estimation
                    (amenities.distanceToCBD * (-164.80) +
                    amenities.distanceToJob * (15.26) +
                    (amenities.pms_1km and 196.30 or 0) +
                    amenities.distanceToMall * (-362.10) +
                    (amenities.mrt_200m and -841.80 or 0) +
                    (amenities.mrt_400m and 367.10 or 0) +
                    (amenities.express_200m and -545.50 or 0) +                    
                    (amenities.bus_200m and -4215.00 or 0) +
                    (amenities.bus_400m and -3580.00 or 0) +
                    (amenities.condo and 1657.00 or 0) +
                    (amenities.detached and -352.10 or 0) +
                    (amenities.semi and 736.20 or 0) +
                    (amenities.terrace and 911.50 or 0) +
                    (amenities.ec and 1319.00 or 0))
 end
    return hedonicPrice;
end

--[[
    Calculates the hedonic price for the given Unit.
    
    @param unit to calculate the hedonic price.
    @param building where the unit belongs
    @param postcode of the unit.
    @param amenities close to the unit.
]]
function calculateHedonicPrice(unit, building, postcode, amenities)
    if amenities ~= nil then
        return (amenities.hdb) and 
                calculateHDB_HedonicPrice(unit, building, postcode, amenities) or
                calculatePrivate_HedonicPrice(unit, building, postcode, amenities);
    end
    return -1
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
    local E = MATH.E
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
    local maximumBids = 20
    local rateOfBuyers = 1.4
    return (maximumBids-unitBids)*entry.askingPrice/rateOfBuyers
end

--[[
    Calculates the willingness to pay based on Household attributes 
    (and importance) and unit attributes.

    This method calculates the willingness to pay following this formula:

    wp = theta0 + (theta1 * UNIT_AREA * (NUMBER_OF_MEMBERS/INCOME)) + 
        (theta2 * INCOME) + (theta3 (IF HH has cars))

    @param household.
    @param unit to calculate the wp.
    @return value of the willingness to pay of the given household.
]]

function calculateWP (household, unit)
    local theta0 = 11.880
    local theta1 = 13.288
    local theta2 = 0.002
    local theta3 = 8.840

    return theta0 + 
           (theta1 * unit.floorArea * (household.size/household.income)) +
           (theta2 * household.income) +
           (CAR_CATEGORIES[household.vehicleCategoryId] and theta3 or 0)
end