#include "Agent.hpp"

using namespace sim_mob;

using std::vector;


vector<Agent*> sim_mob::Agent::all_agents;



sim_mob::Agent::Agent(unsigned int id) : Entity(id), xPos(0), yPos(0) {
	toRemoved = false;
}


/*void sim_mob::Agent::update(frame_t frameNumber) {

}*/


/*void sim_mob::Agent::subscribe(BufferedDataManager* mgr, bool isNew)
{
	if (isNew) {
		xPos.migrate(mgr);
		yPos.migrate(mgr);
	} else {
		xPos.migrate(NULL);
		yPos.migrate(NULL);
	}

}*/

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

	//Allowed?
	/*if (callerParent->getId() == this->getId()) {
		toRemoved = value;
	}*/

	//If not, fail.
	/*std::cout <<"Error: Role tried to modify Agent it wasn't responsible for.";
	throw 1;*/
}



///////////////////////////////
// Temporary location
///////////////////////////////
/*void sim_mob::Agent::pathChoice(Agent& a) {
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
}*/





