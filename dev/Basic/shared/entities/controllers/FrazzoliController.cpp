/*
 * FrazzoliController.cpp
 *
 *  Created on: 21 Jun 2017
 *      Author: araldo
 */

#include "FrazzoliController.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "logging/ControllerLog.hpp"
#include "message/MessageBus.hpp"
#include "message/MobilityServiceControllerMessage.hpp"
#include "path/PathSetManager.hpp"

namespace sim_mob {



FrazzoliController::~FrazzoliController() {
	// TODO Auto-generated destructor stub
}


void FrazzoliController::computeSchedules()
{
	/************************************
	 * WORK IN PROGRESS
	 * ***********************************
	 * **********************************
	 */



	const std::map<unsigned int, Node*>& nodeIdMap = RoadNetwork::getInstance()->getMapOfIdvsNodes();


	for (const TripRequestMessage& request : requestQueue)
	{
		for (const std::pair<const Person*, const Schedule>& p : driverSchedules )
		{
			const Person* driver = p.first;
			const Schedule& currentSchedule = p.second;
			const Node *driverNode = getCurrentNode(driver);
			std::vector<TripRequestMessage> additionalRequests; additionalRequests.push_back(request);
			Schedule newSchedule;
			computeOptimalSchedule(driverNode, currentSchedule, additionalRequests, newSchedule);
		}

	}

}

} /* namespace sim_mob */
