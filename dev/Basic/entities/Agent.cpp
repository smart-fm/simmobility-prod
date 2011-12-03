/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Agent.hpp"

#include "util/OutputUtil.hpp"

using namespace sim_mob;

using std::vector;
using std::priority_queue;

#ifndef DISABLE_DYNAMIC_DISPATCH
boost::mutex sim_mob::Agent::all_agents_lock;
priority_queue<Entity*> sim_mob::Agent::pending_agents = priority_queue<Entity*>(sim_mob::cmp_agent_start());
#endif

vector<Entity*> sim_mob::Agent::all_agents;



//Implementation of our comparison function for Agents by start time.
bool sim_mob::cmp_agent_start::operator() (const Entity* x, const Entity* y) const
{
	//We want a lower start time to translate into a higher priority.
	return x->getStartTime() > y->getStartTime();
}


unsigned int sim_mob::Agent::next_agent_id = 0;
unsigned int sim_mob::Agent::GetAndIncrementID(int preferredID)
{
	//If the ID is valid, modify next_agent_id;
	if (preferredID>static_cast<int>(next_agent_id)) {
		next_agent_id = static_cast<unsigned int>(preferredID);
	}

	//Assign and increment
	return next_agent_id++;
}



sim_mob::Agent::Agent(int id) : Entity(GetAndIncrementID(id)), originNode(nullptr), destNode(nullptr), /*startTime(0),*/ xPos(0), yPos(0)
		/*xVel(0), yVel(0)*//*,xAcc(0), yAcc(0)*//*, currentLink(0),currentCrossing(-1)*/{
	toRemoved = false;
}

sim_mob::Agent::~Agent()
{

}



void sim_mob::Agent::buildSubscriptionList()
{
	subscriptionList_cached.push_back(&xPos);
	subscriptionList_cached.push_back(&yPos);
	//subscriptionList_cached.push_back(&xVel);
	//subscriptionList_cached.push_back(&yVel);
	subscriptionList_cached.push_back(&fwdVel);
	subscriptionList_cached.push_back(&latVel);
	//subscriptionList_cached.push_back(&xAcc);
	//subscriptionList_cached.push_back(&yAcc);
	//subscriptionList_cached.push_back(&currentLink);
	//subscriptionList_cached.push_back(&currentCrossing);

}


bool sim_mob::Agent::isToBeRemoved()
{
	return toRemoved;
}


void sim_mob::Agent::setToBeRemoved()
{
	toRemoved = true;
}

