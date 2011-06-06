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
#include "entities/Entity.hpp"


class WorkGroup {
public:
	WorkGroup(size_t size);
	~WorkGroup();

	template <class WorkType>
	void initWorkers(boost::function<void(Worker*)>* action=NULL);

	Worker& getWorker(size_t id);
	void startAll();
	//void joinAll();
	void interrupt();
	size_t size();

	void wait();

	//TODO: Move this to the Worker, not the work group.
	void migrate(void * ag, int fromID, int toID);

private:
	//bool allWorkersUsed();

private:
	//Shared barrier
	boost::barrier shared_barr;
	boost::barrier external_barr;

	//Worker object management
	std::vector<Worker*> workers;

	//Only used once
	size_t total_size;

};


/**
 * Template function must be defined in the same translational unit as it is declared.
 */
template <class WorkType>
void WorkGroup::initWorkers(boost::function<void(Worker*)>* action)
{
	for (size_t i=0; i<total_size; i++) {
		workers.push_back(new WorkType(action, &shared_barr, &external_barr));
	}
}



