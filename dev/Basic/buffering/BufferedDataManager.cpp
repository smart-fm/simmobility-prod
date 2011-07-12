#include "BufferedDataManager.hpp"

using namespace sim_mob;

using std::vector;


//TEMP
boost::mutex sim_mob::BufferedBase::global_mutex;



////////////////////////////////////////////////////
// Implementation of our simple BufferedBase class
////////////////////////////////////////////////////
sim_mob::BufferedBase::BufferedBase(/*BufferedDataManager* mgr*/) : refCount(0) {
	/*if (mgr!=NULL) {
		mgr->add(this);
	}*/
}

sim_mob::BufferedBase::~BufferedBase() {
	if (refCount != 0) {
		//This represents an error
		{
			boost::mutex::scoped_lock local_lock(BufferedBase::global_mutex);
			std::cout <<"ERROR: Buffered<> type freed while still being referenced." <<std::endl;
		}
	}
	/*if (mgr!=NULL) {
		mgr->rem(this);
	}*/
}

//BufferedBase& sim_mob::BufferedBase::operator=(const BufferedBase& rhs)
//{

	//If we were being managed, we're not any more.
	/*if (this->mgr!=NULL) {
		mgr->rem(this);
	}

	this->mgr = rhs.mgr;

	//Need to register this class with the new maanger.
	if (this->mgr!=NULL) {
		mgr->add(this);
	}*/

//	return *this;
//}


/*void sim_mob::BufferedBase::migrate(sim_mob::BufferedDataManager* newMgr)
{
	//TEMP: Is it right to unsubscribe first?
	if (mgr!=NULL) {
		mgr->rem(this);
	}

	mgr = newMgr;

	//TEMP: Is it right to then subscribe
	if (newMgr!=NULL) {
		newMgr->add(this);
	}
}*/




////////////////////////////////////////////////////
// Implementation of our BufferedDataManager
////////////////////////////////////////////////////


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



