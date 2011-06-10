#include "BufferedDataManager.hpp"

namespace sim_mob
{

using std::vector;


////////////////////////////////////////////////////
// Implementation of our simple BufferedBase class
////////////////////////////////////////////////////
BufferedBase::BufferedBase(BufferedDataManager* mgr) : mgr(mgr) {
	if (mgr!=NULL) {
		mgr->add(this);
	}
}

BufferedBase::~BufferedBase() {
	if (mgr!=NULL) {
		mgr->rem(this);
	}
}

BufferedBase& BufferedBase::operator=(const BufferedBase& rhs)
{
	//If we were being managed, we're not any more.
	if (this->mgr!=NULL) {
		mgr->rem(this);
	}

	this->mgr = rhs.mgr;

	//Need to register this class with the new maanger.
	if (this->mgr!=NULL) {
		mgr->add(this);
	}

	return *this;
}


//This function might be named wrongly, since it only accomplishes half of the migration.
void BufferedBase::migrate(sim_mob::BufferedDataManager* newMgr)
{
	mgr = newMgr;
}




////////////////////////////////////////////////////
// Implementation of our BufferedDataManager
////////////////////////////////////////////////////



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
