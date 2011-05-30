#include "ShortestPathWorker.hpp"

using std::vector;
using boost::function;
using boost::barrier;

ShortestPathWorker::ShortestPathWorker(function<void(Worker*)>* action, barrier* internal_barr, barrier* external_barr)
    : EntityWorker(action, internal_barr, external_barr)
{
}


/**
 * Update all entities that this Worker controls.
 */
void ShortestPathWorker::perform_main()
{
	for (vector<Entity*>::iterator it=getEntities().begin(); it!=getEntities().end(); it++) {
		//TODO: Either use templates or make a base class of "entities" (items with flip-able properties)
		//TODO: Use a dynamic_cast if there's a common base object.
		Agent* ag = (Agent*)(*it);
		ag->updateShortestPath();
	}

	//Advance local time-step
	if (++localTimestep>=simulationEnd) {
		this->active.set(false);
	}
}

