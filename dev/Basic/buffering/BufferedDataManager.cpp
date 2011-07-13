#include "BufferedDataManager.hpp"

using namespace sim_mob;

using std::vector;


//TEMP
boost::mutex sim_mob::BufferedBase::global_mutex;



void sim_mob::BufferedDataManager::beginManaging(BufferedBase* datum)
{
	managedData.push_back(datum);

	//Helps with debugging.
	datum->refCount++;
}

void sim_mob::BufferedDataManager::stopManaging(BufferedBase* datum)
{
	std::vector<BufferedBase*>::iterator it = std::find(managedData.begin(), managedData.end(), datum);
	if (it!=managedData.end()) {
		managedData.erase(it);
	}

	//Helps with debugging.
	datum->refCount--;
}

void sim_mob::BufferedDataManager::flip()
{
	for (vector<BufferedBase*>::iterator it=managedData.begin(); it!=managedData.end(); it++) {
		(*it)->flip();
	}
}



