/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <map>
#include <set>
#include <string>
#include <sstream>

#include <boost/utility.hpp>

#include "util/DailyTime.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/RoadNetwork.hpp"


namespace sim_mob
{

//Forward declarations
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
public:
	/***
	 * Singleton. Retrieve an instance of the ConfigParams object.
	 */
	static ConfigParams& GetInstance();

	/**
	 * Load the defualt user config file; initialize all vectors. This function must be called
	 * once before GetInstance() will return meaningful data.
	 */
	static bool InitUserConf(const std::string& configPath, std::vector<Agent*>& agents);

	/**
	 * Retrieve a reference to the current RoadNetwork.
	 */
	sim_mob::RoadNetwork& getNetwork() { return network; }

	///Retrieve a reference to the list of trip chains.
	std::vector<sim_mob::TripChain*>& getTripChains() { return tripchains; }


private:
	ConfigParams();
	static ConfigParams instance;

	sim_mob::RoadNetwork network;
	std::vector<sim_mob::TripChain*> tripchains;
};

}
