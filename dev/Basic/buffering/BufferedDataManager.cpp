#include "BufferedDataManager.hpp"

namespace sim_mob
{

using std::vector;


BufferedDataManager::BufferedDataManager()
{
}

BufferedDataManager::~BufferedDataManager()
{
	//Avoid possible invalid pointers
	managedData.clear();
}


void BufferedDataManager::add(BufferedBase* datum)
{
	managedData.push_back(datum);
}

void BufferedDataManager::rem(BufferedBase* datum)
{
	std::vector<BufferedBase*>::iterator it = std::find(managedData.begin(), managedData.end(), datum);
	if (it!=managedData.end()) {
		managedData.erase(it);
	}
}

void BufferedDataManager::flip()
{
	for (vector<BufferedBase*>::iterator it=managedData.begin(); it!=managedData.end(); it++) {
		(*it)->flip();
	}
}


}
