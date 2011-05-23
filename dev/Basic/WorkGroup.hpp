/*
 * A WorkGroup provides a convenient wrapper for Workers, similarly to
 *   how a ThreadGroup manages threads. The main difference is that the number of
 *   worker threads cannot be changed once the object has been constructed.
 * A WorkGroup maintains one extra hold on the shared barrier; to "advance"
 *   a group, call WorkGroup::wait().
 */

#pragma once

#include <vector>
#include <stdexcept>
#include <boost/thread.hpp>

#include "Worker.hpp"
#include "simple_classes.h"


class WorkGroup {
public:
	WorkGroup(size_t size);
	~WorkGroup();

	Worker& initWorker(boost::function<void(Worker*)> action);
	Worker& getWorker(size_t id);
	void interrupt();
	size_t size();

	void wait();
	//void waitTick();   //We can have a second, "global" wait for each tick.
	//   That way, we can call "wait", then update agent information, then call "waitTick()"

	void migrate(Agent* ag, int fromID, int toID);

private:
	bool allWorkersUsed();

private:
	//Shared barrier
	boost::barrier shared_barr;

	//Worker object management
	size_t totalWorkers;
	Worker** workers;
	size_t currID;

};


