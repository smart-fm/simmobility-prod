/* Copyright Singapore-MIT Alliance for Research and Technology */

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
 *
 * \todo
 * Need to re-write, combine this with EntityWorker. Basically, the AddEntity function
 * should be templatized with void* or Entity*, instead of having 2 classes.
 */

#pragma once
#include <iostream>

#include <vector>
#include <set>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/unordered_map.hpp>
#include "util/FlexiBarrier.hpp"

#include "GenConfig.h"

#include "entities/Entity.hpp"

#include "metrics/Frame.hpp"
#include "util/LangHelpers.hpp"
#include "buffering/Buffered.hpp"
#include "buffering/BufferedDataManager.hpp"
#include "geospatial/Link.hpp"
#include "entities/conflux/SegmentStats.hpp"

namespace sim_mob
{

class WorkGroup;
class Conflux;

class Worker : public BufferedDataManager {
public:
	/**
	 * Create a Worker object.
	 *
	 * \param tickStep How many ticks to advance per update(). It is beneficial to have one WorkGroup where
	 *        this value is 1, since any WorkGroup with a greater value will have to wait 2 times (due to
	 *        the way we synchronize data).
	 *
	 */
	Worker(WorkGroup* parent, sim_mob::FlexiBarrier* frame_tick, sim_mob::FlexiBarrier* buff_flip, sim_mob::FlexiBarrier* aura_mgr, boost::barrier* macro_tick, std::vector<Entity*>* entityRemovalList, uint32_t endTick, uint32_t tickStep);

	virtual ~Worker();

	//Thread-style operations
	void start();
	void interrupt();  ///<Note: I am not sure how this will work with multiple granularities. ~Seth
	void join();

	//Manage entities
	void addEntity(Entity* entity);
	void remEntity(Entity* entity);
	const std::vector<Entity*>& getEntities()const ;


	//
	//NOTE: Allowing a Worker or any Agent to access the current Work Group is extremely
	//      dangerous. Please see the note in BusController.hpp; it works for now, but
	//      risks introducing hard-to-debug errors later. ~Seth
	//
	WorkGroup* const getParent() { return parent; }

	//Manage Links
	void addLink(Link* link);
	void remLink(Link* link);
	bool isLinkManaged(Link* link);
	bool isThisLinkManaged(unsigned int linkID);

	void scheduleForAddition(Entity* entity);
	void scheduleForRemoval(Entity* entity);

	int getAgentSize() { return managedEntities.size(); }

protected:
	virtual void perform_main(timeslice currTime);
	virtual void perform_flip();
	virtual void perform_handover();


private:
	void barrier_mgmt();

	void migrateOut(Entity& ent);
	void migrateIn(Entity& ent);

	void migrateAllOut();


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

	//For migration. The first array is accessed by WorkGroup in the flip() phase, and should be
	//   emptied by this worker at the beginning of the update() phase.
	//   The second array is accessed by Agents (rather, the *action function) in update() and should
	//   be cleared by this worker some time before the next update. For now we clear it right after
	//   update(), but it might make sense to clear directly before update(), so that the WorkGroup
	//   has the ability to schedule Agents for deletion in flip().
	std::vector<Entity*> toBeAdded;
	std::vector<Entity*> toBeRemoved;

private:
	//Helper methods
	void addPendingEntities();
	void removePendingEntities();

	///The main thread which this Worker wraps
	boost::thread main_thread;

	///Entities managed by this worker
	std::vector<Entity*> managedEntities;
	std::vector<Link*> managedLinks;
	std::set<Conflux*> managedConfluxes;

	//add by xuyan, in order to call migrate in and migrate out
public:
	friend class WorkGroup;

public:
	std::stringstream debugMsg; //TODO: Delete
};

}


