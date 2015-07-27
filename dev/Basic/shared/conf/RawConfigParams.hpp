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
#include "entities/AuraManager.hpp"
#include "geospatial/Point2D.hpp"
#include "workers/WorkGroup.hpp"
#include "util/DailyTime.hpp"

//TODO: Need to move the useful "Constructs" out of this file (e.g., Passwords)
#include "conf/Constructs.hpp"

namespace sim_mob {

///Represents the FMOD controller section of the config file.
struct FMOD_ControllerParams {
	FMOD_ControllerParams() : enabled(false), port(0), updateTimeMS(0), blockingTimeSec(0) {}

	bool enabled;
	std::string ipAddress;
	unsigned int port;
	unsigned int updateTimeMS;
	std::string mapfile;
	unsigned int blockingTimeSec;
};

struct AMOD_ControllerParams
{
	AMOD_ControllerParams() : enabled(false) {}

	bool enabled;
};

//Represents the long-term developer model of the config file
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
	} developerModel;

	struct HousingModel{
		HousingModel();
		bool enabled;
		unsigned int timeInterval;
		unsigned int timeOnMarket;
		unsigned int timeOffMarket;
		float vacantUnitActivationProbability;
		int initialHouseholdsOnMarket;
		float housingMarketSearchPercentage;
		float housingMoveInDaysInterval;
		bool  outputHouseholdLogsums;
	} housingModel;

	struct VehicleOwnershipModel{
		VehicleOwnershipModel();
		bool enabled;
		unsigned int vehicleBuyingWaitingTimeInDays;
	}vehicleOwnershipModel;
};

///Represents the loop-detector_counts section of the configuration file
struct LoopDetectorCounts
{
  LoopDetectorCounts() : frequency(0), outputEnabled(false), fileName("")
  {
  }
  
  ///The frequency of aggregating the vehicle counts at the loop detector
  unsigned int frequency;
  
  ///Indicates whether the counts have to be output to a file
  bool outputEnabled;
  
  ///Name of the output file
  std::string fileName;
} ;

///Represents the short-term_density-map section of the configuration file
struct SegmentDensityMap
{
  SegmentDensityMap() : updateInterval(0), outputEnabled(false), fileName("")
  {
  }
  
  ///The interval at which the density map is to be output
  unsigned int updateInterval;
  
  ///Indicates whether the density map is to be output to a file
  bool outputEnabled;
  
  ///Name of the output file
  std::string fileName;
} ;

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

///Represents a Person's Characteristic in the config file. (NOTE: Further documentation needed.)
struct PersonCharacteristics {
	PersonCharacteristics() : lowerAge(0), upperAge(0), lowerSecs(0), upperSecs(0) {}

	unsigned int lowerAge; // lowerAge
	unsigned int upperAge; // upperAge
	int lowerSecs;// lowerSecs
	int upperSecs;// upperSecs
};

///represent the person characteristics data section of the config file
struct PersonCharacteristicsParams {

	PersonCharacteristicsParams() : lowestAge(100), highestAge(0), DEFAULT_LOWER_SECS(3), DEFAULT_UPPER_SECS(10) {}
	int lowestAge;
	int highestAge;
	const int DEFAULT_LOWER_SECS;
	const int DEFAULT_UPPER_SECS;
	//Some settings for person characteristics(age range, boarding alighting secs)
	std::map<int, PersonCharacteristics> personCharacteristics;
};

///Represents a Bust Stop in the config file. (NOTE: Further documentation needed.)
struct BusStopScheduledTime {
	BusStopScheduledTime() : offsetAT(0), offsetDT(0) {}

	unsigned int offsetAT; //<Presumably arrival time?
	unsigned int offsetDT; //<Presumably departure time?
};

///Represents a complete connection to the database, via Construct ID.
struct DatabaseDetails {
	std::string database;
	std::string credentials;
	std::string procedures;
};


///Represents the "Constructs" section of the config file.
class Constructs {
public:
	//std::map<std::string, Distribution> distributions; //<TODO
	std::map<std::string, Database> databases;
	std::map<std::string, StoredProcedureMap> procedureMaps;
	std::map<std::string, Credential> credentials;
};

///Represents the "Simulation" section of the config file.
class SimulationParams {
public:
	SimulationParams();

	///Sources of Agents.
	enum LoadAgentsOrderOption {
		LoadAg_Drivers,       ///<Load Drivers from the config file.
		LoadAg_Pedestrians,   ///<Load Pedestrians from the config file.
		LoadAg_Passengers,    ///<Load Passengers from the config file.
		LoadAg_Database,      ///<Load Trip-Chain based entities from the database.
		LoadAg_XmlTripChains, ///<Not sure what this does exactly....
	};

	///Our reaction time distributions.
	struct ReactionTimeDistDescription {
		ReactionTimeDistDescription() : typeId(0), mean(0), stdev(0) {}

		int typeId;
		int mean;
		int stdev;
	};

	unsigned int baseGranMS;       ///<Base system granularity, in milliseconds. Each "tick" is this long.
	double baseGranSecond;         ///<Base system granularity, in seconds. Each "tick" is this long.
	unsigned int totalRuntimeMS;   ///<Total time (in milliseconds) to run the simulation for. (Includes "warmup" time.)
	unsigned int totalWarmupMS;    ///<Total time (in milliseconds) considered "warmup".

	DailyTime simStartTime; ///<When the simulation begins(based on configuration)

	AuraManager::AuraManagerImplementation auraManagerImplementation; ///<What type of Aura Manager we're using.

	WorkGroup::ASSIGNMENT_STRATEGY workGroupAssigmentStrategy;  ///<Defautl assignment strategy for Workgroups.

	int partitioningSolutionId;  ///<Property specific to MPI version; not fully documented.

	std::vector<LoadAgentsOrderOption> loadAgentsOrder; ///<What order to load agents in.

	int startingAutoAgentID; ///<Default starting ID for agents with auto-generated IDs.

	sim_mob::MutexStrategy mutexStategy; ///<Locking strategy for Shared<> properties.

	struct Commsim {
		bool enabled;  ///< True if commsim is enabled. If false, no Broker will be created.
		int numIoThreads; ///< How many threads to allocate to boost's io processor.
		int minClients;  ///< The minimum number of simultaneous clients required to proceed with the simulation.
		int holdTick;    ///< The simulation tick that we will pause on until minClients connections are made.
		bool useNs3;  ///< If true, waits for the ns-3 simulator to connect.
		Commsim() : enabled(false), numIoThreads(1), minClients(1), holdTick(1), useNs3(false) {}
	};
	Commsim commsim;

	//Reaction time parameters.
	//TODO: This should be one of the first areas we clean up.
	ReactionTimeDistDescription reactTimeDistribution1;
	ReactionTimeDistDescription reactTimeDistribution2;

    //Passenger distribution parameters.
    //TODO: This should be the second thing we clean up.
    int passenger_distribution_busstop;
    int passenger_mean_busstop;
    int passenger_standardDev_busstop;
    int passenger_percent_boarding;
    int passenger_percent_alighting;
    int passenger_min_uniform_distribution;
    int passenger_max_uniform_distribution;
};

///Represents the "Workers" section of the config file.
class WorkerParams {
public:
	struct Worker {
		Worker();
		unsigned int count;
		unsigned int granularityMs;
	};

	Worker person;
	Worker signal;
	Worker intersectionMgr;
	Worker communication;
};


///Represents the "System" section of the config file.
class SystemParams {
public:
	SystemParams();

	enum NetworkSource {
		NETSRC_XML,
		NETSRC_DATABASE,
	};

	SimulationParams simulation;
	WorkerParams workers;

	bool singleThreaded; ///<If true, we are running everything on one thread.
	bool mergeLogFiles;  ///<If true, we take time to merge the output of the individual log files after the simulation is complete.

	NetworkSource networkSource; ///<Whethere to load the network from the database or from an XML file.
	std::string networkXmlInputFile;  ///<If loading the network from an XML file, which file? Empty=private/SimMobilityInput.xml
	std::string networkXmlOutputFile;  ///<If loading the network from an XML file, which file? Empty=private/SimMobilityInput.xml
	DatabaseDetails networkDatabase; //<If loading from the database, how do we connect?

	std::string roadNetworkXsdSchemaFile; ///<Valid path to a schema file for loading XML road network files.

	std::map<std::string, std::string> genericProps; ///<Generic properties, for testing new features.
};


///Represents an entity in the "Drivers" or "Pedestrians" section of the config file.
struct EntityTemplate {
	EntityTemplate();
	Point2D originPos;
	Point2D destPos;
	unsigned int startTimeMs;// default is zero
	unsigned int laneIndex;// default is zero
	int angentId;
	int initSegId;
	int initDis;
	double initSpeed;
	int originNode;
	int destNode;
};

/**
 * contains the path and finle names of external scripts used in the simulation
 *
 * \author Harish Loganathan
 */
class ModelScriptsMap
{
public:
	ModelScriptsMap(const std::string& scriptFilesPath = "", const std::string& scriptsLang = "");

	const std::string& getPath() const
	{
		return path;
	}

	const std::string& getScriptLanguage() const
	{
		return scriptLanguage;
	}

	std::string getScriptFileName(std::string key) const
	{
		//at() is used intentionally so that an out_of_range exception is triggered when invalid key is passed
		return scriptFileNameMap.at(key);
	}

	void addScriptFileName(const std::string& key, const std::string& value)
	{
		this->scriptFileNameMap[key] = value;
	}

private:
	std::string path;
	std::string scriptLanguage;
	std::map<std::string, std::string> scriptFileNameMap; //key=>value
};

///Represents the loop-detector_counts section of the configuration file
struct ScreenLineParams
{
	ScreenLineParams() : interval(0), outputEnabled(false), fileName("") {}

	///The frequency of aggregating the vehicle counts at the loop detector
	unsigned int interval;

	///Indicates whether the counts have to be output to a file
	bool outputEnabled;

	///Name of the output file
	std::string fileName;
};

struct PathSetConf
{
	PathSetConf() : enabled(false), RTTT_Conf(""), DTT_Conf(""), psRetrieval(""), psRetrievalWithoutBannedRegion(""), interval(0), recPS(false), reroute(false),
			subTripOP(""), perturbationRange(std::pair<unsigned short,unsigned short>(0,0)), kspLevel(0),
			perturbationIteration(0), threadPoolSize(0), alpha(0), maxSegSpeed(0), publickShortestPathLevel(10), simulationApproachIterations(10),
			publicPathSetEnabled(true), privatePathSetEnabled(true)
	{}
	bool enabled;
	bool privatePathSetEnabled;
	std::string privatePathSetMode;//pathset operation mode "normal" , "generation"(for bulk pathset generation)

	bool publicPathSetEnabled;
	std::string publicPathSetMode;

	std::string publicPathSetOdSource;
	std::string publicPathSetOutputFile;
	// Public PathSet Generation Algorithm Configurations

	int publickShortestPathLevel;
	int simulationApproachIterations;

	int threadPoolSize;
	std::string bulkFile; //in case of using pathset manager in "generation" mode, the results will be outputted to this file
	std::string odSourceTableName; //data source for getting ODs for bulk pathset generation
	std::string pathSetTableName;
	std::string RTTT_Conf;//realtime travel time table name
	std::string DTT_Conf;//default travel time table name
	std::string psRetrieval;// pathset retrieval stored procedure name
	std::string psRetrievalWithoutBannedRegion; // pathset retrival (excluding banned area) stored procedure name
	std::string upsert;//	historical travel time updation
	int interval; //travel time recording iterval(in seconds)
	double alpha; //travel time updation coefficient
	///	recursive pathset Generation
	bool recPS;
	///	 enable rerouting?
	bool reroute;
	/// subtrip level travel metrics output file(for preday use)
	std::string subTripOP;
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
			bTTVOT = -0.01373;//-0.0108879;
			bCommonFactor = 1.0;
			bLength = -0.001025;//0.0; //negative sign proposed by milan
			bHighway = 0.00052;//0.0;
			bCost = 0.0;
			bSigInter = -0.13;//0.0;
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
	double maxSegSpeed; //represents max_segment_speed attribute in xml, used in travel time based a_star heuristic function
	/// Utility Parameters
	UtilityParams params;

	///pt route choice model scripts params
	ModelScriptsMap ptRouteChoiceScriptsMap;
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
	///Settings used for generation/retrieval of paths
	PathSetConf pathset;
public:
	RawConfigParams();

	///"Constructs" for general re-use.
	Constructs constructs;

	///"Sytem" level settings, including simulation parameters and global flags.
	SystemParams system;

	///Available gemetries (currently only database geometries).
	//GeometryParams geometry;

	///Settings for the FMOD controller.
	FMOD_ControllerParams fmod;

	///Settings for the AMOD controller
	AMOD_ControllerParams amod;

	///Settings for Long Term Parameters
	LongTermParams ltParams;

	///pathset configuration file
	std::string pathsetFile;
        
	///Settings for the loop detector counts
	LoopDetectorCounts loopDetectorCounts;
        
	///Settings for the short-term density map
	SegmentDensityMap segDensityMap;

	///Settings for the Screen Line Count
	ScreenLineParams screenLineParams;

	///	is CBD area restriction enforced
	bool cbd;
	bool generateBusRoutes;

	// Public transit enabled if this flag set to true
	bool publicTransitEnabled;
        
	///setting for the incidents
	std::vector<IncidentParams> incidents;

	///Some settings for bus stop arrivals/departures.
	std::map<int, BusStopScheduledTime> busScheduledTimes; //The int is a "bus stop ID", starting from 0.

	//Person characteristics parameters
	PersonCharacteristicsParams personCharacteristicsParams;

	///container for lua scripts
	ModelScriptsMap luaScriptsMap;

	//@{
	///Templates for creating entities of various types.
	std::vector<EntityTemplate> driverTemplates;
	std::vector<EntityTemplate> taxiDriverTemplates;
	std::vector<EntityTemplate> pedestrianTemplates;
	std::vector<EntityTemplate> busDriverTemplates;
	std::vector<EntityTemplate> signalTemplates;
	std::vector<EntityTemplate> passengerTemplates;
	std::vector<EntityTemplate> busControllerTemplates;
	//@}
};


}
