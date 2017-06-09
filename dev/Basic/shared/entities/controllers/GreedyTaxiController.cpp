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
void GreedyTaxiController::computeSchedules()
{
	ControllerLog()<<"Computing schedule: "<< requestQueue.size()<<" requests are in the queue"<<std::endl;

	std::list<TripRequestMessage>::iterator request = requestQueue.begin();
	if ( !availableDrivers.empty() )
	{
		while ( request != requestQueue.end())
		{
			//{ RETRIEVE NODES
			const Node* startNode, destinationNode;
			std::map<unsigned int, Node*> nodeIdMap = RoadNetwork::getInstance()->getMapOfIdvsNodes();
			std::map<unsigned int, Node*>::iterator itStart = nodeIdMap.find((*request).startNodeId);
			std::map<unsigned int, Node*>::iterator itDestination = nodeIdMap.find((*request).destinationNodeId);

	#ifndef NDEBUG
			if (itStart == nodeIdMap.end())
			{
				std::stringstream msg; msg << "Request contains bad start node " << (*request).startNodeId << std::endl;
				throw std::runtime_error(msg.str() );
			}
			else if (itDestination == nodeIdMap.end()) {
				std::stringstream msg; msg  << "Request contains bad destination node " << (*request).destinationNodeId << std::endl;
				throw std::runtime_error(msg.str() );
			}
	#endif
			//} RETRIEVE NODES


			const Person* bestDriver = findClosestDriver(startNode) ;

#ifndef NDEBUG
			if (bestDriver == NULL)
			{
				std::stringstream msg; msg << "bestDriver==NULL means that there are no drivers available. "
				<<"This is impossible as in that case I would have not entered here";
				throw std::runtime_error(msg.str() );
			}
#endif



				Schedule schedule;

				const ScheduleItem pickUpScheduleItem(ScheduleItemType::PICKUP,*request);

				const ScheduleItem dropOffScheduleItem(ScheduleItemType::DROPOFF,*request);

				ControllerLog()<<"Items constructed"<<std::endl;

				schedule.push_back(pickUpScheduleItem);
				schedule.push_back(dropOffScheduleItem );

				sendScheduleProposition(bestDriver, schedule);

				ControllerLog() << "Assignment sent for " << request->personId << " at time " << currTick.frame()
				<< ". Message was sent at " << request->currTick.frame() << " with startNodeId " << request->startNodeId
				<< ", destinationNodeId " << request->destinationNodeId << ", and driverId " << bestDriver->getDatabaseId()
				<< std::endl;

				request = requestQueue.erase(request);

		}
	}else // no available drivers
		ControllerLog()<<"Requests to be scheduled "<< requestQueue.size() << ", available drivers "<<availableDrivers.size() <<std::endl;
}


}

