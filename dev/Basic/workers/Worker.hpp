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


class Worker : public BufferedDataManager {
public:
	Worker(boost::function<void(Worker*)>* action =NULL, boost::barrier* internal_barr =NULL, boost::barrier* external_barr =NULL, unsigned int endTick=0);

	//Thread-style operations
	void start();
	void interrupt();
	void join();

	//Manage entities
	void addEntity(void* entity);
	void remEntity(void* entity);
	std::vector<void*>& getEntities();


protected:
	virtual void perform_main();
	virtual void perform_flip();


private:
	void barrier_mgmt();


protected:
	//Properties
	boost::barrier* internal_barr;
	boost::barrier* external_barr;
	boost::function<void(Worker*)>* action;

	//Time management
	unsigned int currTick;
	unsigned int endTick;


public:
	sim_mob::Buffered<bool> active;

private:
	//Thread management
	boost::thread main_thread;

	//Object management
	std::vector<void*> data;
};


}

