//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <ostream>
#include <vector>
#include <set>
#include <boost/thread.hpp>
#include "buffering/BufferedDataManager.hpp"
#include "event/EventCollectionMgr.hpp"
#include "metrics/Frame.hpp"

namespace sim_mob {

class FlexiBarrier;
class ProfileBuilder;
class ControlManager;
class WorkGroup;
class Conflux;
class Entity;


/**
 * A "WorkerProvider" is a restrictive parent class of Worker that provides Worker-related
 * functionality to Agents. This prevents the Agent from requiring full access to the Worker.
 * All methods are abstract virtual; the Worker is expected to fill them in.
 *
 * \note
 * See the Worker class for documentation on these functions.
 */
class WorkerProvider : public BufferedDataManager {
public:
	//NOTE: Allowing access to the BufferedDataManager is somewhat risky; we need it for Roles, but we might
	//      want to organize this differently.
	virtual ~WorkerProvider() {}

	virtual std::ostream* getLogFile() const = 0;

	virtual void scheduleForBred(Entity* entity) = 0;

	virtual const std::vector<Entity*>& getEntities() const = 0;

	virtual event::EventCollectionMgr& getEventManager() = 0;
};


/**
 * A "worker" performs a task asynchronously. Workers are managed by WorkGroups, which are
 *   themselves managed by the WorkGroupManager. You usually don't need to deal with
 *   Workers directly.
 *
 * Workers can be run in "singleThreaded" mode (by specifying this parameter to the WorkGroupManager).
 *   This will cause them to use no threads or barriers, and to simply be stepped through one-by-one by
 *   their parent WorkGroups.
 *
 * \author Seth N. Hetu
 * \author LIM Fung Chai
 * \author Xu Yan
 */
class Worker : public WorkerProvider {
private:
	friend class WorkGroup;

	/**
	 * Create a Worker object.
	 *
	 * \param tickStep How many ticks to advance per update(). It is beneficial to have one WorkGroup where
	 *        this value is 1, since any WorkGroup with a greater value will have to wait 2 times (due to
	 *        the way we synchronize data).
	 * \param logFile Pointer to the file (or cout, cerr) that will receive all logging for this Worker's agents.
	 *        Note that this stream should *not* require logging, so any shared ostreams should be on the same
	 *        thread (usually that means on the same worker).
	 */
	Worker(WorkGroup* parent, std::ostream* logFile, sim_mob::FlexiBarrier* frame_tick, sim_mob::FlexiBarrier* buff_flip, sim_mob::FlexiBarrier* aura_mgr, boost::barrier* macro_tick, std::vector<Entity*>* entityRemovalList, std::vector<Entity*>* entityBredList, uint32_t endTick, uint32_t tickStep);

	void start();
	void interrupt();  ///<Note: I am not sure how this will work with multiple granularities. ~Seth
	void join();       ///<Note: This will probably only work if called at the end of the main simulation loop.

	void scheduleForAddition(Entity* entity);
	int getAgentSize(bool includeToBeAdded=false);
	void migrateAllOut();
	bool beginManagingConflux(Conflux* cf); ///<Returns true if the Conflux was inserted; false if it already exists in the managedConfluxes map.

	//End of functions for friend WorkGroup


public:
	virtual ~Worker();

	//Removing entities and scheduling them for removal is allowed (but adding is restricted).
	const std::vector<Entity*>& getEntities() const;
	void remEntity(Entity* entity);
	void scheduleForRemoval(Entity* entity);
	void scheduleForBred(Entity* entity);

	void processVirtualQueues();
	void outputSupplyStats(uint32_t currTick);
	event::EventCollectionMgr& getEventManager();

	virtual std::ostream* getLogFile() const;

protected:
	///Simple struct that holds all of the params used throughout threaded_function_loop().
	struct MgmtParams {
		MgmtParams();

		///Helper: returns true if the "extra" (macro) tick is active.
		bool extraActive(uint32_t endTick) const;


		///TODO: Using ConfigParams here is risky, since unit-tests may not have access to an actual config file.
		///      We might want to remove this later, but most of our simulator relies on ConfigParams anyway, so
		///      this will be a major cleanup effort anyway.

		uint32_t msPerFrame;  ///< How long is each frame tick in ms.
		sim_mob::ControlManager* ctrlMgr;  ///< The Control Manager (if any)
		uint32_t currTick;    ///< The current time tick.
		bool active;          ///< Is the simulation currently active?
	};

	//These functions encapsulate all of the non-initialization, non-barrier behavior of threaded_function_loop().
	//This allows us to call these functions individually if singleThreaded is set to true.
	//Some minor hacking is done via the MgmtParams struct to make this possible.
	void perform_frame_tick();
	void perform_buff_flip();
	//void perform_aura_mgr();  //Would do nothing (by chance).
	//void perform_macro();     //Would do nothing (by definition).

private:
	//The function that forms the basis of each Worker thread.
	void threaded_function_loop();

	//Helper functions for various update functionality.
	virtual void update_entities(timeslice currTime);

	void migrateOut(Entity& ent);
	void migrateOutConflux(Conflux& cfx);
	void migrateIn(Entity& ent);

	//Entity management. Adding is restricted (use WorkGroups).
	void addEntity(Entity* entity);

	//Helper methods
	void addPendingEntities();
	void removePendingEntities();
	void breedPendingEntities();


protected:
	//Our various barriers.
	sim_mob::FlexiBarrier* frame_tick_barr;
	sim_mob::FlexiBarrier* buff_flip_barr;
	sim_mob::FlexiBarrier* aura_mgr_barr;
	boost::barrier* macro_tick_barr;

	//Time management
	uint32_t endTick;
	uint32_t tickStep;

	//Saved
	WorkGroup* const parent;

	//Assigned by the parent.
	std::vector<Entity*>* entityRemovalList;
	std::vector<Entity*>* entityBredList;

	//For migration. The first array is accessed by WorkGroup in the flip() phase, and should be
	//  emptied by this worker at the beginning of the update() phase.
	//  The second array is accessed by Agents (rather, the *action function) in update() and should
	//  be cleared by this worker some time before the next update. For now we clear it right after
	//  update(), but it might make sense to clear directly before update(), so that the WorkGroup
	//  has the ability to schedule Agents for deletion in flip().
	std::vector<Entity*> toBeAdded;
	std::vector<Entity*> toBeRemoved;
	std::vector<Entity*> toBeBred;


private:
	///Logging
	std::ostream* logFile;

	///The main thread which this Worker wraps
	boost::thread main_thread;

	///All parameters used by main_thread. Maintained by the object so that we don't need to use boost::coroutines.
	MgmtParams loop_params;

	///Entities managed by this worker
	std::vector<Entity*> managedEntities;
	std::set<Conflux*> managedConfluxes;

	///If non-null, used for profiling.
	sim_mob::ProfileBuilder* profile;
	event::EventCollectionMgr eventManager;
};

}


