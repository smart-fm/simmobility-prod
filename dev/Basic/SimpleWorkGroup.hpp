/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <stdexcept>
#include <boost/thread.hpp>

#include "entities/Agent.hpp"
#include "workers/Worker.hpp"

#include "util/LangHelpers.hpp"
#include "util/DebugFlags.hpp"


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

	Worker<EntityType>* getWorker(int id);

	void stageAgents();


protected:
	//Shared barrier
	boost::barrier shared_barr;
	boost::barrier external_barr;

	//Worker object management
	std::vector<Worker<EntityType>*> workers;

	//Used to coordinate which Worker gets the next Agent; currently round-robin.
	size_t nextWorkerID;

	//Passed along to Workers
	unsigned int endTick;
	unsigned int tickStep;

	//Maintain an offset. When it reaches zero, reset to tickStep and sync barriers
	unsigned int tickOffset;

	//Only used once
	size_t total_size;

	bool auraManagerActive;

	//Pointers to _actually_ be deleted during this time tick.
	//std::vector<EntityType*> toBeDeletedNow;

	//Entities to be moved during this update tick.
	//std::vector<MoveInstruction> toBeMovedNow;

	//Entities to be moved in the next time tick. Refreshed in flip()
	//std::vector<MoveInstruction> toBeMovedLater;

	//Locking for these arrays
	//static boost::mutex add_remove_array_lock;

	//Needed to stay in sync with the workers
	frame_t nextTimeTickToStage;

	//For collaboration
	virtual void addEntityToWorker(EntityType* ent, Worker<EntityType>* wrk) {
		throw std::runtime_error("Simple workers cannot add Entities at arbitrary times.");
	}
	virtual void remEntityFromCurrWorker(EntityType* ent) {
		throw std::runtime_error("Simple workers cannot remove Entities at arbitrary times.");
	}

};


} //End sim_mob namespace



/**
 * Template function must be defined in the same translational unit as it is declared.
 */
template <class EntityType>
void sim_mob::SimpleWorkGroup<EntityType>::initWorkers(typename Worker<EntityType>::ActionFunction* action)
{
	for (size_t i=0; i<total_size; i++) {
		workers.push_back(new Worker<EntityType>(this, action, &shared_barr, &external_barr, endTick, tickStep, auraManagerActive));
	}
}



//////////////////////////////////////////////
// These also need the temoplate parameter,
// but don't actually do anything with it.
//////////////////////////////////////////////


template <class EntityType>
sim_mob::SimpleWorkGroup<EntityType>::SimpleWorkGroup(size_t size, unsigned int endTick, unsigned int tickStep, bool auraManagerActive) :
		shared_barr(size+1), external_barr(size+1), nextWorkerID(0), endTick(endTick), tickStep(tickStep), total_size(size), auraManagerActive(auraManagerActive),
		nextTimeTickToStage(0)
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
	//Stage any Agents that will become active within the first time tick (in time for the next tick)
	nextTimeTickToStage = 0;
	//stageAgents();

	//Start all workers
	tickOffset = tickStep;
	for (size_t i=0; i<workers.size(); i++) {
		workers[i]->start();
	}
}


template <class EntityType>
void sim_mob::SimpleWorkGroup<EntityType>::stageAgents()
{
	unsigned int nextTickMS = nextTimeTickToStage*ConfigParams::GetInstance().baseGranMS;
	while (!Agent::pending_agents.empty() && Agent::pending_agents.top()->startTime <= nextTickMS) {
		if (sim_mob::Debug::WorkGroupSemantics) {
			std::cout <<"Staging agent ID: " <<Agent::pending_agents.top()->getId() <<" in time for tick: " <<nextTimeTickToStage <<"\n";
		}

		//Find a worker to assign this to and send it the Entity to manage.
		workers.at(nextWorkerID++)->scheduleForAddition(Agent::pending_agents.top());
		nextWorkerID %= workers.size();
		Agent::pending_agents.pop();
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
sim_mob::Worker<EntityType>* sim_mob::SimpleWorkGroup<EntityType>::getWorker(int id)
{
	if (id<0) {
		return nullptr;
	}
	return workers.at(id);
}


template <class EntityType>
void sim_mob::SimpleWorkGroup<EntityType>::wait()
{
	if (--tickOffset>0) {
		return;
	}
	tickOffset = tickStep;

	//Stay in sync with the workers.
	nextTimeTickToStage += tickStep;

	shared_barr.wait();

	//Stage Agent updates based on nextTimeTickToStage
	stageAgents();

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







