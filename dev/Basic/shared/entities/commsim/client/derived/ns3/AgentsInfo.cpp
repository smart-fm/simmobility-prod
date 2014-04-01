//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include "AgentsInfo.hpp"
#include <json/json.h>
#include "entities/Agent.hpp"
#include "entities/commsim/service/Services.hpp"
#include "entities/commsim/serialization/CommsimSerializer.hpp"
#include <boost/foreach.hpp>

using namespace sim_mob;
/*
sim_mob::AgentsInfo::AgentsInfo()
{
}

sim_mob::AgentsInfo::~AgentsInfo()
{
}


void sim_mob::AgentsInfo::insertInfo(std::map<Mode, std::set<sim_mob::Entity*> > &values)
{
	std::map<Mode, std::set<sim_mob::Entity*> >::iterator it(values.begin()), it_end(values.end());
	for(; it != it_end; it++ ) {
		all_agents[it->first].insert(it->second.begin(), it->second.end());
	}

}

void sim_mob::AgentsInfo::insertInfo(Mode mode, std::set<sim_mob::Entity*>  &values)
{
	all_agents[mode].insert(values.begin(), values.end());
}

void sim_mob::AgentsInfo::insertInfo(Mode mode,sim_mob::Entity* value)
{
	all_agents[mode].insert(value);
}

std::string sim_mob::AgentsInfo::toJson()
{
	std::vector<unsigned int> addAgIds;
	std::vector<unsigned int> remAgIds;
	for(std::map<Mode, std::set<sim_mob::Entity*> >::iterator it = all_agents.begin(); it != all_agents.end(); it++) {
		std::vector<unsigned int>* currVec = nullptr;
		switch(it->first) {
			case ADD_AGENT: {
				currVec = &addAgIds;
				break;
			}
			case REMOVE_AGENT: {
				currVec = &remAgIds;
				break;
			}
			default: { throw std::runtime_error("Unknown add/rem agent enum type."); }
		}

		for (std::set<Entity*>::const_iterator eIt=it->second.begin(); eIt!=it->second.end(); eIt++) {
			currVec->push_back((*eIt)->getId());
		}
	}

	return CommsimSerializer::makeAgentsInfo(addAgIds, remAgIds);
}
*/
