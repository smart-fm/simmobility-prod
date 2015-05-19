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
            [0]  = 2.458,    -- intercept
            [1]  = 0.480,   -- b1 unit floor area (m^2) 
            [2]  = 0.111,   -- b2 freehold (boolean)
            [3]  = 3.406,   -- b3 logsum accessebility
            [4]  = 0.034,   -- b4 within 1KM of top primary school (boolean)
            [5]  = -0.003,  -- b5 Distance to mall (km)
            [6]  = 0.020,   -- b6 within 200m of a MRT station (boolean)
            [7]  = 0.003,   -- b7 within 200m-400m of MRT station (boolean)
            [8]  = 0.004,   -- b8 within 200m of express way (boolean)
            [9]  = 0.031,   -- b9 Within 200-400m of a bus stop (boolean)
            [10] = 0.224   -- b10 Beyond 400m of a bus stop (boolean)
        }, 
    [2]=
        {   -- Semi-Detached houses
            [0]  = -19.390,  -- intercept
            [1]  = 0.4446,   -- b1 unit floor area (m^2) 
            [2]  = 0.096,    -- b2 freehold (boolean)
            [3]  = 11.640,   -- b3 logsum accessebility
            [4]  = 0.020,    -- b4 within 1KM of top primary school (boolean)
            [5]  = 0.007,    -- b5 Distance to mall (km)
            [6]  = -0.229,   -- b6 within 200m of a MRT station (boolean)
            [7]  = -0.050,   -- b7 within 200m-400m of MRT station (boolean)
            [8]  = -0.168,   -- b8 within 200m of express way (boolean)
            [9]  = 0.023,    -- b9 Within 200-400m of a bus stop (boolean)
            [10] = 0.153    -- b10 Beyond 400m of a bus stop (boolean)
        }, 
    [3]=
        {   -- Detached houses
            [0]  = -26.490,  -- intercept
            [1]  = 0.849,    -- b1 unit floor area (m^2) 
            [2]  = -0.026,   -- b2 freehold (boolean)
            [3]  = 13.420,   -- b3 logsum accessebility
            [4]  = -0.011,   -- b4 within 1KM of top primary school (boolean)
            [5]  = 0.011,    -- b5 Distance to mall (km)
            [6]  = -0.008,   -- b6 within 200m of a MRT station (boolean)
            [7]  = -0.049,   -- b7 within 200m-400m of MRT station (boolean)
            [8]  = -0.113,   -- b8 within 200m of express way (boolean)
            [9]  = 0.111,    -- b9 Within 200-400m of a bus stop (boolean)
            [10] = 0.282    -- b10 Beyond 400m of a bus stop (boolean)
        }, 
    [4]=
        {   -- Apartement
            [0]  = -20.320,  -- intercept
            [1]  = 0.808,    -- b1 unit floor area (m^2) 
            [2]  = 0.114,    -- b2 freehold (boolean)
            [3]  = 11.150,   -- b3 logsum accessebility
            [4]  = 0.048,    -- b4 within 1KM of top primary school (boolean)
            [5]  = -0.110,   -- b5 Distance to mall (km)
            [6]  = -0.030,   -- b6 within 200m of a MRT station (boolean)
            [7]  = -0.168,   -- b7 within 200m-400m of MRT station (boolean)
            [8]  = -0.084,   -- b8 within 200m of express way (boolean)
            [9]  = 0.102,    -- b9 Within 200-400m of a bus stop (boolean)
            [10] = 0.262    -- b10 Beyond 400m of a bus stop (boolean)
        }, 
    [5]=
        {   -- Executive Condo
            [0]  = 1.771,    -- intercept
            [1]  = 0.722,    -- b1 unit floor area (m^2) 
            [2]  = 0   ,    -- b2 freehold (boolean)
            [3]  = 3.108,    -- b3 logsum accessebility
            [4]  = -0.043,   -- b4 within 1KM of top primary school (boolean)
            [5]  = -0.019,   -- b5 Distance to mall (km)
            [6]  =  0 ,   -- b6 within 200m of a MRT station (boolean)
            [7]  = 0.006,    -- b7 within 200m-400m of MRT station (boolean)
            [8]  = -0.046,   -- b8 within 200m of express way (boolean)
            [9]  = -0.011,   -- b9 Within 200-400m of a bus stop (boolean)
            [10] = -0.030   -- b10 Beyond 400m of a bus stop (boolean)
        }, 
    [6]=
        {  -- Condo
            [0]  = -23.560,  -- intercept
            [1]  = 0.939,    -- b1 unit floor area (m^2) 
            [2]  = 0.186,    -- b2 freehold (boolean)
            [3]  = 12.220,   -- b3 logsum accessebility
            [4]  = 0.013,    -- b4 within 1KM of top primary school (boolean)
            [5]  = -0.054,   -- b5 Distance to mall (km)
            [6]  = -0.556,   -- b6 within 200m of a MRT station (boolean)
            [7]  = -0.002,   -- b7 within 200m-400m of MRT station (boolean)
            [8]  = -0.032,   -- b8 within 200m of express way (boolean)
            [9]  = 0.056,    -- b9 Within 200-400m of a bus stop (boolean)
            [10] = 0.362    -- b10 Beyond 400m of a bus stop (boolean)
        }, 
}

UNIT_TAOVALUESQ1 = readOnlyTable 
{
    [1]=
        {   -- values for 2008Q1 : terraced houses
            [0]  = -0.282,    -- HPI ^ (t-4)
            [1]  = -0.3155,   -- HPI ^ (t-5)
            [2]  = -0.3774,   -- HPI ^ (t-6)
            [3]  = -0.3499,   -- HPI ^ (t-7)
        }, 
    [2]=
        {   -- values for 2008Q1 : semi detached houses
            [0]  = -0.1838,  -- HPI ^ (t-4)
            [1]  = -0.27,    -- HPI ^ (t-5)
            [2]  = -0.3205,  -- HPI ^ (t-6)
            [3]  = -0.34,    -- HPI ^ (t-7)
        }, 
    [3]=
        {   -- values for 2008Q1 : detached houses
            [0]  = -0.00163,  -- HPI ^ (t-4)
            [1]  = -0.1035,    -- HPI ^ (t-5)
            [2]  = -0.2107,  -- HPI ^ (t-6)
            [3]  = -0.1809,    -- HPI ^ (t-7)
        }, 
    [4]=
        {   -- values for 2008Q1 : Apartments
            [0]  = 0.2114,  -- HPI ^ (t-4)
            [1]  = 0.04926,    -- HPI ^ (t-5)
            [2]  = -0.01006,  -- HPI ^ (t-6)
            [3]  = -0.06399,    -- HPI ^ (t-7)
        }, 
    [5]=
        {   -- values for 2008Q1 : Executive condos
            [0]  = -0.1633,  -- HPI ^ (t-4)
            [1]  = -0.1968,    -- HPI ^ (t-5)
            [2]  = -0.1843,  -- HPI ^ (t-6)
            [3]  = -0.1583,    -- HPI ^ (t-7)
        },  
    [6]=
        {   -- values for 2008Q1 : Condos
            [0]  = -0.05231,  -- HPI ^ (t-4)
            [1]  = -0.08905,    -- HPI ^ (t-5)
            [2]  = -0.155,  -- HPI ^ (t-6)
            [3]  = -0.2565,    -- HPI ^ (t-7)
        },  
}

UNIT_TAOVALUESQ2 = readOnlyTable 
{
    [1]=
        {   -- values for 2008Q2 : terraced houses
            [0]  = -0.1826,    -- HPI ^ (t-4)
            [1]  = -0.282,   -- HPI ^ (t-5)
            [2]  = -0.3155,   -- HPI ^ (t-6)
            [3]  = -0.3774,   -- HPI ^ (t-7)
        }, 
    [2]=
        {   -- values for 2008Q2 : semi detached houses
            [0]  = -0.09528,  -- HPI ^ (t-4)
            [1]  = -0.1838,  -- HPI ^ (t-5)
            [2]  = -0.27,    -- HPI ^ (t-6)
            [3]  = -0.3205,  -- HPI ^ (t-7)
        }, 
    [3]=
        {   -- values for 2008Q2 : detached houses
            [0]  = 0.1321,  -- HPI ^ (t-4)
            [1]  = -0.00163,  -- HPI ^ (t-5)
            [2]  = -0.1035,    -- HPI ^ (t-6)
            [3]  = -0.2107,  -- HPI ^ (t-7)
        }, 
    [4]=
        {   -- values for 2008Q2 : Apartments
            [0]  = 0.2444,  -- HPI ^ (t-4)
            [1]  = 0.2114,  -- HPI ^ (t-5)
            [2]  = 0.04926,    -- HPI ^ (t-6)
            [3]  = -0.01006,  -- HPI ^ (t-7)
        }, 
    [5]=
        {   -- values for 2008Q2 : Executive condos
            [0]  = -0.08069,  -- HPI ^ (t-4)
            [1]  = -0.1633,  -- HPI ^ (t-5)
            [2]  = -0.1968,    -- HPI ^ (t-6)
            [3]  = -0.1843,  -- HPI ^ (t-7)
        },  
    [6]=
        {   -- values for 2008Q2 : Condos
            [0]  = 0.03072,  -- HPI ^ (t-4)
            [1]  = -0.05231,    -- HPI ^ (t-5)
            [2]  = -0.08905,  -- HPI ^ (t-6)
            [3]  = -0.155,    -- HPI ^ (t-7)
        },  
}

UNIT_TAOVALUESQ3 = readOnlyTable 
{
    [1]=
        {   -- values for 2008Q3 : terraced houses
            [0]  = -0.06558,    -- HPI ^ (t-4)
            [1]  = -0.1826,   -- HPI ^ (t-5)
            [2]  = -0.282,   -- HPI ^ (t-6)
            [3]  = -0.3155,   -- HPI ^ (t-7)
        }, 
    [2]=
        {   -- values for 2008Q3 : semi detached houses
            [0]  = 0.02236,  -- HPI ^ (t-4)
            [1]  = -0.09528,  -- HPI ^ (t-5)
            [2]  = -0.1838,    -- HPI ^ (t-6)
            [3]  = -0.27,  -- HPI ^ (t-7)
        }, 
    [3]=
        {   -- values for 2008Q3 : detached houses
            [0]  = 0.236,  -- HPI ^ (t-4)
            [1]  = 0.1321,  -- HPI ^ (t-5)
            [2]  = -0.00163,    -- HPI ^ (t-6)
            [3]  = -0.1035,  -- HPI ^ (t-7)
        }, 
    [4]=
        {   -- values for 2008Q3 : Apartments
            [0]  = 0.3985,  -- HPI ^ (t-4)
            [1]  = 0.2444,  -- HPI ^ (t-5)
            [2]  = 0.2114,    -- HPI ^ (t-6)
            [3]  = 0.04926,  -- HPI ^ (t-7)
        }, 
    [5]=
        {   -- values for 2008Q3 : Executive condos
            [0]  = 0.05983,  -- HPI ^ (t-4)
            [1]  = -0.08069,  -- HPI ^ (t-5)
            [2]  = -0.1633,    -- HPI ^ (t-6)
            [3]  = -0.1968,  -- HPI ^ (t-7)
        },  
    [6]=
        {   -- values for 2008Q3 : Condos
            [0]  = 0.1838,  -- HPI ^ (t-4)
            [1]  = 0.03072,    -- HPI ^ (t-5)
            [2]  = -0.05231,  -- HPI ^ (t-6)
            [3]  = -0.08905,    -- HPI ^ (t-7)
        },  
}

UNIT_TAOVALUESQ4 = readOnlyTable 
{
    [1]=
        {   -- values for 2008Q4 : terraced houses
            [0]  = 0.06931,    -- HPI ^ (t-4)
            [1]  = -0.06558,   -- HPI ^ (t-5)
            [2]  = -0.1826,   -- HPI ^ (t-6)
            [3]  = -0.282,   -- HPI ^ (t-7)
        }, 
    [2]=
        {   -- values for 2008Q4 : semi detached houses
            [0]  = 0.02953,  -- HPI ^ (t-4)
            [1]  = 0.02236,  -- HPI ^ (t-5)
            [2]  = -0.09528,    -- HPI ^ (t-6)
            [3]  = -0.1838,  -- HPI ^ (t-7)
        }, 
    [3]=
        {   -- values for 2008Q4 : detached houses
            [0]  = 0.2003,  -- HPI ^ (t-4)
            [1]  = 0.236,  -- HPI ^ (t-5)
            [2]  = 0.1321,    -- HPI ^ (t-6)
            [3]  = -0.00163,  -- HPI ^ (t-7)
        }, 
    [4]=
        {   -- values for 2008Q4 : Apartments
            [0]  = 0.3303,  -- HPI ^ (t-4)
            [1]  = 0.3985,  -- HPI ^ (t-5)
            [2]  = 0.2444,    -- HPI ^ (t-6)
            [3]  = 0.2114,  -- HPI ^ (t-7)
        }, 
    [5]=
        {   -- values for 2008Q4 : Executive condos
            [0]  = 0.1285,  -- HPI ^ (t-4)
            [1]  = 0.05983,  -- HPI ^ (t-5)
            [2]  = -0.08069,    -- HPI ^ (t-6)
            [3]  = -0.1633,  -- HPI ^ (t-7)
        },  
    [6]=
        {   -- values for 2008Q4 : Condos
            [0]  = 0.1932,  -- HPI ^ (t-4)
            [1]  = 0.1838,    -- HPI ^ (t-5)
            [2]  = 0.03072,  -- HPI ^ (t-6)
            [3]  = -0.05231,    -- HPI ^ (t-7)
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

--exp value is calculated for amenties only: This should be changed as per "calculateUnitRevenueCondo(amenities,unit)" function once HPI values are received.
function calculateUnitRevenueApartment(amenities,unit,logsum,quarter)
	local revenue = 0
	revenue = UNIT_TYPE_COEFFICIENTS[4][0] + UNIT_TYPE_COEFFICIENTS[4][1]* math.log(unit.floorArea) + UNIT_TYPE_COEFFICIENTS	[4][2] * unit.freehold  + UNIT_TYPE_COEFFICIENTS[4][3] * logsum.accessibility + UNIT_TYPE_COEFFICIENTS[4][4] * amenities.pms_1km + UNIT_TYPE_COEFFICIENTS[4][5] * (amenities.distanceToMall/1000) + UNIT_TYPE_COEFFICIENTS[4][6]* amenities.mrt_200m + UNIT_TYPE_COEFFICIENTS[4][7]* amenities.mrt_400m + UNIT_TYPE_COEFFICIENTS[4][8]* amenities.express_200m + UNIT_TYPE_COEFFICIENTS[4][9]* amenities.bus_200m + UNIT_TYPE_COEFFICIENTS[4][10]* amenities.bus_400m
	local HPI = 0
	if(quarter == 1) then HPI = 0.03016 + 0.93910 * UNIT_TAOVALUESQ1[4][0] -0.14051 * UNIT_TAOVALUESQ1[4][1] -0.30319 * UNIT_TAOVALUESQ1[4][2] + 0.32669 * UNIT_TAOVALUESQ1[4][3]
	elseif(quarter == 2) then HPI = 0.03016 + 0.93910 * UNIT_TAOVALUESQ2[4][0] -0.14051 * UNIT_TAOVALUESQ2[4][1] -0.30319 * UNIT_TAOVALUESQ2[4][2] + 0.32669 * UNIT_TAOVALUESQ2[4][3]
	elseif(quarter == 3) then HPI = 0.03016 + 0.93910 * UNIT_TAOVALUESQ3[4][0] -0.14051 * UNIT_TAOVALUESQ3[4][1] -0.30319 * UNIT_TAOVALUESQ3[4][2] + 0.32669 * UNIT_TAOVALUESQ3[4][3]
	elseif(quarter == 4) then HPI = 0.03016 + 0.93910 * UNIT_TAOVALUESQ4[4][0] -0.14051 * UNIT_TAOVALUESQ4[4][1] -0.30319 * UNIT_TAOVALUESQ4[4][2] + 0.32669 * UNIT_TAOVALUESQ4[4][3]
	end
	local totalRevenue = math.exp(revenue + HPI)
	return totalRevenue;
end

function calculateUnitRevenueCondo(amenities,unit,logsum,quarter)
	local revenue = 0
	revenue = UNIT_TYPE_COEFFICIENTS[6][0] + UNIT_TYPE_COEFFICIENTS[6][1]* math.log(unit.floorArea) + UNIT_TYPE_COEFFICIENTS [6][2] * unit.freehold  + UNIT_TYPE_COEFFICIENTS[6][3] * logsum.accessibility + 	UNIT_TYPE_COEFFICIENTS[6][4] * amenities.pms_1km + UNIT_TYPE_COEFFICIENTS[6][5] * (amenities.distanceToMall/1000) + UNIT_TYPE_COEFFICIENTS[6][6]* amenities.mrt_200m + UNIT_TYPE_COEFFICIENTS[6][7]* amenities.mrt_400m + UNIT_TYPE_COEFFICIENTS[6][8]* amenities.express_200m + UNIT_TYPE_COEFFICIENTS[6][9]* amenities.bus_200m + UNIT_TYPE_COEFFICIENTS[6][10]* amenities.bus_400m
	local HPI = 0
	if(quarter == 1) then HPI = -0.01455 + 1.49396 * UNIT_TAOVALUESQ1[6][0] -0.90378 * UNIT_TAOVALUESQ1[6][1] -0.20183 * UNIT_TAOVALUESQ1[6][2] + 0.31161 * UNIT_TAOVALUESQ1[6][3]
	elseif(quarter == 2) then HPI = -0.01455 + 1.49396 * UNIT_TAOVALUESQ2[6][0] -0.90378 * UNIT_TAOVALUESQ2[6][1] -0.20183 * UNIT_TAOVALUESQ2[6][2] + 0.31161 * UNIT_TAOVALUESQ2[6][3]
	elseif(quarter == 3) then HPI = -0.01455 + 1.49396 * UNIT_TAOVALUESQ3[6][0] -0.90378 * UNIT_TAOVALUESQ3[6][1] -0.20183 * UNIT_TAOVALUESQ3[6][2] + 0.31161 * UNIT_TAOVALUESQ3[6][3]
	elseif(quarter == 4) then HPI = -0.01455 + 1.49396 * UNIT_TAOVALUESQ4[6][0] -0.90378 * UNIT_TAOVALUESQ4[6][1] -0.20183 * UNIT_TAOVALUESQ4[6][2] + 0.31161 * UNIT_TAOVALUESQ4[6][3]
	end
	local totalRevenue = math.exp(revenue + HPI)
	return totalRevenue;
end

function calculateUnitRevenueSemiDetatched(amenities,unit,logsum,quarter)
	local revenue = 0
	revenue = UNIT_TYPE_COEFFICIENTS[2][0] + UNIT_TYPE_COEFFICIENTS[2][1]* math.log(unit.floorArea) + UNIT_TYPE_COEFFICIENTS	[2][2] * unit.freehold  + UNIT_TYPE_COEFFICIENTS[2][3] * logsum.accessibility + UNIT_TYPE_COEFFICIENTS[2][4] * amenities.pms_1km + UNIT_TYPE_COEFFICIENTS[2][5] * (amenities.distanceToMall/1000) + UNIT_TYPE_COEFFICIENTS[2][6]* amenities.mrt_200m + UNIT_TYPE_COEFFICIENTS[2][7]* amenities.mrt_400m + UNIT_TYPE_COEFFICIENTS[2][8]* amenities.express_200m + UNIT_TYPE_COEFFICIENTS[2][9]* amenities.bus_200m + UNIT_TYPE_COEFFICIENTS[2][10]* amenities.bus_400m
	local HPI = 0
	if(quarter == 1) then HPI = 0.01308 + 1.51888 * UNIT_TAOVALUESQ1[2][0] -0.62345 * UNIT_TAOVALUESQ1[2][1] -0.49512 * UNIT_TAOVALUESQ1[2][2] + 0.53292 * UNIT_TAOVALUESQ1[2][3]
	elseif(quarter == 2) then HPI = 0.01308 + 1.51888 * UNIT_TAOVALUESQ2[2][0] -0.62345 * UNIT_TAOVALUESQ2[2][1] -0.49512 * UNIT_TAOVALUESQ2[2][2] + 0.53292 * UNIT_TAOVALUESQ2[2][3]
	elseif(quarter == 3) then HPI = 0.01308 + 1.51888 * UNIT_TAOVALUESQ3[2][0] -0.62345 * UNIT_TAOVALUESQ3[2][1] -0.49512 * UNIT_TAOVALUESQ3[2][2] + 0.53292 * UNIT_TAOVALUESQ3[2][3]
	elseif(quarter == 4) then HPI = 0.01308 + 1.51888 * UNIT_TAOVALUESQ4[2][0] -0.62345 * UNIT_TAOVALUESQ4[2][1] -0.49512 * UNIT_TAOVALUESQ4[2][2] + 0.53292 * UNIT_TAOVALUESQ4[2][3]
	end
	local totalRevenue = math.exp(revenue + HPI)
	return totalRevenue;
end

function calculateUnitRevenueDetatched(amenities,unit,logsum,quarter)
	local revenue = 0
	revenue = UNIT_TYPE_COEFFICIENTS[3][0] + UNIT_TYPE_COEFFICIENTS[3][1]* math.log(unit.floorArea) + UNIT_TYPE_COEFFICIENTS	[3][2] * unit.freehold  + UNIT_TYPE_COEFFICIENTS[3][3] * logsum.accessibility + UNIT_TYPE_COEFFICIENTS[3][4] * amenities.pms_1km + UNIT_TYPE_COEFFICIENTS[3][5] * (amenities.distanceToMall/1000) + UNIT_TYPE_COEFFICIENTS[3][6]* amenities.mrt_200m + UNIT_TYPE_COEFFICIENTS[3][7]* amenities.mrt_400m + UNIT_TYPE_COEFFICIENTS[3][8]* amenities.express_200m + UNIT_TYPE_COEFFICIENTS[3][9]* amenities.bus_200m + UNIT_TYPE_COEFFICIENTS[3][10]* amenities.bus_400m
	local HPI = 0
	if(quarter == 1) then HPI = 0.02164 + 1.19593 * UNIT_TAOVALUESQ1[3][0] -0.36370 * UNIT_TAOVALUESQ1[3][1] -1.1963 * UNIT_TAOVALUESQ1[3][2] + 0.25972 * UNIT_TAOVALUESQ1[3][3]
	elseif(quarter == 2) then HPI = 0.02164 + 1.19593 * UNIT_TAOVALUESQ2[3][0] -0.36370 * UNIT_TAOVALUESQ2[3][1] -1.1963 * UNIT_TAOVALUESQ2[3][2] + 0.25972 * UNIT_TAOVALUESQ2[3][3]
	elseif(quarter == 3) then HPI = 0.02164 + 1.19593 * UNIT_TAOVALUESQ3[3][0] -0.36370 * UNIT_TAOVALUESQ3[3][1] -1.1963 * UNIT_TAOVALUESQ3[3][2] + 0.25972 * UNIT_TAOVALUESQ3[3][3]
	elseif(quarter == 4) then HPI = 0.02164 + 1.19593 * UNIT_TAOVALUESQ4[3][0] -0.36370 * UNIT_TAOVALUESQ4[3][1] -1.1963 * UNIT_TAOVALUESQ4[3][2] + 0.25972 * UNIT_TAOVALUESQ4[3][3]
	end
	local totalRevenue = math.exp(revenue + HPI)
	return totalRevenue;
end

function calculateUnitRevenueExecutiveCondo(amenities,unit,logsum,quarter)
	local revenue = 0
	revenue = UNIT_TYPE_COEFFICIENTS[5][0] + UNIT_TYPE_COEFFICIENTS[5][1]* math.log(unit.floorArea) + UNIT_TYPE_COEFFICIENTS	[5][2] * unit.freehold  + UNIT_TYPE_COEFFICIENTS[5][3] * logsum.accessibility + UNIT_TYPE_COEFFICIENTS[5][4] * amenities.pms_1km + UNIT_TYPE_COEFFICIENTS[5][5] * (amenities.distanceToMall/1000) + UNIT_TYPE_COEFFICIENTS[5][6]* amenities.mrt_200m + UNIT_TYPE_COEFFICIENTS[5][7]* amenities.mrt_400m + UNIT_TYPE_COEFFICIENTS[5][8]* amenities.express_200m + UNIT_TYPE_COEFFICIENTS[5][9]* amenities.bus_200m + UNIT_TYPE_COEFFICIENTS[5][10]* amenities.bus_400m
	local HPI = 0
	if(quarter == 1) then HPI = 0.01781 + 1.13111 * UNIT_TAOVALUESQ1[5][0] -0.34371 * UNIT_TAOVALUESQ1[5][1] -0.25454 * UNIT_TAOVALUESQ1[5][2] + 0.41047 * UNIT_TAOVALUESQ1[5][3]
	elseif(quarter == 2) then HPI = 0.01781 + 1.13111 * UNIT_TAOVALUESQ2[5][0] -0.34371 * UNIT_TAOVALUESQ2[5][1] -0.25454 * UNIT_TAOVALUESQ2[5][2] + 0.41047 * UNIT_TAOVALUESQ2[5][3]
	elseif(quarter == 3) then HPI = 0.01781 + 1.13111 * UNIT_TAOVALUESQ3[5][0] -0.34371 * UNIT_TAOVALUESQ3[5][1] -0.25454 * UNIT_TAOVALUESQ3[5][2] + 0.41047 * UNIT_TAOVALUESQ3[5][3]
	elseif(quarter == 4) then HPI = 0.01781 + 1.13111 * UNIT_TAOVALUESQ4[5][0] -0.34371 * UNIT_TAOVALUESQ4[5][1] -0.25454 * UNIT_TAOVALUESQ4[5][2] + 0.41047 * UNIT_TAOVALUESQ4[5][3]
	end
	local totalRevenue = math.exp(revenue + HPI)
	return totalRevenue;
end

function calculateUnitRevenueTerrace(amenities,unit,logsum,quarter)
	local revenue = 0
	revenue = UNIT_TYPE_COEFFICIENTS[1][0] + UNIT_TYPE_COEFFICIENTS[1][1]* math.log(unit.floorArea) + UNIT_TYPE_COEFFICIENTS	[1][2] * unit.freehold  + UNIT_TYPE_COEFFICIENTS[1][3] * logsum.accessibility + UNIT_TYPE_COEFFICIENTS[1][4] * amenities.pms_1km + UNIT_TYPE_COEFFICIENTS[1][5] * (amenities.distanceToMall/1000) + UNIT_TYPE_COEFFICIENTS[1][6]* amenities.mrt_200m + UNIT_TYPE_COEFFICIENTS[1][7]* amenities.mrt_400m + UNIT_TYPE_COEFFICIENTS[1][8]* amenities.express_200m + UNIT_TYPE_COEFFICIENTS[1][9]* amenities.bus_200m + UNIT_TYPE_COEFFICIENTS[1][10]* amenities.bus_400m
	local HPI = 0
	if(quarter == 1) then HPI = -0.00466 + 1.132720 * UNIT_TAOVALUESQ1[1][0] -0.71526 * UNIT_TAOVALUESQ1[1][1] -0.04628 * UNIT_TAOVALUESQ1[1][2] + 0.28911 * UNIT_TAOVALUESQ1[1][3]
	elseif(quarter == 2) then HPI = -0.00466 + 1.132720 * UNIT_TAOVALUESQ2[1][0] -0.71526 * UNIT_TAOVALUESQ2[1][1] -0.04628 * UNIT_TAOVALUESQ2[1][2] + 0.28911 * UNIT_TAOVALUESQ2[1][3]
	elseif(quarter == 3) then HPI = -0.00466 + 1.132720 * UNIT_TAOVALUESQ3[1][0] -0.71526 * UNIT_TAOVALUESQ3[1][1] -0.04628 * UNIT_TAOVALUESQ3[1][2] + 0.28911 * UNIT_TAOVALUESQ3[1][3]
	elseif(quarter == 4) then HPI = -0.00466 + 1.132720 * UNIT_TAOVALUESQ4[1][0] -0.71526 * UNIT_TAOVALUESQ4[1][1] -0.04628 * UNIT_TAOVALUESQ4[1][2] + 0.28911 * UNIT_TAOVALUESQ4[1][3]
	end
	local totalRevenue = math.exp(revenue + HPI)
	return totalRevenue;
end

--[[
    Calculates the revenue for the given future unit.
]]
function calculateUnitRevenue(unit,amenities,logsum,quarter)
    local revenuePerUnit = 0
    local buildingTypeId = getBuildingTypeFromUnitType(unit.unitTypeId)
    if     (buildingTypeId == 2)then revenuePerUnit = calculateUnitRevenueApartment(amenities,unit,logsum,quarter)
    elseif (buildingTypeId == 3)then revenuePerUnit = calculateUnitRevenueCondo(amenities,unit,logsum,quarter)
    elseif (buildingTypeId == 5)then revenuePerUnit = calculateUnitRevenueSemiDetatched(amenities,unit,logsum,quarter)
    elseif (buildingTypeId == 6)then revenuePerUnit = calculateUnitRevenueDetatched(amenities,unit,logsum,quarter)
    elseif (buildingTypeId == 7)then revenuePerUnit = calculateUnitRevenueExecutiveCondo(amenities,unit,logsum,quarter)
    elseif (buildingTypeId == 4)then revenuePerUnit = calculateUnitRevenueTerrace(amenities,unit,logsum,quarter)
    end
    return revenuePerUnit;
end

