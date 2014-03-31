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

--[[
    Calculates the revenue for the given future unit.
]]
function calculateUnitRevenue(unit, amenities)
    print(string.format("UnitType: %d, FloorArea: %f, Freehold: %s", unit.unitTypeId, unit.floorArea, unit.freehold))
    --print( .. unit.unitTypeId .. " FloorArea: " .. unit.floorArea .. " Freehold: ".. unit.freehold )
    return 0;
end

--print(UNIT_TYPE_COEFFICIENTS[6][11])

