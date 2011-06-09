//Placeholder file while we figure out how config files will work.


#pragma once

#include <sstream>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>


#include "../simple_classes.h"

#include "../entities/Entity.hpp"
#include "../entities/Agent.hpp"
#include "../entities/Region.hpp"
#include "../workers/Worker.hpp"


bool loadUserConf(std::vector<Agent>& agents, std::vector<Region>& regions,
		          std::vector<TripChain>& trips, std::vector<ChoiceSet>& chSets,
		          std::vector<Vehicle>& vehicles);

