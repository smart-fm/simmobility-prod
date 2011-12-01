/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <stdexcept>
#include <boost/thread.hpp>

#include "workers/Worker.hpp"

#include "util/LangHelpers.hpp"


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
	SimpleWorkGroup(size_t size, unsigned int endTick=0, unsigned int tickStep=1, bool auraManagerActive=false);

	virtual ~SimpleWorkGroup();

	//template <typename WorkType>  //For now, just assume Workers
	void initWorkers(typename Worker<EntityType>::ActionFunction* action = nullptr);

	//Worker<EntityType>* const getWorker(size_t id);
	void startAll();
	void interrupt();
	size_t size();

	void wait();
	void waitExternAgain();
	void migrate(EntityType* ag, Worker<EntityType>* from, Worker<EntityType>* to);

	Worker<EntityType>* getWorker(int id) {
		if (id<0) {
			return nullptr;
		}
		return workers.at(id);
	}

protected:
	//Does nothing; see sub-class WorkGroup
	//virtual void manageData(sim_mob::BufferedDataManager* mgr, EntityType* ag, bool takeControl) = 0;


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

	bool auraManagerActive;

};


} //End sim_mob namespace



/**
 * Template function must be defined in the same translational unit as it is declared.
 */
template <class EntityType>
void sim_mob::SimpleWorkGroup<EntityType>::initWorkers(typename Worker<EntityType>::ActionFunction* action)
{
	for (size_t i=0; i<total_size; i++) {
		workers.push_back(new Worker<EntityType>(action, &shared_barr, &external_barr, endTick, tickStep, auraManagerActive));
	}
}



//////////////////////////////////////////////
// These also need the temoplate parameter,
// but don't actually do anything with it.
//////////////////////////////////////////////


template <class EntityType>
sim_mob::SimpleWorkGroup<EntityType>::SimpleWorkGroup(size_t size, unsigned int endTick, unsigned int tickStep, bool auraManagerActive) :
		shared_barr(size+1), external_barr(size+1), endTick(endTick), tickStep(tickStep), total_size(size), auraManagerActive(auraManagerActive)
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
void sim_mob::SimpleWorkGroup<EntityType>::migrate(EntityType* ag, Worker<EntityType>* from, Worker<EntityType>* to)
{
	if (!ag)
		return;

	if (from) {
		//Remove
		from->remEntity(ag);
	}

	if (to) {
		//Add
		to->addEntity(ag);
	}
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
void sim_mob::SimpleWorkGroup<EntityType>::waitExternAgain()
{
	external_barr.wait();
}


template <class EntityType>
void sim_mob::SimpleWorkGroup<EntityType>::interrupt()
{
	for (size_t i=0; i<workers.size(); i++)
		workers[i]->interrupt();
}







