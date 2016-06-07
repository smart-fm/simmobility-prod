//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>
#include <map>
#include <string>

#include "buffering/Shared.hpp"
#include "conf/CMakeConfigParams.hpp"
#include "conf/Constructs.hpp"
#include "geospatial/network/Point.hpp"
#include "workers/WorkGroup.hpp"
#include "util/DailyTime.hpp"
#include "conf/Constructs.hpp"

namespace sim_mob {

/**
 * Represents the long-term developer model of the config file
 */
struct LongTermParams{
    LongTermParams();
    bool enabled;
    unsigned int workers;
    unsigned int days;
    unsigned int tickStep;
    unsigned int maxIterations;

    struct DeveloperModel{
        DeveloperModel();
        bool enabled;
        unsigned int timeInterval;
        int initialPostcode;
        int initialUnitId;
        int initialBuildingId;
        int initialProjectId;
        int year;
        double minLotSize;
    } developerModel;

    struct HousingModel{
        HousingModel();
        bool enabled;
        unsigned int timeInterval; ///time interval before a unit drops its asking price by a certain percentage.
        unsigned int timeOnMarket; ///for units on the housing market
        unsigned int timeOffMarket;///for units on the housing market
        float vacantUnitActivationProbability;
        int initialHouseholdsOnMarket;
        float housingMarketSearchPercentage;
        float housingMoveInDaysInterval;
        bool  outputHouseholdLogsums;
        int offsetBetweenUnitBuyingAndSelling;
        int bidderUnitsChoiceSet;
        int householdBiddingWindow;
    } housingModel;

    struct VehicleOwnershipModel{
        VehicleOwnershipModel();
        bool enabled;
        unsigned int vehicleBuyingWaitingTimeInDays;
    }vehicleOwnershipModel;
};

/**
 * Represents a Person's Characteristic in the config file. (NOTE: Further documentation needed.)
 */
struct PersonCharacteristics {
	PersonCharacteristics() : lowerAge(0), upperAge(0), lowerSecs(0), upperSecs(0) {}

	unsigned int lowerAge; // lowerAge
	unsigned int upperAge; // upperAge
	int lowerSecs;// lowerSecs
	int upperSecs;// upperSecs
};

/**
 * represent the person characteristics data section of the config file
 */
struct PersonCharacteristicsParams {

	PersonCharacteristicsParams() : lowestAge(100), highestAge(0), DEFAULT_LOWER_SECS(3), DEFAULT_UPPER_SECS(10) {}
	int lowestAge;
	int highestAge;
	const int DEFAULT_LOWER_SECS;
	const int DEFAULT_UPPER_SECS;
    ///Some settings for person characteristics(age range, boarding alighting secs)
	std::map<int, PersonCharacteristics> personCharacteristics;
};

/**
 * Represents the "Constructs" section of the config file.
 */
class Constructs {
public:
    std::map<std::string, Database> databases;
    std::map<std::string, Credential> credentials;
};

/**
 * Represents the "Simulation" section of the config file.
 */
class SimulationParams {
public:
    /**
     * Constructor
     */
	SimulationParams();

    /// Base system granularity, in milliseconds. Each "tick" is this long.
    unsigned int baseGranMS;

    /// Base system granularity, in seconds. Each "tick" is this long.
    double baseGranSecond;

    /// Total time (in milliseconds) to run the simulation for. (Includes "warmup" time.)
    unsigned int totalRuntimeMS;

    /// Total time (in milliseconds) considered "warmup".
    unsigned int totalWarmupMS;

    /// When the simulation begins(based on configuration)
    DailyTime simStartTime;
	
    /// Indicates the percentage of persons which will use in-simulation travel times instead of historical travel times
    unsigned int inSimulationTTUsage;

    /// Defautl assignment strategy for Workgroups.
    WorkGroup::ASSIGNMENT_STRATEGY workGroupAssigmentStrategy;

    /// Default starting ID for agents with auto-generated IDs.
    int startingAutoAgentID;

    /// Locking strategy for Shared<> properties.
    sim_mob::MutexStrategy mutexStategy;
};

/**
 * Represents a complete connection to the database, via Construct ID.
 */
struct DatabaseDetails {
    /// Key for database detail in constructs
    std::string database;

    /// Key for credential detail in constructs
    std::string credentials;

    /// Key for stored procedures in proceduremaps
    std::string procedures;
};

/**
 * contains the path and finle names of external scripts used in the simulation
 *
 * \author Harish Loganathan
 */
class ModelScriptsMap
{
public:
    /**
     * Contructor
     *
     * @param scriptFilesPath - path where the scripts are located
     * @param scriptsLang - script language (lua/python..)
     */
	ModelScriptsMap(const std::string& scriptFilesPath = "", const std::string& scriptsLang = "");

    /**
     * Retrives the scripts file path
     *
     * @return scripts file path
     */
	const std::string& getPath() const
	{
		return path;
	}

    /**
     * Retrieves the script language
     *
     * @return script language
     */
	const std::string& getScriptLanguage() const
	{
		return scriptLanguage;
	}

    /**
     * Retrives a script file name given its key
     *
     * @param key
     *
     * @return script file name
     */
	std::string getScriptFileName(std::string key) const
	{
        /// at() is used intentionally so that an out_of_range exception is triggered when invalid key is passed
		return scriptFileNameMap.at(key);
	}

    /**
     * Sets a script file name
     *
     * @param key
     * @param value filename
     */
	void addScriptFileName(const std::string& key, const std::string& value)
	{
		this->scriptFileNameMap[key] = value;
	}

private:
    /// scripts file path
	std::string path;

    /// script language
	std::string scriptLanguage;

    /// Scripts file map
    std::map<std::string, std::string> scriptFileNameMap;
};

/**
 * Representation of pathset config file
 */
struct PathSetConf
{
    /**
     * Constructor
     */
	PathSetConf() : enabled(false), RTTT_Conf(""), DTT_Conf(""), psRetrieval(""), psRetrievalWithoutBannedRegion(""), interval(0), recPS(false), reroute(false),
			perturbationRange(std::pair<unsigned short,unsigned short>(0,0)), kspLevel(0),
			perturbationIteration(0), threadPoolSize(0), alpha(0), maxSegSpeed(0), publickShortestPathLevel(10), simulationApproachIterations(10),
			publicPathSetEnabled(true), privatePathSetEnabled(true)
	{}

    /// Whether pathset enabled
	bool enabled;

    /// Whether private pathset enabled
	bool privatePathSetEnabled;

    /// pathset operation mode "normal" , "generation"(for bulk pathset generation)
    std::string privatePathSetMode;

    /// Whether public pathset enabled
	bool publicPathSetEnabled;

    /// public pathset mode (normal / generation)
	std::string publicPathSetMode;

    /// public pathset od source table
	std::string publicPathSetOdSource;

    /// Public pathset output file
	std::string publicPathSetOutputFile;

    /// Public PathSet Generation Algorithm Configurations
    /// 'k' value for kShortestPathAlgorithm
    int publickShortestPathLevel;

    /// Num of simulation approach iterations
	int simulationApproachIterations;

    /// thread pool size for pathset generation
	int threadPoolSize;

    /// in case of using pathset manager in "generation" mode, the results will be outputted to this file
    std::string bulkFile;

    /// data source for getting ODs for bulk pathset generation
    std::string odSourceTableName;

    /// path set table name
    std::string pathSetTableName;

    /// realtime travel time table name
    std::string RTTT_Conf;

    /// default travel time table name
    std::string DTT_Conf;

    /// pathset retrieval stored procedure name
    std::string psRetrieval;

    /// pathset retrival (excluding banned area) stored procedure name
    std::string psRetrievalWithoutBannedRegion;

    ///historical travel time updation
    std::string upsert;

    /// travel time recording iterval(in seconds)
    int interval;

    /// travel time updation coefficient
    double alpha;

    ///	recursive pathset Generation
	bool recPS;

    ///	 enable rerouting?
	bool reroute;

    ///	number of iterations in random perturbation
	int perturbationIteration;

    ///	range of uniform distribution in random perturbation
	std::pair<unsigned short,unsigned short> perturbationRange;

    ///k-shortest path level
	int kspLevel;

    /// Link Elimination types
	std::vector<std::string> LE;

	/// Utility parameters
	struct UtilityParams
	{

		double bTTVOT;
		double bCommonFactor;
		double bLength;
		double bHighway;
		double bCost;
		double bSigInter;
		double bLeftTurns;
		double bWork;
		double bLeisure;
		double highwayBias;
		double minTravelTimeParam;
		double minDistanceParam;
		double minSignalParam;
		double maxHighwayParam;
		UtilityParams()
		{
            bTTVOT = -0.01373;///-0.0108879;
			bCommonFactor = 1.0;
            bLength = -0.001025;///0.0; ///negative sign proposed by milan
            bHighway = 0.00052;///0.0;
			bCost = 0.0;
            bSigInter = -0.13;///0.0;
			bLeftTurns = 0.0;
			bWork = 0.0;
			bLeisure = 0.0;
			highwayBias = 0.5;
			minTravelTimeParam = 0.879;
			minDistanceParam = 0.325;
			minSignalParam = 0.256;
			maxHighwayParam = 0.422;
		}
	};

    /// represents max_segment_speed attribute in xml, used in travel time based a_star heuristic function
    double maxSegSpeed;

    /// Utility Parameters
	UtilityParams params;

    /// pt route choice model scripts params
	ModelScriptsMap ptRouteChoiceScriptsMap;
	ModelScriptsMap ServiceControllerScriptsMap;
};

/**
 * Represents bus controller parameter section
 */
struct BusControllerParams
{
    /**
     * Constructor
     */
    BusControllerParams() : enabled(false), busLineControlType("")
    {}

    /// Is bus controller enabled?
    bool enabled;

    /// bus line control type
    std::string busLineControlType;
};

/**
 * Represents train controller parameter section
 */
struct TrainControllerParams
{
    /**
     * Constructor
     */
	TrainControllerParams() : enabled(false), trainControlType(""),
			safeDistance(0), safeHeadway(0), miniDwellTime(0),
			maxDwellTime(0),outputEnabled(false),maxCapacity(0)
    {}

    /// Is train controller enabled?
    bool enabled;

    /// train line control type
    std::string trainControlType;
    /// safe operation distance;
    double safeDistance;
    /// safe operation headway
    double safeHeadway;
    /// dwell time minimum value
    double miniDwellTime;
    /// dwell time maximum value
    double maxDwellTime;
    /// train output enabled
    bool outputEnabled;
    /// train capacity
    unsigned int maxCapacity;
};

/**
 * Represents a Bust Stop in the config file. (NOTE: Further documentation needed.)
 */
struct BusStopScheduledTime {
    BusStopScheduledTime() : offsetAT(0), offsetDT(0) {}

    unsigned int offsetAT; //<Presumably arrival time?
    unsigned int offsetDT; //<Presumably departure time?
};

struct TravelTimeConfig {
	unsigned int intervalMS;
	std::string fileName;
	bool enabled;

	TravelTimeConfig() : intervalMS(0), fileName(""), enabled(false) {}
};

/**
 * Contains the properties of the config file as they appear in, e.g., test_road_network.xml, with
 *   minimal conversion.
 * Derived properties (such as the road network) are listed in ConfigParams.
 *
 * \author Seth N. Hetu
 */
class RawConfigParams : public sim_mob::CMakeConfigParams {
protected:
    /// Pathset config
    PathSetConf pathset;

public:
    /**
     * Constructor
     */
	RawConfigParams();

    /**
     * Enumerator for simmobility running mode
     */
    enum SimMobRunMode
    {
    	UNKNOWN_RUN_MODE,
        SHORT_TERM,
        MID_TERM,
        LONG_TERM
    };

    /// Simmobility running mode
    SimMobRunMode simMobRunMode;

    /// "Constructs" for general re-use.
	Constructs constructs;

    /// Settings for Long Term Parameters
	LongTermParams ltParams;

    /// pathset configuration file
	std::string pathsetFile;

    /// If loading from the database, how do we connect?
    DatabaseDetails networkDatabase;

    /// Represents simulation section
    SimulationParams simulation;

    /// Bus controller parameters
    BusControllerParams busController;


    /// Train controller parameters
    TrainControllerParams trainController;

    //OD Travel Time configurations
    TravelTimeConfig odTTConfig;

    //OD Travel Time configurations
	TravelTimeConfig rsTTConfig;


    ///Some settings for bus stop arrivals/departures.
    std::map<int, BusStopScheduledTime> busScheduledTimes; //The int is a "bus stop ID", starting from 0.

    /// Stored procedure mappings
    std::map<std::string, StoredProcedureMap> procedureMaps;

    /// Generic properties, for testing new features.
    std::map<std::string, std::string> genericProps;

    /// If true, we take time to merge the output of the individual log files after the simulation is complete.
    bool mergeLogFiles;

    /// If true, bus routes will be generated by buscontroller
    bool generateBusRoutes;

    /// subtrip level travel metrics output enabled
    bool subTripTravelTimeEnabled;

    /// subtrip level travel metrics output file
    std::string subTripLevelTravelTimeOutput;

    /// Person characteristics parameters
	PersonCharacteristicsParams personCharacteristicsParams;

    /// container for lua scripts
	ModelScriptsMap luaScriptsMap;
};


}
