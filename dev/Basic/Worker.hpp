/*
 * A "worker" performs a task asynchronously.
 *    There are two ways to use a worker:
 *    1) Use the default constructor. Call "join" when you need its result. (NOTE: This behavior is currently disabled.)
 *    2) Create it with a non-null barrier. Best used with a "WorkGroup"
 * To customize the Worker, either subclass it and override "main_loop", or
 *    use a normal Worker and pass in a bindable function in the constructor.
 *    In this case, the "main_loop" will wrap "action" in a "while(active)" loop; calling
 *    "join" will set "active" to false and wait for the thread to terminate.
 */

#pragma once

#include <vector>
#include <boost/thread.hpp>
#include <boost/function.hpp>

class Worker {
public:
	Worker(boost::function<void()>* action =NULL, boost::barrier* barr =NULL);

	//Thread-style operations
	void start();
	void interrupt();
	//void join();

	//Manage entities
	void addEntity(void* entity);
	void remEntity(void* entity);


protected:
	virtual void main_loop();


private:
	//Properties
	boost::barrier* barr;
	boost::function<void()>* action;

	//Thread management
	boost::thread main_thread;
	//bool active;

	//Object management
	std::vector<void*> entities;
};


