/*
 * Copyright Singapore-MIT Alliance for Research and Technology
 *
 * File:   MT_Config.hpp
 * Author: zhang huai peng
 *
 * Created on 2 Jun, 2014
 */

#pragma once
#include <string>
#include <map>
#include <vector>

#include <boost/noncopyable.hpp>

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/Constructs.hpp"
#include "conf/RawConfigParams.hpp"
#include "database/DB_Connection.hpp"
#include "util/ProtectedCopyable.hpp"
#include "entities/AuraManager.hpp"

namespace sim_mob
{

/**
 * Represents a vehicleType defined in the config file
 */
struct VehicleType
{
	std::string name;

	double length;

	double width;

	int capacity;

	bool isValidRole(const std::string& role) const;

	bool operator==(const std::string& rhs) const;

	bool operator!=(const std::string& rhs) const;

	VehicleType();

	VehicleType(const std::string& name, const double len,
				const double width, const double capacity);

};

/**
 * Represents an entity in the "Drivers" or "Pedestrians" section of the config file.
 */
struct EntityTemplate
{
	EntityTemplate();

	/**Starting time of the agent's trip (Default is 0)*/
	unsigned int startTimeMs;

	/**Index of the agent's starting lane (Default is 0)*/
	unsigned int startLaneIndex;

	/**The user defined id of the agent (This will override the auto-generated agent id)*/
	int agentId;

	/**The id of the agent's starting segment*/
	int startSegmentId;

	/**The offset from the segment at which the person starts*/
	int segmentStartOffset;

	/**Starting speed of the person (m/s) (Default 0)*/
	double initialSpeed;

	/**The starting node of the person's trip*/
	int originNode;

	/**The destination node of the person's trip*/
	int destNode;

	/**The trip id*/
	std::pair<unsigned int, unsigned int> tripId;

	/**The mode of travel*/
	std::string mode;
};

/**
 * Sources of Agents.
 */
enum LoadAgentsOrderOption
{
	/**Load drivers from the configuration file.*/
	LoadAg_Drivers,
	
	/**Load Pedestrians from the configuration file.*/
	LoadAg_Pedestrians,
	
	/**Load Passengers from the configuration file.*/
	LoadAg_Passengers,
	
	/**Load Trip-Chain based entities from the database.*/
	LoadAg_Database,
};

/**
 * Represents the commsim element defined in the config xml
 */
struct Commsim
{
	bool enabled; ///< True if commsim is enabled. If false, no Broker will be created.
	int numIoThreads; ///< How many threads to allocate to boost's io processor.
	int minClients; ///< The minimum number of simultaneous clients required to proceed with the simulation.
	int holdTick; ///< The simulation tick that we will pause on until minClients connections are made.
	bool useNs3; ///< If true, waits for the ns-3 simulator to connect.

	Commsim() : enabled(false), numIoThreads(1), minClients(1), holdTick(1), useNs3(false)
	{
	}
};

/**
 * Represents the "Workers" section of the config file.
 */
class WorkerParams
{
public:

	struct WorkerConf
	{

		WorkerConf() :
		count(0), granularityMs(0)
		{
		}
		unsigned int count;
		unsigned int granularityMs;
	};

	WorkerConf person;
	WorkerConf signal;
	WorkerConf intersectionMgr;
	WorkerConf communication;
};

/**
 * Represents the network source element of the config file
 */
enum NetworkSource
{
	NETSRC_XML,
	NETSRC_DATABASE,
};

/**
 * Represents the loop-detector_counts section of the configuration file
 */
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
};

/**
 * Represents the short-term_density-map section of the configuration file
 */
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
};

/**
 * Represents the FMOD controller section of the config file.
 */
struct FMOD_ControllerParams
{

	FMOD_ControllerParams() :
	enabled(false), port(0), updateTimeMS(0), blockingTimeSec(0)
	{
	}

	bool enabled;
	std::string ipAddress;
	unsigned int port;
	unsigned int updateTimeMS;
	std::string mapfile;
	unsigned int blockingTimeSec;
	std::map<std::string, TripChainItem*> allItems;
};

/**
 * Represents the AMOD Controller section in the config file
 */
struct AMOD_ControllerParams
{

	AMOD_ControllerParams() : enabled(false)
	{
	}

	bool enabled;
};

class ST_Config : public boost::noncopyable
{
public:

	/**
	 * Destructor
	 */
	~ST_Config()
	{
	}

	/**
	 * retrieves the singleton instance of ST_Config instance
	 *
	 * @return reference to the singleton instance of ST_Config
	 */
	static ST_Config& getInstance();

	/**
	 * Deletes the singleton instance of ST_Config
	 */
	static void deleteIntance();

	/**
	 * checks whether commsim is enabled
	 *
	 * @return true if commsim is enabled, else false
	 */
	bool commSimEnabled() const;

	/**
	 * If loading the network from an XML file, which file? Empty=private/SimMobilityInput.xml
	 *
	 * @return Network Xml input file name
	 */
	std::string& getNetworkXmlInputFile();

	/**
	 * If loading the network from an XML file, which file? Empty=private/SimMobilityInput.xml
	 *
	 * @return Network Xml input file name (const reference)
	 */
	const std::string& getNetworkXmlInputFile() const;

	/**
	 * If writing the network to an XML file, which file? Empty= dont write at all
	 *
	 * @return Network Xml output file name
	 */
	std::string& getNetworkXmlOutputFile();

	/**
	 * If writing the network to an XML file, which file? Empty= dont write at all
	 *
	 * @return Network Xml output file name (const reference)
	 */
	const std::string& getNetworkXmlOutputFile() const;

	/**
	 * retrieves road network xsd schema file name
	 * If empty, use the default provided in "xsi:schemaLocation".
	 *
	 * @return road network xsd file name
	 */
	std::string& getRoadNetworkXsdSchemaFile();

	/**
	 * retrieves road network xsd schema file name
	 * If empty, use the default provided in "xsi:schemaLocation".
	 *
	 * @return road network xsd file name (const reference)
	 */
	const std::string& getRoadNetworkXsdSchemaFile() const;

	/**
	 * sets the road network xsd schema file name
	 *
	 * @param name road network xsd schema file name
	 */
	void setRoadNetworkXsdSchemaFile(std::string& name);

	/**
	 * Number of workers for handling persons in the simulation
	 *
	 * @return number of workers
	 */
	unsigned int& personWorkGroupSize();

	/**
	 * Number of workers for handling persons in the simulation
	 *
	 * @return number of workers (const)
	 */
	const unsigned int& personWorkGroupSize() const;

	/**
	 * Number of workers for handling signals in the simulation
	 *
	 * @return number of workers
	 */
	unsigned int& signalWorkGroupSize();

	/**
	 * Number of workers for handling signals in the simulation
	 *
	 * @return number of workers (const)
	 */
	const unsigned int& signalWorkGroupSize() const;

	/**
	 * Number of workers for handling intersection manangers
	 *
	 * @return number of workers
	 */
	unsigned int& intMgrWorkGroupSize();

	/**
	 * Number of workers for handling intersection manangers
	 *
	 * @return number of workers (const)
	 */
	const unsigned int& intMgrWorkGroupSize() const;

	/**
	 * Number of workers for handling communication brokers
	 *
	 * @return number of workers
	 */
	unsigned int& commWorkGroupSize();

	/**
	 * Number of workers for handling communication brokers
	 *
	 * @return number of workers (const)
	 */
	const unsigned int& commWorkGroupSize() const;

	///Which tree implementation to use for spatial partitioning for the aura manager.
	AuraManager::AuraManagerImplementation& aura_manager_impl();
	const AuraManager::AuraManagerImplementation& aura_manager_impl() const;

	unsigned int personTimeStepInMilliSeconds() const;

	unsigned int signalTimeStepInMilliSeconds() const;

	unsigned int intMgrTimeStepInMilliSeconds() const;

	unsigned int communicationTimeStepInMilliSeconds() const;

	/// Container to store vehicle types
	std::vector<VehicleType> vehicleTypes;

	/// Tripfile map
	std::map<std::string, std::string> tripFiles;

	/// Future agents (drivers, pedestrains, passengers.. etc) map
	std::map<std::string, std::vector<EntityTemplate> > futureAgents;

	/// Container for storing load agents order
	std::vector<LoadAgentsOrderOption> loadAgentsOrder;

	/// CommSim element
	Commsim commsim;

	/// Type of aura-manager used
	AuraManager::AuraManagerImplementation auraManagerImplementation;

	/// Property specific to MPI version; not fully documented.
	int partitioningSolutionId;

	/// Worker (person, signal, intersection..) configuration
	WorkerParams workers;

	/// Whether to load the network from the database or from an XML file.
	NetworkSource networkSource;

	/// If loading the network from an XML file, which file? Empty=private/SimMobilityInput.xml
	std::string networkXmlInputFile;

	/// If loading the network from an XML file, which file? Empty=private/SimMobilityInput.xml
	std::string networkXmlOutputFile;

	/// Valid path to a schema file for loading XML road network files.
	std::string roadNetworkXsdSchemaFile;

	///Settings for the loop detector counts
	LoopDetectorCounts loopDetectorCounts;

	///Settings for the short-term density map
	SegmentDensityMap segDensityMap;

	///Settings for the FMOD controller.
	FMOD_ControllerParams fmod;

	///Settings for the AMOD controller
	AMOD_ControllerParams amod;

	///Settings used for generation/retrieval of paths
	PathSetConf pathset;

	/// Generic properties, for testing new features.
	std::map<std::string, std::string> genericProps;

	unsigned int granPersonTicks; ///<Number of ticks to wait before updating all Person agents.
	unsigned int granSignalsTicks; ///<Number of ticks to wait before updating all signals.
	unsigned int granIntMgrTicks; ///<Number of ticks to wait before updating all intersection managers.
	unsigned int granCommunicationTicks; ///<Number of ticks to wait before updating all communication brokers.
private:
	/**
	 * Constructor
	 */
	ST_Config();

	/// Singleton instance
	static ST_Config* instance;
};

}
