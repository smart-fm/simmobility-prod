/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "WorkGroup.hpp"

using std::vector;
using boost::barrier;
using boost::function;

using namespace sim_mob;


void sim_mob::WorkGroup::manageData(sim_mob::BufferedDataManager* mgr, Entity* ag, bool takeControl)
{
	for (std::vector<sim_mob::BufferedBase*>::iterator it=ag->getSubscriptionList().begin(); it!=ag->getSubscriptionList().end(); it++) {
		if (takeControl) {
			mgr->beginManaging(*it);
		} else {
			mgr->stopManaging(*it);
		}
	}
}


/**
 * Set "toID" to -1 to skip that step. Automatically removes the Agent from its given Worker if that Worker
 *  has been set.
 */
void sim_mob::WorkGroup::migrate(Entity* ag, int toID)
{
	if (!ag)
		return;

	//Call the parent migrate function.
	sim_mob::Worker<Entity>* from = ag->currWorker;
	sim_mob::Worker<Entity>* to = (toID>=0) ? workers.at(toID) : nullptr;
	sim_mob::SimpleWorkGroup<Entity>::migrate(ag, from, to);

	//Update our Entity's pointer.
	ag->currWorker = to;

	//More automatic updating
	if (from) {
		//Remove this entity's Buffered<> types from our list
		manageData(dynamic_cast<BufferedDataManager*>(from), ag, false);
	}
	if (to) {
		//Add this entity's Buffered<> types to our list
		manageData(dynamic_cast<BufferedDataManager*>(to), ag, true);
	}
}

