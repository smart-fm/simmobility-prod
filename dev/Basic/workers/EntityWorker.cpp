#include "EntityWorker.hpp"

using std::vector;
using boost::function;
using boost::barrier;

EntityWorker::EntityWorker(function<void(Worker*)>* action, barrier* internal_barr, barrier* external_barr, unsigned int endTick)
    : Worker(action, internal_barr, external_barr, endTick)
{
}


//TODO: This should be part of Worker, not EntityWorker
/*void EntityWorker::start()
{
	//localTimestep = 0;
	Worker::start();
}*/

/*void EntityWorker::setSimulationEnd(unsigned int time)
{
	simulationEnd = time;
}*/


/**
 * Update all entities that this Worker controls.
 */
void EntityWorker::perform_main()
{
	for (vector<Entity*>::iterator it=getEntities().begin(); it!=getEntities().end(); it++) {
		//TODO: Either use templates or make a base class of "entities" (items with flip-able properties)
		//TODO: Use a dynamic_cast if there's a common base object.
		Entity* ag = (Entity*)(*it);
		ag->update();
	}

}


/**
 * Flip the memory used to store each agent's properties.
 */
/*void EntityWorker::perform_flip()
{
	Worker::perform_flip();
}*/


void EntityWorker::addEntity(Entity* entity)
{
	entities.push_back(entity);
	entity->subscribe(this, true);
}

void EntityWorker::remEntity(Entity* entity)
{
	vector<Entity*>::iterator it = std::find(entities.begin(), entities.end(), entity);
	if (it!=entities.end()) {
		entities.erase(it);
	}
	entity->subscribe(this, false);
}

vector<Entity*>& EntityWorker::getEntities() {
	return entities;
}

