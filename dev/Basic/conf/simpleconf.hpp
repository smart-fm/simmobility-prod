/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <map>
#include <set>
#include <string>
#include <sstream>

#include "GenConfig.h"

#include <boost/utility.hpp>

#include "buffering/Shared.hpp"
#include "util/DailyTime.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/RoadNetwork.hpp"


namespace sim_mob
{

//Forward declarations
class Entity;
class Agent;
class Person;
class Region;
class TripChain;


/**
 * Temporary configuration parser. Operates as a singleton. Contains all basic
 * configuation parameters.
 */
class ConfigParams : private boost::noncopyable {
public:
	unsigned int baseGranMS;          ///<Base system granularity, in milliseconds. Each "tick" is this long.
	unsigned int totalRuntimeTicks;   ///<Number of ticks to run the simulation for. (Includes "warmup" ticks.)
	unsigned int totalWarmupTicks;    ///<Number of ticks considered "warmup".

	unsigned int granAgentsTicks;     ///<Number of ticks to wait before updating all agents.
	unsigned int granSignalsTicks;    ///<Number of ticks to wait before updating all signals.
	unsigned int granPathsTicks;      ///<Number of ticks to wait before updating all paths.
	unsigned int granDecompTicks;     ///<Number of ticks to wait before updating agent decomposition.

	//reaction time parameters
	unsigned int reacTime_LeadingVehicle;
	unsigned int reacTime_SubjectVehicle;
	unsigned int reacTime_Gap;

	//Number of agents skipped in loading
	unsigned int numAgentsSkipped;

	//Locking strategy
	sim_mob::MutexStrategy mutexStategy;

	//When the simulation begins
	DailyTime simStartTime;

	std::map<std::string, Point2D> boundaries;  ///<Indexed by position, e.g., "bottomright"
	std::map<std::string, Point2D> crossings;   ///<Indexed by position, e.g., "bottomright"

	std::string connectionString;

	bool is_run_on_many_computers;
	bool is_simulation_repeatable;

	unsigned int totalRuntimeInMilliSeconds() const { return totalRuntimeTicks * baseGranMS; }
	unsigned int warmupTimeInMilliSeconds() const { return totalWarmupTicks * baseGranMS; }
	unsigned int agentTimeStepInMilliSeconds() const { return granAgentsTicks * baseGranMS; }
	unsigned int signalTimeStepInMilliSeconds() const { return granSignalsTicks * baseGranMS; }
	unsigned int pathsTimeStepInMilliSeconds() const { return granPathsTicks * baseGranMS; }
	unsigned int DecompTimeStepInMilliSeconds() const { return granDecompTicks * baseGranMS; }

	bool TEMP_ManualFixDemoIntersection;

	///Synced to the value of SIMMOB_DISABLE_DYNAMIC_DISPATCH; used for runtime checks.
	bool DynamicDispatchDisabled() const {
#ifdef SIMMOB_DISABLE_DYNAMIC_DISPATCH
		return true;
#else
		return false;
#endif
	}

	///Synced to the value of SIMMOB_DISABLE_MPI; used for runtime checks.
	bool MPI_Disabled() const {
#ifdef SIMMOB_DISABLE_MPI
		return true;
#else
		return false;
#endif
	}

public:
	/***
	 * Singleton. Retrieve an instance of the ConfigParams object.
	 */
	static ConfigParams& GetInstance() { return ConfigParams::instance; }

	/**
	 * Load the defualt user config file; initialize all vectors. This function must be called
	 * once before GetInstance() will return meaningful data.
	 */
	static bool InitUserConf(const std::string& configPath, std::vector<Entity*>& agents);

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

	///Retrieve a reference to the list of trip chains.
	std::vector<sim_mob::TripChain*>& getTripChains() { return tripchains; }


private:
	ConfigParams() : mutexStategy(MtxStrat_Buffered), TEMP_ManualFixDemoIntersection(false), sealedNetwork(false)
        {
        }
	static ConfigParams instance;

	sim_mob::RoadNetwork network;
	std::vector<sim_mob::TripChain*> tripchains;
	bool sealedNetwork;
};

}
