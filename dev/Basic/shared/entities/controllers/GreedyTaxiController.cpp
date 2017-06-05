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
	ControllerLog()<<"Computing schedule: "<< requestQueue.size()<<" requests are in the queue"<<std::endl;
	std::vector<MobilityServiceController::MessageResult> results;

	for (std::vector<TripRequestMessage>::const_iterator request = requestQueue.begin(); request != requestQueue.end(); request++)
	{
		//{ RETRIEVE NODES
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
		//} RETRIEVE NODES

		Person* bestDriver = NULL;
		double bestDistanceSqr = std::numeric_limits<double>::max();
		double bestX, bestY;


		auto driver = availableDrivers.begin();

		while (driver != availableDrivers.end())
		{
			if (isCruising(*driver) )
			{
					const Node* driverNode = getCurrentNode(*driver);
					double currDistanceSqr =
						(startNode->getPosX() - driverNode->getPosX() )*
						(startNode->getPosX() - driverNode->getPosX() );
					currDistanceSqr+=
						(startNode->getPosY() - driverNode->getPosY() )*
						(startNode->getPosY() - driverNode->getPosY() );

					if (currDistanceSqr < bestDistanceSqr)
					{
						bestDriver = *driver;
						bestDistanceSqr = currDistanceSqr;
						bestX = driverNode->getPosX();
						bestY = driverNode->getPosY();
					}
			}
			driver++;
		}


		if (bestDriver != NULL)
		{

			ControllerLog() << "Closest vehicle is at (" << bestX << ", " << bestY << ")" << std::endl;


			Schedule schedule;

			try{
			const ScheduleItem pickUpScheduleItem(ScheduleItemType::PICKUP,*request);

			const ScheduleItem dropOffScheduleItem(ScheduleItemType::DROPOFF,*request);

			ControllerLog()<<"Items constructed"<<std::endl;

			schedule.push_back(pickUpScheduleItem);
			schedule.push_back(dropOffScheduleItem );

			}catch(std::exception& e)
			{
				ControllerLog()<<"Error in "<< __FILE__ << ":" << __LINE__ <<": "<< e.what() << std::endl;
				exit(0);
			}

			sendScheduleProposition(bestDriver, schedule);


			/* OLD CODE REPLACED NOW BY sendScheduleProposition(..)
			messaging::MessageBus::PostMessage((messaging::MessageHandler*) bestDriver, MSG_SCHEDULE_PROPOSITION, messaging::MessageBus::MessagePtr(
				new SchedulePropositionMessage(currTick, (*request).personId, (*request).startNodeId,
					(*request).destinationNodeId, (*request).extraTripTimeThreshold)
			));
			*/

			ControllerLog() << "Assignment sent for " << request->personId << " at time " << currTick.frame()
			<< ". Message was sent at " << request->currTick.frame() << " with startNodeId " << request->startNodeId
			<< ", destinationNodeId " << request->destinationNodeId << ", and driverId null" << std::endl;

			results.push_back(MESSAGE_SUCCESS);
		} else{
			ControllerLog() << "No available vehicles" << std::endl;
			results.push_back(MESSAGE_ERROR_VEHICLES_UNAVAILABLE);
		}
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
    	const Node* currentNode =currDriver->getCurrentNode();
        return currentNode;
    }
    return nullptr;
}
}

