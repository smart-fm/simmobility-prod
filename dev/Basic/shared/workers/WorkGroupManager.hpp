//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <list>
#include <set>
#include <string>
#include <vector>
#include "entities/Entity.hpp"
#include "entities/PersonLoader.hpp"
#include "util/FlexiBarrier.hpp"
#include "util/LangHelpers.hpp"
#include "util/StateSwitcher.hpp"
#include "util/Utils.hpp"

namespace sim_mob
{

class PartitionManager;
class AuraManager;
class WorkGroup;

/**
 * A WorkGroupManager is used to coordinate all access to WorkGroups (since they have specific usage requirements).
 * This class should be used to create, advance, and manage WorkGroups.
 *
 * \author Seth N. Hetu
 */
class WorkGroupManager
{
public:
	WorkGroupManager() : currBarrierCount(1), frameTickBarr(nullptr), buffFlipBarr(nullptr), msgBusBarr(nullptr), singleThreaded(false), currState(INIT)
	{
	}

	~WorkGroupManager();

	/**
	 * Set "single-threaded" mode on all WorkGroups.
	 * This must be done *before* initAllGroups/startAllGroups (not sure which one yet).
	 */
	void setSingleThreadMode(bool enable);

	/**
	 * Retrieve a list of (output) file names
	 * @return list of output file names
	 */
	std::list<std::string> retrieveOutFileNames() const;

	/**
	 * Create a new WorkGroup and start tracking it. All WorkGroups must be created using this method so that their
	 * various synchronization barriers operate globally. Note that if both auraMgr and partitionMgr are null, this
	 * WorkGroup will *still* have an AuraManager barrier phase, *unless* all other WorkGroups are also missing these
	 * parameters. This requirement may be dropped later (if it can be shown that the AuraMaanger never accesses variables
	 * that might conflict with the frame phase of another WorkGroup.)
	 *
	 * @param numWorkers The number of workers in this WorkGroup. Each Worker will reserve a single wait() on the barrier.
	 * @param numSimTicks The number of ticks in this simulation. The last tick which fires is numSimTicks-1
	 * @param tickStep The granularity of this WorkGroup. If the tickStep is 5, this WorkGroup only advances once every 5 base ticks.
	 * @param auraMgr The aura manager. Will be updated during the AuraManager barrier wait if it exists.
	 * @param partitionMgr The partition manager. Will be updated during the AuraManager barrier wait (but *before*) the AuraMaanger if it exists.
	 *
	 * @return created work group
	 */
	sim_mob::WorkGroup* newWorkGroup(unsigned int numWorkers, unsigned int numSimTicks = 0, unsigned int tickStep = 1, sim_mob::AuraManager* auraMgr = nullptr,
			sim_mob::PartitionManager* partitionMgr = nullptr, sim_mob::PeriodicPersonLoader* periodicLoader = nullptr);

	/**
	 * Initialize all WorkGroups.
	 * Before this function is called, WorkGroups cannot have Workers added to them.
	 * After this function is called, no new WorkGroups may be added.
	 */
	void initAllGroups();

	/** Helper: Calls "startAll()" on all registered WorkGroups; */
	void startAllWorkGroups();

	/** Call the various wait* functions for all WorkGroups in the correct order. */
	void waitAllGroups();

	/** wait on frame_tick barrier */
	void waitAllGroups_FrameTick();

	/**
	 * wait on flip buffers barrier
	 * @param removedEntities set of entities to be removed
	 */
	void waitAllGroups_FlipBuffers(std::set<Entity*>* removedEntities);

	/**
	 * wait on distribute message barrier
	 * @param removedEntities set of entities to be removed
	 */
	void waitAllGroups_DistributeMessages(std::set<Entity*>& removedEntities);

	/**
	 * wait on macro tick barrier
	 */
	void waitAllGroups_MacroTimeTick();

private:
	/**
	 * WorkGroup management proceeds like a state machine. At each point, only a set number of (usually 1) actions can be performed
	 */
	enum STATE
	{
		INIT,     ///< The Manager has just been created.
		CREATE,   ///< The Manager is in the process of creating WorkGroups (at least one exists).
		BARRIERS, ///< The Manager has just initialized all barriers (and informed the WorkGroups).
		STARTED,  ///< The Manager has informed WorkGroups to start their Workers.
	};

	/**
	 * The current state of the Management life cycle.
	 * TODO: StateSwitcher class with "test()" and "set()" methods. The second always returns true; e.g.,
	 if (currState.test(x) && currState.set(x)) {}
	 */
	sim_mob::StateSwitcher<STATE> currState;

	/** container for the set of known WorkGroups */
	std::vector<sim_mob::WorkGroup*> registeredWorkGroups;

	/**
	 * The current barrier count for the main three barriers (frame, flip, aura), +1 for the static WorkGroup itself.
	 * Note: Compared to previous implementations, each WorkGroup does NOT add 1 to the barrier count.
	 */
	unsigned int currBarrierCount;

	/** Are we operating in "single-threaded" mode? Default is false. */
	bool singleThreaded;

	//Our shared barriers for the main three barriers (macro barriers are handled internal to each WorkGroup).
	/** frame tick barrier */
	sim_mob::FlexiBarrier* frameTickBarr;

	/** flip buffer barrier */
	sim_mob::FlexiBarrier* buffFlipBarr;

	/** message bus barrier */
	sim_mob::FlexiBarrier* msgBusBarr;
};

} // namespace sim_mob
