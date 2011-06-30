#include "Person.hpp"

using namespace sim_mob;


sim_mob::Person::Person(unsigned int id) : Agent(id), currRole(NULL)
{

}

void sim_mob::Person::update(frame_t frameNumber) {
	if (currRole!=NULL) {
		currRole->update(frameNumber);
	}

	//Output (temp)
	{
		boost::mutex::scoped_lock local_lock(Agent::global_mutex);
		std::cout <<"(" <<this->getId() <<"," <<frameNumber<<","<<this->xPos.get()<<"," <<this->yPos.get() /*<<","<<currPhase*/ <<")" <<std::endl;
	}
}

void sim_mob::Person::subscribe(sim_mob::BufferedDataManager* mgr, bool isNew) {

}

void sim_mob::Person::changeRole(sim_mob::Role* newRole) {
	if (this->currRole!=NULL) {
		this->currRole->setParent(NULL);
	}

	this->currRole = newRole;

	if (this->currRole!=NULL) {
		this->currRole->setParent(this);
	}
}
