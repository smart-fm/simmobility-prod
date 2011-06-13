#include "WorkGroup.hpp"

using std::vector;
using boost::barrier;
using boost::function;

using namespace sim_mob;

sim_mob::WorkGroup::WorkGroup(size_t size, unsigned int endTick, unsigned int tickStep) :
		shared_barr(size+1), external_barr(size+1), endTick(endTick), tickStep(tickStep), total_size(size)
{
}

sim_mob::WorkGroup::~WorkGroup()
{
	for (size_t i=0; i<workers.size(); i++) {
		workers[i]->join();  //NOTE: If we don't join all Workers, we get threading exceptions.
		delete workers[i];
	}
}


void sim_mob::WorkGroup::startAll()
{
	tickOffset = tickStep;
	for (size_t i=0; i<workers.size(); i++) {
		workers[i]->start();
	}
}


size_t sim_mob::WorkGroup::size()
{
	return workers.size();
}


Worker& sim_mob::WorkGroup::getWorker(size_t id)
{
	if (id >= workers.size())
		throw std::runtime_error("Invalid Worker id.");
	return *workers[id];
}


void sim_mob::WorkGroup::wait()
{
	if (--tickOffset>0) {
		return;
	}
	tickOffset = tickStep;

	shared_barr.wait();
	external_barr.wait();
}

void sim_mob::WorkGroup::interrupt()
{
	for (size_t i=0; i<workers.size(); i++)
		workers[i]->interrupt();
}


/**
 * Set "fromID" or "toID" to -1 to skip that step.
 */
void sim_mob::WorkGroup::migrate(void* ag, int fromID, int toID)
{
	if (ag==NULL)
		return;

	//Remove from the old location
	if (fromID >= 0) {
		getWorker(fromID).remEntity(ag);
	}

	//Add to the new location
	if (toID >= 0) {
		getWorker(toID).addEntity(ag);
	}
}



