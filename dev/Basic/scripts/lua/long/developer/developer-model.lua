--ATTENTION requies cant be used with c++ (for now)

package.path = package.path .. ";../scripts/lua/long/?.lua;../?.lua"
require "common"

--[[****************************************************************************
    OBJECTS INFORMATION
******************************************************************************]]

--[[

    Unit fields:
        - id (long integer)                : Household identifier.
        - buildingId (long integer)        : Building identifier.
        - typeId (long integer)            : Unit type identifier.
        - postcodeId (long integer)        : Postcode identifier.
        - floorArea (real)                 : Floor area.
        - storey (integer)                 : Number of storeys.
        - rent (real)                      : Montly rent.

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

    PotentialUnit:
        - floorArea (real)                 : Floor area.
        - unitTypeId (long integer)        : Unit type identifier.
        - freehold (boolean)               : Tells if unit is freehold or not.
]]

--[[****************************************************************************
    GLOBAL STATIC LOOKUP DATA
******************************************************************************]]
UNIT_BASEVALUES = readOnlyTable 
{
    [1]=2525, -- Terraced houses 
    [2]=2775, -- Semi-Detached houses
    [3]=3500, -- Detached houses
    [4]=1975, -- Apartement
    [5]=1800, -- Executive Condo
    [6]=2500,  -- Condo
}

UNIT_TYPE_COEFFICIENTS = readOnlyTable 
{
    [1]=
        {   -- Terraced houses 
            [0]  = 11.540,  -- b0 constant
            [1]  = 0.477,   -- b1 unit floor area (m^2) 
            [2]  = 0.091,   -- b2 freehold (boolean)
            [3]  = 0.003,   -- b3 straight line distance to CBD (KM)
            [4]  = 0.000,   -- b4 job accessibility by Car (10^6)
            [5]  = 0.030,   -- b5 within 1KM of top primary school (boolean)
            [6]  = 0.005,   -- b6 straight line distance to Shopping Mall (KM)
            [7]  = -0.047,  -- b7 within 200m of MRT station (boolean)
            [8]  = 0.001,   -- b8 within 200m-400m of MRT station (boolean)
            [9]  = -0.028,  -- b9 within 200m of an express way (boolean)
            [10] = -0.190,  -- b10 within 200m of a bus stop (boolean)
            [11] = -0.158   -- b11 within 200m-400m of a bus stop (boolean)
        }, 
    [2]=
        {   -- Semi-Detached houses
            [0]  = 11.890,  -- b0 constant
            [1]  = 0.440,   -- b1 unit floor area (m^2) 
            [2]  = 0.106,   -- b2 freehold (boolean)
            [3]  = -0.015,   -- b3 straight line distance to CBD (KM)
            [4]  = 1.337,   -- b4 job accessibility by Car (10^6)
            [5]  = 0.033,   -- b5 within 1KM of top primary school (boolean)
            [6]  = 0.010,   -- b6 straight line distance to Shopping Mall (KM)
            [7]  = -0.210,  -- b7 within 200m of MRT station (boolean)
            [8]  = -0.033,   -- b8 within 200m-400m of MRT station (boolean)
            [9]  = -0.191,  -- b9 within 200m of an express way (boolean)
            [10] = -0.125,  -- b10 within 200m of a bus stop (boolean)
            [11] = -0.100   -- b11 within 200m-400m of a bus stop (boolean)
        }, 
    [3]=
        {   -- Detached houses
            [0]  = 10.040,  -- b0 constant
            [1]  = 0.807,   -- b1 unit floor area (m^2) 
            [2]  = -0.008,   -- b2 freehold (boolean)
            [3]  = -0.038,   -- b3 straight line distance to CBD (KM)
            [4]  = 1.095,   -- b4 job accessibility by Car (10^6)
            [5]  = 0.029,   -- b5 within 1KM of top primary school (boolean)
            [6]  = 0.043,   -- b6 straight line distance to Shopping Mall (KM)
            [7]  = 0.009,  -- b7 within 200m of MRT station (boolean)
            [8]  = -0.043,   -- b8 within 200m-400m of MRT station (boolean)
            [9]  = -0.118,  -- b9 within 200m of an express way (boolean)
            [10] = -0.170,  -- b10 within 200m of a bus stop (boolean)
            [11] = -0.084   -- b11 within 200m-400m of a bus stop (boolean)
        }, 
    [4]=
        {   -- Apartement
            [0]  = 9.306,   -- b0 constant
            [1]  = 0.814,   -- b1 unit floor area (m^2) 
            [2]  = 0.151,   -- b2 freehold (boolean)
            [3]  = 0.0121,   -- b3 straight line distance to CBD (KM)
            [4]  = 1.355,   -- b4 job accessibility by Car (10^6)
            [5]  = 0.091,   -- b5 within 1KM of top primary school (boolean)
            [6]  = -0.063,   -- b6 straight line distance to Shopping Mall (KM)
            [7]  = 0.011,  -- b7 within 200m of MRT station (boolean)
            [8]  = 0.150,   -- b8 within 200m-400m of MRT station (boolean)
            [9]  = -0.095,  -- b9 within 200m of an express way (boolean)
            [10] = -0.247,  -- b10 within 200m of a bus stop (boolean)
            [11] = -0.170   -- b11 within 200m-400m of a bus stop (boolean)
        }, 
    [5]=
        {   -- Executive Condo
            [0]  = 10.310,  -- b0 constant
            [1]  = 0.732,   -- b1 unit floor area (m^2) 
            [2]  = 0,       -- b2 freehold (boolean)
            [3]  = -0.014,  -- b3 straight line distance to CBD (KM)
            [4]  = 0.008,   -- b4 job accessibility by Car (10^6)
            [5]  = -0.020,  -- b5 within 1KM of top primary school (boolean)
            [6]  = -0.014,  -- b6 straight line distance to Shopping Mall (KM)
            [7]  = 0,       -- b7 within 200m of MRT station (boolean)
            [8]  = 0.029,   -- b8 within 200m-400m of MRT station (boolean)
            [9]  = -0.030,  -- b9 within 200m of an express way (boolean)
            [10] = 0.035,   -- b10 within 200m of a bus stop (boolean)
            [11] = 0.027    -- b11 within 200m-400m of a bus stop (boolean)
        }, 
    [6]=
        {  -- Condo
            [0]  = 9.796,   -- b0 constant
            [1]  = 0.909,   -- b1 unit floor area (m^2) 
            [2]  = 0.169,   -- b2 freehold (boolean)
            [3]  = -0.031,   -- b3 straight line distance to CBD (KM)
            [4]  = 0.609,   -- b4 job accessibility by Car (10^6)
            [5]  = 0.063,   -- b5 within 1KM of top primary school (boolean)
            [6]  = -0.031,   -- b6 straight line distance to Shopping Mall (KM)
            [7]  = -0.003,  -- b7 within 200m of MRT station (boolean)
            [8]  = 0.043,   -- b8 within 200m-400m of MRT station (boolean)
            [9]  = -0.063,  -- b9 within 200m of an express way (boolean)
            [10] = -0.273,  -- b10 within 200m of a bus stop (boolean)
            [11] = -0.215   -- b11 within 200m-400m of a bus stop (boolean)
        }, 
}


--[[****************************************************************************
    FUNCTIONS
******************************************************************************]]
function getBuildingTypeFromUnitType(unitTypeId)
    if unitTypeId >= 1 and unitTypeId <= 6 then return 1
    elseif unitTypeId >= 7 and unitTypeId <= 11 then return 2
    elseif unitTypeId >= 12 and unitTypeId <= 16 then return 3
    elseif unitTypeId >= 17 and unitTypeId <= 21 then return 4
    elseif unitTypeId >= 22 and unitTypeId <= 26 then return 5
    elseif unitTypeId >= 27 and unitTypeId <= 31 then return 6
    elseif unitTypeId >= 32 and unitTypeId <= 36 then return 7
    elseif unitTypeId >= 37 and unitTypeId <= 41 then return 8
    elseif unitTypeId >= 42 and unitTypeId <= 47 then return 9
    elseif unitTypeId >= 48 and unitTypeId <= 52 then return 10
    elseif unitTypeId == 53  then return 11
    elseif unitTypeId == 54 then return 12
    elseif unitTypeId == 55 then return 13
    elseif unitTypeId == 56  then return 14
    elseif unitTypeId == 57 then return 15
    elseif unitTypeId == 58 then return 16
    elseif unitTypeId == 59  then return 17
    elseif unitTypeId == 60 then return 18
    elseif unitTypeId == 61  then return 19
    elseif unitTypeId == 62 then return 20
    elseif unitTypeId == 63 then return 21
    elseif unitTypeId == 64 then return 22
    elseif unitTypeId == 65 then return 23
    elseif unitTypeId == 66 then return 24
    else return 0
    end	   	
end


function calculateUnitRevenueApartment(amenities,unit)
	local revenue = 0
	revenue = UNIT_TYPE_COEFFICIENTS[4][0] + UNIT_TYPE_COEFFICIENTS[4][1]* math.log(unit.floorArea) + UNIT_TYPE_COEFFICIENTS	[4][2] * unit.freehold  + UNIT_TYPE_COEFFICIENTS[4][3] * amenities.distanceToCBD + UNIT_TYPE_COEFFICIENTS[4][4] * amenities.distanceToJob + UNIT_TYPE_COEFFICIENTS[4][5] * amenities.pms_1km + UNIT_TYPE_COEFFICIENTS[4][6]* amenities.distanceToMall + UNIT_TYPE_COEFFICIENTS[4][7]* amenities.mrt_200m + UNIT_TYPE_COEFFICIENTS[4][8]* amenities.mrt_400m + UNIT_TYPE_COEFFICIENTS[4][9]* amenities.express_200m + UNIT_TYPE_COEFFICIENTS[4][10]* amenities.bus_200m + UNIT_TYPE_COEFFICIENTS[4][11]* amenities.bus_400m
	return revenue;
end

function calculateUnitRevenueCondo(amenities,unit)
	local revenue = 0
	revenue = UNIT_TYPE_COEFFICIENTS[6][0] + UNIT_TYPE_COEFFICIENTS[6][1]* math.log(unit.floorArea) + 		UNIT_TYPE_COEFFICIENTS	[6][2] * unit.freehold  + UNIT_TYPE_COEFFICIENTS[6][3] * amenities.distanceToCBD + 	UNIT_TYPE_COEFFICIENTS[6][4] * amenities.distanceToJob + UNIT_TYPE_COEFFICIENTS[6][5] * amenities.pms_1km + UNIT_TYPE_COEFFICIENTS[6][6]* amenities.distanceToMall + UNIT_TYPE_COEFFICIENTS[6][7]* amenities.mrt_200m + UNIT_TYPE_COEFFICIENTS[6][8]* amenities.mrt_400m + UNIT_TYPE_COEFFICIENTS[6][9]* amenities.express_200m + UNIT_TYPE_COEFFICIENTS[6][10]* amenities.bus_200m + UNIT_TYPE_COEFFICIENTS[6][11]* amenities.bus_400m
	return revenue;
end

function calculateUnitRevenueSemiDetatched(amenities,unit)
	local revenue = 0
	revenue = UNIT_TYPE_COEFFICIENTS[2][0] + UNIT_TYPE_COEFFICIENTS[2][1]* math.log(unit.floorArea) + UNIT_TYPE_COEFFICIENTS	[2][2] * unit.freehold  + UNIT_TYPE_COEFFICIENTS[2][3] * amenities.distanceToCBD + UNIT_TYPE_COEFFICIENTS[2][4] * amenities.distanceToJob + UNIT_TYPE_COEFFICIENTS[2][5] * amenities.pms_1km + UNIT_TYPE_COEFFICIENTS[2][6]* amenities.distanceToMall + UNIT_TYPE_COEFFICIENTS[2][7]* amenities.mrt_200m + UNIT_TYPE_COEFFICIENTS[2][8]* amenities.mrt_400m + UNIT_TYPE_COEFFICIENTS[2][9]* amenities.express_200m + UNIT_TYPE_COEFFICIENTS[2][10]* amenities.bus_200m + UNIT_TYPE_COEFFICIENTS[2][11]* amenities.bus_400m
	return revenue;
end

function calculateUnitRevenueDetatched(amenities,unit)
	local revenue = 0
	revenue = UNIT_TYPE_COEFFICIENTS[3][0] + UNIT_TYPE_COEFFICIENTS[3][1]* math.log(unit.floorArea) + UNIT_TYPE_COEFFICIENTS	[3][2] * unit.freehold  + UNIT_TYPE_COEFFICIENTS[3][3] * amenities.distanceToCBD + UNIT_TYPE_COEFFICIENTS[3][4] * amenities.distanceToJob + UNIT_TYPE_COEFFICIENTS[3][5] * amenities.pms_1km + UNIT_TYPE_COEFFICIENTS[3][6]* amenities.distanceToMall + UNIT_TYPE_COEFFICIENTS[3][7]* amenities.mrt_200m + UNIT_TYPE_COEFFICIENTS[3][8]* amenities.mrt_400m + UNIT_TYPE_COEFFICIENTS[3][9]* amenities.express_200m + UNIT_TYPE_COEFFICIENTS[3][10]* amenities.bus_200m + UNIT_TYPE_COEFFICIENTS[3][11]* amenities.bus_400m
	return revenue;
end

function calculateUnitRevenueExecutiveCondo(amenities,unit)
	local revenue = 0
	revenue = UNIT_TYPE_COEFFICIENTS[5][0] + UNIT_TYPE_COEFFICIENTS[5][1]* math.log(unit.floorArea) + UNIT_TYPE_COEFFICIENTS	[5][2] * unit.freehold  + UNIT_TYPE_COEFFICIENTS[5][3] * amenities.distanceToCBD + UNIT_TYPE_COEFFICIENTS[5][4] * amenities.distanceToJob + UNIT_TYPE_COEFFICIENTS[5][5] * amenities.pms_1km + UNIT_TYPE_COEFFICIENTS[5][6]* amenities.distanceToMall + UNIT_TYPE_COEFFICIENTS[5][7]* amenities.mrt_200m + UNIT_TYPE_COEFFICIENTS[5][8]* amenities.mrt_400m + UNIT_TYPE_COEFFICIENTS[5][9]* amenities.express_200m + UNIT_TYPE_COEFFICIENTS[5][10]* amenities.bus_200m + UNIT_TYPE_COEFFICIENTS[5][11]* amenities.bus_400m
	return revenue;
end

function calculateUnitRevenueTerrace(amenities,unit)
	local revenue = 0
	revenue = UNIT_TYPE_COEFFICIENTS[1][0] + UNIT_TYPE_COEFFICIENTS[1][1]* math.log(unit.floorArea) + UNIT_TYPE_COEFFICIENTS	[1][2] * unit.freehold  + UNIT_TYPE_COEFFICIENTS[1][3] * amenities.distanceToCBD + UNIT_TYPE_COEFFICIENTS[1][4] * amenities.distanceToJob + UNIT_TYPE_COEFFICIENTS[1][5] * amenities.pms_1km + UNIT_TYPE_COEFFICIENTS[1][6]* amenities.distanceToMall + UNIT_TYPE_COEFFICIENTS[1][7]* amenities.mrt_200m + UNIT_TYPE_COEFFICIENTS[1][8]* amenities.mrt_400m + UNIT_TYPE_COEFFICIENTS[1][9]* amenities.express_200m + UNIT_TYPE_COEFFICIENTS[1][10]* amenities.bus_200m + UNIT_TYPE_COEFFICIENTS[1][11]* amenities.bus_400m
	return revenue;
end

--[[
    Calculates the revenue for the given future unit.
]]
function calculateUnitRevenue(unit,amenities)
    local revenuePerUnit = 0
    local buildingTypeId = getBuildingTypeFromUnitType(unit.unitTypeId)
    if(buildingTypeId == 2)then revenuePerUnit = calculateUnitRevenueApartment(amenities,unit)
    elseif (buildingTypeId == 3)then revenuePerUnit = calculateUnitRevenueCondo(amenities,unit)
    elseif (buildingTypeId == 5)then revenuePerUnit = calculateUnitRevenueSemiDetatched(amenities,unit)
    elseif (buildingTypeId == 6)then revenuePerUnit = calculateUnitRevenueDetatched(amenities,unit)
    elseif (buildingTypeId == 7)then revenuePerUnit = calculateUnitRevenueExecutiveCondo(amenities,unit)
    elseif (buildingTypeId == 4)then revenuePerUnit = calculateUnitRevenueTerrace(amenities,unit);
    end
    return revenuePerUnit;
end

