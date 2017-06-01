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


namespace sim_mob {

Rebalancer::Rebalancer() {
	// TODO Auto-generated constructor stub

}

Rebalancer::~Rebalancer() {
	// TODO Auto-generated destructor stub
}


void Rebalancer::sendCruiseTAZ_Command(const Person* driver, unsigned tazId, const timeslice currTick) const
{
	CruiseTAZ_ScheduleItem* item = new CruiseTAZ_ScheduleItem(tazId);
	Schedule* schedule = new Schedule();
	schedule->push(item );


	messaging::MessageBus::PostMessage((messaging::MessageHandler*) driver, MSG_SCHEDULE_PROPOSITION,
				messaging::MessageBus::MessagePtr(new SchedulePropositionMessage(currTick, schedule) ) );
}



void Rebalancer::onRequestReceived(const Node* startNode)
{
	latestStartNodes.push_back(startNode);
}

void SimpleRebalancer::rebalance(const std::vector<Person*>& availableDrivers, const timeslice currTick)
{
	std::random_device random_device;
	std::mt19937 engine{random_device()};
	std::uniform_int_distribution<unsigned> distDriver(0, availableDrivers.size() - 1);
	Person* driver = availableDrivers[distDriver(engine)];
	std::uniform_int_distribution<unsigned> distNode(0, latestStartNodes.size() - 1);
	const Node* node = latestStartNodes[distNode(engine)];

	sendCruiseTAZ_Command(driver, node->getTazId(), currTick );
	latestStartNodes.clear();
}

} /* namespace sim_mob */


