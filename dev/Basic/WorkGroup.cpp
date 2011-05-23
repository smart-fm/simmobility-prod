#include "WorkGroup.hpp"

using std::vector;
using boost::barrier;
using boost::function;


WorkGroup::WorkGroup(size_t size) : totalWorkers(size)
{
	currID = 0;
	shared_barr = barrier(totalWorkers+1);

	//Ensure we won't later invalidate our references
	workers.reserve(totalWorkers);
}

size_t WorkGroup::size()
{
	return totalWorkers;
}

Worker& WorkGroup::getWorker(boost::function& action)
{
	if (allWorkersUsed())
		throw std::runtime_error("WorkGroup is already full!");

	workers.push_back(Worker(action, shared_barr));
	return workers[currID++];
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









