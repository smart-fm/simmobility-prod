//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "conf/settings/DisableMPI.h"

#include <set>
#include <list>
#include <queue>
#include <vector>
#include <stdexcept>
#include <boost/thread.hpp>
#include <string>

#include "util/LangHelpers.hpp"
#include "util/DebugFlags.hpp"
#include "util/FlexiBarrier.hpp"
#include "logging/Log.hpp"


namespace sim_mob {

class ProfileBuilder;
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
 * A Worker wrapper which uses barriers to synchronize between all Workers of the same group.
 * WorkGroups are managed by the WorkGroupManager; this includes creation/destruction/advancement.
 *   The only function you need to call at the WorkGroup level is assignAWorker().
 * A WorkGroup maintains one extra hold on the shared barrier.
 *
 * \author Seth N. Hetu
 * \author Xu Yan
 */
class WorkGroup {
private:
	friend class WorkGroupManager;

	//Private constructor: Use the static NewWorkGroup function instead.
	WorkGroup(unsigned int wgNum, unsigned int numWorkers, unsigned int numSimTicks, unsigned int tickStep, sim_mob::AuraManager* auraMgr, sim_mob::PartitionManager* partitionMgr);

	//Helper method; find the least congested worker (leas number of Agents). O(n), so be careful.
	static sim_mob::Worker* GetLeastCongestedWorker(const std::vector<sim_mob::Worker*>& workers);

public:
	/**
	 * Type of Worker assignment strategy. Determines how a newly-dispatched Agent
	 * will be distributed among the various Worker threads.
	 */
	enum ASSIGNMENT_STRATEGY {
		ASSIGN_ROUNDROBIN,  ///< Assign an Agent to Worker 1, then Worker 2, etc.
		ASSIGN_SMALLEST,    ///< Assign an Agent to the Worker with the smallest number of Agents.
		//TODO: Something like "ASSIGN_TIMEBASED", based on actual time tick length.
	};

	//Migration parameters
	struct EntityLoadParams {
		StartTimePriorityQueue& pending_source;
		std::set<Entity*>& entity_dest;
		EntityLoadParams(StartTimePriorityQueue& pending_source, std::set<Entity*>& entity_dest)
					: pending_source(pending_source), entity_dest(entity_dest) {}
	};



public:
	virtual ~WorkGroup();

	void initWorkers(EntityLoadParams* loader);
	void assignAWorker(Entity* ag);
	void assignConfluxToWorkers();
	void putAgentOnConflux(Agent* ag);
	void processVirtualQueues();
	void outputSupplyStats();
	/*
	 * This function will check for
	 * 1. Confluxes that have upstream Confluxes assigned to a different worker
	 * 2. Confluxes that receive persons from multiple workers. i.e. a conflux
	 * with immediate upstream confluxes that belong to multiple workers. This condition will occur only
	 * when there are three or more person workers
	 *
	 * If the conditions are met, it will set isBoundary and isMultipleReceiver for the respective confluxes
	 */
	void findBoundaryConfluxes();

	unsigned int getNumAgentsWithNoPath() {
		return numAgentsWithNoPath;
	}
        
        unsigned int getNumberOfWorkers() const;
private:
	void clear();
	void interrupt();
	size_t size();

	Worker* getWorker(int id);

	//Schedule an entity. Only call this during the "update" phase.
	void scheduleEntity(Agent* ag);

	void stageEntities();
	void collectRemovedEntities(std::set<sim_mob::Agent*>* removedAgents);
	std::vector< std::vector<Entity*> > entToBeRemovedPerWorker;
	std::vector< std::vector<Entity*> > entToBeBredPerWorker;

	void assignAWorkerConstraint(Entity* ag);

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
	//Add to the list the name of each thread's output file.
	void addOutputFileNames(std::list<std::string>& res) const;


	void startAll(bool singleThreaded);

	//Various stages of "wait"-ing
	//These functions are designed so that you can simply call them one after the other. However,
	// make sure that you call all wait* functions for a given category before moving on.
	//E.g., call "waitFrameTick()" for all WorkGroups, THEN call "waitFlipBuffers()" for all
	// work groups, etc.
	//If "removedAgents" is non-null, any Agents which are removed this time tick are flagged.
	//NOTE: By passing in a non-null "removedAgents", you are taking ownership of these Agents (YOU must delete them).
	void waitFrameTick(bool singleThreaded);
	void waitFlipBuffers(bool singleThreaded, std::set<sim_mob::Agent*>* removedAgents);
	void waitAuraManager(const std::set<sim_mob::Agent*>& removedAgents);
	void waitMacroTimeTick();

	//Initialize our shared (static) barriers. These barriers don't technically need to be copied
	//  locally, but we'd rather avoid relying on static variables in case we ever make a WorkGroupGroup (or whatever) class.
	void initializeBarriers(sim_mob::FlexiBarrier* frame_tick, sim_mob::FlexiBarrier* buff_flip, sim_mob::FlexiBarrier* aura_mgr);

	bool assignConfluxToWorkerRecursive(sim_mob::Conflux* conflux, sim_mob::Worker* worker, int numConfluxesInWorker);

private:
	//The "number" of this WorkGroup. E.g., the first one created is 0, the second is 1, etc. Used ONLY for generating Log files; DON'T use this as an ID.
	unsigned int wgNum;

	//Number of workers we are handling. Should be equal to workers.size() once the workers vector has been initialized.
	unsigned int numWorkers;

	//Passed along to Workers
	unsigned int numSimTicks;
	unsigned int tickStep;

	//Log files that we are managing for Workers in this group.
	std::list<std::ostream*> managed_logs;
	std::list<std::string> logFileNames; //..and their names.

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

	//Profile
	sim_mob::ProfileBuilder* profile;

	//Number of agents who have no path and hence discarded from the simulation.
	unsigned int numAgentsWithNoPath;
};


}
