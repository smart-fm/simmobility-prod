#pragma once

#include <vector>
#include <stdexcept>
#include <boost/thread.hpp>

#include "workers/Worker.hpp"
#include "entities/Entity.hpp"

#include "constants.h"


namespace sim_mob
{

/*
 * Worker wrapper, similar to thread_group but using barriers.
 * A SimpleWorkGroup provides a convenient wrapper for Workers, similarly to
 *   how a ThreadGroup manages threads. The main difference is that the number of
 *   worker threads cannot be changed once the object has been constructed.
 * A SimpleWorkGroup maintains one extra hold on the shared barrier; to "advance"
 *   a group, call SimpleWorkGroup::wait().
 */
template <class EntityType>
class SimpleWorkGroup {
public:
	//These are passed along to the Workers:
	//  endTick=0 means run forever.
	//  tickStep is used to allow Workers to skip ticks; no barriers are locked.
	SimpleWorkGroup(size_t size, unsigned int endTick=0, unsigned int tickStep=1);

	virtual ~SimpleWorkGroup();

	//template <typename WorkType>  //For now, just assume Workers
	void initWorkers(typename Worker<EntityType>::actionFunction* action = nullptr);

	Worker<EntityType>* const getWorker(size_t id);
	void startAll();
	void interrupt();
	size_t size();

	void wait();

	//TODO: Move this to the Worker, not the work group. Maybe?
	void migrate(EntityType * ag, int fromID, int toID);

protected:
	//Does nothing; see sub-class WorkGroup
	virtual void manageData(sim_mob::BufferedDataManager* mgr, EntityType* ag, bool takeControl);


protected:
	//Shared barrier
	boost::barrier shared_barr;
	boost::barrier external_barr;

	//Worker object management
	std::vector<Worker<EntityType>*> workers;

	//Passed along to Workers
	unsigned int endTick;
	unsigned int tickStep;

	//Maintain an offset. When it reaches zero, reset to tickStep and sync barriers
	unsigned int tickOffset;

	//Only used once
	size_t total_size;

};


/**
 * A WorkGroup is like a SimpleWorkGroup, except that it also manages all Buffered<>
 * types that each EntityType contains. This requires EntityType to be a sub-class
 * of Entity, and to implement getSubscriptionList().
 */
template <class EntityType>
class WorkGroup : public sim_mob::SimpleWorkGroup<EntityType> {
public:
	WorkGroup(size_t size, unsigned int endTick=0, unsigned int tickStep=1)
	: SimpleWorkGroup<EntityType>(size, endTick, tickStep) {}

protected:
	//Migrates all subscribed types.
	virtual void manageData(sim_mob::BufferedDataManager* mgr, EntityType* ag, bool takeControl);

};


}


/**
 * Template function must be defined in the same translational unit as it is declared.
 */
template <class EntityType>
void sim_mob::SimpleWorkGroup<EntityType>::initWorkers(typename Worker<EntityType>::actionFunction* action)
{
	for (size_t i=0; i<total_size; i++) {
		workers.push_back(new Worker<EntityType>(action, &shared_barr, &external_barr, endTick/tickStep));
	}
}


template <class EntityType>
sim_mob::Worker<EntityType>* const sim_mob::SimpleWorkGroup<EntityType>::getWorker(size_t id)
{
	if (id >= workers.size())
		throw std::runtime_error("Invalid Worker id.");
	return workers[id];
}


//////////////////////////////////////////////
// These also need the temoplate parameter,
// but don't actually do anythign with it.
//////////////////////////////////////////////


template <class EntityType>
sim_mob::SimpleWorkGroup<EntityType>::SimpleWorkGroup(size_t size, unsigned int endTick, unsigned int tickStep) :
		shared_barr(size+1), external_barr(size+1), endTick(endTick), tickStep(tickStep), total_size(size)
{
}


template <class EntityType>
sim_mob::SimpleWorkGroup<EntityType>::~SimpleWorkGroup()
{
	for (size_t i=0; i<workers.size(); i++) {
		workers[i]->join();  //NOTE: If we don't join all Workers, we get threading exceptions.
		delete workers[i];
	}
}


template <class EntityType>
void sim_mob::SimpleWorkGroup<EntityType>::startAll()
{
	tickOffset = tickStep;
	for (size_t i=0; i<workers.size(); i++) {
		workers[i]->start();
	}
}


template <class EntityType>
size_t sim_mob::SimpleWorkGroup<EntityType>::size()
{
	return workers.size();
}



template <class EntityType>
void sim_mob::SimpleWorkGroup<EntityType>::wait()
{
	if (--tickOffset>0) {
		return;
	}
	tickOffset = tickStep;

	shared_barr.wait();
	external_barr.wait();
}


template <class EntityType>
void sim_mob::SimpleWorkGroup<EntityType>::interrupt()
{
	for (size_t i=0; i<workers.size(); i++)
		workers[i]->interrupt();
}


template <class EntityType>
void sim_mob::SimpleWorkGroup<EntityType>::manageData(sim_mob::BufferedDataManager* mgr, EntityType* ag, bool takeControl)
{
	/*for (std::vector<sim_mob::BufferedBase*>::iterator it=ag->getSubscriptionList().begin(); it!=ag->getSubscriptionList().end(); it++) {
		if (takeControl) {
			mgr->beginManaging(*it);
		} else {
			mgr->stopManaging(*it);
		}
	}*/
}


template <class EntityType>
void sim_mob::WorkGroup<EntityType>::manageData(sim_mob::BufferedDataManager* mgr, EntityType* ag, bool takeControl)
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
 * Set "fromID" or "toID" to -1 to skip that step.
 */
template <class EntityType>
void sim_mob::SimpleWorkGroup<EntityType>::migrate(EntityType* ag, int fromID, int toID)
{
	if (ag==nullptr)
		return;

	if (fromID >= 0) {
		//Remove from the old location
		sim_mob::Worker<EntityType>* const from = getWorker(fromID);
		from->remEntity(ag);

		//Remove this entity's Buffered<> types from our list
		manageData(dynamic_cast<BufferedDataManager*>(from), ag, false);
	}

	if (toID >= 0) {
		//Add to the new location
		sim_mob::Worker<EntityType>* const to = getWorker(toID);
		to->addEntity(ag);

		//Add this entity's Buffered<> types to our list
		manageData(dynamic_cast<BufferedDataManager*>(to), ag, true);
	}
}


