/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Entity.hpp"

#include "buffering/BufferedDataManager.hpp"
#include "conf/simpleconf.hpp"
#include "logging/Log.hpp"

using std::string;
using std::vector;

using namespace sim_mob;
typedef Entity::UpdateStatus UpdateStatus;


sim_mob::Entity::Entity(unsigned int id)
	: id(id),  startTime(0), currWorkerProvider(nullptr), isFake(false), parentEntity(nullptr), can_remove_by_RTREE(false)
{


}


sim_mob::Entity::~Entity()
{
	if (currWorkerProvider) {
		//Note: If a worker thread is still active for this agent, that's a major problem. But
		//      we can't throw an exception since that may lead to a call of terminate().
		//      So we'll output a message and terminate manually, since throwing exceptions from
		//      a destructor is iffy at best.
		Warn() <<"Error: Deleting an Entity which is still being managed by a Worker." <<std::endl;
		abort();
	}
}

string sim_mob::Entity::getGlobalId() const
{
	std::stringstream res;
	res <<"127.0.0.1:" <<id;
	return res.str();
}


vector<BufferedBase*> sim_mob::Entity::getSubscriptionList()
{
	std::vector<sim_mob::BufferedBase*> subsList;
	buildSubscriptionList(subsList);
	return subsList;
}



const UpdateStatus sim_mob::Entity::UpdateStatus::Continue(UpdateStatus::RS_CONTINUE);
const UpdateStatus sim_mob::Entity::UpdateStatus::Done(UpdateStatus::RS_DONE);

sim_mob::Entity::UpdateStatus::UpdateStatus(UpdateStatus::RET_STATUS status, const vector<BufferedBase*>& currTickVals, const vector<BufferedBase*>& nextTickVals)
	: status(status)
{
	//Any property not in the previous time tick but in the next is to be added. Any in the previous
	// but not in the next is to be removed. The rest remain throughout.
	for (vector<BufferedBase*>::const_iterator it=currTickVals.begin(); it!=currTickVals.end(); it++) {
		toRemove.insert(*it);
	}
	for (vector<BufferedBase*>::const_iterator it=nextTickVals.begin(); it!=nextTickVals.end(); it++) {
		if (toRemove.erase(*it)==0) {
			toAdd.insert(*it);
		}
	}

}



