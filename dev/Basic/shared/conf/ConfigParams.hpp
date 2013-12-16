//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <set>
#include <map>
#include <string>
#include <vector>

#include "conf/RawConfigParams.hpp"
#include "entities/roles/RoleFactory.hpp"
#include "entities/misc/PublicTransit.hpp"
#include "geospatial/RoadNetwork.hpp"
#include "util/DailyTime.hpp"


namespace sim_mob {

//Forward declarations
class BusStop;
class BusSchedule;
class Conflux;
class CommunicationDataManager;
class ControlManager;
class ConfigManager;
class ReactionTimeDist;
class RoadSegment;
class PassengerDist;
class PT_trip;
class ProfileBuilder;
class TripChainItem;
class Broker;



/**
 * Contains our ConfigParams class. Replaces simpleconf.hpp, providing a more logical ordering (e.g., the file
 *    is parsed FIRST) and some other benefits (fewer include dependencies).
 *
 * To load this class, you can use something like this:
 *
 *   \code
 *   ParseConfigFile parse(configPath, ConfigParams::GetInstanceRW());
 *   ExpandAndValidateConfigFile expand(ConfigParams::GetInstanceRW(), active_agents, pending_agents);
 *   \endcode
 *
 * Note that the separate "Parse" and "Expand" steps exist to allow config-file parameters (like ns3/android brokerage)
 *  to be loaded without requiring anything to be constructed (since this might rely on parsed parameters).
 *
 * \note
 * Developers should make sure NOT to include "GenConfig.h" here; otherwise, this forces most of the source
 * tree to be rebuilt whenever a single parameter changes. (In other words, don't implement any functions in
 * this header file, except perhaps constructors.)
 *
 * \author Seth N. Hetu
 * \author LIM Fung Chai
 * \author Luo Linbo
 * \author Wang Xinyuan
 * \author Runmin Xu
 * \author Zhang Shuai
 * \author Li Zhemin
 * \author Matthew Bremer Bruchon
 * \author Xu Yan
 */
class ConfigParams : public RawConfigParams {
public:
	friend class ConfigManager;

	~ConfigParams();

	//If any Agents specify manual IDs, we must ensure that:
	//   * the ID is < startingAutoAgentID
	//   * all manual IDs are unique.
	//We do this using the Agent constraints struct
	struct AgentConstraints {
		AgentConstraints() : startingAutoAgentID(0) {}
		int startingAutoAgentID;
		std::set<unsigned int> manualAgentIDs;
	};

	/*enum ClientType {
		UNKNOWN = 0,
		ANDROID_EMULATOR = 1,
		NS3_SIMULATOR = 2,
		//add your client type here
	};*/

	/*enum NetworkSource {
		NETSRC_XML,
		NETSRC_DATABASE,
	};*/

	unsigned int totalRuntimeTicks;   ///<Number of ticks to run the simulation for. (Includes "warmup" ticks.)
	unsigned int totalWarmupTicks;    ///<Number of ticks considered "warmup".

	unsigned int granPersonTicks;     ///<Number of ticks to wait before updating all Person agents.
	unsigned int granSignalsTicks;    ///<Number of ticks to wait before updating all signals.
	unsigned int granCommunicationTicks;      ///<Number of ticks to wait before updating all communication brokers.

	///TEMP: Need to customize this later.
	std::string outNetworkFileName;

	//The role factory used for generating roles.
	const sim_mob::RoleFactory& getRoleFactory() const;

	//Use caution here.
	sim_mob::RoleFactory& getRoleFactoryRW();

	//For generating reaction times
	ReactionTimeDist* reactDist1;
	ReactionTimeDist* reactDist2;

	//for generating passenger distribution
	PassengerDist* passengerDist_busstop;
	PassengerDist* passengerDist_crowdness;

	//Number of agents skipped in loading
	unsigned int numAgentsSkipped;

	//Does not appear to be used any more. ~Seth
	//std::map<int, std::vector<int> > scheduledTimes;//store the actual scheduledAT and DT.assumed dwell time as 6 sec for all stops.

	bool using_MPI;
	bool is_run_on_many_computers;
	bool is_simulation_repeatable;

	//These no longer appear to be set-able.
	//int signalTimingMode;
	//int signalAlgorithm;

	//std::vector<SubTrip> subTrips;//todo, check anyone using this? -vahid

public:
	///Retrieve a read-only version of the singleton. Use this function often.
	//static const ConfigParams& GetInstance();

	///Retrieve a writable version of the singleton. Use this function sparingly.
	//static ConfigParams& GetInstanceRW();

	///Reset this instance of the static ConfigParams instance.
	///WARNING: This should *only* be used by the interactive loop of Sim Mobility.
	//void reset();

	///Retrieve/build the connection string.
	std::string getDatabaseConnectionString(bool maskPassword=true) const;
	StoredProcedureMap getDatabaseProcMappings() const;


	/**
	 * Load the defualt user config file; initialize all vectors. This function must be called
	 * once before GetInstance() will return meaningful data.
	 *
	 * \param active_agents Vector to hold all agents that will be active during time tick zero.
	 * \param pending_agents Priority queue to hold all agents that will become active after time tick zero.
	 */
	//static void InitUserConf(const std::string& configPath, std::vector<Entity*>& active_agents, StartTimePriorityQueue& pending_agents, ProfileBuilder* prof, const sim_mob::Config::BuiltInModels& builtInModels);

	/**
	 * Retrieve a reference to the current RoadNetwork.
	 */
	const sim_mob::RoadNetwork& getNetwork() const;

	/**
	 * Retrieve a reference to the current RoadNetwork; read-write access.
	 * Fails if the network has been sealed.
	 */
	sim_mob::RoadNetwork& getNetworkRW();

	std::vector<IncidentParams>& getIncidents();

	/**
	 * Seal the network. After this, no more editing of the network can take place.
	 */
	void sealNetwork();

	sim_mob::CommunicationDataManager&  getCommDataMgr() const ;

	sim_mob::ControlManager* getControlMgr() const;

	///Retrieve a reference to the list of trip chains.
	std::map<std::string, std::vector<sim_mob::TripChainItem*> >& getTripChains();
	const std::map<std::string, std::vector<sim_mob::TripChainItem*> >& getTripChains() const;

	std::vector<sim_mob::BusSchedule*>& getBusSchedule();
	std::vector<sim_mob::PT_trip*>& getPT_trip();

	std::vector<sim_mob::PT_bus_dispatch_freq>& getPT_bus_dispatch_freq();
	const std::vector<sim_mob::PT_bus_dispatch_freq>& getPT_bus_dispatch_freq() const;

	std::vector<sim_mob::PT_bus_routes>& getPT_bus_routes();
	std::vector<sim_mob::PT_bus_stops>& getPT_bus_stops();

	//Temporary: Santhosh
	std::map<int, std::vector<int> > scheduledTImes;//store the actual scheduledAT and DT.assumed dwell time as 6 sec for all stops.

	//NOTE: Technically we use the "sealed" property to indicate data structures that cannot change later.
	//      From that point, there's no need for a "RW" function. For now, this is a necessary workaround, but
	//      it also indicates that these data structures should not be located in simpleconf.hpp. ~Seth
	const std::map<std::string, std::vector<const sim_mob::RoadSegment*> >& getRoadSegments_Map() const;
	std::map<std::string, std::vector<const sim_mob::RoadSegment*> >& getRoadSegments_Map();

	std::map<std::string, sim_mob::BusStop*>& getBusStopNo_BusStops();
	const std::map<std::string, sim_mob::BusStop*>& getBusStopNo_BusStops() const;

	std::map<std::string, std::vector<const sim_mob::BusStop*> >& getBusStops_Map();
	const std::map<std::string, std::vector<const sim_mob::BusStop*> >& getBusStops_Map() const;

	std::set<sim_mob::Conflux*>& getConfluxes();
	const std::set<sim_mob::Conflux*>& getConfluxes() const;

	std::map<const sim_mob::MultiNode*, sim_mob::Conflux*>& getConfluxNodes();

private:
	ConfigParams();

	//static ConfigParams instance;

	sim_mob::RoadNetwork network;
	sim_mob::RoleFactory roleFact;
	std::map<std::string, sim_mob::BusStop*> busStopNo_busStops;
	std::map<std::string, std::vector<sim_mob::TripChainItem*> > tripchains; //map<personID,tripchains>

	//Mutable because they are set when retrieved.
	mutable CommunicationDataManager* commDataMgr;
	mutable ControlManager* controlMgr;

	// Temporary: Yao Jin
	std::vector<sim_mob::BusSchedule*> busschedule; // Temporary
	std::vector<sim_mob::PT_trip*> pt_trip;
	std::vector<sim_mob::PT_bus_dispatch_freq> pt_busdispatch_freq;
	std::vector<sim_mob::PT_bus_routes> pt_bus_routes;
	std::vector<sim_mob::PT_bus_stops> pt_bus_stops;
	// Temporary: Yao Jin

	std::map<std::string, std::vector<const sim_mob::RoadSegment*> > routeID_roadSegments; // map<routeID, vector<RoadSegment*>>
	std::map<std::string, std::vector<const sim_mob::BusStop*> > routeID_busStops; // map<routeID, vector<BusStop*>>
	bool sealedNetwork;

	//Confluxes in this network
	std::set<sim_mob::Conflux*> confluxes;
	std::map<const sim_mob::MultiNode*, sim_mob::Conflux*> multinode_confluxes; //map <MultiNode*, Conflux*>

public:
	/////////////////////////////////////////////////////////////////////////////////////
	//These are helper functions, to make compatibility between old/new parsing easier.
	/////////////////////////////////////////////////////////////////////////////////////

	///Number of workers handling Agents.
	unsigned int& personWorkGroupSize();
	const unsigned int& personWorkGroupSize() const;

	///Number of workers handling Signals.
	unsigned int& signalWorkGroupSize();
	const unsigned int& signalWorkGroupSize() const;

	///Number of workers handling Signals.
	unsigned int& commWorkGroupSize();
	const unsigned int& commWorkGroupSize() const;

	///Base system granularity, in milliseconds. Each "tick" is this long.
	unsigned int& baseGranMS();
	const unsigned int& baseGranMS() const;

	///If true, we are running everything on one thread.
	bool& singleThreaded();
	const bool& singleThreaded() const;

	///If true, we take time to merge the output of the individual log files after the simulation is complete.
	bool& mergeLogFiles();
	const bool& mergeLogFiles() const;

	///Whether to load the network from the database or from an XML file.
	SystemParams::NetworkSource& networkSource();
	const SystemParams::NetworkSource& networkSource() const;

	///Whether configuration has been set to run mid-term supply or demand. Used for runtime checks
	bool RunningMidSupply() const;
	bool RunningMidDemand() const;

	///If loading the network from an XML file, which file? Empty=private/SimMobilityInput.xml
	std::string& networkXmlInputFile();
	const std::string& networkXmlInputFile() const;

	///If writing the network to an XML file, which file? Empty= dont write at all
	std::string& networkXmlOutputFile();
	const std::string& networkXmlOutputFile() const;

	///If empty, use the default provided in "xsi:schemaLocation".
	std::string& roadNetworkXsdSchemaFile();
	const std::string& roadNetworkXsdSchemaFile() const;

	///Which tree implementation to use for spatial partitioning for the aura manager.
	AuraManager::AuraManagerImplementation& aura_manager_impl();
	const AuraManager::AuraManagerImplementation& aura_manager_impl() const;

	int& percent_boarding();
	const int& percent_boarding() const;

	int& percent_alighting();
	const int& percent_alighting() const;

	WorkGroup::ASSIGNMENT_STRATEGY& defaultWrkGrpAssignment();
	const WorkGroup::ASSIGNMENT_STRATEGY& defaultWrkGrpAssignment() const;

	sim_mob::MutexStrategy& mutexStategy();
	const sim_mob::MutexStrategy& mutexStategy() const;
	//Communication Simulator accessors and configurators
	bool& commSimEnabled();
	const bool& commSimEnabled() const;
	bool& androidClientEnabled();
	const bool& androidClientEnabled() const;
	const std::string& getAndroidClientType() const;
	 std::string& getAndroidClientType() ;

	DailyTime& simStartTime();
	const DailyTime& simStartTime() const;
	const std::string& getTravelTimeTmpTableName() const;

	//This one's slightly tricky, as it's in generic_props
	std::string busline_control_type() const;

	/////////////////////////////////////////////////////////////////////////////////////
	//These are sort of similar, but might need some fixing after we validate the input file.
	/////////////////////////////////////////////////////////////////////////////////////

	unsigned int totalRuntimeInMilliSeconds() const;

	unsigned int warmupTimeInMilliSeconds() const;

	unsigned int personTimeStepInMilliSeconds() const;

	unsigned int signalTimeStepInMilliSeconds() const;

	unsigned int communicationTimeStepInMilliSeconds() const;
};


}
