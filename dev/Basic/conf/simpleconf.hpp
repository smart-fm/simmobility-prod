#pragma once

#include <sstream>

#include <boost/utility.hpp>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>


#include "../simple_classes.h"

#include "../entities/Entity.hpp"
#include "../entities/Agent.hpp"
#include "../entities/Region.hpp"
#include "../workers/Worker.hpp"


namespace sim_mob
{


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

public:
	/***
	 * Singleton. Retrieve an instance of the ConfigParams object.
	 */
	static ConfigParams& GetInstance();

	/**
	 * Load the defualt user config file; initialize all vectors. This function must be called
	 * once before GetInstance() will return meaningful data.
	 */
	static bool InitUserConf(std::vector<Agent>& agents, std::vector<Region>& regions,
	          std::vector<TripChain>& trips, std::vector<ChoiceSet>& chSets,
	          std::vector<Vehicle>& vehicles);

private:
	ConfigParams();
	static ConfigParams instance;
};

}
