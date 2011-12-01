/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "WorkGroup.hpp"

//For debugging
#include <boost/thread.hpp>
#include "entities/Agent.hpp"
#include "util/OutputUtil.hpp"

using std::vector;
using boost::barrier;
using boost::function;

using namespace sim_mob;


const bool sim_mob::WorkGroup::DebugOn = true;


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
void sim_mob::WorkGroup::migrateByID(Entity* ag, int toID)
{
	//Dispatch
	migrate(ag, (toID>=0) ? workers.at(toID) : nullptr);
}


void sim_mob::WorkGroup::migrate(Entity* ag, Worker<Entity>* toWorker)
{
	if (!ag)
		return;

	//Call the parent migrate function.
	sim_mob::Worker<Entity>* from = ag->currWorker;
	sim_mob::Worker<Entity>* to = toWorker;
	sim_mob::SimpleWorkGroup<Entity>::migrate(ag, from, to);

	//Update our Entity's pointer.
	ag->currWorker = to;

	//More automatic updating
	if (from) {
		//Remove this entity's Buffered<> types from our list
		manageData(dynamic_cast<BufferedDataManager*>(from), ag, false);

		//Debugging output
		if (DebugOn) {
			Agent* agent = dynamic_cast<Agent*>(ag);
			if (agent) {
				boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
				std::cout <<"Removing Agent " <<agent->getId() <<" from worker: " <<from <<std::endl;
			}
		}
	}
	if (to) {
		//Add this entity's Buffered<> types to our list
		manageData(dynamic_cast<BufferedDataManager*>(to), ag, true);

		//Debugging output
		if (DebugOn) {
			Agent* agent = dynamic_cast<Agent*>(ag);
			if (agent) {
				boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
				std::cout <<"Adding Agent " <<agent->getId() <<" to worker: " <<to <<std::endl;
			}
		}
	}
}

