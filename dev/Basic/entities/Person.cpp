/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Person.hpp"
//#include "SimpleWorkGroup.hpp"

using std::vector;
using namespace sim_mob;


sim_mob::Person::Person(int id) : Agent(id), currRole(nullptr), currTripChain(nullptr)
{

}


sim_mob::Person::~Person()
{
	safe_delete(currRole);
}


bool sim_mob::Person::update(frame_t frameNumber)
{
	//Update this agent's role
	if (currRole) {
		currRole->update(frameNumber);
	}

	//Return true unless we are scheduled for removal.
	//NOTE: Make sure you set this flag AFTER performing your final output.
	return !isToBeRemoved();
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
