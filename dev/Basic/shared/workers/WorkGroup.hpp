//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/thread.hpp>
#include <list>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include "conf/settings/DisableMPI.h"
#include "entities/PersonLoader.hpp"
#include "util/DebugFlags.hpp"
#include "util/FlexiBarrier.hpp"
#include "util/LangHelpers.hpp"

namespace sim_mob
{

class AuraManager; //TODO: to be removed
class Entity;
class PartitionManager;
class ProfileBuilder;
class RoadSegment;
class StartTimePriorityQueue;
class Worker;
class WorkGroupManager;

/**
 * A Worker wrapper which uses barriers to synchronize between all Workers of the same group.
 * WorkGroups are managed by the WorkGroupManager; this includes creation/destruction/advancement.
 * The only function you need to call at the WorkGroup level is assignAWorker().
 * A WorkGroup maintains one extra hold on the shared barrier.
 *
 * \author Seth N. Hetu
 * \author Harish Loganathan
 */
class WorkGroup
{
private:
	friend class WorkGroupManager;

	/** Private constructor: Use the static newWorkGroup function instead. */
	WorkGroup(unsigned int wgNum, unsigned int numWorkers, unsigned int numSimTicks, unsigned int tickStep, sim_mob::AuraManager* auraMgr,
			sim_mob::PartitionManager* partitionMgr, sim_mob::PeriodicPersonLoader* periodicLoader, uint32_t _simulationStart);

	/**
	 * Helper method; find the least loaded worker (least number of Agents). O(n), so be careful.
	 *
	 * @param workers input set of workers
	 *
	 * @return least loaded worker from workers
	 */
	static sim_mob::Worker* getLeastLoadedWorker(const std::vector<sim_mob::Worker*>& workers);

public:
	/**
	 * Type of Worker assignment strategy. Determines how a newly-dispatched Agent
	 * will be distributed among the various Worker threads.
	 */
	enum ASSIGNMENT_STRATEGY
	{
		ASSIGN_ROUNDROBIN,  ///< Assign an Agent to Worker 1, then Worker 2, etc.
		ASSIGN_SMALLEST,    ///< Assign an Agent to the Worker with the smallest number of Agents.
	//TODO: Something like "ASSIGN_TIMEBASED", based on actual time tick length.
	};

	/** Entity load/migration parameters */
	struct EntityLoadParams
	{
		StartTimePriorityQueue& pending_source;
		std::set<Entity*>& entity_dest;
		EntityLoadParams(StartTimePriorityQueue& pending_source, std::set<Entity*>& entity_dest) :
				pending_source(pending_source), entity_dest(entity_dest)
		{
		}
	};

	virtual ~WorkGroup();

	/**
	 * returns the number of workers in workgroup
	 *
	 * @return number of workers
	 */
	size_t size() const;

	/**
	 * initialize workers in work group
	 *
	 * @param loader entity loaders for work group
	 */
	void initWorkers(EntityLoadParams* loader);

	/**
	 * assigns a suitable worker for entity
	 *
	 * @param ag the entity needing worker assignment
	 */
	void assignAWorker(Entity* ag);

	/**
	 * assigns specified worker for entitiy
	 *
	 * @param ag the entity needing worker assignment
	 * @param workerId index of worker in workers list
	 *
	 * @return true if assignment was successful; false otherwise
	 */
	bool assignWorker(Entity* ag, unsigned int workerId);

	/**
	 * processes multi-update entities
	 *
	 * @param removedEntities input parameter to collect removed entities
	 */
	void processMultiUpdateEntities(std::set<Entity*>& removedEntities);

	/**
	 * adds a loader entity
	 * @param loaderEntity a loader entity to be registered
	 */
	void registerLoaderEntity(Entity* loaderEntity);

	/**
	 * adds a person to a loader Entity for loading in subsequent tick
	 * @param person Person to be loaded
	 */
	void loadPerson(Entity* personEntity);

#ifndef SIMMOB_DISABLE_MPI
	void removeAgentFromWorker(Entity * ag);
	void addAgentInWorker(Entity * ag);
	int getTheMostFreeWorkerID() const;
#endif

private:
	/**
	 * deletes all workers and non-shared barriers
	 */
	void clear();

	/**
	 * interrupts executing of worker
	 * NOTE: not used anywhere at the moment
	 */
	void interrupt();

	/**
	 * gets requested worker
	 *
	 * @param id index of requested worker
	 */
	Worker* getWorker(int id);

	/**
	 * Schedule an entity for addition into the simulation. Only call this during the "update" phase.
	 *
	 * @param ag entity to be scheduled
	 */
	void scheduleEntity(Entity* ag);

	/**
	 * stages entities from pending source into the simulation
	 */
	void stageEntities();

	/**
	 * collects entities removed from simulation
	 * TODO: the usage of this function is fuzzy at the moment. Check whether this is really required
	 *
	 * @param removedAgents input parameter to collect removed entities
	 */
	void collectRemovedEntities(std::set<sim_mob::Entity*>* removedAgents);

	/**
	 * container for entities removed from each worker
	 * Each worker is given 1 mutually exclusive vector from this container
	 */
	std::vector<std::vector<Entity*> > entToBeRemovedPerWorker;

	/**
	 * container for entities bred into each worker
	 * Each worker is given 1 mutually exclusive vector from this container
	 */
	std::vector<std::vector<Entity*> > entToBeBredPerWorker;

	/**
	 * Add to the list the name of each thread's output file.
	 *
	 * @param res input param to add file names to
	 */
	void addOutputFileNames(std::list<std::string>& res) const;

	/**
	 * starts all workers in group
	 *
	 * @param singleThreaded indicates whether the execution must be single threaded
	 */
	void startAll(bool singleThreaded);

	//Various stages of "wait"-ing
	//These functions are designed so that you can simply call them one after the other. However,
	// make sure that you call all wait* functions for a given category before moving on.
	//E.g., call "waitFrameTick()" for all WorkGroups, THEN call "waitFlipBuffers()" for all
	// work groups, etc.
	//If "removedAgents" is non-null, any Agents which are removed this time tick are flagged.
	//NOTE: By passing in a non-null "removedAgents", you are taking ownership of these Agents (YOU must delete them).

	/**
	 * wait on frame tick
	 *
	 * @param singleThreaded indicates whether the execution must be single threaded
	 */
	void waitFrameTick(bool singleThreaded);

	/**
	 * wait on flip buffers
	 *
	 * @param singleThreaded indicates whether the execution must be single threaded
	 * @param removedAgents input parameter to collect removed entities
	 */
	void waitFlipBuffers(bool singleThreaded, std::set<sim_mob::Entity*>* removedAgents);

	/**
	 * wait on distribute messages barrier
	 * TODO: this function must be renamed when AuraMAnager is moved out of WorkGroup
	 *
	 * @param removedAgents input parameter to collect removed entities
	 */
	void waitAuraManager(const std::set<sim_mob::Entity*>& removedAgents);

	/**
	 * wait on macro tick
	 */
	void waitMacroTimeTick();

	/**
	 * Initialize our shared (static) barriers. These barriers don't technically need to be copied
	 * locally, but we'd rather avoid relying on static variables in case we ever make a WorkGroupGroup (or whatever) class.
	 *
	 * @param frame_tick frame tick barrier
	 * @param buff_flip flip buffer barrier
	 * @param msg_bus message bus barrier
	 */
	void initializeBarriers(sim_mob::FlexiBarrier* frame_tick, sim_mob::FlexiBarrier* buff_flip, sim_mob::FlexiBarrier* msg_bus);

	/** The "number" of this WorkGroup. E.g., the first one created is 0, the second is 1, etc. Used ONLY for generating Log files; DON'T use this as an ID. */
	unsigned int wgNum;

	/** Number of workers we are handling. Should be equal to workers.size() once the workers vector has been initialized. */
	unsigned int numWorkers;

	/** Passed along to Workers */
	unsigned int numSimTicks;

	/** granularity of tick in w.r.t the base granularity */
	unsigned int tickStep;

	/** Maintain an offset. When it reaches zero, reset to tickStep and sync barriers */
	unsigned int tickOffset;

	/** Log files that we are managing for Workers in this group. */
	std::list<std::ostream*> managed_logs;

	/** names of Log files that we are managing for Workers in this group. */
	std::list<std::string> logFileNames;

	/**
	 * Aura manager to update, if any (either may be null).
	 * TODO: to be refactored out of work group
	 */
	sim_mob::AuraManager* auraMgr;

	/**
	 * partition manager to update, if any (either may be null).
	 */
	sim_mob::PartitionManager* partitionMgr;

	/** started state indicator for work group */
	bool started;

	//Stays in sync with the Workers' time ticks; tells us which Workers to stage next.
	/** current time tick */
	uint32_t currTimeTick;

	/** next time tick. All entities are staged based on this. */
	uint32_t nextTimeTick;

	/** Container for information needed to migrate Entities */
	EntityLoadParams* loader;

	/**
	 * List of all known workers, along with the ID of the next Worker to receive an agent.
	 * Uses a round-robin approach to select the next Worker.
	 */
	std::vector<Worker*> workers;

	/** worker index used for round-robin selection of worker for agent assignment */
	size_t nextWorkerID;

	/**
	 * Container for any loader entities responsible for loading other entities in-turn.
	 * In mid-term, workers manage conflux entities which in-turn manage person entities.
	 * Each worker is assigned a loader conflux entity which are responsible for
	 * dispatching new person entities to target conflux entities.
	 * These loader confluxes are stored in the vector below.
	 */
	std::vector<Entity*> loaderEntities;

	/**
	 * index used for dispatching new persons to loader confluxes in round robin fashion
	 */
	size_t nextLoaderIdx;

	/** Barrier for frame_tick stage */
	sim_mob::FlexiBarrier* frame_tick_barr;

	/** Barrier for flip buffers stage */
	sim_mob::FlexiBarrier* buff_flip_barr;

	/** Barrier for message distribution stage */
	sim_mob::FlexiBarrier* msg_bus_barr;

	/**
	 * An optional barrier phase unique to each WorkGroup. If the timeStep is >1, then
	 * one additional locking barrier is required to prevent Workers from rushing ahead
	 * into the next time tick. Using a restricted boost::barrier helps to reinforce this.
	 */
	boost::barrier* macro_tick_barr;

	/** Profiler */
	sim_mob::ProfileBuilder* profile;

	/** entity loader to load person entities periodically */
	PeriodicPersonLoader* periodicPersonLoader;

	uint32_t simulationStart;
};

} // namespace sim_mob
