/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Person.hpp"

using namespace sim_mob;


sim_mob::Person::Person(unsigned int id) : Agent(id), currRole(nullptr), currTripChain(nullptr)
{

}

void sim_mob::Person::update(frame_t frameNumber)
{
	//Update this agent's role
	if (currRole) {
		currRole->update(frameNumber);
	}

	//Output (temp)
	{
		//boost::mutex::scoped_lock local_lock(BufferedBase::global_mutex);
		//std::cout <<"(" <<this->getId() <<"," <<frameNumber<<","<<this->xPos.get()<<"," <<this->yPos.get() <<",1)"<<std::endl;
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
	if(dynamic_cast<Driver*>(this->getRole())){
		Driver* d=dynamic_cast<Driver*>(this->getRole());
		subscriptionList_cached.push_back(&(d->currLink_));
		subscriptionList_cached.push_back(&(d->currRoadSegment_));
		subscriptionList_cached.push_back(&(d->currLane_));
		subscriptionList_cached.push_back(&(d->polylineIndex_));
		subscriptionList_cached.push_back(&(d->offsetInPolyline_));
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

sim_mob::Role* sim_mob::Person::getRole(){
	return currRole;
}
