/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Agent.hpp"

#include "util/OutputUtil.hpp"
#include "partitions/PartitionManager.hpp"

using namespace sim_mob;

using std::vector;

vector<Agent*> sim_mob::Agent::all_agents;

unsigned int sim_mob::Agent::next_agent_id = 0;
unsigned int sim_mob::Agent::GetAndIncrementID(int preferredID)
{
	//If the ID is valid, modify next_agent_id;
	if (preferredID > static_cast<int> (next_agent_id))
	{
		next_agent_id = static_cast<unsigned int> (preferredID);
	}

	const ConfigParams& config = ConfigParams::GetInstance();
	if (config.is_run_on_many_computers == false)
	{
		return next_agent_id++;
	}
	else
	{
		PartitionManager& partitionImpl = PartitionManager::instance();
		int mpi_id = partitionImpl.partition_config->partition_id;
		int cycle = partitionImpl.partition_config->maximum_agent_id;
		return (next_agent_id++) + cycle * mpi_id;
	}
}

sim_mob::Agent::Agent(int id) :
	Entity(GetAndIncrementID(id)), originNode(nullptr), destNode(nullptr), startTime(0), xPos(0), yPos(0), xVel(0),
			yVel(0), xAcc(0), yAcc(0)/*, currentLink(0),currentCrossing(-1)*/
{
	toRemoved = false;
}

void sim_mob::Agent::buildSubscriptionList()
{
	subscriptionList_cached.push_back(&xPos);
	subscriptionList_cached.push_back(&yPos);
	subscriptionList_cached.push_back(&xVel);
	subscriptionList_cached.push_back(&yVel);
	subscriptionList_cached.push_back(&xAcc);
	subscriptionList_cached.push_back(&yAcc);
	//subscriptionList_cached.push_back(&currentLink);
	//subscriptionList_cached.push_back(&currentCrossing);

}

bool sim_mob::Agent::isToBeRemoved()
{
	return toRemoved;
}

void sim_mob::Agent::setToBeRemoved(bool value)
{
	//Do nothing?
	if (value == toRemoved)
	{
		return;
	}

	toRemoved = value;
}

void sim_mob::Agent::output(frame_t frameNumber)
{
	//currently, do nothing
}

