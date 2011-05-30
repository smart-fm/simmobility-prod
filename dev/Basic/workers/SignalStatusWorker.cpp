#include "SignalStatusWorker.hpp"

#include <vector>
#include <boost/thread.hpp>

#include "../entities/Region.hpp"

using std::vector;
using boost::function;
using boost::barrier;

SignalStatusWorker::SignalStatusWorker(function<void(Worker*)>* action, barrier* internal_barr, barrier* external_barr)
    : Worker(action, internal_barr, external_barr)
{
}


//TODO: Storing a "localTimestep" is probably better done in the "worker" class itself.
void SignalStatusWorker::start()
{
	localTimestep = 0;
	Worker::start();
}

void SignalStatusWorker::setSimulationEnd(unsigned int time)
{
	simulationEnd = time;
}


/**
 * Update all regions that this Worker controls.
 */
void SignalStatusWorker::perform_main()
{
	for (vector<void*>::iterator it=getEntities().begin(); it!=getEntities().end(); it++) {
		//TODO: Either use templates or make a base class of "entities" (items with flip-able properties)
		//TODO: Use a dynamic_cast if there's a common base object.
		Region* rg = (Region*)(*it);
		rg->update();
	}

	//Advance local time-step
	if (++localTimestep>=simulationEnd) {
		this->active.set(false);
	}
}


/**
 * Flip the memory used to store each agent's properties.
 */
void SignalStatusWorker::perform_flip()
{
	Worker::perform_flip();
}
