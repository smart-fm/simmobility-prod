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
#include "OnCallController.hpp"


namespace sim_mob {

Rebalancer::Rebalancer(const OnCallController* parentController_):parentController(parentController_ ) {
	// TODO Auto-generated constructor stub

}

Rebalancer::~Rebalancer() {
	// TODO Auto-generated destructor stub
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

		parentController->sendCruiseCommand(driver, node, currTick );
		latestStartNodes.clear();
	}
}

void LazyRebalancer::rebalance(const std::vector<const Person*>& availableDrivers, const timeslice currTick)
{
	//Does nothing
}

} /* namespace sim_mob */


