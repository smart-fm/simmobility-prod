/*
 * GreedyTaxiController.cpp
 *
 *  Created on: Apr 18, 2017
 *      Author: Akshay Padmanabha
 */

#include "GreedyTaxiController.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "logging/ControllerLog.hpp"
#include "message/MessageBus.hpp"
#include "message/MobilityServiceControllerMessage.hpp"
#include "entities/mobilityServiceDriver/MobilityServiceDriver.hpp"
#include "entities/Person.hpp"

namespace sim_mob
{
std::vector<MobilityServiceController::MessageResult> GreedyTaxiController::computeSchedules()
{
	std::vector<MobilityServiceController::MessageResult> results;

	for (std::vector<TripRequest>::iterator request = requestQueue.begin(); request != requestQueue.end(); request++)
	{
		Person* bestDriver;
		double bestDistance = -1;
		double bestX, bestY;

		std::map<unsigned int, Node*> nodeIdMap = RoadNetwork::getInstance()->getMapOfIdvsNodes();

		std::map<unsigned int, Node*>::iterator it = nodeIdMap.find((*request).startNodeId); 
		if (it == nodeIdMap.end()) {
			ControllerLog() << "Request contains bad start node " << (*request).startNodeId << std::endl;
			results.push_back(MESSAGE_ERROR_BAD_NODE);
			continue;
		}
		Node* startNode = it->second;

		it = nodeIdMap.find((*request).destinationNodeId); 
		if (it == nodeIdMap.end()) {
			ControllerLog() << "Request contains bad destination node " << (*request).destinationNodeId << std::endl;
			results.push_back(MESSAGE_ERROR_BAD_NODE);
			continue;
		}
		Node* destinationNode = it->second;

		auto person = availableDrivers.begin();

		while (person != availableDrivers.end())
		{
			if (!isCruising(*person))
			{
				person++;
				continue;
			}

			if (bestDistance < 0)
			{
				bestDriver = *person;

				const Node* node = getCurrentNode(bestDriver);
				bestDistance = std::abs(startNode->getPosX()
					- node->getPosX());
				bestDistance += std::abs(startNode->getPosY()
					- node->getPosY());
			}
			else
			{
				const Node* node = getCurrentNode(*person);
				double currDistance = std::abs(startNode->getPosX()
					- node->getPosX());
				currDistance += std::abs(startNode->getPosY()
					- node->getPosY());

				if (currDistance < bestDistance)
				{
					bestDriver = *person;
					bestDistance = currDistance;
					bestX = node->getPosX();
					bestY = node->getPosY();
				}
			}

			person++;
		}

		if (bestDistance == -1)
		{
			ControllerLog() << "No available vehicles" << std::endl;
			results.push_back(MESSAGE_ERROR_VEHICLES_UNAVAILABLE);
			continue;
		}

		ControllerLog() << "Closest vehicle is at (" << bestX << ", " << bestY << ")" << std::endl;

		messaging::MessageBus::PostMessage((messaging::MessageHandler*) bestDriver, MSG_SCHEDULE_PROPOSITION, messaging::MessageBus::MessagePtr(
			new SchedulePropositionMessage(currTick, (*request).personId, (*request).startNodeId,
				(*request).destinationNodeId, (*request).extraTripTimeThreshold)));

		ControllerLog() << "Assignment sent for " << (*request).personId << " at time " << currTick.frame()
		<< ". Message was sent at " << (*request).currTick.frame() << " with startNodeId " << (*request).startNodeId
		<< ", destinationNodeId " << (*request).destinationNodeId << ", and driverId null" << std::endl;

		results.push_back(MESSAGE_SUCCESS);
	}

	return results;
}

bool GreedyTaxiController::isCruising(Person* p) 
{
    MobilityServiceDriver* currDriver = p->exportServiceDriver();
    if (currDriver) 
    {
        if (currDriver->getServiceStatus() == MobilityServiceDriver::SERVICE_FREE) 
        {
            return true;
        }
    }
    return false;
}

const Node* GreedyTaxiController::getCurrentNode(Person* p) 
{
    MobilityServiceDriver* currDriver = p->exportServiceDriver();
    if (currDriver) 
    {
        return currDriver->getCurrentNode();
    }
    return nullptr;
}
}

