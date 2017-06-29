/*
 * Rebalancer.cpp
 *
 *  Created on: Jun 1, 2017
 *      Author: araldo
 */

#include "Rebalancer.hpp"
#include <queue>
#include <vector>
#include "message/MobilityServiceControllerMessage.hpp"
#include "message/MessageBus.hpp"
#include "logging/ControllerLog.hpp"


namespace sim_mob {

Rebalancer::Rebalancer() {
	// TODO Auto-generated constructor stub

}

Rebalancer::~Rebalancer() {
	// TODO Auto-generated destructor stub
}


void Rebalancer::sendCruiseCommand(const Person* driver, const Node* nodeToCruiseTo, const timeslice currTick) const
{
	ScheduleItem item(ScheduleItemType::CRUISE, nodeToCruiseTo);
	sim_mob::Schedule schedule;
	schedule.push_back(ScheduleItem(item) );


	messaging::MessageBus::PostMessage((messaging::MessageHandler*) driver, MSG_SCHEDULE_PROPOSITION,
				messaging::MessageBus::MessagePtr(new SchedulePropositionMessage(currTick, schedule) ) );
}



void Rebalancer::onRequestReceived(const Node* startNode)
{
	latestStartNodes.push_back(startNode);
}

void SimpleRebalancer::rebalance(const std::vector<const Person*>& availableDrivers, const timeslice currTick)
{
	if (!availableDrivers.empty() && !latestStartNodes.empty() )
	{
		int seed = 1;
		srand(seed);
		const Person* driver = availableDrivers[rand()%availableDrivers.size() ];
		const Node* node = latestStartNodes[rand()%latestStartNodes.size()];

		sendCruiseCommand(driver, node, currTick );
		latestStartNodes.clear();
	}
}

} /* namespace sim_mob */


