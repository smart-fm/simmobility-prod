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

namespace sim_mob
{

struct VehicleType
{
    std::string name;

    double length;

    double width;

    int capacity;

    bool operator==(const VehicleType& rhs) const;

    bool operator!=(const VehicleType& rhs) const;

    VehicleType();
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
    int tripId;
    int vehicleId;
    std::string vehicleType;
};

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

struct Commsim {
    bool enabled;  ///< True if commsim is enabled. If false, no Broker will be created.
    int numIoThreads; ///< How many threads to allocate to boost's io processor.
    int minClients;  ///< The minimum number of simultaneous clients required to proceed with the simulation.
    int holdTick;    ///< The simulation tick that we will pause on until minClients connections are made.
    bool useNs3;  ///< If true, waits for the ns-3 simulator to connect.
    Commsim() : enabled(false), numIoThreads(1), minClients(1), holdTick(1), useNs3(false) {}
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

///Represents a complete connection to the database, via Construct ID.
struct DatabaseDetails {
    std::string database;
    std::string credentials;
    std::string procedures;
};

enum NetworkSource {
    NETSRC_XML,
    NETSRC_DATABASE,
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
};

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
};

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

class ST_Config : public boost::non_copyable
{
public:
    static ST_Config& getInstance();

    static void deleteIntance();

    std::vector<VehicleType> vehicleTypes;

    std::map<std::string, std::string> tripFiles;

    std::map<std::string, std::vector<EntityTemplate> > futureAgents;

    std::map<std::string, std::string> genericProps;

    std::vector<LoadAgentsOrderOption> loadAgentsOrder;

    Commsim commsim;

    //Reaction time parameters.
    //TODO: This should be one of the first areas we clean up.
    ReactionTimeDistDescription reactTimeDistribution1;
    ReactionTimeDistDescription reactTimeDistribution2;

    AuraManager::AuraManagerImplementation auraManagerImplementation; ///<What type of Aura Manager we're using.

    int partitioningSolutionId;  ///<Property specific to MPI version; not fully documented.

    std::map<std::string, StoredProcedureMap> procedureMaps;

    WorkerParams workers;

    DatabaseDetails networkDatabase; //<If loading from the database, how do we connect?

    NetworkSource networkSource; ///<Whethere to load the network from the database or from an XML file.
    std::string networkXmlInputFile;  ///<If loading the network from an XML file, which file? Empty=private/SimMobilityInput.xml
    std::string networkXmlOutputFile;  ///<If loading the network from an XML file, which file? Empty=private/SimMobilityInput.xml

    std::string roadNetworkXsdSchemaFile; ///<Valid path to a schema file for loading XML road network files.

    ///Settings for the loop detector counts
    LoopDetectorCounts loopDetectorCounts;

    ///Settings for the short-term density map
    SegmentDensityMap segDensityMap;

    ///Settings for the FMOD controller.
    FMOD_ControllerParams fmod;

    ///Settings for the AMOD controller
    AMOD_ControllerParams amod;

    bool isBusControllerEnabled;
private:
    ST_Config(){}

    ~ST_Config(){}

    static ST_Config* instance;
};

}
