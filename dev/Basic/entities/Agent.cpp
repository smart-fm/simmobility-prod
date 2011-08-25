/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Agent.hpp"

using namespace sim_mob;

using std::vector;


vector<Agent*> sim_mob::Agent::all_agents;



sim_mob::Agent::Agent(unsigned int id) : Entity(id), originNode(nullptr), destNode(nullptr), startTime(0), xPos(0), yPos(0),
		xVel(0), yVel(0),xAcc(0), yAcc(0), currentLink(0),currentCrossing(-1){
	toRemoved = false;
}



void sim_mob::Agent::buildSubscriptionList()
{
	subscriptionList_cached.push_back(&xPos);
	subscriptionList_cached.push_back(&yPos);
	subscriptionList_cached.push_back(&xVel);
	subscriptionList_cached.push_back(&yVel);
	subscriptionList_cached.push_back(&xAcc);
	subscriptionList_cached.push_back(&yAcc);
	subscriptionList_cached.push_back(&currentLink);
	subscriptionList_cached.push_back(&currentCrossing);

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

