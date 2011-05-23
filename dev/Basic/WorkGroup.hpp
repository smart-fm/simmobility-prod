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


class WorkGroup {
public:
	WorkGroup(size_t size);

	Worker& getWorker(boost::function& action);
	void wait();
	void interrupt();
	size_t size();

private:
	bool allWorkersUsed();

private:
	//Shared barrier
	boost::barrier shared_barr;

	//Worker object management
	std::vector workers;
	size_t totalWorkers;
	size_t currID;

};


