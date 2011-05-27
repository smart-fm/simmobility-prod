#include "WorkGroup.hpp"

using std::vector;
using boost::barrier;
using boost::function;


WorkGroup::WorkGroup(size_t size) : shared_barr(size+1), external_barr(size+1), total_size(size)
{
	//currID = 0;
	//workers = new Worker*[size];
}

WorkGroup::~WorkGroup()
{
	for (size_t i=0; i<workers.size(); i++) {
		delete workers[i];
	}
	//delete [] workers;
}


size_t WorkGroup::size()
{
	return workers.size();
	//return totalWorkers;
}


Worker& WorkGroup::getWorker(size_t id)
{
	if (id >= workers.size())
		throw std::runtime_error("Invalid Worker id.");
	return *workers[id];
}

/*bool WorkGroup::allWorkersUsed()
{
	return currID==totalWorkers;
}*/

void WorkGroup::wait()
{
	shared_barr.wait();
	external_barr.wait();
}

void WorkGroup::interrupt()
{
//	if (!allWorkersUsed())
//		throw std::runtime_error("Can't join_all; WorkGroup is not full (and will not overcome the barrier).");

	for (size_t i=0; i<workers.size(); i++)
		workers[i]->interrupt();
}


/**
 * Set "fromID" or "toID" to -1 to skip that step.
 */
void WorkGroup::migrate(Agent* ag, int fromID, int toID)
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










