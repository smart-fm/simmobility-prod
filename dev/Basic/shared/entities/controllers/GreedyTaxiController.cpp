/*
 * GreedyTaxiController.cpp
 *
 *  Created on: Apr 18, 2017
 *      Author: Akshay Padmanabha
 */

#include "GreedyTaxiController.hpp"

#include "geospatial/network/RoadNetwork.hpp"
#include "message/MessageBus.hpp"
#include "message/MobilityServiceControllerMessage.hpp"

namespace sim_mob
{
std::vector<MobilityServiceController::MessageResult> GreedyTaxiController::assignVehiclesToRequests()
{
	std::vector<MobilityServiceController::MessageResult> results;

	for (std::vector<VehicleRequest>::iterator request = requestQueue.begin(); request != requestQueue.end(); request++)
	{
		Person* bestDriver;
		double bestDistance = -1;
		double bestX, bestY;

		std::map<unsigned int, Node*> nodeIdMap = RoadNetwork::getInstance()->getMapOfIdvsNodes();

		std::map<unsigned int, Node*>::iterator it = nodeIdMap.find((*request).startNodeId); 
		if (it == nodeIdMap.end()) {
			Print() << "Request contains bad start node " << (*request).startNodeId << std::endl;
			results.push_back(MESSAGE_ERROR_BAD_NODE);
			continue;
		}
		Node* startNode = it->second;

		it = nodeIdMap.find((*request).destinationNodeId); 
		if (it == nodeIdMap.end()) {
			Print() << "Request contains bad destination node " << (*request).destinationNodeId << std::endl;
			results.push_back(MESSAGE_ERROR_BAD_NODE);
			continue;
		}
		Node* destinationNode = it->second;

		auto person = vehicleDrivers.begin();

		while (person != vehicleDrivers.end())
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
			Print() << "No available vehicles" << std::endl;
			results.push_back(MESSAGE_ERROR_VEHICLES_UNAVAILABLE);
			continue;
		}

		Print() << "Closest vehicle is at (" << bestX << ", " << bestY << ")" << std::endl;

		messaging::MessageBus::PostMessage((messaging::MessageHandler*) bestDriver, MSG_VEHICLE_ASSIGNMENT, messaging::MessageBus::MessagePtr(
			new VehicleAssignmentMessage(currTick, (*request).personId, (*request).startNodeId,
				(*request).destinationNodeId, (*request).extraTripTimeThreshold)));

		Print() << "Assignment sent for " << (*request).personId << " at time " << currTick.frame()
		<< ". Message was sent at " << (*request).currTick.frame() << " with startNodeId " << (*request).startNodeId
		<< ", destinationNodeId " << (*request).destinationNodeId << ", and driverId null" << std::endl;

		results.push_back(MESSAGE_SUCCESS);
	}

	return results;
	}
}



