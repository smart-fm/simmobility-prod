/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "EntityWorker.hpp"

using std::vector;
using boost::function;
using boost::barrier;


using namespace sim_mob;


sim_mob::EntityWorker::EntityWorker(Worker<Entity>::actionFunction* action, barrier* internal_barr, barrier* external_barr, unsigned int endTick)
    : Worker<Entity>(action, internal_barr, external_barr, endTick)
{
}

void sim_mob::EntityWorker::perform_main(frame_t frameNumber)
{
	for (vector<Entity*>::iterator it=getEntities().begin(); it!=getEntities().end(); it++) {
		(*it)->update(frameNumber);
	}

}







