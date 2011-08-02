/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "BufferedDataManager.hpp"

using namespace sim_mob;

using std::vector;


//TEMP
boost::mutex sim_mob::BufferedBase::global_mutex;


sim_mob::BufferedDataManager::~BufferedDataManager()
{
	//Stop managing all items
	while (!managedData.empty()) {
		stopManaging(managedData[0]);
	}

}



void sim_mob::BufferedDataManager::beginManaging(BufferedBase* datum)
{
	//Only add if we're not managing it already.
	std::vector<BufferedBase*>::iterator it = std::find(managedData.begin(), managedData.end(), datum);
	if (it==managedData.end()) {
		managedData.push_back(datum);

		//Helps with debugging.
		datum->refCount++;
	}
}

void sim_mob::BufferedDataManager::stopManaging(BufferedBase* datum)
{
	//Only remove if we are actually managing it.
	std::vector<BufferedBase*>::iterator it = std::find(managedData.begin(), managedData.end(), datum);
	if (it!=managedData.end()) {
		managedData.erase(it);

		//Helps with debugging.
		datum->refCount--;
	}
}

void sim_mob::BufferedDataManager::flip()
{
	for (vector<BufferedBase*>::iterator it=managedData.begin(); it!=managedData.end(); it++) {
		(*it)->flip();
	}
}



