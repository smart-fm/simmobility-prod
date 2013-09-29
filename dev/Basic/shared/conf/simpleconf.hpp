/* Copyright Singapore-MIT Alliance for Research and Technology */


/**
 * \file simpleconf.hpp
 * Central location for configuration-loading code.
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

#pragma once

#include <map>
#include <set>
#include <string>
#include <sstream>

#include "geospatial/xmlWriter/xmlWriter.hpp"
#include <boost/utility.hpp>

#include "buffering/Shared.hpp"
#include "util/DailyTime.hpp"
#include "util/LangHelpers.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/RoadNetwork.hpp"

#include "entities/AuraManager.hpp"
#include "entities/misc/TripChain.hpp"
#include "entities/misc/BusTrip.hpp"
#include "entities/misc/PublicTransit.hpp"
#include "entities/roles/RoleFactory.hpp"
#include "util/ReactionTimeDistributions.hpp"
#include "util/PassengerDistribution.hpp"
#include "workers/WorkGroup.hpp"

#include "network/CommunicationDataManager.hpp"
#include "network/CommunicationManager.hpp"
#include "network/ControlManager.hpp"

#include "Config.hpp"


namespace sim_mob
{

//Forward declarations
class Entity;
class Agent;
class Person;
class BusController;// add by Yao Jin
class StartTimePriorityQueue;
class EventTimePriorityQueue;
class ProfileBuilder;
class BusSchedule;


/**
 * Temporary configuration pConfigParamsarser. Operates as a singleton. Contains all basic
 * configuation parameters.
 */

enum DAY_OF_WEEK {
	MONDAY,
	TUESDAY,
	WEDNESDAY,
	THURSDAY,
	FRIDAY,
	SATURDAY,
	SUNDAY
};

class ConfigParams : private boost::noncopyable {
public:
	unsigned int baseGranMS;          ///<Base system granularity, in milliseconds. Each "tick" is this long.
	unsigned int totalRuntimeTicks;   ///<Number of ticks to run the simulation for. (Includes "warmup" ticks.)
	unsigned int totalWarmupTicks;    ///<Number of ticks considered "warmup".

	unsigned int granAgentsTicks;     ///<Number of ticks to wait before updating all agents.
	unsigned int granSignalsTicks;    ///<Number of ticks to wait before updating all signals.
	unsigned int granPathsTicks;      ///<Number of ticks to wait before updating all paths.
	unsigned int granDecompTicks;     ///<Number of ticks to wait before updating agent decomposition.

	unsigned int agentWorkGroupSize;   ///<Number of workers handling Agents.
	unsigned int signalWorkGroupSize;  ///<Number of workers handling Signals.

	DAY_OF_WEEK day_of_week;

	//The role factory used for generating roles.
	const sim_mob::RoleFactory& getRoleFactory() { return roleFact; }

	//Use caution here.
	sim_mob::RoleFactory& getRoleFactoryRW() { return roleFact; }

	///What type of Aura Manager we're using.
	AuraManager::AuraManagerImplementation aura_manager_impl;


	//For generating reaction times
	ReactionTimeDist* reactDist1;
	ReactionTimeDist* reactDist2;
	//for generating passenger distribution


	PassengerDist* passengerDist_busstop;
	PassengerDist* passengerDist_crowdness;

	int percent_boarding;
	int percent_alighting;
//	PassengerDist* passengerDist_alighting;

	//Defautl assignment strategy for Workgroups.
	WorkGroup::ASSIGNMENT_STRATEGY defaultWrkGrpAssignment;

	//Number of agents skipped in loading
	unsigned int numAgentsSkipped;

	//Locking strategy
	sim_mob::MutexStrategy mutexStategy;

	//Busline_Control_Type
	std::string busline_control_type;

	// temporary maps
	std::map<int, std::vector<int> > scheduledTimes;//store the actual scheduledAT and DT.assumed dwell time as 6 sec for all stops.

//TODO: Add infrastructure for private members; some things like "dynamicDispatch" should NOT
//      be modified once set.
//private:
	//Is dynamic dispatch disabled?
	bool dynamicDispatchDisabled;

public:
	int signalTimingMode;
	int signalAlgorithm;

	int drivingModel;

	//When the simulation begins
	DailyTime simStartTime;

	std::map<std::string, Point2D> boundaries;  ///<Indexed by position, e.g., "bottomright"
	std::map<std::string, Point2D> crossings;   ///<Indexed by position, e.g., "bottomright"

	std::string connectionString;

	bool using_MPI;
	bool is_simulation_repeatable;

	unsigned int totalRuntimeInMilliSeconds() const { return totalRuntimeTicks * baseGranMS; }
	unsigned int warmupTimeInMilliSeconds() const { return totalWarmupTicks * baseGranMS; }
	unsigned int agentTimeStepInMilliSeconds() const { return granAgentsTicks * baseGranMS; }
	unsigned int signalTimeStepInMilliSeconds() const { return granSignalsTicks * baseGranMS; }
	unsigned int pathsTimeStepInMilliSeconds() const { return granPathsTicks * baseGranMS; }
	unsigned int DecompTimeStepInMilliSeconds() const { return granDecompTicks * baseGranMS; }

	bool TEMP_ManualFixDemoIntersection;

	///Synced to the value of disable_dynamic_dispatch in the config file; used for runtime checks.
	bool DynamicDispatchDisabled() const {
		return dynamicDispatchDisabled;
	}

	///Synced to the value of SIMMOB_DISABLE_MPI; used for runtime checks.
	bool MPI_Disabled() const;

	///Synced to the value of SIMMOB_DISABLE_OUTPUT; used for runtime checks.
	bool OutputDisabled() const;

	///Synced to the value of SIMMOB_DISABLE_OUTPUT; used for runtime checks.
	bool OutputEnabled() const {
		return !OutputDisabled();
	}

	///Synced to the value of SIMMOB_INTERACTIVE_MODE; used for to detect if we're running "interactively"
	/// with the GUI or console.
	bool InteractiveMode() const {
#ifdef SIMMOB_INTERACTIVE_MODE
		return true;
#else
		return false;
#endif
	}


	///Synced to the value of SIMMOB_STRICT_AGENT_ERRORS; used for runtime checks.
	bool StrictAgentErrors() const;

	///Synced to the value of SIMMOB_PROFILE_ON; used for runtime checks.
	bool ProfileOn() const ;

	///Synced to the value of SIMMOB_PROFILE_AGENT_UPDATES; used for runtime checks.
	///If "accountForOnFlag" is false, *only* the cmake define flag is checked.
	bool ProfileAgentUpdates(bool accountForOnFlag=true) const ;

	///Synced to the value of SIMMOB_PROFILE_WORKER_UPDATES; used for runtime checks.
	///If "accountForOnFlag" is false, *only* the cmake define flag is checked.
	bool ProfileWorkerUpdates(bool accountForOnFlag=true) const ;

public:
	/***
	 * Singleton. Retrieve an instance of the ConfigParams object.
	 */
	static ConfigParams& GetInstance() { return ConfigParams::instance; }
	void reset()
	{
		sealedNetwork=false;
		roleFact.clear();
	}
	std::vector<SubTrip> subTrips;//todo, check anyone using this? -vahid

	/**
	 * Load the defualt user config file; initialize all vectors. This function must be called
	 * once before GetInstance() will return meaningful data.
	 *
	 * \param active_agents Vector to hold all agents that will be active during time tick zero.
	 * \param pending_agents Priority queue to hold all agents that will become active after time tick zero.
	 */
	static void InitUserConf(const std::string& configPath, std::vector<Entity*>& active_agents, StartTimePriorityQueue& pending_agents, ProfileBuilder* prof, const sim_mob::Config::BuiltInModels& builtInModels);

	/**
	 * Retrieve a reference to the current RoadNetwork.
	 */
	const sim_mob::RoadNetwork& getNetwork() { return network; }

	/**
	 * Retrieve a reference to the current RoadNetwork; read-write access.
	 * Fails if the network has been sealed.
	 */
	sim_mob::RoadNetwork& getNetworkRW() {
		if (sealedNetwork) {
			throw std::runtime_error("getNetworkRW() failed; network has been sealed.");
		}
		return network;
	}

	/**
	 * Seal the network. After this, no more editing of the network can take place.
	 */
	void sealNetwork() {
		sealedNetwork = true;
	}

	sim_mob::CommunicationDataManager&  getCommDataMgr() {
#ifdef SIMMOB_INTERACTIVE_MODE
		return commDataMgr;
#else
		throw std::runtime_error("ConfigParams::getCommDataMgr() not supported; SIMMOB_INTERACTIVE_MODE is off.");
#endif
	}

	sim_mob::ControlManager* getControlMgr() {
#ifdef SIMMOB_INTERACTIVE_MODE
		//In this case, ControlManager's constructor performs some logic, so it's best to use a pointer.
		if (!controlMgr) {
			controlMgr = new ControlManager();
		}
		return controlMgr;
#else
		throw std::runtime_error("ConfigParams::getControlMgr() not supported; SIMMOB_INTERACTIVE_MODE is off.");
#endif
	}

	///Retrieve a reference to the list of trip chains.
//	std::vector<sim_mob::TripChainItem*>& getTripChains() { return tripchains; }
	std::map<std::string, std::vector<sim_mob::TripChainItem*> >& getTripChains() { return tripchains; }
	std::vector<sim_mob::BusSchedule*>& getBusSchedule() { return busschedule;}
	std::vector<sim_mob::PT_trip*>& getPT_trip() { return pt_trip; }
	std::vector<sim_mob::PT_bus_dispatch_freq>& getPT_bus_dispatch_freq() { return pt_busdispatch_freq; }
	std::vector<sim_mob::PT_bus_routes>& getPT_bus_routes() { return pt_bus_routes; }
	std::vector<sim_mob::PT_bus_stops>& getPT_bus_stops() { return pt_bus_stops; }

	//Temporary: Santhosh
	std::map<int, std::vector<int> > scheduledTImes;//store the actual scheduledAT and DT.assumed dwell time as 6 sec for all stops.

	//NOTE: Technically we use the "sealed" property to indicate data structures that cannot change later.
	//      From that point, there's no need for a "RW" function. For now, this is a necessary workaround, but
	//      it also indicates that these data structures should not be located in simpleconf.hpp. ~Seth
	const std::map<std::string, std::vector<const sim_mob::RoadSegment*> >& getRoadSegments_Map() const { return routeID_roadSegments;}
	std::map<std::string, std::vector<const sim_mob::RoadSegment*> >& getRoadSegments_MapRW() { return routeID_roadSegments;}

	std::map<std::string, sim_mob::BusStop*>& getBusStopNo_BusStops() { return busStopNo_busStops; }
	std::map<std::string, std::vector<const sim_mob::BusStop*> >& getBusStops_Map() { return routeID_busStops; }

	std::set<sim_mob::Conflux*>& getConfluxes() { return confluxes; }

private:
	ConfigParams() : baseGranMS(0), totalRuntimeTicks(0), totalWarmupTicks(0), granAgentsTicks(0), granSignalsTicks(0),
		granPathsTicks(0), granDecompTicks(0), agentWorkGroupSize(0), signalWorkGroupSize(0), day_of_week(MONDAY),
		aura_manager_impl(AuraManager::IMPL_RSTAR), reactDist1(nullptr), reactDist2(nullptr), numAgentsSkipped(0), mutexStategy(MtxStrat_Buffered),
		dynamicDispatchDisabled(false), signalAlgorithm(0), using_MPI(false),
		is_simulation_repeatable(false), TEMP_ManualFixDemoIntersection(false), sealedNetwork(false), controlMgr(nullptr),
		defaultWrkGrpAssignment(WorkGroup::ASSIGN_ROUNDROBIN)
	{}

	static ConfigParams instance;

	sim_mob::RoadNetwork network;
	sim_mob::RoleFactory roleFact;
	std::map<std::string, sim_mob::BusStop*> busStopNo_busStops;
	std::map<std::string, std::vector<sim_mob::TripChainItem*> > tripchains; //map<personID,tripchains>

	CommunicationDataManager commDataMgr;
	ControlManager* controlMgr;

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
};

}
