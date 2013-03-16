/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <iostream>
#include <queue>
#include <vector>
#include <stdexcept>
#include <boost/thread.hpp>
#include <string>

#include "conf/settings/DisableMPI.h"

#include "util/LangHelpers.hpp"
#include "util/DebugFlags.hpp"
#include "util/FlexiBarrier.hpp"

//Needed for ActionFunction
#include "workers/Worker.hpp"


namespace sim_mob
{

class StartTimePriorityQueue;
class EventTimePriorityQueue;
class Agent;
class PartitionManager;
class AuraManager;
class Conflux;

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

	/**
	 * Create a new WorkGroup and start tracking it. All WorkGroups must be created using this method so that their
	 *   various synchronization barriers operate globally. Note that if both auraMgr and partitionMgr are null,
	 *   this WorkGroup will *still* have an AuraManager barrier phase, *unless* all other WorkGroups are also missing these
	 *   parameters. This requirement may be dropped later (if it can be shown that the AuraMaanger never accesses variables
	 *   that might conflict with the frame phase of another WorkGroup.)
	 *
	 * \param numWorkers The number of workers in this WorkGroup. Each Worker will reserve a single wait() on the barrier.
	 * \param numSimTicks The number of ticks in this simulation. The last tick which fires is numSimTicks-1
	 * \param tickStep The granularity of this WorkGroup. If the tickStep is 5, this WorkGroup only advances once every 5 base ticks.
	 * \param auraMgr The aura manager. Will be updated during the AuraManager barrier wait if it exists.
	 * \param partitionMgr The partition manager. Will be updated during the AuraManager barrier wait (but *before*) the AuraMaanger if it exists.
	 */
	static sim_mob::WorkGroup* NewWorkGroup(unsigned int numWorkers, unsigned int numSimTicks=0, unsigned int tickStep=1, sim_mob::AuraManager* auraMgr=nullptr, sim_mob::PartitionManager* partitionMgr=nullptr);

	///Initialize all WorkGroups. Before this function is called, WorkGroups cannot have Workers added to them. After this function is
	///  called, no new WorkGroups may be added.
	static void InitAllGroups();

	///Helper: Calls "startAll()" on all registered WorkGroups;
	static void StartAllWorkGroups();

	///Call the various wait* functions for all WorkGroups in the correct order.
	static void WaitAllGroups();

	//Call the various wait* functions individually.
	static void WaitAllGroups_FrameTick();     ///< Wait on barriers: 1. You should use WaitAllGroups unless you really need fine-grained control.
	static void WaitAllGroups_FlipBuffers();   ///< Wait on barriers: 2. You should use WaitAllGroups unless you really need fine-grained control.
	static void WaitAllGroups_AuraManager();   ///< Wait on barriers: 3. You should use WaitAllGroups unless you really need fine-grained control.
	static void WaitAllGroups_MacroTimeTick(); ///< Wait on barriers: 4. You should use WaitAllGroups unless you really need fine-grained control.

	///Call when the simulation is done. This deletes all WorkGroups (after joining them) and resets
	///  for the next simulation.
	static void FinalizeAllWorkGroups();
	void clear();


private: //Static fields

	//For holding the set of known WorkGroups
	static std::vector<sim_mob::WorkGroup*> RegisteredWorkGroups;

	//The current barrier count for the main three barriers (frame, flip, aura), +1 for the static WorkGroup itself.
	//  Note: Compared to previous implementations, each WorkGroup does NOT add 1 to the barrier count.
	static unsigned int CurrBarrierCount;

	//True if we need an AuraManager barrier at all.
	static bool AuraBarrierNeeded;

	//Our shared barriers for the main three barriers (macro barriers are handled internal to each WorkGroup).
	//Only the aura manager barrier may be null. If any other barrier is null, it means we haven't called Init() yet.
	static sim_mob::FlexiBarrier* FrameTickBarr;
	static sim_mob::FlexiBarrier* BuffFlipBarr;
	static sim_mob::FlexiBarrier* AuraMgrBarr;


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

	void startAll();
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
	void waitFrameTick();
	void waitFlipBuffers();
	void waitMacroTimeTick();
	void waitAuraManager();

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
