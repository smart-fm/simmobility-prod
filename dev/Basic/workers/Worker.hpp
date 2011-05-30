/*
 * A "worker" performs a task asynchronously.
 *    There are two ways to use a worker:
 *    1) Use the default constructor. Call "join" when you need its result. (NOTE: This behavior is currently disabled.)
 *    2) Create it with a non-null barrier. Best used with a "WorkGroup"
 * To customize the Worker, either subclass it and override "main_loop", or
 *
 *    use a normal Worker and pass in a bindable function in the constructor.
 *        ---NOTE: Haven't tested using a bindable function.
 *
 *    In this case, the "main_loop" will wrap "action" in a "while(active)" loop; calling
 *    "join" will set "active" to false and wait for the thread to terminate.
 */

#pragma once

#include <vector>
#include <boost/thread.hpp>
#include <boost/function.hpp>

#include "../entities/Entity.hpp"


//An example of a set-able property
// We will most likely templatize this later, and allow a memory layout within workers
//   for easy flipping.
class BoolProp {
public:
	BoolProp(bool val=false) : val(val) {}
	bool get() {
		return val;
	}
	BoolProp& set(bool val) {
		next = val;
		return *this;
	}
	void flip() {
		val = next;
	}
private:
	bool val;
	bool next;
};


class Worker {
public:
	Worker(boost::function<void(Worker*)>* action =NULL, boost::barrier* internal_barr =NULL, boost::barrier* external_barr =NULL);

	//Thread-style operations
	virtual void start();
	virtual void interrupt();
	//void join();

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

public:
	BoolProp active;

private:
	//Thread management
	boost::thread main_thread;

	//Object management
	std::vector<void*> data;
};


