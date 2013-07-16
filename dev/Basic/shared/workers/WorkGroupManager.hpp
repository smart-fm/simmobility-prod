/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>

#include "util/LangHelpers.hpp"
#include "util/FlexiBarrier.hpp"
#include "util/StateSwitcher.hpp"


namespace sim_mob {

class PartitionManager;
class AuraManager;
class WorkGroup;



/*
 * A WorkGroupManager is used to coordinate all access to WorkGroups (since they have specific usage requirements).
 * You should use this class to create, advance, and manage WorkGroups.
 *
 * \author Seth N. Hetu
 */
class WorkGroupManager {
public:
	WorkGroupManager() :
		currBarrierCount(1), auraBarrierNeeded(false), frameTickBarr(nullptr), buffFlipBarr(nullptr), auraMgrBarr(nullptr),
		singleThreaded(false), currState(INIT)
	{}

	~WorkGroupManager();

	///Set "single-threaded" mode on all WorkGroups. This must be done *before* initAllGroups/startAllGroups (not sure which one yet).
	void setSingleThreadMode(bool enable);

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
	sim_mob::WorkGroup* newWorkGroup(unsigned int numWorkers, unsigned int numSimTicks=0, unsigned int tickStep=1, sim_mob::AuraManager* auraMgr=nullptr, sim_mob::PartitionManager* partitionMgr=nullptr);

	///Initialize all WorkGroups. Before this function is called, WorkGroups cannot have Workers added to them. After this function is
	///  called, no new WorkGroups may be added.
	void initAllGroups();

	///Helper: Calls "startAll()" on all registered WorkGroups;
	void startAllWorkGroups();

	///Call the various wait* functions for all WorkGroups in the correct order.
	void waitAllGroups();

	//Call the various wait* functions individually.
	void waitAllGroups_FrameTick();     ///< Wait on barriers: 1. You should use WaitAllGroups unless you really need fine-grained control.
	void waitAllGroups_FlipBuffers();   ///< Wait on barriers: 2. You should use WaitAllGroups unless you really need fine-grained control.
	void waitAllGroups_AuraManager();   ///< Wait on barriers: 3. You should use WaitAllGroups unless you really need fine-grained control.
	void waitAllGroups_MacroTimeTick(); ///< Wait on barriers: 4. You should use WaitAllGroups unless you really need fine-grained control.

	// providing read only access to public for RegisteredWorkGroups. AuraManager requires this. - Harish
	const std::vector<sim_mob::WorkGroup*> getRegisteredWorkGroups();

private:
	//WorkGroup management proceeds like a state machine. At each point, only a set number of actions can be performed (usually 1).
	enum STATE {
		INIT,     ///< The Manager has just been created.
		CREATE,   ///< The Manager is in the process of creating WorkGroups (at least one exists).
		BARRIERS, ///< The Manager has just initialized all barriers (and informed the WorkGroups).
		STARTED,  ///< The Manager has informed WorkGroups to start their Workers.
	};

	//The current state of the Management life cycle.
	//TODO: StateSwitcher class with "test()" and "set()" methods. The second always returns true; e.g.,
	// if (currState.test(x) && currState.set(x)) {}
	sim_mob::StateSwitcher<STATE> currState;

	//For holding the set of known WorkGroups
	std::vector<sim_mob::WorkGroup*> registeredWorkGroups;

	//The current barrier count for the main three barriers (frame, flip, aura), +1 for the static WorkGroup itself.
	//  Note: Compared to previous implementations, each WorkGroup does NOT add 1 to the barrier count.
	unsigned int currBarrierCount;

	//True if we need an AuraManager barrier at all.
	bool auraBarrierNeeded;

	//Are we operating in "single-threaded" mode? Default is false.
	bool singleThreaded;

	//Our shared barriers for the main three barriers (macro barriers are handled internal to each WorkGroup).
	//Only the aura manager barrier may be null. If any other barrier is null, it means we haven't called Init() yet.
	sim_mob::FlexiBarrier* frameTickBarr;
	sim_mob::FlexiBarrier* buffFlipBarr;
	sim_mob::FlexiBarrier* auraMgrBarr;

};

}
