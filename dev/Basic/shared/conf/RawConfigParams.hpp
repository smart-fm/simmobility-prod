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
#include "util/Profiler.hpp"
#include "conf/Constructs.hpp"

namespace sim_mob {


struct Schemas
{
	Schemas();
	bool enabled;
	std::string main_schema;
	std::string calibration_schema;
	std::string public_schema;
	std::string demand_schema;
};

/**
 * Represents the long-term developer model of the config file
 */
struct LongTermParams
{
	LongTermParams();
	bool enabled;
	unsigned int workers;
	unsigned int days;
	unsigned int tickStep;
	unsigned int maxIterations;
	unsigned int year;
	std::string simulationScenario;
	bool resume;
	std::string currentOutputSchema;
	std::string mainSchemaVersion;
	std::string configSchemaVersion;
	std::string calibrationSchemaVersion;
	std::string geometrySchemaVersion;
	unsigned int opSchemaloadingInterval;
	bool initialLoading;

	struct DeveloperModel{
		DeveloperModel();
		bool enabled;
		unsigned int timeInterval;
		int initialPostcode;
		int initialUnitId;
		int initialBuildingId;
		int initialProjectId;
		double minLotSize;
	} developerModel;

	struct HousingModel{
		HousingModel();
		bool enabled;
		unsigned int timeInterval; //time interval before a unit drops its asking price by a certain percentage.
		unsigned int timeOnMarket; //for units on the housing market
		unsigned int timeOffMarket;//for units on the housing market
		float vacantUnitActivationProbability;
		float housingMarketSearchPercentage;
		float housingMoveInDaysInterval;
		int offsetBetweenUnitBuyingAndSelling;
		int bidderUnitsChoiceSet;
		int bidderBTOUnitsChoiceSet;
		int householdBiddingWindow;
		float householdAwakeningPercentageByBTO;
		int offsetBetweenUnitBuyingAndSellingAdvancedPurchase;

		struct AwakeningModel
		{
			AwakeningModel();
			int initialHouseholdsOnMarket;
			int dailyHouseholdAwakenings;
			bool awakenModelJingsi;
			bool awakenModelShan;
			bool awakenModelRandom;
			int awakeningOffMarketSuccessfulBid;
			int awakeningOffMarketUnsuccessfulBid;
		} awakeningModel;
	} housingModel;

	struct OutputHouseholdLogsums
	{
		OutputHouseholdLogsums();
		bool enabled;
		bool vehicleOwnership;
		bool fixedHomeVariableWork;
		bool fixedWorkVariableHome;
	} outputHouseholdLogsums;

	struct VehicleOwnershipModel{
		VehicleOwnershipModel();
		bool enabled;
		unsigned int vehicleBuyingWaitingTimeInDays;
	}vehicleOwnershipModel;

	struct TaxiAccessModel{
		TaxiAccessModel();
		bool enabled;
	}taxiAccessModel;

	struct SchoolAssignmentModel{
		SchoolAssignmentModel();
		bool enabled;
		unsigned int schoolChangeWaitingTimeInDays;
	}schoolAssignmentModel;


	struct OutputFiles{

		OutputFiles();
        bool bids;
        bool expectations;
        bool parcels;
        bool units;
        bool projects;
        bool hh_pc;
        bool units_in_market;
        bool log_taxi_availability;
        bool log_vehicle_ownership;
        bool log_taz_level_logsum;
        bool log_householdgrouplogsum;
        bool log_individual_hits_logsum;
        bool log_householdbidlist;
        bool log_individual_logsum_vo;
        bool log_screeningprobabilities;
        bool log_hhchoiceset;
        bool log_error;
        bool log_school_assignment;
        bool log_pre_school_assignment;
        bool log_hh_awakening;
        bool log_hh_exit;
        bool log_random_nums;
        bool log_dev_roi;
        bool log_household_statistics;
        bool log_out_xx_files;
        bool enabled;

	}outputFiles;


	struct ToaPayohScenario{
		ToaPayohScenario();
		bool enabled;
		bool workInToaPayoh;
		bool liveInToaPayoh;
		bool moveToToaPayoh;
	}toaPayohScenario;


	struct Scenario
	{
		Scenario();
		bool   enabled;
		std::string scenarioName;

	} scenario;

};

///represent the incident data section of the config file
struct IncidentParams {
	IncidentParams() : incidentId(-1), visibilityDistance(0), segmentId(-1), position(0), severity(0),
			capFactor(0), startTime(0), duration(0), length(0),	compliance(0), accessibility(0){}

	struct LaneParams {
		LaneParams() : laneId(0), speedLimit(0), xLaneStartPos(0), yLaneStartPos(0), xLaneEndPos(0),yLaneEndPos(0){}
		unsigned int laneId;
		float speedLimit;
		float xLaneStartPos;
		float yLaneStartPos;
		float xLaneEndPos;
		float yLaneEndPos;
	};

	unsigned int incidentId;
	float visibilityDistance;
	unsigned int segmentId;
	float position;
	unsigned int severity;
	float capFactor;
	unsigned int startTime;
	unsigned int duration;
	float length;
	float compliance;
	float accessibility;
	std::vector<LaneParams> laneParams;
};
/**
 * represent disruption data section of the config file
 */
struct DisruptionParams{
	DisruptionParams():startTime(0),duration(0),id(0){}
	unsigned int id;
	DailyTime startTime;
	DailyTime duration;
	std::vector<std::string> platformNames;
	std::vector<std::string> platformLineIds;
};
/**
 * represent walking time in the train station
 */
struct WalkingTimeParams{
	std::string stationName;
	double alpha = 0.0;
	double beta = 0.0;
};
/**
 * Represents a Person's Characteristic in the config file. (NOTE: Further documentation needed.)
 */
struct PersonCharacteristics
{
	PersonCharacteristics() : lowerAge(0), upperAge(0), lowerSecs(0), upperSecs(0), walkSpeed(0.0)
	{
	}

	unsigned int lowerAge; // lowerAge
	unsigned int upperAge; // upperAge
	int lowerSecs; // lowerSecs
	int upperSecs; // upperSecs
	double walkSpeed; //walking speed in m/s
} ;

/**
 * represent the person characteristics data section of the config file
 */
struct PersonCharacteristicsParams
{
	PersonCharacteristicsParams() : lowestAge(100), highestAge(0), DEFAULT_LOWER_SECS(3), DEFAULT_UPPER_SECS(10)
	{
	}

	int lowestAge;
	int highestAge;
	const int DEFAULT_LOWER_SECS;
	const int DEFAULT_UPPER_SECS;
	
	///Some settings for person characteristics(age range, boarding alighting secs)
	std::map<int, PersonCharacteristics> personCharacteristics;
} ;

/**
 * Represents the "Constructs" section of the config file.
 */
class Constructs {
public:
    std::map<std::string, Database> databases;
    std::map<std::string, Credential> credentials;
};

/**
 * Defines the configuration settings for the closed loop manager
 */
struct ClosedLoopParams
{
    bool enabled;
	bool isGuidanceDirectional;
    int sensorStepSize;
    std::string guidanceFile;
    std::string tollFile;
    std::string incentivesFile;
    std::string sensorOutputFile;
	BasicLogger *logger;

	ClosedLoopParams() : enabled(false), isGuidanceDirectional(false), sensorStepSize(0), guidanceFile(""),
		tollFile(""), incentivesFile(""), sensorOutputFile(""), logger(nullptr)
    {
    }
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

    /// The settings for the closed loop manager
    ClosedLoopParams closedLoop;
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
	PathSetConf() : enabled(false), supplyLinkFile(""), RTTT_Conf(""), DTT_Conf(""), psRetrievalWithoutBannedRegion(""), interval(0), recPS(false), reroute(false),
			perturbationRange(std::pair<unsigned short,unsigned short>(0,0)), kspLevel(0),
			perturbationIteration(0), threadPoolSize(0), maxSegSpeed(0), publickShortestPathLevel(10), simulationApproachIterations(10),
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

    /// supply link travel time file name
    std::string supplyLinkFile;

    /// realtime travel time table name
    std::string RTTT_Conf;

    /// default travel time table name
    std::string DTT_Conf;

    /// pathset retrival (excluding banned area) stored procedure name
    std::string psRetrievalWithoutBannedRegion;

    ///historical travel time updation
    std::string upsert;

    /// travel time recording iterval(in seconds)
    int interval;

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
		double highwayBias;
		UtilityParams()
		{
			highwayBias = 0.5;
		}
	};

    /// represents max_segment_speed attribute in xml, used in travel time based a_star heuristic function
    double maxSegSpeed;

    /// Utility Parameters
	UtilityParams params;

    /// pt route choice model scripts params
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

 struct TrainDwellTimeInfo
 {
	 double dwellTimeAtNormalStation;
	 double dwellTimeAtInterchanges;
	 double dwellTimeAtTerminalStaions;
	 double maxDwellTime;
	 double firstCoeff;
	 double secondCoeff;
	 double thirdCoeff;
	 double fourthCoeff;
 };
struct TrainProperties
{
	TrainProperties() :safeDistance(0), safeHeadway(0),maxCapacity(0)
	{

	}
	/// safe operation distance;
	double safeDistance;
	/// safe operation headway
	double safeHeadway;
	/// train capacity
	unsigned int maxCapacity;
    /// train length
	double trainLength;
	double minDistanceTrainBehindForUnscheduledTrain;
	TrainDwellTimeInfo dwellTimeInfo;
};

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

	double distanceArrivingAtPlatform;
    std::map<const std::string,TrainProperties> trainLinePropertiesMap;
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

struct MobilityServiceControllerConfig {
	unsigned int type;
	unsigned int scheduleComputationPeriod;
	std::vector<std::string> vehicleIds;

	MobilityServiceControllerConfig() : type(0), scheduleComputationPeriod(0) {}
};

/**
 * Represents vehicle controller parameter section
 */
struct MobilityServiceControllerParams
{
    /**
     * Constructor
     */
    MobilityServiceControllerParams() : enabled(false)
    {}

    /// Is vehicle controller enabled?
    bool enabled;

	/// Maps controller IDs to controller configurations
	std::map<unsigned int, MobilityServiceControllerConfig> enabledControllers;
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

	Schemas schemas;

    /// Settings for Long Term Parameters
	LongTermParams ltParams;

    /// pathset configuration file
	std::string pathsetFile;

    /// If loading from the database, how do we connect?
    DatabaseDetails networkDatabase;

    /// If loading population from the database, how do we connect?
    DatabaseDetails populationDatabase;

    /// Represents simulation section
    SimulationParams simulation;

    /// Bus controller parameters
    BusControllerParams busController;

    /// Vehicle controller parameters
    MobilityServiceControllerParams mobilityServiceController;

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





