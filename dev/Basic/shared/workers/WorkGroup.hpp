/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "conf/settings/DisableMPI.h"

#include <queue>
#include <vector>
#include <stdexcept>
#include <boost/thread.hpp>
#include <string>

#include "util/LangHelpers.hpp"
#include "util/DebugFlags.hpp"
#include "util/FlexiBarrier.hpp"
#include "logging/Log.hpp"


namespace sim_mob
{

class RoadSegment;
class StartTimePriorityQueue;
class EventTimePriorityQueue;
class Agent;
class Person;
class Entity;
class PartitionManager;
class AuraManager;
class Conflux;
class Worker;
class WorkGroupManager;




/*
 * Worker wrapper, similar to thread_group but using barriers.
 * A SimpleWorkGroup provides a convenient wrapper for Workers, similarly to
 *   how a ThreadGroup manages threads. The main difference is that the number of
 *   worker threads cannot be changed once the object has been constructed.
 * A SimpleWorkGroup maintains one extra hold on the shared barrier; to "advance"
 *   a group, call SimpleWorkGroup::wait().
 *
 * \author Seth N. Hetu
 * \author Xu Yan
 */
class WorkGroup {
public:  //Static methods
	friend class WorkGroupManager;

	/**
	 * Type of Worker assignment strategy. Determines how a newly-dispatched Agent
	 * will be distributed among the various Worker threads.
	 */
	enum ASSIGNMENT_STRATEGY {
		ASSIGN_ROUNDROBIN,  ///< Assign an Agent to Worker 1, then Worker 2, etc.
		ASSIGN_SMALLEST,    ///< Assign an Agent to the Worker with the smallest number of Agents.
		//TODO: Something like "ASSIGN_TIMEBASED", based on actual time tick length.
	};

	void clear();

private:
	//Helper method; find the least congested worker (leas number of Agents). O(n), so be careful.
	static sim_mob::Worker* GetLeastCongestedWorker(const std::vector<sim_mob::Worker*>& workers);


public:

	//Migration parameters
	struct EntityLoadParams {
		StartTimePriorityQueue& pending_source;
		std::vector<Entity*>& entity_dest;
		EntityLoadParams(StartTimePriorityQueue& pending_source, std::vector<Entity*>& entity_dest)
					: pending_source(pending_source), entity_dest(entity_dest) {}
	};


private:
	//Private constructor: Use the static NewWorkGroup function instead.
	WorkGroup(unsigned int numWorkers, unsigned int numSimTicks, unsigned int tickStep, sim_mob::AuraManager* auraMgr, sim_mob::PartitionManager* partitionMgr);

public:
	virtual ~WorkGroup();

	void initWorkers(EntityLoadParams* loader);

private:
	void startAll(bool singleThreaded);

public:
	void interrupt();
	size_t size();

	Worker* getWorker(int id);

	//Schedule an entity. Only call this during the "update" phase.
	void scheduleEntity(Agent* ag);

	void stageEntities();
	void collectRemovedEntities();
	std::vector< std::vector<Entity*> > entToBeRemovedPerWorker;
	std::vector< std::vector<Entity*> > entToBeBredPerWorker;

	void assignAWorker(Entity* ag);

	void assignLinkWorker();

	void assignAWorkerConstraint(Entity* ag);

	void assignConfluxToWorkers();

	void putAgentOnConflux(Agent* ag);

	const sim_mob::RoadSegment* findStartingRoadSegment(Person* p);

	Worker* locateWorker(unsigned int linkID);

	// providing read only access to public for RegisteredWorkGroups. AuraManager requires this. - Harish
	static const std::vector<sim_mob::WorkGroup*> getRegisteredWorkGroups();

//add by xuyan
#ifndef SIMMOB_DISABLE_MPI
public:
	void removeAgentFromWorker(Entity * ag);
	void addAgentInWorker(Entity * ag);
	int getTheMostFreeWorkerID() const;
#endif


private:
	//Various stages of "wait"-ing
	//These functions are designed so that you can simply call them one after the other. However,
	// make sure that you call all wait* functions for a given category before moving on.
	//E.g., call "waitFrameTick()" for all WorkGroups, THEN call "waitFlipBuffers()" for all
	// work groups, etc.
	void waitFrameTick(bool singleThreaded);
	void waitFlipBuffers(bool singleThreaded);
	void waitAuraManager();
	void waitMacroTimeTick();

	//Initialize our shared (static) barriers. These barriers don't technically need to be copied
	//  locally, but we'd rather avoid relying on static variables in case we ever make a WorkGroupGroup (or whatever) class.
	void initializeBarriers(sim_mob::FlexiBarrier* frame_tick, sim_mob::FlexiBarrier* buff_flip, sim_mob::FlexiBarrier* aura_mgr);

	bool assignConfluxToWorkerRecursive(sim_mob::Conflux* conflux, sim_mob::Worker* worker, int numConfluxesInWorker);

private:
	//Number of workers we are handling. Should be equal to workers.size() once the workers vector has been initialized.
	unsigned int numWorkers;

	//Passed along to Workers
	unsigned int numSimTicks;
	unsigned int tickStep;

	//Aura manager and partition manager to update, if any (either may be null).
	sim_mob::AuraManager* auraMgr;
	sim_mob::PartitionManager* partitionMgr;

	//Maintain an offset. When it reaches zero, reset to tickStep and sync barriers
	unsigned int tickOffset;
	bool started;

	//Stays in sync with the Workers' time ticks; tells us which Workers to stage next.
	uint32_t currTimeTick;
	uint32_t nextTimeTick;  //All entities are staged based on this.

	//Contains information needed to migrate Entities
	EntityLoadParams* loader;

	//List of all known workers, along with the ID of the next Worker to receive an agent.
	// Uses a round-robin approach to select the next Worker.
	std::vector<Worker*> workers;
	size_t nextWorkerID;

	//Barriers for each locking stage
	sim_mob::FlexiBarrier* frame_tick_barr;
	sim_mob::FlexiBarrier* buff_flip_barr;
	sim_mob::FlexiBarrier* aura_mgr_barr;

	//An optional barrier phase unique to each WorkGroup. If the timeStep is >1, then
	// one additional locking barrier is required to prevent Workers from rushing ahead
	// into the next time tick. Using a restricted boost::barrier helps to reinforce this.
	boost::barrier* macro_tick_barr;

public:
	//Temp
	std::stringstream debugMsg;

};


}
