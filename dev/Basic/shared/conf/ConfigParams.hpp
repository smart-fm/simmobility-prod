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
#include "util/Factory.hpp"


namespace sim_mob {

//Forward declarations
class BusStop;
class CommunicationDataManager;
class ControlManager;
class ConfigManager;
class RoadSegment;
class PassengerDist;
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

    /**
     * If any Agents specify manual IDs, we must ensure that:
     * the ID is < startingAutoAgentID
     * all manual IDs are unique.
     * We do this using the Agent constraints struct
     */
	struct AgentConstraints {
		AgentConstraints() : startingAutoAgentID(0) {}
		int startingAutoAgentID;
		std::set<unsigned int> manualAgentIDs;
	};

    /**
     * Retrieves the Broker Factory
     *
     * @return broker factory
     */
    sim_mob::Factory<sim_mob::Broker>& getBrokerFactoryRW();

    /// Number of ticks to run the simulation for. (Includes "warmup" ticks.)
    unsigned int totalRuntimeTicks;

    /// Number of ticks considered "warmup".
    unsigned int totalWarmupTicks;

    /// Output network file name
	std::string outNetworkFileName;

    /// Is the simulation run using MPI?
	bool using_MPI;

    /// Is the simulation repeatable?
	bool is_simulation_repeatable;

    /// Number of agents skipped in loading
    unsigned int numAgentsSkipped;

public:
    /**
     * Retrieves/Builds the database connection string
     *
     * @param maskPassword is the password masked/encoded (default is true)
     *
     * @return database connection string
     */
    std::string getDatabaseConnectionString(bool maskPassword=true) const;

    /**
     * Retrieves the stored procedure mappings
     *
     * @return stored procedure map
     */
    StoredProcedureMap getDatabaseProcMappings() const;

	/**
	 * Retrieve a reference to the current RoadNetwork.
     *
     * @return Reference to the current RoadNetwork
	 */
	const sim_mob::RoadNetwork& getNetwork() const;

	/**
	 * Retrieve a reference to the current RoadNetwork; read-write access.
	 * Fails if the network has been sealed.
     *
     * @return Reference to the current RoadNetwork with read-write access
	 */
	sim_mob::RoadNetwork& getNetworkRW();

	/**
	 * Seal the network. After this, no more editing of the network can take place.
	 */
	void sealNetwork();

    /**
     * Retrieves a reference to the current communication data manager
     *
     * @return Reference to the current communication data manager
     */
    //sim_mob::CommunicationDataManager&  getCommDataMgr() const ;

    /**
     * Retrieves a reference to the current control manager
     *
     * @return Reference to the current control manager
     */
	sim_mob::ControlManager* getControlMgr() const;

    /**
     * Retrieve a reference to the person id to tripchains mapping
     *
     * @return Reference to the person id to tripchains mapping
     */
	std::map<std::string, std::vector<sim_mob::TripChainItem*> >& getTripChains();

    /**
     * Retrieves a const reference to the person id to tripchains mapping
     *
     * @return const reference to the person id to tripchains mapping
     */
	const std::map<std::string, std::vector<sim_mob::TripChainItem*> >& getTripChains() const;

    /**
     * Retrieves a reference to the list of PT Bus Dispatch frequency
     *
     * @return reference to the list of PT Bus Dispatch frequency
     */
	std::vector<sim_mob::PT_BusDispatchFreq>& getPT_BusDispatchFreq();

    /**
     * Retrieves a const reference to the list of PT Bus Dispatch frequency
     *
     * @return const reference to the list of PT Bus Dispatch frequency
     */
	const std::vector<sim_mob::PT_BusDispatchFreq>& getPT_BusDispatchFreq() const;

    /**
     * Retrieves a reference to the list of PT Bus routes
     *
     * @return reference to the list of PT bus routes
     */
	std::vector<sim_mob::PT_BusRoutes>& getPT_BusRoutes();

    /**
     * Retrieves a reference to the list of PT Bus Stops
     *
     * @return reference to the list of PT Bus Stops
     */
    std::vector<sim_mob::PT_BusStops>& getPT_BusStops();

    /// NOTE: Technically we use the "sealed" property to indicate data structures that cannot change later.
    ///      From that point, there's no need for a "RW" function. For now, this is a necessary workaround, but
    ///      it also indicates that these data structures should not be located in simpleconf.hpp. ~Seth

    /**
     * Retrieves a const reference to the route id to RoadSegments map
     *
     * @return const reference to the route id to roadsegments map
     */
    const std::map<std::string, std::vector<const sim_mob::RoadSegment*> >& getRoadSegments_Map() const;

    /**
     * Retrieves a reference to the route id to roadsegments map
     *
     * @return reference to the route id to roadsegments map
     */
	std::map<std::string, std::vector<const sim_mob::RoadSegment*> >& getRoadSegments_Map();

    /**
     * Retrives a reference to the bus stop num to bus stop map
     *
     * @return reference to the bus stop num to bus stop map
     */
	std::map<std::string, sim_mob::BusStop*>& getBusStopNo_BusStops();

    /**
     * Retrives a const reference to the bus stop num to bus stop map
     *
     * @return const reference to the bus stop num to bus stop map
     */
	const std::map<std::string, sim_mob::BusStop*>& getBusStopNo_BusStops() const;

    /**
     * Retrieves a reference to the route id to bus stops map
     *
     * @return reference to the route id to bus stops map
     */
	std::map<std::string, std::vector<const sim_mob::BusStop*> >& getBusStops_Map();

    /**
     * Retrieves a const reference to the route id to bus stops map
     *
     * @return const reference to the route id to bus stops map
     */
    const std::map<std::string, std::vector<const sim_mob::BusStop*> >& getBusStops_Map() const;

    /**
     * Retrives a const reference to the lua scripts map
     *
     * @return const refernence to the lua scripts map
     */
	const ModelScriptsMap& getLuaScriptsMap() const;

    /**
     * Checks whether pathset mode is enabled
     *
     * @return true if enabled, else false
     */
	bool PathSetMode() const;

    /**
     * Retrieves a const reference to the pathset configuration
     *
     * @return const reference to the pathset configuration
     */
    const PathSetConf & pathSet() const;

    bool RunningMidTerm() const;

    bool RunningShortTerm() const;

    bool RunningLongTerm() const;

private:
    /**
     * Constructor
     */
	ConfigParams();

    /// Road Network
    sim_mob::RoadNetwork network;

    /// Broker Factory
    sim_mob::Factory<sim_mob::Broker> brokerFact;

    /// Bus stop number to Bus stop mapping
	std::map<std::string, sim_mob::BusStop*> busStopNo_busStops;

    /// Person ID to trip chains mapping
    std::map<std::string, std::vector<sim_mob::TripChainItem*> > tripchains;

    /// Mutable because they are set when retrieved.
	mutable ControlManager* controlMgr;

    /// PT Bus Dispatch Frequency list
	std::vector<sim_mob::PT_BusDispatchFreq> ptBusDispatchFreq;

    /// PT Bus Routes list
	std::vector<sim_mob::PT_BusRoutes> ptBusRoutes;

    /// PT Bus Stops list
	std::vector<sim_mob::PT_BusStops> ptBusStops;

    /// OD Trips list
	std::vector<sim_mob::OD_Trip> ODsTripsMap;

    /// Route ID to road segments mapping
    std::map<std::string, std::vector<const sim_mob::RoadSegment*> > routeID_roadSegments;

    /// Route ID to Bus Stops mapping
    std::map<std::string, std::vector<const sim_mob::BusStop*> > routeID_busStops;

    /// Flag to indicate whether the network is sealed after loading
	bool sealedNetwork;

    bool workerPublisherEnabled;

public:
	/////////////////////////////////////////////////////////////////////////////////////
    /// These are helper functions, to make compatibility between old/new parsing easier.
	/////////////////////////////////////////////////////////////////////////////////////

    /**
     * Retrives a reference to Base system granularity, in milliseconds. Each "tick" is this long.
     *
     * @return reference to base system granularity in milliseconds
     */
	unsigned int& baseGranMS();

    /**
     * Retrieves a const reference to base system granularity in milliseconds. Each "tick" is this long.
     *
     * @return const reference to base system granularity in milliseconds
     */
	const unsigned int& baseGranMS() const;

    /**
     * Retrievs a const reference to base system granularity, in seconds. Each "tick" is this long.
     *
     * @return const reference to base system granularity in seconds
     */
	const double& baseGranSecond() const;

    /**
     * Checks whether to merge log files at the end of simulation
     * If true, we take time to merge the output of the individual log files after the simulation is complete.
     *
     * @return true is allowed to merge, else false
     */
	bool& isMergeLogFiles();

    /**
     * Checks whether to merge log files at the end of simulation
     * If true, we take time to merge the output of the individual log files after the simulation is complete.
     *
     * @return true is allowed to merge, else false
     */
    const bool isMergeLogFiles() const;

    /**
     * Retrieves the default workgroup assignment stratergy
     *
     * @return workgroup assignment stratergy
     */
	WorkGroup::ASSIGNMENT_STRATEGY& defaultWrkGrpAssignment();

    /**
     * Retrieves the default workgroup assignment stratergy
     *
     * @return workgroup assignment stratergy
     */
	const WorkGroup::ASSIGNMENT_STRATEGY& defaultWrkGrpAssignment() const;

    /**
     * Retrieves the mutex stratergy
     *
     * @return mutex stratergy
     */
	sim_mob::MutexStrategy& mutexStategy();

    /**
     * Retrieves the mutex stratergy
     *
     * @return mutex stratergy
     */
    const sim_mob::MutexStrategy& mutexStategy() const;

    /**
     * Retrieves the simulation start time
     *
     * @return simulation start time
     */
	DailyTime& simStartTime();

    /**
     * Retrieves the simulation start time
     *
     * @return simulation start time (const reference)
     */
	const DailyTime& simStartTime() const;

    /**
     * retrieves realtime travel time table name for route choice model
     *
     * @return realtime travel time table
     */
	const std::string& getRTTT() const;

    /**
     * retrieves default travel time table name for route choice model
     *
     * @return default travel time table
     */
	const std::string& getDTT() const;

    /**
     * Retrieves the total runtime in milliseconds
     *
     * @return total runtime in milliseconds
     */
    unsigned int totalRuntimeInMilliSeconds() const;

    /**
     * Retrieves the warmup time in milliseconds
     *
     * @return warmup time in milliseconds
     */
	unsigned int warmupTimeInMilliSeconds() const;

    bool isWorkerPublisherEnabled() const;

    void setWorkerPublisherEnabled(bool value);

    bool isGenerateBusRoutes() const;
};


}
