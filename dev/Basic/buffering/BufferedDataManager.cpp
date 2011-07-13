#include "BufferedDataManager.hpp"

using namespace sim_mob;

using std::vector;


//TEMP
boost::mutex sim_mob::BufferedBase::global_mutex;



sim_mob::BufferedDataManager::~BufferedDataManager()
{

}


void sim_mob::BufferedDataManager::add(BufferedBase* datum)
{
	managedData.push_back(datum);

	//Helps with debugging.
	datum->refCount++;

	//TEMP
	//if (BufferedBase::TEMP_DEBUG) {
	//	std::cout <<"  "  <<this <<"  now managing: " <<datum <<"  total " <<managedData.size() <<"\n";
	//}
}

void sim_mob::BufferedDataManager::rem(BufferedBase* datum)
{
	std::vector<BufferedBase*>::iterator it = std::find(managedData.begin(), managedData.end(), datum);
	if (it!=managedData.end()) {
		managedData.erase(it);
	}

	//Helps with debugging.
	datum->refCount--;

	//TEMP
	//if (BufferedBase::TEMP_DEBUG) {
	//	std::cout <<"  "  <<this <<"  stops managing: " <<datum <<"  total " <<managedData.size() <<"\n";
	//}
}

void sim_mob::BufferedDataManager::flip()
{
	//std::cout <<"  Mgr " <<this <<" is flipping " <<managedData.size() <<" items.\n";

	for (vector<BufferedBase*>::iterator it=managedData.begin(); it!=managedData.end(); it++) {
		(*it)->flip();
	}
}



