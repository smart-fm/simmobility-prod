/* Copyright Singapore-MIT Alliance for Research and Technology */
//tripChains Branch

#include "LoadAgents.hpp"

#include "conf/Config.hpp"


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

	//Ordering is simply pre, load, post
	PreLoad(constraints);
	OnLoad(constraints);
	PostLoad();
}


void sim_mob::LoadAgents::PreLoad(AgentConstraints& constraints)
{
	//Ensure that all manual IDs are accounted for. This only works for "explicit" agents right now;
	//   XML and DB agents have their IDs checked later.
	for (list<DataLoader*>::const_iterator it=cfg.simulation().agentsLoaders.begin(); it!=cfg.simulation().agentsLoaders.end(); it++) {
		(*it)->checkManualIDs(constraints);
	}











}

void sim_mob::LoadAgents::OnLoad(AgentConstraints& constraints)
{
	//TODO: XML and DB-loaded Agents *must* check their IDs here using the constraints array.
	//      ("explicit" agents were already checked.)


}

void sim_mob::LoadAgents::PostLoad()
{
}





