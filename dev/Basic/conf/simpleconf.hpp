/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <sstream>

#include <boost/utility.hpp>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "../geospatial/Point2D.hpp"
#include "../simple_classes.h"


namespace sim_mob
{

//Forward declarations
class Agent;
class Person;
//class Pedestrian;
//class Driver;
class Region;

//Doesn't work for some reason.
//class ChoiceSet;
//class TripChain;
//class Vehicle;



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

	std::map<std::string, Point2D> boundaries;  ///<Indexed by position, e.g., "bottomright"
	std::map<std::string, Point2D> crossings;   ///<Indexed by position, e.g., "bottomright"

public:
	/***
	 * Singleton. Retrieve an instance of the ConfigParams object.
	 */
	static ConfigParams& GetInstance();

	/**
	 * Load the defualt user config file; initialize all vectors. This function must be called
	 * once before GetInstance() will return meaningful data.
	 */
	static bool InitUserConf(std::vector<Agent*>& agents, std::vector<Region*>& regions,
	          std::vector<TripChain*>& trips, std::vector<ChoiceSet*>& chSets,
	          std::vector<Vehicle*>& vehicles);

private:
	ConfigParams();
	static ConfigParams instance;
};

}
