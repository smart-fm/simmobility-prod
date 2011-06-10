//Placeholder file while we figure out how config files will work.


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


class ConfigParams : private boost::noncopyable {
public:
	unsigned int baseGranMS;
	unsigned int totalRuntimeTicks;
	unsigned int totalWarmupTicks;

	unsigned int granAgentsTicks;
	unsigned int granSignalsTicks;
	unsigned int granPathsTicks;
	unsigned int granDecompTicks;

public:
	static ConfigParams& GetInstance();
	static bool InitUserConf(std::vector<Agent>& agents, std::vector<Region>& regions,
	          std::vector<TripChain>& trips, std::vector<ChoiceSet>& chSets,
	          std::vector<Vehicle>& vehicles);

private:
	ConfigParams();
	static ConfigParams instance;
};


