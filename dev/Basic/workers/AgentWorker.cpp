#include "AgentWorker.hpp"

using std::vector;
using boost::function;
using boost::barrier;

AgentWorker::AgentWorker(function<void(Worker*)>* action, barrier* internal_barr, barrier* external_barr)
    : Worker(action, internal_barr, external_barr)
{
}


void AgentWorker::start()
{
	localTimestep = 0;
	Worker::start();
}

void AgentWorker::setSimulationEnd(unsigned int time)
{
	simulationEnd = time;
}


/**
 * Update all agents that this Worker controls.
 */
void AgentWorker::perform_main()
{
	for (vector<void*>::iterator it=getEntities().begin(); it!=getEntities().end(); it++) {
		//TODO: Either use templates or make a base class of "entities" (items with flip-able properties)
		//TODO: Use a dynamic_cast if there's a common base object.
		Agent* ag = (Agent*)(*it);
		ag->update();
	}

	//Advance local time-step
	if (++localTimestep>=simulationEnd) {
		this->active.set(false);
	}
}


/**
 * Flip the memory used to store each agent's properties.
 */
void AgentWorker::perform_flip()
{
	Worker::perform_flip();
}
