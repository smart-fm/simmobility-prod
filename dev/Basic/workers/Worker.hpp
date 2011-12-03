/* Copyright Singapore-MIT Alliance for Research and Technology */

/**
 * A "worker" performs a task asynchronously.
 *    There are two ways to use a worker:
 *    - Use the default constructor. Call "wait" once. (See: WorkGroup)
 *    - Create it with a non-null barrier. (Again, see: WorkGroup)
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

//#include "WorkGroup.hpp"
//#include "entities/Agent.hpp"
#include "entities/Entity.hpp"

#include "metrics/Frame.hpp"
#include "util/LangHelpers.hpp"
#include "buffering/Buffered.hpp"
#include "buffering/BufferedDataManager.hpp"


namespace sim_mob
{

template <class EntityType>
class SimpleWorkGroup;



template <class EntityType>
class Worker : public BufferedDataManager {
public:
	//! The function type for the 1st parameter to the Worker constructor.
	//!
	//! Any procedure that takes a Worker object and an unsigned integer can be used
	//! to construct a Worker object.  This procedure will be called repeatedly; the 1st
	//! argument will a reference to the constructed Worker object and the 2nd argument
	//! will be a strictly monotonic increasing number which represent the time-step.
	typedef boost::function<void(Worker<EntityType>& worker, frame_t frameNumber)> ActionFunction;
	Worker(SimpleWorkGroup<EntityType>* parent, ActionFunction* action =nullptr, boost::barrier* internal_barr =nullptr, boost::barrier* external_barr =nullptr, frame_t endTick=0, frame_t tickStep=0, bool auraManagerActive=false);
	virtual ~Worker();

	//Thread-style operations
	void start();
	void interrupt();
	void join();

	//Manage entities
	void addEntity(EntityType* entity);
	void remEntity(EntityType* entity);
	std::vector<EntityType*>& getEntities();
	void scheduleForRemoval(EntityType* entity);


protected:
	virtual void perform_main(frame_t frameNumber);
	virtual void perform_flip();


private:
	void barrier_mgmt();


protected:
	//Properties
	boost::barrier* internal_barr;
	boost::barrier* external_barr;
	ActionFunction* action;

	//Time management
	frame_t currTick;
	frame_t endTick;
        frame_t tickStep;

	bool auraManagerActive;

	//Saved
	SimpleWorkGroup<EntityType>* const parent;

	//For migration. The first array is accessed by WorkGroup in the flip() phase, and should be
	//   emptied by this worker at the beginning of the update() phase.
	//   The second array is accessed by Agents (rather, the *action function) in update() and should
	//   be cleared by this worker some time before the next update. For now we clear it right after
	//   update(), but it might make sense to clear directly before update(), so that the WorkGroup
	//   has the ability to schedule Agents for deletion in flip().
	std::vector<EntityType*> toBeAdded;
	std::vector<EntityType*> toBeRemoved;


public:
	sim_mob::Buffered<bool> active;

private:
	//Thread management
	boost::thread main_thread;

	//Object management
	std::vector<EntityType*> data;

	//Entities to remove after this time tick.
	//std::vector<EntityType*> toBeRemoved;
};

}



