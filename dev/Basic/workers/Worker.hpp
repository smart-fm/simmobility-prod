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

#include "frame.hpp"
#include "constants.h"
#include "entities/Entity.hpp"
#include "buffering/Buffered.hpp"
#include "buffering/BufferedDataManager.hpp"


namespace sim_mob
{


template <class EntityType>
class Worker : public BufferedDataManager {
public:
	//! The function type for the 1st parameter to the Worker constructor.
	//!
	//! Any procedure that takes a Worker object and an unsigned integer can be used
	//! to construct a Worker object.  This procedure will be called repeatedly; the 1st
	//! argument will a reference to the constructed Worker object and the 2nd argument
	//! will be a strictly monotonic increasing number which represent the time-step.
	typedef boost::function<void(Worker<EntityType>& worker, frame_t frameNumber)> actionFunction;
	Worker(actionFunction* action =nullptr, boost::barrier* internal_barr =nullptr, boost::barrier* external_barr =nullptr, unsigned int endTick=0, bool auraManagerActive=false);
	virtual ~Worker();

	//Thread-style operations
	void start();
	void interrupt();
	void join();

	//Manage entities
	void addEntity(EntityType* entity);
	void remEntity(EntityType* entity);
	std::vector<EntityType*>& getEntities();


protected:
	virtual void perform_main(frame_t frameNumber);
	virtual void perform_flip();


private:
	void barrier_mgmt();


protected:
	//Properties
	boost::barrier* internal_barr;
	boost::barrier* external_barr;
	actionFunction* action;

	//Time management
	frame_t currTick;
	frame_t endTick;

	bool auraManagerActive;


public:
	sim_mob::Buffered<bool> active;

private:
	//Thread management
	boost::thread main_thread;

	//Object management
	std::vector<EntityType*> data;
};

}


//////////////////////////////////////////////
// Template implementation
//////////////////////////////////////////////



template <class EntityType>
void sim_mob::Worker<EntityType>::addEntity(EntityType* entity)
{
	//Save this entity in the data vector.
	data.push_back(entity);
}

template <class EntityType>
void sim_mob::Worker<EntityType>::remEntity(EntityType* entity)
{
	//Remove this entity from the data vector.
	typename std::vector<EntityType*>::iterator it = std::find(data.begin(), data.end(), entity);
	if (it!=data.end()) {
		data.erase(it);
	}
}

template <class EntityType>
std::vector<EntityType*>& sim_mob::Worker<EntityType>::getEntities() {
	return data;
}



//////////////////////////////////////////////
// These also need the temoplate parameter,
// but don't actually do anythign with it.
//////////////////////////////////////////////


template <class EntityType>
sim_mob::Worker<EntityType>::Worker(actionFunction* action, boost::barrier* internal_barr, boost::barrier* external_barr, unsigned int endTick, bool auraManagerActive)
    : BufferedDataManager(),
      internal_barr(internal_barr), external_barr(external_barr), action(action),
      endTick(endTick),
      auraManagerActive(auraManagerActive),
      active(/*this, */false)  //Passing the "this" pointer is probably ok, since we only use the base class (which is constructed)
{
	this->beginManaging(&active);
}

template <class EntityType>
sim_mob::Worker<EntityType>::~Worker()
{
	//Clear all tracked entitites
	while (!data.empty()) {
		remEntity(data[0]);
	}

	//Clear all tracked data
	while (!managedData.empty()) {
		stopManaging(managedData[0]);
	}
}

template <class EntityType>
void sim_mob::Worker<EntityType>::start()
{
	active.force(true);
	currTick = 0;
	main_thread = boost::thread(boost::bind(&Worker::barrier_mgmt, this));
}

template <class EntityType>
void sim_mob::Worker<EntityType>::join()
{
	main_thread.join();
}

template <class EntityType>
void sim_mob::Worker<EntityType>::interrupt()
{
	if (main_thread.joinable()) {
		main_thread.interrupt();
	}
}


template <class EntityType>
void sim_mob::Worker<EntityType>::barrier_mgmt()
{
	for (;active.get();) {
		perform_main(currTick);

		if (internal_barr)
			internal_barr->wait();

		//Advance local time-step
		if (endTick>0 && ++currTick>=endTick) {
			this->active.set(false);
		}

		perform_flip();

		if (external_barr)
			external_barr->wait();

        // Wait for the AuraManager
		if (auraManagerActive) {
			if (external_barr)
				external_barr->wait();
		}
	}
}


template <class EntityType>
void sim_mob::Worker<EntityType>::perform_main(frame_t frameNumber)
{
	if (action)
		(*action)(*this, frameNumber);
}

template <class EntityType>
void sim_mob::Worker<EntityType>::perform_flip()
{
	//Flip all data managed by this worker.
	this->flip();
}

