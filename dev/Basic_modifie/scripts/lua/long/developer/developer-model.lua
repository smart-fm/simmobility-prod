--ATTENTION requies cant be used with c++ (for now)

package.path = package.path .. ";scripts/lua/long/?.lua;../?.lua"
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
             [0]  = 2.115617129,    -- intercept
            [1]  = 0.468612265,   -- b1 unit floor area (m^2) 
            [2]  = 0.135114204,   -- b2 freehold (boolean)
            [3]  = 3.591528177,   -- b3 logsum accessebility
            [4]  = 0.029715641,   -- b4 within 1KM of top primary school (boolean)
            [5]  = -0.0026513,  -- b5 Distance to mall (km)
            [6]  = 0.008829052,   -- b6 within 200m of a MRT station (boolean)
            [7]  = 0.00153972,   -- b7 within 200m-400m of MRT station (boolean)
            [8]  = 0.004728968,   -- b8 within 200m of express way (boolean)
            [9]  = 0.028124909,   -- b9 Within 200-400m of a bus stop (boolean)
            [10] = 0.220693908,  -- b10 Beyond 400m of a bus stop (boolean)
	    [11] = -0.031700344, -- age
	    [12] = 0.001123349, -- I(age^2)
            [13] = -0.165532208, -- agem25_50
	    [14] = 0.044950323 --agem50
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
            [10] = 0.153,    -- b10 Beyond 400m of a bus stop (boolean)
  	    [11] = -0.025372566, -- age
   	    [12] = 0.000807147, --I(age^2)
	    [13] = -0.166701637, --agem25_50
	    [14] = 0.023981051--agem50
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
            [10] = 0.282,    -- b10 Beyond 400m of a bus stop (boolean)
	    [11] = -0.023193191, -- age
	    [12] = 0.000394237, -- I(age^2)
	    [13] = -0.077718103, -- age25_50
	    [14] = -0.068276746 -- agem50
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
            [10] = 0.262 ,   -- b10 Beyond 400m of a bus stop (boolean)
	    [11] = -0.02603531, -- age
	    [12] = 0.0000867, -- I(age^2)
	    [13] = 0.076709586, -- age25_50
	    [14] = 0.523295627 -- agem50
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
            [10] = -0.030,   -- b10 Beyond 400m of a bus stop (boolean)
 	    [11] = 0.005228958, -- age
	    [12] = -0.000607114, -- I(age^2)
	    [13] = 0, -- age25_50
	    [14] = 0 -- agem50
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
            [10] = 0.362,    -- b10 Beyond 400m of a bus stop (boolean)
	    [11] = -0.016519062, -- age
	    [12] = -0.00012883, -- I(age^2)
	    [13] = 0.134831364, -- age25_50
	    [14] = 0.474173547 -- agem50
        }, 
}

UNIT_TAOVALUESQ1 = readOnlyTable 
{
    [1]=
        {   -- values for 2012Q1 : terraced houses
            [0]  = 0.1815617669,    -- HPI ^ (t-4)
            [1]  = 0.163533933896742,   -- HPI ^ (t-5)
            [2]  = 0.102878760564595,   -- HPI ^ (t-6)
            [3]  = 0.105948367719416,   -- HPI ^ (t-7)
        }, 
    [2]=
        {   -- values for 2012Q1 : semi detached houses
            [0]  = 0.273818788169916,  -- HPI ^ (t-4)
            [1]  = 0.242556599492551,    -- HPI ^ (t-5)
            [2]  = 0.203747043174862,  -- HPI ^ (t-6)
            [3]  = 0.17422599068812,    -- HPI ^ (t-7)
        }, 
    [3]=
        {   -- values for 2012Q1 : detached houses
            [0]  = 0.568839035483969,  -- HPI ^ (t-4)
            [1]  = 0.5175845833414,    -- HPI ^ (t-5)
            [2]  = 0.451920010991659,  -- HPI ^ (t-6)
            [3]  = 0.458164100123879,    -- HPI ^ (t-7)
        }, 
    [4]=
        {   -- values for 2012Q1 : Apartments
            [0]  = 0.427404480461843,  -- HPI ^ (t-4)
            [1]  = 0.370360649107001,    -- HPI ^ (t-5)
            [2]  = 0.363839249058549,  -- HPI ^ (t-6)
            [3]  = 0.369395618624643,    -- HPI ^ (t-7)
        }, 
    [5]=
        {   -- values for 2012Q1 : Executive condos
            [0]  = 0.2356767281,  -- HPI ^ (t-4)
            [1]  = 0.175035436640919,    -- HPI ^ (t-5)
            [2]  = 0.136131197497864,  -- HPI ^ (t-6)
            [3]  = 0.052211018375668,    -- HPI ^ (t-7)
        },  
    [6]=
        {   -- values for 2012Q1 : Condos
            [0]  = 0.2873868073759,  -- HPI ^ (t-4)
            [1]  = 0.272203556771869,    -- HPI ^ (t-5)
            [2]  = 0.25355279771315,  -- HPI ^ (t-6)
            [3]  = 0.271690179450914,    -- HPI ^ (t-7)
        },  
}

UNIT_TAOVALUESQ2 = readOnlyTable 
{
    [1]=
        {   -- values for 2012Q2 : terraced houses
            [0]  = 0.218528839490279,    -- HPI ^ (t-4)
            [1]  = 0.181561766948983,   -- HPI ^ (t-5)
            [2]  = 0.163533933896742,   -- HPI ^ (t-6)
            [3]  = 0.102878760564595,   -- HPI ^ (t-7)
        }, 
    [2]=
        {   -- values for 2012Q2 : semi detached houses
            [0]  = 0.290713792040363,  -- HPI ^ (t-4)
            [1]  = 0.273818788169916,  -- HPI ^ (t-5)
            [2]  = 0.242556599492551,    -- HPI ^ (t-6)
            [3]  = 0.203747043174862,  -- HPI ^ (t-7)
        }, 
    [3]=
        {   -- values for 2012Q2 : detached houses
            [0]  = 0.551315999905967,  -- HPI ^ (t-4)
            [1]  = 0.568839035483969,  -- HPI ^ (t-5)
            [2]  = 0.5175845833414,    -- HPI ^ (t-6)
            [3]  = 0.451920010991659,  -- HPI ^ (t-7)
        }, 
    [4]=
        {   -- values for 2012Q2 : Apartments
            [0]  = 0.418771050575756,  -- HPI ^ (t-4)
            [1]  = 0.427404480461843,  -- HPI ^ (t-5)
            [2]  = 0.370360649107001,    -- HPI ^ (t-6)
            [3]  = 0.363839249058549,  -- HPI ^ (t-7)
        }, 
    [5]=
        {   -- values for 2012Q2 : Executive condos
            [0]  = 0.322377634105401,  -- HPI ^ (t-4)
            [1]  = 0.321543456261122,  -- HPI ^ (t-5)
            [2]  = 0.322861619868308,    -- HPI ^ (t-6)
            [3]  = 0.285247414992157,  -- HPI ^ (t-7)
        },  
    [6]=
        {   -- values for 2012Q2 : Condos
            [0]  = 0.312595026022714,  -- HPI ^ (t-4)
            [1]  = 0.2873868073759,    -- HPI ^ (t-5)
            [2]  = 0.272203556771869,  -- HPI ^ (t-6)
            [3]  = 0.25355279771315,    -- HPI ^ (t-7)
        },  
}

UNIT_TAOVALUESQ3 = readOnlyTable 
{
    [1]=
        {   -- values for 2012Q3 : terraced houses
            [0]  = 0.228518355082749,    -- HPI ^ (t-4)
            [1]  = 0.218528839490279,   -- HPI ^ (t-5)
            [2]  = 0.181561766948983,   -- HPI ^ (t-6)
            [3]  = 0.163533933896742,   -- HPI ^ (t-7)
        }, 
    [2]=
        {   -- values for 2012Q3 : semi detached houses
            [0]  = 0.304551693689864,  -- HPI ^ (t-4)
            [1]  = 0.290713792040363,  -- HPI ^ (t-5)
            [2]  = 0.273818788169916,    -- HPI ^ (t-6)
            [3]  = 0.242556599492551,  -- HPI ^ (t-7)
        }, 
    [3]=
        {   -- values for 2012Q3 : detached houses
            [0]  = 0.528119327291905,  -- HPI ^ (t-4)
            [1]  = 0.551315999905967,  -- HPI ^ (t-5)
            [2]  = 0.568839035483969,    -- HPI ^ (t-6)
            [3]  = 0.5175845833414,  -- HPI ^ (t-7)
        }, 
    [4]=
        {   -- values for 2012Q3 : Apartments
            [0]  = 0.379255681096981,  -- HPI ^ (t-4)
            [1]  = 0.418771050575756,  -- HPI ^ (t-5)
            [2]  = 0.427404480461843,    -- HPI ^ (t-6)
            [3]  = 0.370360649107001,  -- HPI ^ (t-7)
        }, 
    [5]=
        {   -- values for 2012Q3 : Executive condos
            [0]  = 0.327486046628756,  -- HPI ^ (t-4)
            [1]  = 0.322377634105401,  -- HPI ^ (t-5)
            [2]  = 0.321543456261122,    -- HPI ^ (t-6)
            [3]  = 0.322861619868308,  -- HPI ^ (t-7)
        },  
    [6]=
        {   -- values for 2012Q3 : Condos
            [0]  = 0.316361497555562,  -- HPI ^ (t-4)
            [1]  = 0.312595026022714,    -- HPI ^ (t-5)
            [2]  = 0.2873868073759,  -- HPI ^ (t-6)
            [3]  = 0.272203556771869,    -- HPI ^ (t-7)
        },  
}

UNIT_TAOVALUESQ4 = readOnlyTable 
{
    [1]=
        {   -- values for 2012Q4 : terraced houses
            [0]  = 0.203224675655529,    -- HPI ^ (t-4)
            [1]  = 0.228518355082749,   -- HPI ^ (t-5)
            [2]  = 0.218528839490279,   -- HPI ^ (t-6)
            [3]  = 0.181561766948983,   -- HPI ^ (t-7)
        }, 
    [2]=
        {   -- values for 2012Q4 : semi detached houses
            [0]  = 0.342123432166705,  -- HPI ^ (t-4)
            [1]  = 0.304551693689864,  -- HPI ^ (t-5)
            [2]  = 0.290713792040363,    -- HPI ^ (t-6)
            [3]  = 0.273818788169916,  -- HPI ^ (t-7)
        }, 
    [3]=
        {   -- values for 2012Q4 : detached houses
            [0]  = 0.569622668278393,  -- HPI ^ (t-4)
            [1]  = 0.528119327291905,  -- HPI ^ (t-5)
            [2]  = 0.551315999905967,    -- HPI ^ (t-6)
            [3]  = 0.568839035483969,  -- HPI ^ (t-7)
        }, 
    [4]=
        {   -- values for 2012Q4 : Apartments
            [0]  = 0.359460403840001,  -- HPI ^ (t-4)
            [1]  = 0.379255681096981,  -- HPI ^ (t-5)
            [2]  = 0.418771050575756,    -- HPI ^ (t-6)
            [3]  = 0.427404480461843,  -- HPI ^ (t-7)
        }, 
    [5]=
        {   -- values for 2012Q4 : Executive condos
            [0]  = 0.344433834068323,  -- HPI ^ (t-4)
            [1]  = 0.327486046628756,  -- HPI ^ (t-5)
            [2]  = 0.322377634105401,    -- HPI ^ (t-6)
            [3]  = 0.321543456261122,  -- HPI ^ (t-7)
        },  
    [6]=
        {   -- values for 2012Q4 : Condos
            [0]  = 0.275805647317985,  -- HPI ^ (t-4)
            [1]  = 0.316361497555562,    -- HPI ^ (t-5)
            [2]  = 0.312595026022714,  -- HPI ^ (t-6)
            [3]  = 0.2873868073759,    -- HPI ^ (t-7)
        },  
}
--[[****************************************************************************
PROFIT CALCULATION EQUATIONS - HPI

> condo market T^j(t)= 0.00237714980545529 + 1.48032117069404 * T^j(t-4) -0.923627197357721 * Tj(t-5) -0.284212117419372 * T^j(t-6) + 0.433488597894368 *T^j(t-7)
> apartment market T^j(t)= 0.0282608504100734 + 1.03635930655003 * T^j(t-4) -0.378464603969125 * Tj(t-5) -0.314277674194136 * T^j(t-6) + 0.456832121648814*T^j(t-7)
> terrace market T^j(t)= -0.00098316227372815 + 1.36802322312425 * T^j(t-4) -0.728803558563642 * Tj(t-5) -0.11355293427981 * T^j(t-6) + 0.32986523762844 *T^j(t-7)
> semi detached market T^j(t)= 0.0199546517730487 + 1.55237761116232 * T^j(t-4) -0.668849657408016 * Tj(t-5) -0.508464219221255 * T^j(t-6) +0.584996653495008*T^j(t-7)
> detached market T^j(t)= 0.0280034220617325 + 1.14904274826808 * T^j(t-4) -0.257409472564278 * Tj(t-5) -0.29270287135104 * T^j(t-6) + 0.307757590239275*T^j(t-7)
> EC market T^j(t)= 0.0179651365079083 + 1.12320737452687 * T^j(t-4) -0.314437384509016 * Tj(t-5) -0.264836990365303 * T^j(t-6) +0.402537928466658 * T^j(t-7)

********************************************************************************]]

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
function calculateUnitRevenueApartment(amenities,unit,logsum,quarter,futureYear,HPIfromData,age)
	local ageCapped = 0
	if age > 25 then ageCapped = 25 else ageCapped = age end
        local agem_25_50 = 0
	if age >= 25 and age <= 50 then agem_25_50 = 1 end
	local agem50 = 0
	if age > 50 then agem50 = 1 end
	local revenue = 0
	revenue = (UNIT_TYPE_COEFFICIENTS[4][0]) + (UNIT_TYPE_COEFFICIENTS[4][1]* math.log(unit.floorArea)) + (UNIT_TYPE_COEFFICIENTS	[4][2] * unit.freehold)  + (UNIT_TYPE_COEFFICIENTS[4][3] * logsum) + (UNIT_TYPE_COEFFICIENTS[4][4] * amenities.pms_1km) + (UNIT_TYPE_COEFFICIENTS[4][5] * amenities.distanceToMall) + (UNIT_TYPE_COEFFICIENTS[4][6]* amenities.mrt_200m) + (UNIT_TYPE_COEFFICIENTS[4][7]* amenities.mrt_400m) + (UNIT_TYPE_COEFFICIENTS[4][8]* amenities.express_200m) + (UNIT_TYPE_COEFFICIENTS[4][9]* amenities.bus_200m) + (UNIT_TYPE_COEFFICIENTS[4][10]* amenities.bus_400m)+ (UNIT_TYPE_COEFFICIENTS[4][11]* ageCapped) + (UNIT_TYPE_COEFFICIENTS[4][12]* (ageCapped*ageCapped))+ 
(UNIT_TYPE_COEFFICIENTS[4][13]* agem_25_50) + (UNIT_TYPE_COEFFICIENTS[4][14]* agem50)
	local HPI = 0
	if(futureYear == 1) then
	--constants are according to the equations - refer "PROFIT CALCULATION EQUATIONS - HPI" above.
	if(quarter == 1) then HPI = 0.0282608504100734 + 1.03635930655003 * UNIT_TAOVALUESQ1[4][0] -0.378464603969125 * UNIT_TAOVALUESQ1[4][1] -0.314277674194136 * UNIT_TAOVALUESQ1[4][2] + 0.456832121648814 * UNIT_TAOVALUESQ1[4][3]
	elseif(quarter == 2) then HPI = 0.0282608504100734 + 1.03635930655003 * UNIT_TAOVALUESQ2[4][0] -0.378464603969125 * UNIT_TAOVALUESQ2[4][1] -0.314277674194136 * UNIT_TAOVALUESQ2[4][2] + 0.456832121648814 * UNIT_TAOVALUESQ2[4][3]
	elseif(quarter == 3) then HPI = 0.0282608504100734 + 1.03635930655003 * UNIT_TAOVALUESQ3[4][0] -0.378464603969125 * UNIT_TAOVALUESQ3[4][1] -0.314277674194136 * UNIT_TAOVALUESQ3[4][2] + 0.456832121648814 * UNIT_TAOVALUESQ3[4][3]
	elseif(quarter == 4) then HPI = 0.0282608504100734 + 1.03635930655003 * UNIT_TAOVALUESQ4[4][0] -0.378464603969125 * UNIT_TAOVALUESQ4[4][1] -0.314277674194136 * UNIT_TAOVALUESQ4[4][2] + 0.456832121648814 * UNIT_TAOVALUESQ4[4][3]
	end
	else HPI = HPIfromData
	end
	local totalRevenue = math.exp(revenue + HPI)
	return totalRevenue;
end

function calculateUnitRevenueCondo(amenities,unit,logsum,quarter,futureYear,HPIfromData,age)
	local ageCapped = 0
	if age > 25 then ageCapped = 25 else ageCapped = age end
        local agem_25_50 = 0
	if age >= 25 and age <= 50 then agem_25_50 = 1 end
	local agem50 = 0
	if age > 50 then agem50 = 1 end
	local revenue = 0
	revenue = (UNIT_TYPE_COEFFICIENTS[6][0]) + (UNIT_TYPE_COEFFICIENTS[6][1]* math.log(unit.floorArea)) + (UNIT_TYPE_COEFFICIENTS [6][2] * unit.freehold ) + (UNIT_TYPE_COEFFICIENTS[6][3] * logsum) + (UNIT_TYPE_COEFFICIENTS[6][4] * amenities.pms_1km) + (UNIT_TYPE_COEFFICIENTS[6][5] * amenities.distanceToMall) +( UNIT_TYPE_COEFFICIENTS[6][6]* amenities.mrt_200m) + (UNIT_TYPE_COEFFICIENTS[6][7]* amenities.mrt_400m) + (UNIT_TYPE_COEFFICIENTS[6][8]* amenities.express_200m) + (UNIT_TYPE_COEFFICIENTS[6][9]* amenities.bus_200m) + (UNIT_TYPE_COEFFICIENTS[6][10]* amenities.bus_400m)+ (UNIT_TYPE_COEFFICIENTS[6][11]* ageCapped) + (UNIT_TYPE_COEFFICIENTS[6][12]* (ageCapped*ageCapped))+ 
(UNIT_TYPE_COEFFICIENTS[6][13]* agem_25_50) + (UNIT_TYPE_COEFFICIENTS[6][14]* agem50)
	local HPI = 0
	if(futureYear == 1) then
	if(quarter == 1) then HPI = 0.00237714980545529 + 1.48032117069404 * UNIT_TAOVALUESQ1[6][0] -0.923627197357721 * UNIT_TAOVALUESQ1[6][1] -0.284212117419372 * UNIT_TAOVALUESQ1[6][2] + 0.433488597894368 * UNIT_TAOVALUESQ1[6][3]
	elseif(quarter == 2) then HPI = 0.00237714980545529 + 1.48032117069404 * UNIT_TAOVALUESQ2[6][0] -0.923627197357721 * UNIT_TAOVALUESQ2[6][1] -0.284212117419372 * UNIT_TAOVALUESQ2[6][2] + 0.433488597894368 * UNIT_TAOVALUESQ2[6][3]
	elseif(quarter == 3) then HPI = 0.00237714980545529 + 1.48032117069404 * UNIT_TAOVALUESQ3[6][0] -0.923627197357721 * UNIT_TAOVALUESQ3[6][1] -0.284212117419372 * UNIT_TAOVALUESQ3[6][2] + 0.433488597894368 * UNIT_TAOVALUESQ3[6][3]
	elseif(quarter == 4) then HPI = 0.00237714980545529 + 1.48032117069404 * UNIT_TAOVALUESQ4[6][0] -0.923627197357721 * UNIT_TAOVALUESQ4[6][1] -0.284212117419372 * UNIT_TAOVALUESQ4[6][2] + 0.433488597894368 * UNIT_TAOVALUESQ4[6][3]
	end
	else HPI = HPIfromData
	end
	local totalRevenue = math.exp(revenue + HPI)
	return totalRevenue;
end

function calculateUnitRevenueSemiDetatched(amenities,unit,logsum,quarter,futureYear,HPIfromData,age)
	local ageCapped = 0
	if age > 25 then ageCapped = 25 else ageCapped = age end
        local agem_25_50 = 0
	if age >= 25 and age <= 50 then agem_25_50 = 1 end
	local agem50 = 0
	if age > 50 then agem50 = 1 end
	local revenue = 0
	revenue = (UNIT_TYPE_COEFFICIENTS[2][0]) + (UNIT_TYPE_COEFFICIENTS[2][1]* math.log(unit.floorArea)) + (UNIT_TYPE_COEFFICIENTS	[2][2] * unit.freehold)  + (UNIT_TYPE_COEFFICIENTS[2][3] * logsum) + (UNIT_TYPE_COEFFICIENTS[2][4] * amenities.pms_1km) + (UNIT_TYPE_COEFFICIENTS[2][5] * amenities.distanceToMall) + (UNIT_TYPE_COEFFICIENTS[2][6]* amenities.mrt_200m) + (UNIT_TYPE_COEFFICIENTS[2][7]* amenities.mrt_400m) + (UNIT_TYPE_COEFFICIENTS[2][8]* amenities.express_200m) + (UNIT_TYPE_COEFFICIENTS[2][9]* amenities.bus_200m) + (UNIT_TYPE_COEFFICIENTS[2][10]* amenities.bus_400m) + (UNIT_TYPE_COEFFICIENTS[2][11]* ageCapped) + (UNIT_TYPE_COEFFICIENTS[2][12]* (ageCapped*ageCapped))+ 
(UNIT_TYPE_COEFFICIENTS[2][13]* agem_25_50) + (UNIT_TYPE_COEFFICIENTS[2][14]* agem50)
	local HPI = 0
	if(futureYear == 1) then
	if(quarter == 1) then HPI = 0.0199546517730487 + 1.55237761116232 * UNIT_TAOVALUESQ1[2][0] -0.668849657408016 * UNIT_TAOVALUESQ1[2][1] -0.508464219221255 * UNIT_TAOVALUESQ1[2][2] + 0.584996653495008 * UNIT_TAOVALUESQ1[2][3]
	elseif(quarter == 2) then HPI = 0.0199546517730487 + 1.55237761116232 * UNIT_TAOVALUESQ2[2][0] -0.668849657408016 * UNIT_TAOVALUESQ2[2][1] -0.508464219221255 * UNIT_TAOVALUESQ2[2][2] + 0.584996653495008 * UNIT_TAOVALUESQ2[2][3]
	elseif(quarter == 3) then HPI = 0.0199546517730487 + 1.55237761116232 * UNIT_TAOVALUESQ3[2][0] -0.668849657408016 * UNIT_TAOVALUESQ3[2][1] -0.508464219221255 * UNIT_TAOVALUESQ3[2][2] + 0.584996653495008 * UNIT_TAOVALUESQ3[2][3]
	elseif(quarter == 4) then HPI = 0.0199546517730487 + 1.55237761116232 * UNIT_TAOVALUESQ4[2][0] -0.668849657408016 * UNIT_TAOVALUESQ4[2][1] -0.508464219221255 * UNIT_TAOVALUESQ4[2][2] + 0.584996653495008 * UNIT_TAOVALUESQ4[2][3]
	end
	else HPI = HPIfromData
	end
	local totalRevenue = math.exp(revenue + HPI)
	return totalRevenue;
end

function calculateUnitRevenueDetatched(amenities,unit,logsum,quarter,futureYear,HPIfromData,age)
	local ageCapped = 0
	if age > 25 then ageCapped = 25 else ageCapped = age end
        local agem_25_50 = 0
	if age >= 25 and age <= 50 then agem_25_50 = 1 end
	local agem50 = 0
	if age > 50 then agem50 = 1 end
	local revenue = 0
	revenue = (UNIT_TYPE_COEFFICIENTS[3][0]) + (UNIT_TYPE_COEFFICIENTS[3][1]* math.log(unit.floorArea)) + (UNIT_TYPE_COEFFICIENTS	[3][2] * unit.freehold)  + (UNIT_TYPE_COEFFICIENTS[3][3] * logsum) + (UNIT_TYPE_COEFFICIENTS[3][4] * amenities.pms_1km) + (UNIT_TYPE_COEFFICIENTS[3][5] * amenities.distanceToMall) + (UNIT_TYPE_COEFFICIENTS[3][6]* amenities.mrt_200m) + (UNIT_TYPE_COEFFICIENTS[3][7]* amenities.mrt_400m) + (UNIT_TYPE_COEFFICIENTS[3][8]* amenities.express_200m) + (UNIT_TYPE_COEFFICIENTS[3][9]* amenities.bus_200m) + (UNIT_TYPE_COEFFICIENTS[3][10]* amenities.bus_400m)+ (UNIT_TYPE_COEFFICIENTS[3][11]* ageCapped) + (UNIT_TYPE_COEFFICIENTS[3][12]* (ageCapped*ageCapped))+ 
(UNIT_TYPE_COEFFICIENTS[3][13]* agem_25_50) + (UNIT_TYPE_COEFFICIENTS[3][14]* agem50)
	local HPI = 0
	if(futureYear == 1) then
	if(quarter == 1) then HPI = 0.0280034220617325 + 1.14904274826808 * UNIT_TAOVALUESQ1[3][0] -0.257409472564278 * UNIT_TAOVALUESQ1[3][1] -0.29270287135104 * UNIT_TAOVALUESQ1[3][2] + 0.307757590239275 * UNIT_TAOVALUESQ1[3][3]
	elseif(quarter == 2) then HPI = 0.0280034220617325 + 1.14904274826808 * UNIT_TAOVALUESQ2[3][0] -0.257409472564278 * UNIT_TAOVALUESQ2[3][1] -0.29270287135104 * UNIT_TAOVALUESQ2[3][2] + 0.307757590239275 * UNIT_TAOVALUESQ2[3][3]
	elseif(quarter == 3) then HPI = 0.0280034220617325 + 1.14904274826808 * UNIT_TAOVALUESQ3[3][0] -0.257409472564278 * UNIT_TAOVALUESQ3[3][1] -0.29270287135104 * UNIT_TAOVALUESQ3[3][2] + 0.307757590239275 * UNIT_TAOVALUESQ3[3][3]
	elseif(quarter == 4) then HPI = 0.0280034220617325 + 1.14904274826808 * UNIT_TAOVALUESQ4[3][0] -0.257409472564278 * UNIT_TAOVALUESQ4[3][1] -0.29270287135104 * UNIT_TAOVALUESQ4[3][2] + 0.307757590239275 * UNIT_TAOVALUESQ4[3][3]
	end
	else HPI = HPIfromData
	end
	local totalRevenue = math.exp(revenue + HPI)
	return totalRevenue;
end

function calculateUnitRevenueExecutiveCondo(amenities,unit,logsum,quarter,futureYear,HPIfromData,age)
	local ageCapped = 0
	if age > 25 then ageCapped = 25 else ageCapped = age end
        local agem_25_50 = 0
	if age >= 25 and age <= 50 then agem_25_50 = 1 end
	local agem50 = 0
	if age > 50 then agem50 = 1 end
	local revenue = 0
	revenue = (UNIT_TYPE_COEFFICIENTS[5][0]) + (UNIT_TYPE_COEFFICIENTS[5][1]* math.log(unit.floorArea)) + (UNIT_TYPE_COEFFICIENTS	[5][2] * unit.freehold)  + (UNIT_TYPE_COEFFICIENTS[5][3] * logsum) + (UNIT_TYPE_COEFFICIENTS[5][4] * amenities.pms_1km) + (UNIT_TYPE_COEFFICIENTS[5][5] * amenities.distanceToMall) + (UNIT_TYPE_COEFFICIENTS[5][6]* amenities.mrt_200m) + (UNIT_TYPE_COEFFICIENTS[5][7]* amenities.mrt_400m) + (UNIT_TYPE_COEFFICIENTS[5][8]* amenities.express_200m) + (UNIT_TYPE_COEFFICIENTS[5][9]* amenities.bus_200m) + (UNIT_TYPE_COEFFICIENTS[5][10]* amenities.bus_400m)+ (UNIT_TYPE_COEFFICIENTS[5][11]* ageCapped) + (UNIT_TYPE_COEFFICIENTS[5][12]* (ageCapped*ageCapped))+ 
(UNIT_TYPE_COEFFICIENTS[5][13]* agem_25_50) + (UNIT_TYPE_COEFFICIENTS[5][14]* agem50)
	local HPI = 0
	if(futureYear == 1) then
	if(quarter == 1) then HPI = 0.0179651365079083 + 1.12320737452687 * UNIT_TAOVALUESQ1[5][0] -0.314437384509016 * UNIT_TAOVALUESQ1[5][1] -0.264836990365303 * UNIT_TAOVALUESQ1[5][2] + 0.402537928466658 * UNIT_TAOVALUESQ1[5][3]
	elseif(quarter == 2) then HPI = 0.0179651365079083 + 1.12320737452687 * UNIT_TAOVALUESQ2[5][0] -0.314437384509016 * UNIT_TAOVALUESQ2[5][1] -0.264836990365303 * UNIT_TAOVALUESQ2[5][2] + 0.402537928466658 * UNIT_TAOVALUESQ2[5][3]
	elseif(quarter == 3) then HPI = 0.0179651365079083 + 1.12320737452687 * UNIT_TAOVALUESQ3[5][0] -0.314437384509016 * UNIT_TAOVALUESQ3[5][1] -0.264836990365303 * UNIT_TAOVALUESQ3[5][2] + 0.402537928466658 * UNIT_TAOVALUESQ3[5][3]
	elseif(quarter == 4) then HPI = 0.0179651365079083 + 1.12320737452687 * UNIT_TAOVALUESQ4[5][0] -0.314437384509016 * UNIT_TAOVALUESQ4[5][1] -0.264836990365303 * UNIT_TAOVALUESQ4[5][2] + 0.402537928466658 * UNIT_TAOVALUESQ4[5][3]
	end
	else HPI = HPIfromData
	end
	local totalRevenue = math.exp(revenue + HPI)
	return totalRevenue;
end

function calculateUnitRevenueTerrace(amenities,unit,logsum,quarter,futureYear,HPIfromData,age)
	local ageCapped = 0
	if age > 25 then ageCapped = 25 else ageCapped = age end
        local agem_25_50 = 0
	if age >= 25 and age <= 50 then agem_25_50 = 1 end
	local agem50 = 0
	if age > 50 then agem50 = 1 end
	local revenue = 0
	revenue = (UNIT_TYPE_COEFFICIENTS[1][0]) + (UNIT_TYPE_COEFFICIENTS[1][1]* math.log(unit.floorArea)) + (UNIT_TYPE_COEFFICIENTS	[1][2] * unit.freehold)  + (UNIT_TYPE_COEFFICIENTS[1][3] * logsum) + (UNIT_TYPE_COEFFICIENTS[1][4] * amenities.pms_1km) + (UNIT_TYPE_COEFFICIENTS[1][5] * amenities.distanceToMall) + (UNIT_TYPE_COEFFICIENTS[1][6]* amenities.mrt_200m) + (UNIT_TYPE_COEFFICIENTS[1][7]* amenities.mrt_400m) + (UNIT_TYPE_COEFFICIENTS[1][8]* amenities.express_200m) + (UNIT_TYPE_COEFFICIENTS[1][9]* amenities.bus_200m) + (UNIT_TYPE_COEFFICIENTS[1][10]* amenities.bus_400m)+ (UNIT_TYPE_COEFFICIENTS[1][11]* ageCapped) + (UNIT_TYPE_COEFFICIENTS[1][12]* (ageCapped*ageCapped))+ 
(UNIT_TYPE_COEFFICIENTS[1][13]* agem_25_50) + (UNIT_TYPE_COEFFICIENTS[1][14]* agem50)
	local HPI = 0
	if(futureYear == 1) then
	if(quarter == 1) then HPI = -0.00098316227372815 + 1.36802322312425 * UNIT_TAOVALUESQ1[1][0] -0.728803558563642 * UNIT_TAOVALUESQ1[1][1] -0.11355293427981 * UNIT_TAOVALUESQ1[1][2] + 0.32986523762844 * UNIT_TAOVALUESQ1[1][3]
	elseif(quarter == 2) then HPI = -0.00098316227372815 + 1.36802322312425 * UNIT_TAOVALUESQ2[1][0] -0.728803558563642 * UNIT_TAOVALUESQ2[1][1] -0.11355293427981 * UNIT_TAOVALUESQ2[1][2] + 0.32986523762844 * UNIT_TAOVALUESQ2[1][3]
	elseif(quarter == 3) then HPI = -0.00098316227372815 + 1.36802322312425 * UNIT_TAOVALUESQ3[1][0] -0.728803558563642 * UNIT_TAOVALUESQ3[1][1] -0.11355293427981 * UNIT_TAOVALUESQ3[1][2] + 0.32986523762844 * UNIT_TAOVALUESQ3[1][3]
	elseif(quarter == 4) then HPI = -0.00098316227372815 + 1.36802322312425 * UNIT_TAOVALUESQ4[1][0] -0.728803558563642 * UNIT_TAOVALUESQ4[1][1] -0.11355293427981 * UNIT_TAOVALUESQ4[1][2] + 0.32986523762844 * UNIT_TAOVALUESQ4[1][3]
	end
	else HPI = HPIfromData
	end
	local totalRevenue = math.exp(revenue + HPI)
	return totalRevenue;
end

--[[
    Calculates the revenue for the given future unit.
]]
function calculateUnitRevenue(unit,amenities,logsum,quarter,futureYear,HPIfromData,age)
    local revenuePerUnit = 0
    local buildingTypeId = getBuildingTypeFromUnitType(unit.unitTypeId)
    if (buildingTypeId == 2)then revenuePerUnit = calculateUnitRevenueApartment(amenities,unit,logsum,quarter,futureYear,HPIfromData,age)
    elseif (buildingTypeId == 3)then revenuePerUnit = calculateUnitRevenueCondo(amenities,unit,logsum,quarter,futureYear,HPIfromData,age)
    elseif (buildingTypeId == 5)then revenuePerUnit = calculateUnitRevenueSemiDetatched(amenities,unit,logsum,quarter,futureYear,HPIfromData,age)
    elseif (buildingTypeId == 6)then revenuePerUnit = calculateUnitRevenueDetatched(amenities,unit,logsum,quarter,futureYear,HPIfromData,age)
    elseif (buildingTypeId == 7)then revenuePerUnit = calculateUnitRevenueExecutiveCondo(amenities,unit,logsum,quarter,futureYear,HPIfromData,age)
    elseif (buildingTypeId == 4)then revenuePerUnit = calculateUnitRevenueTerrace(amenities,unit,logsum,quarter,futureYear,HPIfromData,age)
    end
    return revenuePerUnit;
end

