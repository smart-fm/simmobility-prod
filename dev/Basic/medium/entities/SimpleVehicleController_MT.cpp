/*
 * SimpleVehicleController_MT.cpp
 *
 *  Created on: Feb 20, 2017
 *      Author: Akshay Padmanabha
 */

#include "SimpleVehicleController_MT.hpp"

#include "entities/roles/driver/TaxiDriver.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "message/MessageBus.hpp"
#include "message/VehicleControllerMessage.hpp"

namespace sim_mob
{
std::vector<VehicleController::MessageResult> SimpleVehicleController_MT::assignVehiclesToRequests()
{
	std::vector<VehicleController::MessageResult> results;

	for (std::vector<VehicleRequest>::iterator request = requestQueue.begin(); request != requestQueue.end(); request++)
	{
		medium::TaxiDriver* bestDriver;
		double bestDistance = -1;
		double bestX, bestY;

		std::map<unsigned int, Node*> nodeIdMap = RoadNetwork::getInstance()->getMapOfIdvsNodes();

		std::map<unsigned int, Node*>::iterator it = nodeIdMap.find((*request).startNodeId); 
		if (it == nodeIdMap.end()) {
			Print() << "Request contains bad start node" << std::endl;
			results.push_back(MESSAGE_ERROR_BAD_NODE);
			continue;
		}
		Node* startNode = it->second;

		it = nodeIdMap.find((*request).destinationNodeId); 
		if (it == nodeIdMap.end()) {
			Print() << "Request contains bad destination node" << std::endl;
			results.push_back(MESSAGE_ERROR_BAD_NODE);
			continue;
		}
		Node* destinationNode = it->second;

		auto person = vehicleDrivers.begin();

		while (person != vehicleDrivers.end())
		{
			medium::Person_MT* person_MT = dynamic_cast<medium::Person_MT*>(*person);

			if (person_MT) {
				if (person_MT->getRole())
				{
					medium::TaxiDriver* currDriver = dynamic_cast<medium::TaxiDriver*>(person_MT->getRole());
					if (currDriver)
					{
						if (currDriver->getDriverMode() != medium::CRUISE)
						{
							person++;
							continue;
						}

						if (bestDistance < 0)
						{
							bestDriver = currDriver;

							medium::TaxiDriverMovement* movement = bestDriver
								->getMovementFacet();
							const Node* node = movement->getCurrentNode();

							bestDistance = std::abs(startNode->getPosX()
								- node->getPosX());
							bestDistance += std::abs(startNode->getPosY()
								- node->getPosY());
						}

						else
						{
							double currDistance = 0.0;

							medium::TaxiDriverMovement* movement = currDriver
								->getMovementFacet();
							const Node* node = movement->getCurrentNode();

							currDistance = std::abs(startNode->getPosX()
								- node->getPosX());
							currDistance += std::abs(startNode->getPosY()
								- node->getPosY());

							if (currDistance < bestDistance)
							{
								bestDriver = currDriver;
								bestDistance = currDistance;
								bestX = node->getPosX();
								bestY = node->getPosY();
							}
						}
					}
				}
			}

			person++;
		}

		if (bestDistance == -1)
		{
			Print() << "No available taxis" << std::endl;
			results.push_back(MESSAGE_ERROR_VEHICLES_UNAVAILABLE);
			continue;
		}

		Print() << "Closest taxi is at (" << bestX << ", " << bestY << ")" << std::endl;

		messaging::MessageBus::PostMessage(bestDriver->getParent(), MSG_VEHICLE_ASSIGNMENT, messaging::MessageBus::MessagePtr(
			new VehicleAssignmentMessage(currTick, (*request).personId, (*request).startNodeId, (*request).destinationNodeId)));

		Print() << "Assignment sent for " << (*request).personId << " at time " << currTick.frame()
			<< ". Message was sent at " << (*request).currTick.frame() << " with startNodeId " << (*request).startNodeId
			<< ", destinationNodeId " << (*request).destinationNodeId << ", and taxiDriverId null" << std::endl;

		results.push_back(MESSAGE_SUCCESS);
		continue;
	}

	return results;
}
}


