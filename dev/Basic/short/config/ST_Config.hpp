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

    std::vector<std::string> associatedRoles;

    bool isValidRole(const std::string& role) const;

    bool operator==(const VehicleType& rhs) const;

    bool operator!=(const VehicleType& rhs) const;

    VehicleType();
};

/**
 * Represents an entity in the "Drivers" or "Pedestrians" section of the config file.
 */
struct EntityTemplate {
    EntityTemplate();
    Point2D originPos;
    Point2D destPos;
    unsigned int startTimeMs;// default is zero
    unsigned int laneIndex;// default is zero
    int agentId;
    int initSegId;
    int initDis;
    double initSpeed;
    int originNode;
    int destNode;
    std::pair<unsigned int, unsigned int> tripId;
    std::string vehicleType;
    std::string roleName;
};

/**
 * Sources of Agents.
 */
enum LoadAgentsOrderOption {
    LoadAg_Drivers,       ///<Load Drivers from the config file.
    LoadAg_Pedestrians,   ///<Load Pedestrians from the config file.
    LoadAg_Passengers,    ///<Load Passengers from the config file.
    LoadAg_Database,      ///<Load Trip-Chain based entities from the database.
    LoadAg_XmlTripChains, ///<Not sure what this does exactly....
};

/**
 * Represents the commsim element defined in the config xml
 */
struct Commsim {
    bool enabled;  ///< True if commsim is enabled. If false, no Broker will be created.
    int numIoThreads; ///< How many threads to allocate to boost's io processor.
    int minClients;  ///< The minimum number of simultaneous clients required to proceed with the simulation.
    int holdTick;    ///< The simulation tick that we will pause on until minClients connections are made.
    bool useNs3;  ///< If true, waits for the ns-3 simulator to connect.
    Commsim() : enabled(false), numIoThreads(1), minClients(1), holdTick(1), useNs3(false) {}
};

/**
 * Represents the "Workers" section of the config file.
 */
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

/**
 * Represents the network source element of the config file
 */
enum NetworkSource {
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
struct FMOD_ControllerParams {
    FMOD_ControllerParams() : enabled(false), port(0), updateTimeMS(0), blockingTimeSec(0) {}

    bool enabled;
    std::string ipAddress;
    unsigned int port;
    unsigned int updateTimeMS;
    std::string mapfile;
    unsigned int blockingTimeSec;
};

/**
 * Represents the AMOD Controller section in the config file
 */
struct AMOD_ControllerParams
{
    AMOD_ControllerParams() : enabled(false) {}

    bool enabled;
};

class ST_Config : public boost::non_copyable
{
public:
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
    std::string& networkXmlInputFile();

    /**
     * If loading the network from an XML file, which file? Empty=private/SimMobilityInput.xml
     *
     * @return Network Xml input file name (const reference)
     */
    const std::string& networkXmlInputFile() const;

    /**
     * If writing the network to an XML file, which file? Empty= dont write at all
     *
     * @return Network Xml output file name
     */
    std::string& networkXmlOutputFile();

    /**
     * If writing the network to an XML file, which file? Empty= dont write at all
     *
     * @return Network Xml output file name (const reference)
     */
    const std::string& networkXmlOutputFile() const;

    /**
     * retrieves road network xsd schema file name
     * If empty, use the default provided in "xsi:schemaLocation".
     *
     * @return road network xsd file name
     */
    std::string& roadNetworkXsdSchemaFile();

    /**
     * retrieves road network xsd schema file name
     * If empty, use the default provided in "xsi:schemaLocation".
     *
     * @return road network xsd file name (const reference)
     */
    const std::string& roadNetworkXsdSchemaFile() const;

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

    /// Container to store vehicle types
    std::vector<VehicleType> vehicleTypes;

    /// Tripfile map
    std::map<std::string, std::string> tripFiles;

    /// Future agents (drivers, pedestrains, passengers.. etc) map
    std::map<std::string, std::vector<EntityTemplate> > futureAgents;

    /// Generic properties map
    std::map<std::string, std::string> genericProps;

    /// Container for storing load agents order
    std::vector<LoadAgentsOrderOption> loadAgentsOrder;

    /// CommSim element
    Commsim commsim;

    /// Type of auromanager used
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
private:
    /**
     * Constructor
     */
    ST_Config(){}

    /**
     * Destructor
     */
    ~ST_Config(){}

    /// Singleton instance
    static ST_Config* instance;
};

}
