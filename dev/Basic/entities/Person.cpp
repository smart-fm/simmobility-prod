/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Person.hpp"
#include "WorkGroup.hpp"

using std::vector;
using namespace sim_mob;


sim_mob::Person::Person(int id) : Agent(id), currRole(nullptr), currTripChain(nullptr)
{

}


sim_mob::Person::~Person()
{
	safe_delete(currRole);
}


void sim_mob::Person::update(frame_t frameNumber)
{
	//Update this agent's role
	if (currRole) {
		currRole->update(frameNumber);
	}

	//Are we done?
	//NOTE: Make sure you set this flag AFTER performing your final output.
	//TODO: Two bugs. First, calling migrate() while still within the "entity_worker" loop will
	//      invalidate our iterator. Second, removing from "all_agents" in update can lead to
	//      inconsistencies due to the Aura manager; this should happen in "flip()" instead.
	if (isToBeRemoved()) {
		//TODO: Everything in this scope will likely be moved to the Dispatch Manager later on.

		//Migrate this Agent off of its current Worker.
		Agent::TMP_AgentWorkGroup->migrate(this, -1);

		//Remove this Agent from the list of discoverable Agents
		vector<Agent*>::iterator it = std::find(Agent::all_agents.begin(),Agent::all_agents.end(), this);
		if (it!=Agent::all_agents.end()) {
			Agent::all_agents.erase(it);
		}

		//Deleting yourself is ok if you're sure there are no lingering references
		// (again, this will be moved to the Dispatch Manager later. So please ignore the ugliness of deleting this for now.)
		delete this;
	}
}

/*void sim_mob::Person::subscribe(sim_mob::BufferedDataManager* mgr, bool isNew) {
	Agent::subscribe(mgr, isNew); //Get x/y subscribed.
}*/

void sim_mob::Person::buildSubscriptionList()
{
	//First, add the x and y co-ordinates
	Agent::buildSubscriptionList();

	//Now, add our own properties.
	vector<BufferedBase*> roleParams = this->getRole()->getSubscriptionParams();
	for (vector<BufferedBase*>::iterator it=roleParams.begin(); it!=roleParams.end(); it++) {
		subscriptionList_cached.push_back(*it);
	}
}

void sim_mob::Person::changeRole(sim_mob::Role* newRole)
{
	if (this->currRole) {
		this->currRole->setParent(nullptr);
	}

	this->currRole = newRole;

	if (this->currRole) {
		this->currRole->setParent(this);
	}
}

sim_mob::Role* sim_mob::Person::getRole() const {
	return currRole;
}
