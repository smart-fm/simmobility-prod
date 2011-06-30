#include "Agent.hpp"

using namespace sim_mob;

using std::vector;


vector<Agent*> sim_mob::Agent::all_agents;
double Agent::collisionForce = 20;
double Agent::agentRadius = 10;

//TEMP
boost::mutex sim_mob::Agent::global_mutex;


sim_mob::Agent::Agent(unsigned int id) : Entity(id), xPos(NULL, 0), yPos(NULL, 0) {
	toRemoved = false;
}


void sim_mob::Agent::update(frame_t frameNumber) {

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


/*void sim_mob::Agent::updateShortestPath() {
	trivial(getId());
}*/



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





