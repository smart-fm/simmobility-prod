#include "Agent.hpp"

using namespace sim_mob;


//TEMP
boost::mutex sim_mob::Agent::global_mutex;


sim_mob::Agent::Agent(unsigned int id) : Entity(id), xPos(NULL, 0), yPos(NULL, 0) {
	int currMode = id%4;

	//TODO: Inheritance, inheritance, inheritance
	if (currMode==0)
		currMode = DRIVER;
	else if (currMode==1)
		currMode = PEDESTRIAN;
	else if (currMode==2)
		currMode = CYCLIST;
	else if (currMode==3)
		currMode = PASSENGER;
}


void sim_mob::Agent::update() {
	//TODO: Migrate this into the agent's behavior using inheritance.
	/*if (currMode==DRIVER) {
		updateDriverBehavior(*this);
	} else if (currMode==PEDESTRIAN || currMode==CYCLIST) {
		updatePedestrianBehavior(*this);
	} else if (currMode==PASSENGER) {
		updatePassengerBehavior(*this);
	}*/

	//Compute
	unsigned int newX = this->xPos.get()+10;
	unsigned int newY = this->yPos.get()+10;

	//Constrain to boundaries.
	newX = std::max(std::min(newX, lowerRight.xPos), topLeft.xPos);
	newY = std::max(std::min(newY, lowerRight.yPos), topLeft.yPos);


	//Set
	this->xPos.set(newX);
	this->yPos.set(newY);

	//Output (temp)
	{
		boost::mutex::scoped_lock local_lock(global_mutex);
		std::cout <<"(" <<this->getId() <<"," <<newX <<"," <<newY <<")" <<std::endl;
	}
}


void sim_mob::Agent::subscribe(BufferedDataManager* mgr, bool isNew)
{
	if (isNew) {
		xPos.migrate(mgr);
		yPos.migrate(mgr);
	} else {
		xPos.migrate(NULL);
		yPos.migrate(NULL);
	}

}


void sim_mob::Agent::updateShortestPath() {
	trivial(getId());
}



///////////////////////////////
// Temporary location
///////////////////////////////
void sim_mob::Agent::pathChoice(Agent& a) {
	trivial(a.getId()); //Trivial. Will update path choice later.
}
void sim_mob::Agent::updateDriverBehavior(Agent& a) {
	trivial(a.getId()); //Trivial. Will update driver behavior later.

	//Trivial. Will detect "end of link" and update path choice later.
	if (trivial(a.getId())) {
		pathChoice(a);
	}
}
void sim_mob::Agent::updatePedestrianBehavior(Agent& a) {
	trivial(a.getId()); //Trivial. Will update pedestrian behavior later.

	//Trivial. Will detect "end of link" and update path choice later.
	if (trivial(a.getId())) {
		pathChoice(a);
	}
}
void sim_mob::Agent::updatePassengerBehavior(Agent& a) {
	trivial(a.getId()); //Trivial. Will update passenger behavior later.

	//Trivial. Will detect "end of link" and update path choice later.
	if (trivial(a.getId())) {
		pathChoice(a);  //NOTE: Do passengers need to do this?
	}
}

