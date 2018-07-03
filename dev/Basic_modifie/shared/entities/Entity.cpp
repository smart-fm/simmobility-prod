//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Entity.hpp"

#include "buffering/BufferedDataManager.hpp"
#include "logging/Log.hpp"

using std::string;
using std::vector;
using namespace sim_mob;

//Implementation of our comparison function for Agents by start time.
bool sim_mob::cmp_agent_start::operator()(const Entity* x, const Entity* y) const
{
	//TODO: Not sure what to do in this case...
	if ((!x) || (!y))
	{
		return 0;
	}

	//We want a lower start time to translate into a higher priority.
	return x->getStartTime() > y->getStartTime();
}

typedef Entity::UpdateStatus UpdateStatus;

const UpdateStatus sim_mob::Entity::UpdateStatus::Continue(UpdateStatus::RS_CONTINUE);
const UpdateStatus sim_mob::Entity::UpdateStatus::ContinueIncomplete(UpdateStatus::RS_CONTINUE_INCOMPLETE);
const UpdateStatus sim_mob::Entity::UpdateStatus::Done(UpdateStatus::RS_DONE);

sim_mob::Entity::Entity(unsigned int id) :
		id(id), startTime(0), currWorkerProvider(nullptr), isFake(false), parentEntity(nullptr), isDuplicateFakeEntity(false), MessageHandler(id),
		multiUpdate(false)
{
}

sim_mob::Entity::~Entity()
{
	if (currWorkerProvider)
	{
		/*Note: If a worker thread is still active for this agent, that's a major problem. But
		we can't throw an exception since that may lead to a call of terminate().
		So we'll output a message and terminate manually, since throwing exceptions from
		a destructor is iffy at best.*/
		Warn() << "Error: Deleting an Entity which is still being managed by a Worker." << std::endl;
	}
}

vector<BufferedBase *> sim_mob::Entity::getSubscriptionList()
{
	return buildSubscriptionList();
}

sim_mob::Entity::UpdateStatus::UpdateStatus(UpdateStatus::RET_STATUS status, const vector<BufferedBase*>& currTickVals,
											const vector<BufferedBase*>& nextTickVals)
: status(status)
{
	//Any property not in the previous time tick but in the next is to be added. Any in the previous
	// but not in the next is to be removed. The rest remain throughout.
	for (vector<BufferedBase*>::const_iterator it = currTickVals.begin(); it != currTickVals.end(); it++)
	{
		toRemove.insert(*it);
	}
	
	for (vector<BufferedBase*>::const_iterator it = nextTickVals.begin(); it != nextTickVals.end(); it++)
	{
		if (toRemove.erase(*it) == 0)
		{
			toAdd.insert(*it);
		}
	}
}

void sim_mob::Entity::onWorkerEnter()
{
}

void sim_mob::Entity::onWorkerExit()
{
}

void sim_mob::Entity::registerChild(Entity* child)
{
}

void sim_mob::Entity::unregisterChild(Entity* child)
{
}
