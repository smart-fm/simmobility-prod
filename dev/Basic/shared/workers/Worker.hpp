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
#include <boost/thread.hpp>
#include <boost/function.hpp>

#include "GenConfig.h"

#include "entities/Entity.hpp"

#include "metrics/Frame.hpp"
#include "util/LangHelpers.hpp"
#include "buffering/Buffered.hpp"
#include "buffering/BufferedDataManager.hpp"
#include "geospatial/Link.hpp"


namespace sim_mob
{

class WorkGroup;


class Worker : public BufferedDataManager {
public:
	//! The function type for the 1st parameter to the Worker constructor.
	//!
	//! Any procedure that takes a Worker object and an unsigned integer can be used
	//! to construct a Worker object.  This procedure will be called repeatedly; the 1st
	//! argument will a reference to the constructed Worker object and the 2nd argument
	//! will be a strictly monotonic increasing number which represent the time-step.
	//typedef boost::function<void(Worker& worker, frame_t frameNumber)> ActionFunction;

	Worker(WorkGroup* parent, boost::barrier& internal_barr, boost::barrier& external_barr, std::vector<Entity*>* entityRemovalList, frame_t endTick, frame_t tickStep, bool auraManagerActive);
	virtual ~Worker();

	//Thread-style operations
	void start();
	void interrupt();
	void join();

	//Manage entities
	void addEntity(Entity* entity);
	void remEntity(Entity* entity);
	std::vector<Entity*>& getEntities();
	WorkGroup* const getParent() { return parent; }

	//Manage Links
	void addLink(Link* link);
	void remLink(Link* link);
	bool isLinkManaged(Link* link);
	bool isThisLinkManaged(std::string linkID);

	void scheduleForAddition(Entity* entity);
	void scheduleForRemoval(Entity* entity);

	int getAgentSize() { return managedEntities.size(); }

protected:
	virtual void perform_main(frame_t frameNumber);
	virtual void perform_flip();


private:
	void barrier_mgmt();

	void migrateOut(Entity& ent);
	void migrateIn(Entity& ent);

	void migrateAllOut();


protected:
	//Properties
	boost::barrier& internal_barr;
	boost::barrier& external_barr;
	//ActionFunction* action;

	//Time management
	//frame_t currTick;
	frame_t endTick;
	frame_t tickStep;

	bool auraManagerActive;

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

	//add by xuyan, in order to call migrate in and migrate out
public:
	friend class WorkGroup;
};

}


