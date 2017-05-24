/*
 * OnHailTaxiController.cpp
 *
 *  Created on: Apr 22, 2017
 *      Author: Akshay Padmanabha
 */

#include "OnHailTaxiController.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "message/MessageBus.hpp"
#include "message/MobilityServiceControllerMessage.hpp"
#include "entities/mobilityServiceDriver/MobilityServiceDriver.hpp"
#include "entities/Person.hpp"

namespace sim_mob
{
std::vector<MobilityServiceController::MessageResult> OnHailTaxiController::computeSchedules()
{
	std::vector<MobilityServiceController::MessageResult> results;

	for (std::vector<TripRequest>::iterator request = requestQueue.begin(); request != requestQueue.end(); request++)
	{
		results.push_back(MESSAGE_ERROR_BAD_NODE);
	}

	return results;
}
}
