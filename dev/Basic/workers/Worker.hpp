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

#include "../entities/Entity.hpp"
#include "../buffering/Buffered.hpp"
#include "../buffering/BufferedDataManager.hpp"


namespace sim_mob
{


template <class EntityType>
class Worker : public BufferedDataManager {
public:
	Worker(boost::function<void(sim_mob::Worker<EntityType>*)>* action =NULL, boost::barrier* internal_barr =NULL, boost::barrier* external_barr =NULL, unsigned int endTick=0);

	//Thread-style operations
	void start();
	void interrupt();
	void join();

	//Manage entities
	void addEntity(EntityType* entity);
	void remEntity(EntityType* entity);
	std::vector<EntityType*>& getEntities();


protected:
	virtual void perform_main();
	virtual void perform_flip();


private:
	void barrier_mgmt();


protected:
	//Properties
	boost::barrier* internal_barr;
	boost::barrier* external_barr;
	boost::function<void(Worker<EntityType>*)>* action;

	//Time management
	unsigned int currTick;
	unsigned int endTick;


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
	data.push_back(entity);
}

template <class EntityType>
void sim_mob::Worker<EntityType>::remEntity(EntityType* entity)
{
	typename std::vector<EntityType*>::iterator it = std::find(data.begin(), data.end(), entity);
	if (it!=data.end())
		data.erase(it);
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
sim_mob::Worker<EntityType>::Worker(boost::function<void(sim_mob::Worker<EntityType>*)>* action, boost::barrier* internal_barr, boost::barrier* external_barr, unsigned int endTick)
    : BufferedDataManager(),
      internal_barr(internal_barr), external_barr(external_barr), action(action),
      endTick(endTick),
      active(this, false)  //Passing the "this" pointer is probably ok, since we only use the base class (which is constructed)
{
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
		perform_main();

		if (internal_barr!=NULL)
			internal_barr->wait();

		//Advance local time-step
		if (endTick>0 && ++currTick>=endTick) {
			this->active.set(false);
		}

		perform_flip();

		if (external_barr!=NULL)
			external_barr->wait();
	}
}


template <class EntityType>
void sim_mob::Worker<EntityType>::perform_main()
{
	if (action!=NULL)
		(*action)(this);
}

template <class EntityType>
void sim_mob::Worker<EntityType>::perform_flip()
{
	//Flip all data managed by this worker.
	this->flip();
}

