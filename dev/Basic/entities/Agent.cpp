#include "Agent.hpp"


Agent::Agent(unsigned int id) : Entity(id) {
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


void Agent::update() {
	//TODO: Migrate this into the agent's behavior using inheritance.
	if (currMode==DRIVER) {
		updateDriverBehavior(*this);
	} else if (currMode==PEDESTRIAN || currMode==CYCLIST) {
		updatePedestrianBehavior(*this);
	} else if (currMode==PASSENGER) {
		updatePassengerBehavior(*this);
	}
}


void Agent::updateShortestPath() {
	trivial(id);
}



///////////////////////////////
// Temporary location
///////////////////////////////
void Agent::pathChoice(Agent& a) {
	trivial(a.getId()); //Trivial. Will update path choice later.
}
void Agent::updateDriverBehavior(Agent& a) {
	trivial(a.getId()); //Trivial. Will update driver behavior later.

	//Trivial. Will detect "end of link" and update path choice later.
	if (trivial(a.getId())) {
		pathChoice(a);
	}
}
void Agent::updatePedestrianBehavior(Agent& a) {
	trivial(a.getId()); //Trivial. Will update pedestrian behavior later.

	//Trivial. Will detect "end of link" and update path choice later.
	if (trivial(a.getId())) {
		pathChoice(a);
	}
}
void Agent::updatePassengerBehavior(Agent& a) {
	trivial(a.getId()); //Trivial. Will update passenger behavior later.

	//Trivial. Will detect "end of link" and update path choice later.
	if (trivial(a.getId())) {
		pathChoice(a);  //NOTE: Do passengers need to do this?
	}
}
