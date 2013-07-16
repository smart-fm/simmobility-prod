/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

/**
 * A "worker" performs a task asynchronously.
 *    There are two ways to use a worker:
 *    - Use the default constructor. Call "wait" once. (See: WorkGroup)
 *    - Create it with a non-null barrier. (Again, see: WorkGroup)
 *
 * \author Seth N. Hetu
 * \author LIM Fung Chai
 * \author Xu Yan
 *
 * To customize the Worker, either subclass it and override "main_loop", or
 * use a normal Worker and pass in a bindable function in the constructor.
 */

#include <vector>
#include <set>

#include <boost/thread.hpp>

#include "buffering/BufferedDataManager.hpp"
#include "event/EventManager.hpp"
#include "metrics/Frame.hpp"

namespace sim_mob {

class FlexiBarrier;
class ProfileBuilder;
class ControlManager;
class WorkGroup;
class Conflux;
class Entity;


class Worker : public BufferedDataManager {
private:
	friend class WorkGroup;

	/**
	 * Create a Worker object.
	 *
	 * \param tickStep How many ticks to advance per update(). It is beneficial to have one WorkGroup where
	 *        this value is 1, since any WorkGroup with a greater value will have to wait 2 times (due to
	 *        the way we synchronize data).
	 */
	Worker(WorkGroup* parent, sim_mob::FlexiBarrier* frame_tick, sim_mob::FlexiBarrier* buff_flip, sim_mob::FlexiBarrier* aura_mgr, boost::barrier* macro_tick, std::vector<Entity*>* entityRemovalList, std::vector<Entity*>* entityBredList, uint32_t endTick, uint32_t tickStep);

	void start();
	void interrupt();  ///<Note: I am not sure how this will work with multiple granularities. ~Seth
	void join();

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

	EventManager& getEventManager();

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
	//   emptied by this worker at the beginning of the update() phase.
	//   The second array is accessed by Agents (rather, the *action function) in update() and should
	//   be cleared by this worker some time before the next update. For now we clear it right after
	//   update(), but it might make sense to clear directly before update(), so that the WorkGroup
	//   has the ability to schedule Agents for deletion in flip().
	std::vector<Entity*> toBeAdded;
	std::vector<Entity*> toBeRemoved;
	std::vector<Entity*> toBeBred;


private:
	///The main thread which this Worker wraps
	boost::thread main_thread;

	///All parameters used by main_thread. Maintained by the object so that we don't need to use boost::coroutines.
	MgmtParams loop_params;

	///Entities managed by this worker
	std::vector<Entity*> managedEntities;
	std::set<Conflux*> managedConfluxes;

	///If non-null, used for profiling.
	sim_mob::ProfileBuilder* profile;
	EventManager eventManager;
};

}


