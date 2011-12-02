/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Worker.hpp"

#include <queue>

using std::vector;
using std::priority_queue;
using boost::barrier;
using boost::function;

#include "WorkGroup.hpp"
#include "entities/Agent.hpp"
#include "entities/Person.hpp"

using namespace sim_mob;



//////////////////////////////////////////////
// Template implementation
//////////////////////////////////////////////

template <class EntityType>
void sim_mob::Worker<EntityType>::addEntity(EntityType* entity)
{
	//Save this entity in the data vector.
	data.push_back(entity);
}

template <class EntityType>
void sim_mob::Worker<EntityType>::remEntity(EntityType* entity)
{
	//Remove this entity from the data vector.
	typename std::vector<EntityType*>::iterator it = std::find(data.begin(), data.end(), entity);
	if (it!=data.end()) {
		data.erase(it);
	}
}

template <class EntityType>
std::vector<EntityType*>& sim_mob::Worker<EntityType>::getEntities() {
	return data;
}

template <class EntityType>
void sim_mob::Worker<EntityType>::scheduleForAddition(EntityType* entity)
{
	//Save for later
	toBeAdded.push_back(entity);
}

template <class EntityType>
void sim_mob::Worker<EntityType>::scheduleForRemoval(EntityType* entity)
{
	//Save for later
	toBeRemoved.push_back(entity);
}



//////////////////////////////////////////////
// These also need the temoplate parameter,
// but don't actually do anything with it.
//////////////////////////////////////////////


template <class EntityType>
sim_mob::Worker<EntityType>::Worker(SimpleWorkGroup<EntityType>* parent, ActionFunction* action, boost::barrier* internal_barr, boost::barrier* external_barr, frame_t endTick, frame_t tickStep, bool auraManagerActive)
    : BufferedDataManager(),
      internal_barr(internal_barr), external_barr(external_barr), action(action),
      endTick(endTick),
      tickStep(tickStep),
      auraManagerActive(auraManagerActive),
      parent(parent),
      active(/*this, */false)  //Passing the "this" pointer is probably ok, since we only use the base class (which is constructed)
{
	this->beginManaging(&active);

	//Test
	if (!internal_barr || !external_barr) {
		throw std::runtime_error("Worker won't function correctly with a non-null barrier.");
	}
}

template <class EntityType>
sim_mob::Worker<EntityType>::~Worker()
{
	//Clear all tracked entitites
	while (!data.empty()) {
		remEntity(data[0]);
	}

	//Clear all tracked data
	while (!managedData.empty()) {
		stopManaging(managedData[0]);
	}
}

template <class EntityType>
void sim_mob::Worker<EntityType>::start()
{
	active.force(true);
	currTick = 0;
	main_thread = boost::thread(boost::bind(&Worker::barrier_mgmt, this));
}

template <class EntityType>
void sim_mob::Worker<EntityType>::join()
{
	main_thread.join();
}

template <class EntityType>
void sim_mob::Worker<EntityType>::interrupt()
{
	if (main_thread.joinable()) {
		main_thread.interrupt();
	}
}


template <class EntityType>
void sim_mob::Worker<EntityType>::barrier_mgmt()
{
	for (;active.get();) {
		//Add Agents as required.
		for (vector<Entity*>::iterator it=toBeAdded.begin(); it!=toBeAdded.end(); it++) {
			//Ensure we're on the same page
			WorkGroup* wg = dynamic_cast<WorkGroup*>(parent);
			if (!wg) {
				throw std::runtime_error("Simple workers cannot add Entities at arbitrary times.");
			}

			//Migrate its Buffered properties.
			migrateOut(**it);
			//wg->migrate(**it, this);
		}
		toBeAdded.clear();

		//Perform all our Agent updates, etc.
		perform_main(currTick);

		//Remove Agents as requires
		for (vector<Entity*>::iterator it=toBeRemoved.begin(); it!=toBeRemoved.end(); it++) {
			//Ensure we're on the same page
			WorkGroup* wg = dynamic_cast<WorkGroup*>(parent);
			if (!wg) {
				throw std::runtime_error("Simple workers cannot remove Entities at arbitrary times.");
			}

			//Migrate out its buffered properties.
			migrateIn(**it);
			//wg->migrate(**it, nullptr);

			//Remove it from our global list. Requires locking
			Agent* ag = dynamic_cast<Agent*>(*it);
			if (ag) {
				boost::mutex::scoped_lock local_lock(sim_mob::Agent::all_agents_lock);
				parent->agToBeRemoved.push_back(ag);
			}

			//Delete this entity
			//delete *it;  //NOTE: For now, I'm leaving it in memory to make debugging slightly eaier. ~Seth
		}
		toBeRemoved.clear();

		//Advance local time-step. This must be done before the barrier or "active" could get out of sync.
		currTick += tickStep;
		this->active.set(endTick==0 || currTick<endTick);

		if (internal_barr) {
			internal_barr->wait();
		}

		//Now flip all remaining data.
		perform_flip();

		if (external_barr) {
			external_barr->wait();
		}

        // Wait for the AuraManager
		if (auraManagerActive) {
			if (external_barr) {
				external_barr->wait();
			}
		}
	}
}


template <class EntityType>
void sim_mob::Worker<EntityType>::migrateOut(EntityType& ag)
{
	//Sanity check
	if (ag.currWorker != this) {
		throw std::runtime_error("Error: Entity has somehow switched workers.");
	}

	//Simple migration
	remEntity(&ag);

	//Update our Entity's pointer.
	ag.currWorker = nullptr;

	//Remove this entity's Buffered<> types from our list
	stopManaging(ag.getSubscriptionList());

	//Debugging output
	if (Debug::WorkGroupSemantics) {
		Agent* agent = dynamic_cast<Agent*>(&ag);
		if (agent && dynamic_cast<Person*>(agent)) {
			boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
			std::cout <<"Removing Agent " <<agent->getId() <<" from worker: " <<this <<std::endl;
		}
	}
}


template <class EntityType>
void sim_mob::Worker<EntityType>::migrateIn(EntityType& ag)
{
	//Simple migration
	addEntity(&ag);

	//Update our Entity's pointer.
	ag.currWorker = this;

	//Add this entity's Buffered<> types to our list
	beginManaging(ag.getSubscriptionList());

	//Debugging output
	if (Debug::WorkGroupSemantics) {
		Agent* agent = dynamic_cast<Agent*>(&ag);
		if (agent && dynamic_cast<Person*>(agent)) {
			boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
			std::cout <<"Adding Agent " <<agent->getId() <<" to worker: " <<this <<" at requested time: " <<agent->startTime <<std::endl;
		}
	}
}


template <class EntityType>
void sim_mob::Worker<EntityType>::perform_main(frame_t frameNumber)
{
	if (action) {
		(*action)(*this, frameNumber);
	}
}

template <class EntityType>
void sim_mob::Worker<EntityType>::perform_flip()
{
	//Flip all data managed by this worker.
	this->flip();
}



//////////////////////////////////////////////
// Manual template instantiation: Entity
//////////////////////////////////////////////
template sim_mob::Worker<sim_mob::Entity>::Worker(SimpleWorkGroup<sim_mob::Entity>* parent, sim_mob::Worker<sim_mob::Entity>::ActionFunction* action =nullptr, boost::barrier* internal_barr =nullptr, boost::barrier* external_barr =nullptr, frame_t endTick=0, frame_t tickStep=0, bool auraManagerActive=false);
template sim_mob::Worker<sim_mob::Entity>::~Worker();

template void sim_mob::Worker<sim_mob::Entity>::start();
template void sim_mob::Worker<sim_mob::Entity>::interrupt();
template void sim_mob::Worker<sim_mob::Entity>::join();

template void sim_mob::Worker<sim_mob::Entity>::addEntity(Entity* entity);
template void sim_mob::Worker<sim_mob::Entity>::remEntity(Entity* entity);
template std::vector<Entity*>& sim_mob::Worker<sim_mob::Entity>::getEntities();

template void sim_mob::Worker<sim_mob::Entity>::scheduleForAddition(Entity* entity);
template void sim_mob::Worker<sim_mob::Entity>::scheduleForRemoval(Entity* entity);

template void sim_mob::Worker<sim_mob::Entity>::perform_main(frame_t frameNumber);
template void sim_mob::Worker<sim_mob::Entity>::perform_flip();
template void sim_mob::Worker<sim_mob::Entity>::barrier_mgmt();

template void sim_mob::Worker<sim_mob::Entity>::migrateOut(Entity& ent);
template void sim_mob::Worker<sim_mob::Entity>::migrateIn(Entity& ent);

