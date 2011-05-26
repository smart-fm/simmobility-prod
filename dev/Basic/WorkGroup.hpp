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

#include "workers/Worker.hpp"
#include "simple_classes.h"

class WorkGroup {
public:
	WorkGroup(size_t size);
	~WorkGroup();

	template <class WorkType>
	Worker& initWorker(boost::function<void(Worker*)> action);

	Worker& getWorker(size_t id);
	void interrupt();
	size_t size();

	void wait();

	//TODO: Move this to the Worker, not the work group.
	void migrate(Agent* ag, int fromID, int toID);

private:
	bool allWorkersUsed();

private:
	//Shared barrier
	boost::barrier shared_barr;
	boost::barrier external_barr;

	//Worker object management
	size_t totalWorkers;
	Worker** workers;
	size_t currID;

};


