--ATTENTION requies cant be used with c++ (for now)

package.path = package.path .. ";scripts/lua/long/?.lua;../?.lua"
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
function calculateHDB_HedonicPrice(unit, building, postcode, amenities, logsum, lagCoefficient)
	local simulationYear = CONSTANTS.SIMULATION_YEAR;
	local hedonicPrice = 0; --getStoreyEstimation(unit.storey) + ((building ~= nil) and ((simulationYear - building.builtYear) * -23.26) or 0)

	local ZZ_pms1km   = 0;
	local ZZ_mrt_200m = 0;
	local ZZ_mrt_400m = 0;
	local ZZ_bus_200m = 0;
	local ZZ_bus_400m = 0;
	local ZZ_express_200m = 0;

	local ZZ_logsum = logsum;

	local ZZ_hdb12 = 0;
	local ZZ_hdb3  = 0;
	local ZZ_hdb4  = 0;
	local ZZ_hdb5m = 0;

	local age = 112 - unit.physicalFromYear;
	local age30m = 0;

	if( age > 30 ) then
		age = 30;
		age30m = 1;
	end

	if( age < 0 ) then
		age = 0;
	end

	local ageSquared = age * age;

	local DD_logarea  = math.log(unit.floorArea);
	local ZZ_dis_cbd  = amenities.distanceToCBD;
	local ZZ_dis_mall = amenities.distanceToMall;	

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

	if( amenities.bus_400m == true) then
		ZZ_bus_400m = 1;
	end

	if( unit.unitType <= 2 or unit.unitType == 65 ) then
		ZZ_hdb12 = 1;
	end

	if( unit.unitType == 3 ) then
		ZZ_hdb3 = 1;
	end	
	
	if( unit.unitType == 4 ) then
		ZZ_hdb4 = 1;
	end	

	if( unit.unitType == 5 ) then
		ZZ_hdb5m = 1;
	end

	-----------------------------
	-----------------------------
	if (ZZ_hdb12 == 1) then
		hedonicPrice =   6.848847355 	+ 
				 0.436454323	*	DD_logarea 	+ 
				 1.425775446	*	ZZ_logsum 	+ 
				-0.001812507	*	ZZ_pms1km 	+ 
				-0.017356589	*	ZZ_dis_mall 	+ 
				 0.12593333	*	ZZ_mrt_200m 	+ 
				 0.008691113	*	ZZ_mrt_400m 	+ 
				-0.012386765	*	ZZ_express_200m + 
				 0.045913095	*	ZZ_bus_400m 	+ 
				-0.00112725	*	age 		+ 
				 3.52E-05	*	ageSquared	+ 
				 0.018782865	*	age30m;
	elseif (ZZ_hdb3 == 1) then
		hedonicPrice = -6.621078298 	+ 
				0.770612786	*	DD_logarea	+ 
				6.043678591	*	ZZ_logsum 	+ 
				4.46E-05	*	ZZ_pms1km 	+ 
				-0.012916874	*	ZZ_dis_mall 	+ 
				0.040907994	*	ZZ_mrt_200m 	+ 
				0.039269537	*	ZZ_mrt_400m 	+ 
				-0.016480515	*	ZZ_express_200m + 
				-0.003329949	*	ZZ_bus_400m 	+ 
				-0.022220604	*	age 		+ 
				0.000376281	*	ageSquared	+ 
				0.00442484	*	age30m;
	elseif (ZZ_hdb4 == 1) then
		hedonicPrice =  -11.05822938 	+ 
				0.555391977	*	DD_logarea 	+ 
				8.068356026	*	ZZ_logsum 	+ 
				-0.007935505	*	ZZ_pms1km 	+ 
				-0.015869669	*	ZZ_dis_mall 	+ 
				0.071967105	*	ZZ_mrt_200m 	+ 
			       	0.051345789	*	ZZ_mrt_400m 	+ 
				-0.017976885	*	ZZ_express_200m	+ 
				0.011143845	*	ZZ_bus_400m 	+ 
				-0.027661065	*	age 		+ 
				0.000529645	*	ageSquared	+ 
				0.016603561	*	age30m;
	elseif (ZZ_hdb5m == 1) then
		hedonicPrice = -15.92090409 	+ 
				0.931672979	*	DD_logarea 	+ 
				9.238094212	*	ZZ_logsum 	+ 
				0.00557408	*	ZZ_pms1km 	+	 
				-0.013448111	*	ZZ_dis_mall 	+ 
				0.082384142	*	ZZ_mrt_200m 	+ 
				0.040629054	*	ZZ_mrt_400m 	+ 
				-0.017177823	*	ZZ_express_200m + 
				0.019294964	*	ZZ_bus_400m 	+ 
				-0.032293775	*	age 		+ 
				0.000655319	*	ageSquared	+ 
				0.082523075	*	age30m;
	else 
		hedonicPrice = -10.56097603 	+ 
				0.770965021	*	DD_logarea 	+ 
				7.454534834	*	ZZ_logsum 	+ 
				0.003687724	*	ZZ_pms1km 	+ 
				-0.023468107	*	ZZ_dis_mall 	+	 
				0.078803451	*	ZZ_mrt_200m 	+ 
				0.028197726	*	ZZ_mrt_400m 	+ 
				-0.018227609	*	ZZ_express_200m	+ 
				0.004238152	*	ZZ_bus_400m 	+ 
				-0.000741825	*	age 		+ 
				-5.07E-05	*	ageSquared	+ 
				0.075272042	*	age30m;
	end

	hedonicPrice = hedonicPrice + lagCoefficient;


   
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
function calculatePrivate_HedonicPrice(unit, building, postcode, amenities, logsum, lagCoefficient)
	local hedonicPrice = 0;
	local DD_logarea  = 0;
	local ZZ_dis_cbd  = 0;
	local ZZ_pms1km   = 0;
	local ZZ_dis_mall = 0;
	local ZZ_mrt_200m = 0;
	local ZZ_mrt_400m = 0;
	local ZZ_express_200m = 0;
	local ZZ_bus_200m = 0;

	local ZZ_freehold = 0; 
	local ZZ_logsum = logsum; 
	local ZZ_bus_400m = 0;
	local ZZ_bus_gt400m = 0;

	local age = 112 - unit.physicalFromYear;

	if( age > 25 ) then
	    age = 25;
	end

	if( age < 0 ) then
	    age = 0;
	end


	local ageSquared =  age *  age;
	local agem25_50 = 0;

	if( age > 25 and age < 50 ) then
		agem25_50 = 1;
	end

	local agem50 = 0;

	if( age > 50 ) then
		agem50 = 1;
	end

	local misage = 0;

	DD_logarea  = math.log(unit.floorArea);
	ZZ_dis_cbd  = amenities.distanceToCBD;
	ZZ_dis_mall = amenities.distanceToMall;

	if( amenities.distanceToPMS30 < 1 ) then
		ZZ_pms1km = 1;
	end

	if( amenities.distanceToMRT < 0.200 ) then
		ZZ_mrt_200m = 1;
	elseif( amenities.distanceToMRT < 0.400 ) then
		ZZ_mrt_400m = 1;
	end

	if( amenities.distanceToExpress < 0.200 ) then
		ZZ_express_200m = 1;
	end

	if( amenities.distanceToBus < 0.200 ) then
		ZZ_bus_200m = 1;
	elseif( amenities.distanceToBus < 0.400 ) then
		ZZ_bus_400m = 1;
	else
		ZZ_bus_gt400m = 1;
	end

	-----------------------------
	-----------------------------
	if( (unit.unitType >= 12 and unit.unitType  <= 16 ) or ( unit.unitType >= 32 and unit.unitType  <= 36 ) or ( unit.unitType >= 37 and unit.unitType  <= 51 )) then -- Executive Condominium and Condominium
		hedonicPrice = -25.54928193 	+ 
				0.969230027	*	DD_logarea 	+ 
				0.178603032	*	ZZ_freehold 	+ 
				12.91048136	*	ZZ_logsum 	+ 
				-0.002453952	*	ZZ_pms1km 	+ 
				-0.041115924	*	ZZ_dis_mall 	+ 
				-0.08167267	*	ZZ_mrt_200m 	+ 
				-0.022713371	*	ZZ_mrt_400m 	+ 
				0.053584268	*	ZZ_express_200m + 
				-0.370359	*	ZZ_bus_400m 	+ 
				0.348895837	*	ZZ_bus_gt400m 	+ 
				-0.016519062	*	age 		+ 
				-0.00012883	*	ageSquared	+ 
				0.134831364	*	agem25_50 	+ 
				0.474173547	*	agem50 		+ 
				-0.105709958	*	misage;
	elseif (unit.unitType >= 7 and unit.unitType  <= 11 or unit.unitType == 64) then --"Apartment"
		hedonicPrice = -23.08166378 	+ 
				0.838905717	*	DD_logarea 	+ 
				0.036665412	*	ZZ_freehold 	+ 
				12.19561161	*	ZZ_logsum 	+ 
				0.041185213	*	ZZ_pms1km 	+ 
				-0.108448088	*	ZZ_dis_mall 	+ 
				-0.071465284	*	ZZ_mrt_200m 	+ 
				0.106852355	*	ZZ_mrt_400m 	+ 
				-0.037059068	*	ZZ_express_200m	+ 
				0.096846927	*	ZZ_bus_400m 	+ 
				0.224145375	*	ZZ_bus_gt400m 	+ 
				-0.02603531	*	age 		+ 
				8.67E-05	*	ageSquared	+ 
				0.076709586	*	agem25_50 	+ 
				0.523295627	*	agem50 		+ 
				-0.124793173	*	misage;
	elseif (unit.unitType >= 17 and unit.unitType  <= 21 ) then --"Terrace House"
		hedonicPrice = 	2.115617129 	+ 
				0.468612265	*	DD_logarea 	+ 
				0.135114204	*	ZZ_freehold 	+ 
				3.591528177	*	ZZ_logsum 	+ 
				0.029715641	*	ZZ_pms1km 	+ 
				-0.0026513	*	ZZ_dis_mall 	+ 
				0.008829052	*	ZZ_mrt_200m 	+ 
				0.00153972	*	ZZ_mrt_400m 	+ 
				0.004728968	*	ZZ_express_200m + 
				0.028124909	*	ZZ_bus_400m 	+ 
				0.220693908	*	ZZ_bus_gt400m 	+ 
				-0.031700344	*	age 		+ 
				0.001123349	*	ageSquared	+ 
				-0.165532208	*	agem25_50 	+ 
				0.044950323	*	agem50 		+ 
				-0.1407557	*	misage;
	elseif ( unit.unitType >= 22 and unit.unitType  <= 26 ) then --"Semi-Detached House"	
		hedonicPrice = -19.1340235 	+ 
				0.439412904	*	DD_logarea 	+ 
				0.078858977	*	ZZ_freehold 	+ 
				11.59805493	*	ZZ_logsum 	+ 
				0.019830039	*	ZZ_pms1km 	+ 
				0.00781082	*	ZZ_dis_mall 	+ 
				-0.228086333	*	ZZ_mrt_200m 	+ 
				-0.046377882	*	ZZ_mrt_400m 	+ 
				-0.164674723	*	ZZ_express_200m	+ 
				0.023352674	*	ZZ_bus_400m 	+ 
				0.150093763	*	ZZ_bus_gt400m 	+ 
				-0.025372566	*	age 		+ 
				0.000807147	*	ageSquared	+ 
				-0.166701637	*	agem25_50 	+ 
				0.023981051	*	agem50 		+ 
				-0.088263362	*	misage;
	elseif ( unit.unitType >= 27 and unit.unitType  <= 31 ) then --"Detached House"	
		hedonicPrice = -26.03977798 	+ 
				0.811576331	*	DD_logarea 	+	 
				-0.053529572	*	ZZ_freehold 	+ 
				13.39605601	*	ZZ_logsum 	+ 
				-0.003305346	*	ZZ_pms1km 	+ 
				0.01970628	*	ZZ_dis_mall 	+ 
				0.035937868	*	ZZ_mrt_200m 	+ 
				-0.07363059	*	ZZ_mrt_400m 	+ 
				-0.109209837	*	ZZ_express_200m + 
				0.106419072	*	ZZ_bus_400m 	+ 
				0.254100391	*	ZZ_bus_gt400m 	+ 
				-0.023193191	*	age 		+ 
				0.000394237	*	ageSquared	+ 
				-0.077718103	*	agem25_50 	+ 
				-0.068276746	*	agem50 		+ 
				-0.221324717	*	misage;
	else 
		hedonicPrice = 	 1.608609087 	+ 
				 0.732167784 	* DD_logarea 		+ 
				 0 		* ZZ_freehold 		+ 
				 3.062183727 	* ZZ_logsum 		+ 
				-0.040562506 	* ZZ_pms1km 		+ 
				-0.02150639  	* ZZ_dis_mall 		+ 
			         0 		* ZZ_mrt_200m 		+ 
				 0.001145249 	* ZZ_mrt_400m 	 	+ 
				-0.043523853 	* ZZ_express_200m 	+ 
				-0.014385463 	* ZZ_bus_400m  		+ 
			        -0.033681751 	* ZZ_bus_gt400m 	+ 
				 0.005228958 	* age 			+ 
				-0.000607114 	* ageSquared		+ 
			         0 		* agem25_50 		+ 
				 0 		* agem50 		+ 
				-0.074834872 	* misage;
	end
	------------------------------------------
	------------------------------------------

	hedonicPrice = hedonicPrice + lagCoefficient;

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
function calculateHedonicPrice(unit, building, postcode, amenities, logsum, lagCoefficient )
  
    if unit ~= nil and building ~= nil and postcode ~= nil and amenities ~= nil then
 	if(unit.unitType <= 6 or unit.unitType == 65 ) then
		return calculateHDB_HedonicPrice(unit, building, postcode, amenities, logsum, lagCoefficient) 
	 else 
		return calculatePrivate_HedonicPrice(unit, building, postcode, amenities, logsum, lagCoefficient);
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
function calulateUnitExpectations (unit, timeOnMarket, logsum, lagCoefficient, building, postcode, amenities)
    local expectations = {}
    -- HEDONIC PRICE in SGD in thousands with average hedonic price (500)

    local hedonicPrice = calculateHedonicPrice(unit, building, postcode, amenities, logsum, lagCoefficient)
    hedonicPrice = math.exp( hedonicPrice ) / 1000000;

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
