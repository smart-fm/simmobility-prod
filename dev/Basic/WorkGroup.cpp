#include "WorkGroup.hpp"

using std::vector;
using boost::barrier;
using boost::function;


WorkGroup::WorkGroup(size_t size) : shared_barr(size+1), external_barr(size+1), total_size(size)
{
}

WorkGroup::~WorkGroup()
{
	for (size_t i=0; i<workers.size(); i++) {
		workers[i]->join();  //NOTE: If we don't join all Workers, we get threading exceptions.
		delete workers[i];
	}
}


void WorkGroup::startAll()
{
	for (size_t i=0; i<workers.size(); i++) {
		workers[i]->start();
	}
}


size_t WorkGroup::size()
{
	return workers.size();
}


Worker& WorkGroup::getWorker(size_t id)
{
	if (id >= workers.size())
		throw std::runtime_error("Invalid Worker id.");
	return *workers[id];
}


void WorkGroup::wait()
{
	shared_barr.wait();
	external_barr.wait();
}

void WorkGroup::interrupt()
{
	for (size_t i=0; i<workers.size(); i++)
		workers[i]->interrupt();
}


/**
 * Set "fromID" or "toID" to -1 to skip that step.
 */
void WorkGroup::migrate(void* ag, int fromID, int toID)
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










