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
	local hedonicPrice = 0;

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

	local DD_logsqrtarea  = math.log( math.sqrt(unit.floorArea));
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
		hedonicPrice =   6.5310301021 	+ 
				0.8613773963 	*	DD_logsqrtarea 	+ 
				0.811106545	*	ZZ_logsum 	+ 
				-0.0085074262	*	ZZ_pms1km 	+ 
				-0.0185456553	*	ZZ_dis_mall 	+ 
				0.1399868038	*	ZZ_mrt_200m 	+ 
				0.0020759659	*	ZZ_mrt_400m 	+ 
				-0.0097096656	*	ZZ_express_200m + 
				0.0478981975	*	ZZ_bus_400m 	+ 
				0.0059314792	*	age 		+ 
				-0.000180216	*	ageSquared	+ 
				0.0256555013	*	age30m;
	elseif (ZZ_hdb3 == 1) then
		hedonicPrice = -7.7081045831 	+ 
				1.4930459921	*	DD_logsqrtarea	+ 
				3.4191163546	*	ZZ_logsum 	+ 
				0.0023263321	*	ZZ_pms1km 	+ 
				-0.0125945483	*	ZZ_dis_mall 	+ 
				0.0342745237	*	ZZ_mrt_200m 	+ 
				0.041827739	*	ZZ_mrt_400m 	+ 
				-0.0180291611	*	ZZ_express_200m + 
				0.0061175243	*	ZZ_bus_400m 	+ 
				-0.0273719681	*	age 		+ 
				0.0005123315	*	ageSquared	+ 
				-0.0008655069	*	age30m;
	elseif (ZZ_hdb4 == 1) then
		hedonicPrice =  -16.3580567974 	+ 
				1.1291639931	*	DD_logsqrtarea 	+ 
				5.2761687931	*	ZZ_logsum 	+ 
				-0.0152024747	*	ZZ_pms1km 	+ 
				-0.0206065721	*	ZZ_dis_mall 	+ 
				0.0745057374	*	ZZ_mrt_200m 	+ 
			       	0.0528127462	*	ZZ_mrt_400m 	+ 
				-0.0122772939	*	ZZ_express_200m	+ 
				0.0063767898	*	ZZ_bus_400m 	+ 
				-0.0296133112	*	age 		+ 
				0.0005875112	*	ageSquared	+ 
				0.0085068925	*	age30m;
	elseif (ZZ_hdb5m == 1) then
		hedonicPrice =  -24.2858586299	+ 
				1.8321634334	*	DD_logsqrtarea 	+ 
				6.5150029612	*	ZZ_logsum 	+ 
				-0.0109540359	*	ZZ_pms1km 	+	 
				-0.0240467625	*	ZZ_dis_mall 	+ 
				0.0753225202	*	ZZ_mrt_200m 	+ 
				0.0433853548	*	ZZ_mrt_400m 	+ 
				-0.016361304	*	ZZ_express_200m + 
				0.0080894577	*	ZZ_bus_400m 	+ 
				-0.0323315744	*	age 		+ 
				0.0006491499	*	ageSquared	+ 
				0.0905129581	*	age30m;
	else 
		hedonicPrice =  -20.691291483	+ 
				1.5644794951	*	DD_logsqrtarea 	+ 
				5.9059531807	*	ZZ_logsum 	+ 
				-0.0167737991	*	ZZ_pms1km 	+ 
				-0.0322549823	*	ZZ_dis_mall 	+	 
				0.0597935913	*	ZZ_mrt_200m 	+ 
				0.0370877628	*	ZZ_mrt_400m 	+ 
				-0.0179136868	*	ZZ_express_200m	+ 
				0.0020874191	*	ZZ_bus_400m 	+ 
				-0.0045397274	*	age 		+ 
				3.76468245285355E-006	*	ageSquared	+ 
				0.0632305412	*	age30m;
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

	DD_logsqrtarea  = math.log(math.sqrt(unit.floorArea));
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
		hedonicPrice =  -34.2789715238	+ 
				1.9543593824	*	DD_logsqrtarea 	+ 
				0.185534802	*	ZZ_freehold 	+ 
				8.4725633834	*	ZZ_logsum 	+ 
				-0.0013503645	*	ZZ_pms1km 	+ 
				-0.0502499853	*	ZZ_dis_mall 	+ 
				-0.0646248265	*	ZZ_mrt_200m 	+ 
				0.0060583414	*	ZZ_mrt_400m 	+ 
				-0.0035296068	*	ZZ_express_200m + 
				0.06315713	*	ZZ_bus_400m 	+ 
				0.4625580825	*	ZZ_bus_gt400m 	+ 
				-0.0162262035	*	age 		+ 
				-6.31981518179487E-005	*	ageSquared	+ 
				0.0663360651	*	agem25_50 	+ 
				0.3712294323	*	agem50 		+ 
				-0.1140053991	*	misage;
	elseif (unit.unitType >= 7 and unit.unitType  <= 11 or unit.unitType == 64) then --"Apartment"
		hedonicPrice =  -35.6211083477	+ 
				1.6860172196	*	DD_logsqrtarea 	+ 
				-0.0196072352	*	ZZ_freehold 	+ 
				8.8540731181	*	ZZ_logsum 	+ 
				0.012462911	*	ZZ_pms1km 	+ 
				-0.1266819716	*	ZZ_dis_mall 	+ 
				-0.0533271461	*	ZZ_mrt_200m 	+ 
				0.0627437655	*	ZZ_mrt_400m 	+ 
				-0.0315005106	*	ZZ_express_200m	+ 
				0.1039117015	*	ZZ_bus_400m 	+ 
				0.2514717677	*	ZZ_bus_gt400m 	+ 
				-0.026829631	*	age 		+ 
				7.55894896563371E-006	*	ageSquared	+ 
				0.1206055026	*	agem25_50 	+ 
				0.5266551424	*	agem50 		+ 
				-0.1323187713	*	misage;
	elseif (unit.unitType >= 17 and unit.unitType  <= 21 ) then --"Terrace House"
		hedonicPrice = 	 -1.6212198517	+ 
				0.9416459241	*	DD_logsqrtarea 	+ 
				0.1214265597	*	ZZ_freehold 	+ 
				2.5867788111	*	ZZ_logsum 	+ 
				0.0411811851	*	ZZ_pms1km 	+ 
				0.0001729015	*	ZZ_dis_mall 	+ 
				0.0199178025	*	ZZ_mrt_200m 	+ 
				0.0389471014	*	ZZ_mrt_400m 	+ 
				-0.0003068885	*	ZZ_express_200m + 
				0.0283373806	*	ZZ_bus_400m 	+ 
				0.2513888642	*	ZZ_bus_gt400m 	+ 
				-0.0346296891	*	age 		+ 
				0.0012772858	*	ageSquared	+ 
				-0.1818059452	*	agem25_50 	+ 
				-0.0309336844	*	agem50 		+ 
				-0.1607324746	*	misage;
	elseif ( unit.unitType >= 22 and unit.unitType  <= 26 ) then --"Semi-Detached House"	
		hedonicPrice = 	-30.0681862696 + 
				0.8777324717	*	DD_logsqrtarea 	+ 
				0.1019885308	*	ZZ_freehold 	+ 
				8.1682835293	*	ZZ_logsum 	+ 
				0.0237208034	*	ZZ_pms1km 	+ 
				0.0043264369	*	ZZ_dis_mall 	+ 
				-0.2410773909	*	ZZ_mrt_200m 	+ 
				-0.0319380609	*	ZZ_mrt_400m 	+ 
				-0.1530572891	*	ZZ_express_200m	+ 
				0.0193466592	*	ZZ_bus_400m 	+ 
				0.1250873123	*	ZZ_bus_gt400m 	+ 
				-0.0178160326	*	age 		+ 
				0.0006145903	*	ageSquared	+ 
				-0.0996274123	*	agem25_50 	+ 
				0.0426239642	*	agem50 		+ 
				-0.0166359598	*	misage;
	elseif ( unit.unitType >= 27 and unit.unitType  <= 31 ) then --"Detached House"	
		hedonicPrice =  -25.3009448577	+ 
				1.6490660756	*	DD_logsqrtarea 	+	 
				-0.1042343778	*	ZZ_freehold 	+ 
				6.8924682898	*	ZZ_logsum 	+ 
				0.0018250513	*	ZZ_pms1km 	+ 
				0.0299415055	*	ZZ_dis_mall 	+ 
				0.0885906365	*	ZZ_mrt_200m 	+ 
				-0.0451837299	*	ZZ_mrt_400m 	+ 
				-0.1107422172	*	ZZ_express_200m + 
				0.1002593268	*	ZZ_bus_400m 	+ 
				0.2618344592	*	ZZ_bus_gt400m 	+ 
				-0.0634903837	*	age 		+ 
				0.0016315979	*	ageSquared	+ 
				-0.0122258749	*	agem25_50 	+ 
				-0.182327548	*	agem50 		+ 
				-0.4809821641	*	misage;
	else 
		hedonicPrice = 	-3.4472485261	+ 
				 1.4596477362	* DD_logsqrtarea 		+ 
				 0		* ZZ_freehold 		+ 
				 2.61076717	* ZZ_logsum 		+ 
				-0.0440980377	* ZZ_pms1km 		+ 
				-0.0180586911	* ZZ_dis_mall 		+ 
			         0		* ZZ_mrt_200m 		+ 
				 0.0031080858	* ZZ_mrt_400m 	 	+ 
				-0.0472882479	* ZZ_express_200m 	+ 
				-0.0306430166	* ZZ_bus_400m  		+ 
			        -0.0664221756	* ZZ_bus_gt400m 	+ 
				 0.0050404258 	* age 			+ 
				-0.0005512869	* ageSquared		+ 
			         0		* agem25_50 		+ 
				 0		* agem50 		+ 
				-0.0709370628	* misage;
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
