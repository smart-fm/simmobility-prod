#include "BufferedDataManager.hpp"

namespace sim_mob
{

using std::vector;

BufferedDataManager BufferedDataManager::instance_;

BufferedDataManager& BufferedDataManager::GetInstance()
{
	return instance_;
}


void BufferedDataManager::add(BufferedBase* datum)
{
	managedData.push_back (datum);
}

void BufferedDataManager::flip()
{
	for (vector<BufferedBase*>::iterator it=managedData.begin(); it!=managedData.end(); it++) {
		(*it)->flip();
	}
}


}
