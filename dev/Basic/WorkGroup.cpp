#include "WorkGroup.hpp"

using std::vector;
using boost::barrier;
using boost::function;


WorkGroup::WorkGroup(size_t size) : shared_barr(size+1), totalWorkers(size)
{
	currID = 0;

	//Ensure we won't later invalidate our references
	workers.reserve(totalWorkers);
}

size_t WorkGroup::size()
{
	return totalWorkers;
}

Worker& WorkGroup::initWorker(boost::function<void()>& action)
{
	if (allWorkersUsed())
		throw std::runtime_error("WorkGroup is already full!");

	workers.push_back(Worker(&action, &shared_barr));    //TODO: "action" can easily become invalid
	return workers[currID++];
}

Worker& WorkGroup::getWorker(size_t id)
{
	if (id >= currID)
		throw std::runtime_error("Invalid Worker id.");
	return workers[id];
}

bool WorkGroup::allWorkersUsed()
{
	return currID==totalWorkers;
}

void WorkGroup::wait()
{
	shared_barr.wait();
}

void WorkGroup::interrupt()
{
//	if (!allWorkersUsed())
//		throw std::runtime_error("Can't join_all; WorkGroup is not full (and will not overcome the barrier).");

	for (vector<Worker>::iterator it=workers.begin(); it!=workers.end(); it++)
		it->interrupt();
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










