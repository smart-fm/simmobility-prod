/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "WorkGroup.hpp"

//For debugging
#include <stdexcept>
#include <boost/thread.hpp>
#include "entities/Agent.hpp"
#include "entities/Person.hpp"
#include "util/OutputUtil.hpp"

using std::vector;
using boost::barrier;
using boost::function;

using namespace sim_mob;



/*void sim_mob::WorkGroup::manageData(sim_mob::BufferedDataManager* mgr, Entity* ag, bool takeControl)
{
	vector<sim_mob::BufferedBase*> subs = ag->getSubscriptionList();
	for (std::vector<sim_mob::BufferedBase*>::iterator it=subs.begin(); it!=subs.end(); it++) {
		if (takeControl) {
			mgr->beginManaging(*it);
		} else {
			mgr->stopManaging(*it);
		}
	}
}*/


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
	if (!ag) {
		return;
	}

	//Call the parent migrate function.
	sim_mob::Worker<Entity>* from = ag->currWorker;
	sim_mob::Worker<Entity>* to = toWorker;
	sim_mob::SimpleWorkGroup<Entity>::migrate(ag, from, to);

	//Temp.
	if ((from && to) || (!from && !to)) {
		throw std::runtime_error("Temporary ban on migrating to/from entities where one of those entities is not null.");
	}

	//Update our Entity's pointer.
	ag->currWorker = to;

	//More automatic updating
	if (from) {
		//Remove this entity's Buffered<> types from our list
		from->stopManaging(ag->getSubscriptionList());

		//Debugging output
		if (Debug::WorkGroupSemantics) {
			Agent* agent = dynamic_cast<Agent*>(ag);
			if (agent && dynamic_cast<Person*>(agent)) {
				boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
				std::cout <<"Removing Agent " <<agent->getId() <<" from worker: " <<from <<std::endl;
			}
		}
	}
	if (to) {
		//Add this entity's Buffered<> types to our list
		to->beginManaging(ag->getSubscriptionList());

		//Debugging output
		if (Debug::WorkGroupSemantics) {
			Agent* agent = dynamic_cast<Agent*>(ag);
			if (agent && dynamic_cast<Person*>(agent)) {
				boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
				std::cout <<"Adding Agent " <<agent->getId() <<" to worker: " <<to <<" at requested time: " <<agent->startTime <<std::endl;
			}
		}
	}
}


void sim_mob::WorkGroup::addEntityToWorker(Entity* ent, Worker<Entity>* wrk)
{
	//Add it to our global list.
	Agent* ag = dynamic_cast<Agent*>(ent);
	if (ag) {
		Agent::all_agents.push_back(ag);
	}

	//Migrate its Buffered properties.
	migrate(ent, wrk);
}



void sim_mob::WorkGroup::remEntityFromCurrWorker(Entity* ent)
{
	//Migrate out its buffered properties.
	migrate(ent, nullptr);

	//Remove it from our global list.
	Agent* ag = dynamic_cast<Agent*>(ent);
	if (ag) {
		std::vector<Agent*>::iterator it2 = std::find(Agent::all_agents.begin(), Agent::all_agents.end(), ag);
		if (it2!=Agent::all_agents.end()) {
			Agent::all_agents.erase(it2);
		}
	}

}

