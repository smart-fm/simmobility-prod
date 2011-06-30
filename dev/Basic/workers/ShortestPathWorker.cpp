#include "ShortestPathWorker.hpp"

using std::vector;
using boost::function;
using boost::barrier;


using namespace sim_mob;


sim_mob::ShortestPathWorker::ShortestPathWorker(Worker<Agent>::actionFunction* action, barrier* internal_barr, barrier* external_barr, unsigned int endTick)
    : Worker<Agent>(action, internal_barr, external_barr, endTick)
{
}


/**
 * Update all entities that this Worker controls.
 */
void sim_mob::ShortestPathWorker::perform_main()
{
	for (vector<Agent*>::iterator it=getEntities().begin(); it!=getEntities().end(); it++) {
		//(*it)->updateShortestPath();
		//TEMP: We don't really use this class at the moment.
	}
}



