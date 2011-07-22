#include "Person.hpp"

using namespace sim_mob;


sim_mob::Person::Person(unsigned int id) : Agent(id), currRole(nullptr)
{

}

void sim_mob::Person::update(frame_t frameNumber)
{
	if (currRole) {
		currRole->update(frameNumber);
	}

	//Output (temp)
	{
		boost::mutex::scoped_lock local_lock(BufferedBase::global_mutex);
		std::cout <<"(" <<this->getId() <<"," <<frameNumber<<","<<this->xPos.get()<<"," <<this->yPos.get() <<",1)"<<std::endl;
		//std::cout<<"           ["<<this->xVel.get()<<","<<this->xAcc.get()<<"]"<<std::endl;
		//if(this->leader==NULL)std::cout<<"   [leading:NULL]"<<std::endl;
		//else std::cout<<"   [leading:" << this->leader->getId()<<"]"<<std::endl;
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
