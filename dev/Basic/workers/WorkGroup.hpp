/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <queue>
#include <vector>
#include <stdexcept>
#include <boost/thread.hpp>

#include "constants.h"

#include "util/LangHelpers.hpp"
#include "util/DebugFlags.hpp"

//Needed for ActionFunction
#include "workers/Worker.hpp"


namespace sim_mob
{

class StartTimePriorityQueue;


/*
 * Worker wrapper, similar to thread_group but using barriers.
 * A SimpleWorkGroup provides a convenient wrapper for Workers, similarly to
 *   how a ThreadGroup manages threads. The main difference is that the number of
 *   worker threads cannot be changed once the object has been constructed.
 * A SimpleWorkGroup maintains one extra hold on the shared barrier; to "advance"
 *   a group, call SimpleWorkGroup::wait().
 */
class WorkGroup {
public:
	//Migration parameters
	struct EntityLoadParams {
		StartTimePriorityQueue& pending_source;
		std::vector<Entity*>& entity_dest;
		boost::mutex& entity_dest_lock;
		EntityLoadParams(StartTimePriorityQueue& pending_source, std::vector<Entity*>& entity_dest, boost::mutex& entity_dest_lock)
			: pending_source(pending_source), entity_dest(entity_dest), entity_dest_lock(entity_dest_lock) {}
	};


	//These are passed along to the Workers:
	//  endTick=0 means run forever.
	//  tickStep is used to allow Workers to skip ticks; no barriers are locked.
	WorkGroup(size_t size, unsigned int endTick=0, unsigned int tickStep=1, bool auraManagerActive=false);

	virtual ~WorkGroup();

	//template <typename WorkType>  //For now, just assume Workers
	void initWorkers(Worker::ActionFunction* action, EntityLoadParams* loader);

	//Worker<EntityType>* const getWorker(size_t id);
	void startAll();
	void interrupt();
	size_t size();

	void wait();
	void waitExternAgain();
	//void migrate(EntityType& ag, Worker<EntityType>* from, Worker<EntityType>* to);

	Worker* getWorker(int id);

#ifndef DISABLE_DYNAMIC_DISPATCH
	void stageEntities();
	std::vector<Entity*> entToBeRemoved;
#endif

	void assignAWorker(Entity* ag);

	void scheduleEntForRemoval(Entity* ag);

//add by xuyan
public:
	//void removeAgentFromWorker(Entity * ag);
	//void addAgentInWorker(Entity * ag);

	//std::map<Entity*, int> agentMapping;
	int getTheMostFreeWorkerID();


protected:
	//Shared barrier
	boost::barrier shared_barr;
	boost::barrier external_barr;

	//Worker object management
	std::vector<Worker*> workers;

	//Used to coordinate which Worker gets the next Agent; currently round-robin.
	size_t nextWorkerID;

	//Passed along to Workers
	unsigned int endTick;
	unsigned int tickStep;

	//Maintain an offset. When it reaches zero, reset to tickStep and sync barriers
	unsigned int tickOffset;

	//Only used once
	size_t total_size;

	//Additional locking is required if the aura manager is active.
	bool auraManagerActive;

	//Needed to stay in sync with the workers
	frame_t nextTimeTickToStage;

	//Contains information needed to migrate Entities
	EntityLoadParams* loader;





};


}
