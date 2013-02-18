/* Copyright Singapore-MIT Alliance for Research and Technology */
//tripChains Branch

#include "LoadAgents.hpp"

#include "conf/Config.hpp"
#include "entities/Agent.hpp"

using std::map;
using std::set;
using std::list;
using std::vector;
using std::string;

using namespace sim_mob;


sim_mob::LoadAgents::LoadAgents(Config& cfg) : cfg(cfg)
{
	//Ensure as we go that all Agents have unique IDs.
	//Note that this is only a mild guarantee, as Agents can override their own IDs in their
	// constructors (as BusController once did).
	AgentConstraints constraints;

	//TODO: The fact that this is used twice indicates that it needs a more permanent storage location
	//      (e.g., in "system"), rather than in "gen_props"
	constraints.startingAutoAgentID = cfg.system().startingAgentAutoID;

	//A list to hold all generated Agents.
	std::list<sim_mob::Agent*> res_agents;

	//Perform key tasks
	CreateAgentShells(constraints, res_agents);
	//TODO: BusControllers need to be initialized here (check pg. 40, simpleconf)
	DispatchOrPend(res_agents);
}


void sim_mob::LoadAgents::CreateAgentShells(AgentConstraints& constraints, std::list<sim_mob::Agent*>& res_agents)
{
	for (list<AbstractAgentLoader*>::const_iterator it=cfg.simulation().agentsLoaders.begin(); it!=cfg.simulation().agentsLoaders.end(); it++) {
		(*it)->loadAgents(res_agents, constraints, cfg);
	}
}

void sim_mob::LoadAgents::DispatchOrPend(const std::list<sim_mob::Agent*>& res_agents)
{
	//TODO: Now add/dispatch them.
}





