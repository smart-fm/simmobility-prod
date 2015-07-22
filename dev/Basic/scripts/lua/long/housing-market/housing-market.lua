--ATTENTION requies cant be used with c++ (for now)

package.path = package.path .. ";../scripts/lua/long/?.lua;../?.lua"
require "common"

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
    Following the documentation prices are in (SGD per sqm).

    @param unit to calculate the hedonic price.
    @param building where the unit belongs
    @param postcode of the unit.
    @param amenities close to the unit.
    @return hedonic price value.
]]
function calculateHDB_HedonicPrice(unit, building, postcode, amenities, logsum)
	local simulationYear = CONSTANTS.SIMULATION_YEAR;
	local hedonicPrice = getStoreyEstimation(unit.storey) + ((building ~= nil) and ((simulationYear - building.builtYear) * -23.26) or 0)

	if amenities == nil then
		return 100000000;
	end

	local DD_logarea  = 0;
	local ZZ_dis_cbd  = 0;
	local ZZ_pms1km   = 0;
	local ZZ_dis_mall = 0;
	local ZZ_mrt_200m = 0;
	local ZZ_mrt_400m = 0;
	local ZZ_mrt_800m = 0;
	local ZZ_express_200m = 0;
	local ZZ_bus_200m = 0;

	local ZZ_freehold = 0;
	local ZZ_logsum = logsum;
	local ZZ_bus_400m = 0;

	if( unit.floorArea ~= nil ) then
		DD_logarea  = math.log(unit.floorArea);
	end
	
	if( amenities.distanceToCBD ~= nil ) then
		ZZ_dis_cbd  = amenities.distanceToCBD;
	end

	if( amenities.distanceToMall ~= nil ) then
		ZZ_dis_mall = amenities.distanceToMall;
	end

	if( amenities.pms_1km == true ) then
		ZZ_pms1km = 1;
	end

	if( amenities.mrt_200m == true ) then
		ZZ_mrt_200m = 1;
	end

	if( amenities.mrt_400m == true ) then
		ZZ_mrt_400m = 1;
	end

	if( amenities.express_200m == true ) then
		ZZ_express_200m = 1;
	end

	if( amenities.bus_200m == true ) then
		ZZ_bus_200m = 1;
	end

	if( amenities.bus_400m ) then
		ZZ_bus_400m = 1;
	end

	if(amenities.distanceToMRT ~= nil and amenities.distanceToMRT < 0.8) then
		ZZ_mrt_800m = 1;
	end

	if( unit.unitType < 4 ) then
		ZZ_hdb123 = 1;
	end
	
	if( unit.unitType == 4 ) then
		ZZ_hdb4 = 1;		
	end	

	if( unit.unitType == 5 ) then
		ZZ_hdb5 = 1;
	end

   	if (ZZ_hdb123 == 1) then
		return(-5.6642939 				+
			0.7399724	*	DD_logarea 	+
			5.7134371	*	ZZ_logsum 	+
			0.0066065	*	ZZ_pms1km 	+
			-0.0090966	*	ZZ_dis_mall	+
			0.0490127	*	ZZ_mrt_200m	+
			0.0332196	*	ZZ_mrt_400m	+
			0.0251086	*	ZZ_mrt_800m	+
			-0.0130123	*	ZZ_express_200m	+
			0.0046816	*	ZZ_bus_200m);	
	elseif (ZZ_hdb4 == 1) then
		 return( -9.524393  				+
			  0.391003	*	DD_logarea 	+
			  7.771394	*	ZZ_logsum 	+
			 -0.002164	*	ZZ_pms1km 	+
			 -0.006755	*	ZZ_dis_mall	+
			  0.139794	*	ZZ_mrt_200m	+
			  0.091637	*	ZZ_mrt_400m	+
			  0.019348	*	ZZ_mrt_800m	+
			 -0.017984	*	ZZ_express_200m	+
			 -0.032859	*	ZZ_bus_200m);
	else
		return(-12.987814 +
			  0.262961	*	DD_logarea 	+
			  9.329013	*	ZZ_logsum 	+
			  0.003089	*	ZZ_pms1km 	+
			 -0.002642	*	ZZ_dis_mall	+
			  0.087935	*	ZZ_mrt_200m	+
			  0.033191	*	ZZ_mrt_400m	+
			  0.014972	*	ZZ_mrt_800m	+
			 -0.022576	*	ZZ_express_200m	+
			 -0.031353	*	ZZ_bus_200m);

 	end

   
--print(string.format("HDB Price: %d, dist_job: %s, dist_cdb: %s, pms1KM: %s, dist_mall: %s, mrt_200m: %s, mrt_400m: %s, dist_express_200m: %s, bus_200m: %s"
--, hedonicPrice, (amenities.distanceToJob * (0.001966)), (amenities.distanceToCBD * (-80.4)), (amenities.pms_1km and 25.67 or 0), (amenities.distanceToMall * (-56.46)), (amenities.mrt_200m and 462.90 or 0),
-- menities.mrt_400m and 274.60 or 0), (amenities.express_200m and -140.10 or 0), (amenities.bus_200m and 62.43 or 0)))
    return hedonicPrice;
end

--[[
    Calculates the hedonic price for the given private Unit.
    Following the documentation prices are in (SGD per sqm).
    
    @param unit to calculate the hedonic price.
    @param building where the unit belongs
    @param postcode of the unit.
    @param amenities close to the unit.
    @return hedonic price value.
]]
function calculatePrivate_HedonicPrice(unit, building, postcode, amenities, logsum)
	local hedonicPrice = 0

	if amenities == nil then
		return 100000000;
	end

	local DD_logarea  = 0;
	local ZZ_dis_cbd  = 0;
	local ZZ_pms1km   = 0;
	local ZZ_dis_mall = 0;
	local ZZ_mrt_200m = 0;
	local ZZ_mrt_400m = 0;
	local ZZ_mrt_800m = 0;
	local ZZ_express_200m = 0;
	local ZZ_bus_200m = 0;

	local ZZ_freehold = 1; 
	local ZZ_logsum = logsum;
	local ZZ_bus_400m = 0;

	if( unit.floorArea ~= nil ) then
		DD_logarea  = math.log(unit.floorArea);
	end
	
	if( amenities.distanceToCBD ~= nil ) then
		ZZ_dis_cbd  = amenities.distanceToCBD;
	end

	if( amenities.distanceToMall ~= nil ) then
		ZZ_dis_mall = amenities.distanceToMall;
	end

	if( amenities.pms_1km == true ) then
		ZZ_pms1km = 1;
	end

	if( amenities.mrt_200m == true ) then
		ZZ_mrt_200m = 1;
	end

	if( amenities.mrt_400m == true ) then
		ZZ_mrt_400m = 1;
	end

	if( amenities.express_200m == true ) then
		ZZ_express_200m = 1;
	end

	if( amenities.bus_200m == true ) then
		ZZ_bus_200m = 1;
	end

	if( amenities.bus_400m ) then
		ZZ_bus_400m = 1;
	end

	if(amenities.distanceToMRT ~= nil and amenities.distanceToMRT < 0.8) then
		ZZ_mrt_800m = 1;
	end

	if( (unit.unitType >= 12 and unit.unitType  <= 16 ) or ( unit.unitType >= 32 and unit.unitType  < 36 ) ) then -- Executive Condominium and Condominium	
	  return( -36.748568 			+
		    0.963625 *	DD_logarea	+
	   	    0.187449 *	ZZ_freehold	+
		   17.272551 *	ZZ_logsum	+
   		    0.038230 *	ZZ_pms1km	+
		   -0.036213 *	ZZ_dis_mall	+
 		    0.091531 *	ZZ_mrt_200m	+
		    0.056021 *	ZZ_mrt_400m	+
		   -0.123693 *	ZZ_mrt_800m	+
		   -0.004624 *	ZZ_express_200m	+
		   -0.370359 *	ZZ_bus_200m	+
		   -0.326108 *	ZZ_bus_400m);

	elseif (unit.unitType >= 7 and unit.unitType  <= 11 ) then --"Apartment"	
	  return(-34.306668 +
		   0.678823	*	DD_logarea	+
		   0.106154	*	ZZ_freehold	+
		  16.846582	*	ZZ_logsum	+
		   0.056804	*	ZZ_pms1km	+
		  -0.075085	*	ZZ_dis_mall	+
		  -0.025750	*	ZZ_mrt_200m	+
		   0.118587	*	ZZ_mrt_400m	+
		  -0.134871	*	ZZ_mrt_800m	+
		  -0.066508	*	ZZ_express_200m	+
		  -0.389808	*	ZZ_bus_200m	+
		  -0.291649	*	ZZ_bus_400m);
	
	elseif (unit.unitType >= 17 and unit.unitType  <= 21 ) then --"Terrace House"	
	  return(-8.918093  +
		  0.580383	*	DD_logarea	+
		  0.136135	*	ZZ_freehold	+
		  7.622885	*	ZZ_logsum	+
		  0.009503	*	ZZ_pms1km	+
		 -0.027296	*	ZZ_dis_mall	+
		  0.038081	*	ZZ_mrt_200m	+
		  0.048420	*	ZZ_mrt_400m	+
		 -0.082811	*	ZZ_mrt_800m	+
		 -0.067742	*	ZZ_express_200m	+
		 -0.282542	*	ZZ_bus_200m	+
		 -0.219494	*	ZZ_bus_400m);
	
	elseif ( unit.unitType >= 22 and unit.unitType  <= 26 ) then --"Semi-Detached House"	
	  return(-26.82173  +
		   0.55857	*	DD_logarea	+
		   0.08751	*	ZZ_freehold	+
		  14.30060	*	ZZ_logsum	+
		   0.01432	*	ZZ_pms1km	+
		   0.01622	*	ZZ_dis_mall	+
		  -0.36268	*	ZZ_mrt_200m	+
		   0.01651	*	ZZ_mrt_400m	+
		  -0.10658	*	ZZ_mrt_800m	+
		  -0.11848	*	ZZ_express_200m	+
		  -0.10518	*	ZZ_bus_200m	+
		  -0.0880	*	ZZ_bus_400m);	
	else	
	  return(-30.93807  +
		   0.85347	*	DD_logarea 	+
		  -0.04880	*	ZZ_freehold	+
		  15.27921	*	ZZ_logsum 	+
		  -0.01221	*	ZZ_pms1km 	+
		   0.04148	*	ZZ_dis_mall	+
		   0.14336	*	ZZ_mrt_200m	+
		   0.13774	*	ZZ_mrt_400m	+
		  -0.22627	*	ZZ_mrt_800m	+
		  -0.15577	*	ZZ_express_200m	+
		  -0.22743	*	ZZ_bus_200m	+
		  -0.15131	*	ZZ_bus_400m);
	end


	return hedonicPrice;
end

--[[
    Calculates the hedonic price for the given Unit.
    Following the documentation prices are in (SGD per sqm).
    
    @param unit to calculate the hedonic price.
    @param building where the unit belongs
    @param postcode of the unit.
    @param amenities close to the unit.
]]
function calculateHedonicPrice(unit, building, postcode, amenities, logsum)
    if unit ~= nil and building ~= nil and postcode ~= nil and amenities ~= nil then
         if(amenities.unitType ~= nil and amenities.unitType < 5) then
		return calculateHDB_HedonicPrice(unit, building, postcode, amenities, logsum) 
	 else 
		return calculatePrivate_HedonicPrice(unit, building, postcode, amenities, logsum);
	 end
    end
    return -1
end

--[[
    Calculates a single expectation based on given params.
    
    @param price of the unit.
    @param v is the last expectation.
    @param a is the ratio of events expected by the seller.
    @param b is the importance of the price for seller.
    @param cost
    @return expectation value.
]]
function calculateExpectation(price, v, a, b, cost)
    local E = Math.E
    local rateOfBuyers = a - (b * price)

    --local expectation = price 
    --                    + (math.pow(E,-rateOfBuyers*(price-v)/price)-1 + rateOfBuyers)*price/rateOfBuyers 
    --                    + math.pow(E,-rateOfBuyers)*v 
    --                    - cost

    if (rateOfBuyers > 0) then
        local expectation = price + (math.pow(E,-rateOfBuyers*(price-v)/price)-1) * price/rateOfBuyers - cost
        return expectation
    end

    return v
end


--[[
    Calculates seller expectations for given unit based on timeOnMarket
    that the seller is able to wait until sell the unit.

    @param unit to sell.
    @param timeOnMarket number of expectations which are necessary to calculate.
    @param building where the unit belongs
    @param postcode of the unit.
    @param amenities close to the unit.
    @return array of ExpectationEntry's with N expectations (N should be equal to timeOnMarket).
]]
function calulateUnitExpectations (unit, timeOnMarket, logsum, building, postcode, amenities)
    local expectations = {}
    -- HEDONIC PRICE in SGD in thousands with average hedonic price (500)

    local hedonicPrice = math.exp(calculateHedonicPrice(unit, building, postcode, amenities, logsum))

    if (hedonicPrice > 0) then
        local reservationPrice = hedonicPrice * 0.8  -- IMPORTANT : The reservation price should be less than the hedonic price and the asking price
        local a = 0 -- ratio of events expected by the seller per (considering that the price is 0)
        local b = 1 -- Importance of the price for seller.
       	local cost = 0.0 -- Cost of being in the market
       	local x0 = 0 -- starting point for price search
        local crit = 0.0001 -- criteria
        local maxIterations = 20 --number of iterations 
        for i=1,timeOnMarket do
            a = 1.5 * reservationPrice
            x0 = 1.4 * reservationPrice     
            entry = ExpectationEntry()  --entry is a class initialized to 0, that will hold the hedonic, asking and target prices.
            entry.hedonicPrice = hedonicPrice
            entry.askingPrice = findMaxArgConstrained(calculateExpectation, x0, reservationPrice, a, b, cost, crit, maxIterations, reservationPrice, 1.2 * reservationPrice )
            entry.targetPrice = calculateExpectation(entry.askingPrice, reservationPrice, a, b, cost );
            reservationPrice = entry.targetPrice;
            expectations[i] = entry
        end
    end
    return expectations
end


--[[****************************************************************************
    BIDDER FUNCTIONS
******************************************************************************]]

--[[
    Calculates the speculation for the given unit.
    
    @param entry market entry.
    @param unitBids number of bids (attempts) to this unit.
    @return the surplus for the given unit.
]]
function calculateSpeculation (entry, unitBids)
    local maximumBids = 20
    local a = 800000 --a is the ratio of events expected by the seller.
    local b = 0.3    --b is the importance of the price for seller.
    local c = 1000   --c is the offset of the speculation price in thousands of dollars. 

    return (maximumBids-unitBids) * entry.askingPrice / (a - (b * entry.askingPrice)) * c
end

--[[
    Calculates the willingness to pay based on Household attributes 
    (and importance) and unit attributes.

    This method calculates the willingness to pay following this formula:

    wp = theta0 + (theta1 * UNIT_AREA * (NUMBER_OF_MEMBERS/INCOME)) + 
        (theta2 * INCOME) + (theta3 (IF HH has cars))

    @param household.
    @param unit to calculate the wp.
    @param tazStats with statictics about taz.
    @param amenities postcode amenities information.
    @return value of the willingness to pay of the given household.
]]

function calculateWP (household, unit, tazStats, amenities)
    local b1 = 2.459
    local b2 = 7.116
    local b3 = -0.066
    local b4 = -0.050
    local hasCar = (CAR_CATEGORIES[household.vehicleCategoryId] and 1 or 0)
    local distanceToCBD = 0;
    
    if(amenities ~= nil ) then
		distanceToCBD = amenities.distanceToCBD;
    end

    local x=  ((b1 * sqfToSqm(unit.floorArea) * Math.ln(household.size))                          --  b1 * Area_Per_Unit  *ln(HouseHold_Size) + 
           +(b2 * ((household.income / 1000) / household.size) * (tazStats.hhAvgIncome / 1000))   --  b2 * HouseHold_Income / HouseHold_Size * Zone_Average_Income +
           +(b3 * (distanceToCBD) * hasCar)                                             --  b3 * Distance_to_CBD*(Dummie_if_car) +
           +(b4 * (distanceToCBD) * (1-hasCar)))                                        --  b4 * Distance_to_CBD*(1-Dummie_if_car)
    --print("WP: " .. x)
    --Area_Per_Unit, HouseHold_Size,HouseHold_Income,Distance_to_CBD, Zone_Average_Income, HasCar, WP
    --print ("HH_ID: " .. household.id .."," .. unit.floorArea..",".. household.size .. "," .. household.income .. ","..amenities.distanceToCBD .. ","..tazStats.hhAvgIncome .. ",".. hasCar .."," .. x)
    return x
end
