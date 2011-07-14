#include "Agent.hpp"

using namespace sim_mob;

using std::vector;


vector<Agent*> sim_mob::Agent::all_agents;



sim_mob::Agent::Agent(unsigned int id) : Entity(id), xPos(0), yPos(0) {
	toRemoved = false;
}



void sim_mob::Agent::buildSubscriptionList()
{
	subscriptionList_cached.push_back(&xPos);
	subscriptionList_cached.push_back(&yPos);
}


bool sim_mob::Agent::isToBeRemoved()
{
	return toRemoved;
}


void sim_mob::Agent::setToBeRemoved(bool value)
{
	//Do nothing?
	if (value==toRemoved) {
		return;
	}

	toRemoved = value;
}

