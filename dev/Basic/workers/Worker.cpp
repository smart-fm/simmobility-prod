/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Worker.hpp"

#include <queue>
#include <sstream>

using std::vector;
using std::priority_queue;
using boost::barrier;
using boost::function;

#include "workers/WorkGroup.hpp"
#include "util/OutputUtil.hpp"
#include "conf/simpleconf.hpp"

using namespace sim_mob;



//////////////////////////////////////////////
// Template implementation
//////////////////////////////////////////////


void sim_mob::Worker::addEntity(Entity* entity)
{
	//Save this entity in the data vector.
	managedEntities.push_back(entity);
}


void sim_mob::Worker::remEntity(Entity* entity)
{
	//Remove this entity from the data vector.
	typename std::vector<Entity*>::iterator it = std::find(managedEntities.begin(), managedEntities.end(), entity);
	if (it!=managedEntities.end()) {
		managedEntities.erase(it);
	}
}


std::vector<Entity*>& sim_mob::Worker::getEntities() {
	return managedEntities;
}


#ifndef SIMMOB_DISABLE_DYNAMIC_DISPATCH
void sim_mob::Worker::scheduleForAddition(Entity* entity)
{
	//Save for later
	toBeAdded.push_back(entity);
}


void sim_mob::Worker::scheduleForRemoval(Entity* entity)
{
	//Save for later
	toBeRemoved.push_back(entity);
}

#else

void sim_mob::Worker::scheduleEntityNow(Entity* entity)
{
	//Add it now.
	migrateIn(*entity);
}


#endif



//////////////////////////////////////////////
// These also need the temoplate parameter,
// but don't actually do anything with it.
//////////////////////////////////////////////



sim_mob::Worker::Worker(WorkGroup* parent, boost::barrier& internal_barr, boost::barrier& external_barr, std::vector<Entity*>* entityRemovalList, ActionFunction* action, frame_t endTick, frame_t tickStep, bool auraManagerActive)
    : BufferedDataManager(),
      internal_barr(internal_barr), external_barr(external_barr),
      action(action),
      endTick(endTick),
      tickStep(tickStep),
      auraManagerActive(auraManagerActive),
      parent(parent),
      entityRemovalList(entityRemovalList)
{
}


sim_mob::Worker::~Worker()
{
	//Clear all tracked entitites
	while (!managedEntities.empty()) {
		remEntity(managedEntities.front());
	}

	//Clear all tracked data
	while (!managedData.empty()) {
		stopManaging(managedData[0]);
	}
}


void sim_mob::Worker::start()
{
	main_thread = boost::thread(boost::bind(&Worker::barrier_mgmt, this));
}


void sim_mob::Worker::join()
{
	main_thread.join();
}


void sim_mob::Worker::interrupt()
{
	if (main_thread.joinable()) {
		main_thread.interrupt();
	}
}


void sim_mob::Worker::addPendingEntities()
{
	if (ConfigParams::GetInstance().DynamicDispatchDisabled()) {
		return;
	}

	for (vector<Entity*>::iterator it=toBeAdded.begin(); it!=toBeAdded.end(); it++) {
		//Migrate its Buffered properties.
		migrateIn(**it);
	}
	toBeAdded.clear();
}


void sim_mob::Worker::removePendingEntities()
{
	if (ConfigParams::GetInstance().DynamicDispatchDisabled()) {
		return;
	}

	for (vector<Entity*>::iterator it=toBeRemoved.begin(); it!=toBeRemoved.end(); it++) {
		//Migrate out its buffered properties.
		migrateOut(**it);

		//Remove it from our global list.
		if (!entityRemovalList) {
			throw std::runtime_error("Attempting to remove an entity from a WorkGroup that doesn't allow it.");
		}
		entityRemovalList->push_back(*it);
	}
	toBeRemoved.clear();
}



void sim_mob::Worker::barrier_mgmt()
{
	frame_t currTick = 0;
	for (bool active=true; active;) {
		//Add Agents as required.
		addPendingEntities();

		//Perform all our Agent updates, etc.
		perform_main(currTick);

		//Remove Agents as requires
		removePendingEntities();

		//Advance local time-step.
		currTick += tickStep;
		active = (endTick==0 || currTick<endTick);

		//First barrier
		internal_barr.wait();

		//Now flip all remaining data.
		perform_flip();

		//Second barrier
		external_barr.wait();

        // Wait for the AuraManager
		if (auraManagerActive) {
			external_barr.wait();
		}
	}
}



void sim_mob::Worker::migrateOut(Entity& ag)
{
	//Sanity check
	if (ag.currWorker != this) {
		std::stringstream msg;
		msg <<"Error: Entity has somehow switched workers: " <<ag.currWorker <<"," <<this;
		throw std::runtime_error(msg.str().c_str());
	}

	//Simple migration
	remEntity(&ag);

	//Update our Entity's pointer.
	ag.currWorker = nullptr;

	//Remove this entity from our Agent mappings.
	//agentMapping.erase(ag);

	//Remove this entity's Buffered<> types from our list
	stopManaging(ag.getSubscriptionList());

	//Debugging output
	if (Debug::WorkGroupSemantics) {
#ifndef SIMMOB_DISABLE_OUTPUT
		boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
		std::cout <<"Removing Entity " <<ag.getId() <<" from worker: " <<this <<std::endl;
#endif
	}
}



void sim_mob::Worker::migrateIn(Entity& ag)
{
	//Sanity check
	if (ag.currWorker) {
		std::stringstream msg;
		msg <<"Error: Entity is already being managed: " <<ag.currWorker <<"," <<this;
		throw std::runtime_error(msg.str().c_str());
	}

	//Simple migration
	addEntity(&ag);

	//Update our Entity's pointer.
	ag.currWorker = this;

	//Add this entity to our Agent mappings.
	//agentMapping[ag] = toID;

	//Add this entity's Buffered<> types to our list
	beginManaging(ag.getSubscriptionList());

	//Debugging output
	if (Debug::WorkGroupSemantics) {
#ifndef SIMMOB_DISABLE_OUTPUT
		boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
		std::cout <<"Adding Entity " <<ag.getId() <<" to worker: " <<this <<std::endl;
#endif
	}
}



void sim_mob::Worker::perform_main(frame_t frameNumber)
{
	if (action) {
		(*action)(*this, frameNumber);
	}
}


void sim_mob::Worker::perform_flip()
{
	//Flip all data managed by this worker.
	this->flip();
}


