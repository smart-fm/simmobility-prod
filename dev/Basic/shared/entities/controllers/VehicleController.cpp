/*
 * VehicleController.cpp
 *
 *  Created on: Feb 20, 2017
 *      Author: Akshay Padmanabha
 */

#include "VehicleController.hpp"

#include "message/MessageBus.hpp"
#include "message/VehicleControllerMessage.hpp"

namespace sim_mob
{
VehicleController::~VehicleController()
{
}

void VehicleController::addVehicleDriver(Person* person)
{
	vehicleDrivers.push_back(person);
}

void VehicleController::removeVehicleDriver(Person* person)
{
	vehicleDrivers.erase(std::remove(vehicleDrivers.begin(),
		vehicleDrivers.end(), person), vehicleDrivers.end());
}

Entity::UpdateStatus VehicleController::frame_init(timeslice now)
{
	if (!GetContext())
	{
		messaging::MessageBus::RegisterHandler(this);
	}
	
	return Entity::UpdateStatus::Continue;
}

Entity::UpdateStatus VehicleController::frame_tick(timeslice now)
{
	if (localTick == messageProcessFrequency)
	{
		localTick = 0;

		std::vector<MessageResult> messageResults = assignVehiclesToRequests();

		std::vector<VehicleRequest>::iterator request = requestQueue.begin();
		std::vector<MessageResult>::iterator messageResult = messageResults.begin();

		std:std::vector<VehicleRequest> retryRequestQueue;

		while (request != requestQueue.end())
		{
		    if ((*messageResult) == MESSAGE_ERROR_VEHICLES_UNAVAILABLE)
		    {
		    	retryRequestQueue.push_back(*request);
		    }

	    	request++;
		    messageResult++;
		}

		requestQueue.clear();

		request = retryRequestQueue.begin();
		while (request != retryRequestQueue.end())
		{
		    requestQueue.push_back(*request);
		    request++;
		}

	}
	else
	{
		localTick += 1;
	}

	return Entity::UpdateStatus::Continue;
}

void VehicleController::frame_output(timeslice now)
{
}

void VehicleController::HandleMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
	switch (type) {
	        case MSG_VEHICLE_REQUEST:
	        {
				const VehicleRequestMessage& requestArgs = MSG_CAST(VehicleRequestMessage, message);

				Print() << "Request received from " << requestArgs.personId << " at time " << currTick.frame() << ". Message was sent at "
					<< requestArgs.currTick.frame() << " with startNodeId " << requestArgs.startNodeId << ", destinationNodeId "
					<< requestArgs.destinationNodeId << ", and taxiDriverId null" << std::endl;

				requestQueue.push_back({requestArgs.currTick, requestArgs.personId, requestArgs.startNodeId, requestArgs.destinationNodeId});
	            break;
	        }

	        case MSG_VEHICLE_ASSIGNMENT_RESPONSE:
	        {
				const VehicleAssignmentResponseMessage& replyArgs = MSG_CAST(VehicleAssignmentResponseMessage, message);
				if (!replyArgs.success) {
					Print() << "Request received from " << replyArgs.personId << " at time " << currTick.frame() << ". Message was sent at "
						<< replyArgs.currTick.frame() << " with startNodeId " << replyArgs.startNodeId << ", destinationNodeId "
						<< replyArgs.destinationNodeId << ", and taxiDriverId null" << std::endl;

					requestQueue.push_back({replyArgs.currTick, replyArgs.personId, replyArgs.startNodeId, replyArgs.destinationNodeId});
				} else {
					Print() << "Assignment response received from " << replyArgs.personId << " at time "
						<< currTick.frame() << ". Message was sent at " << replyArgs.currTick.frame() << " with startNodeId "
						<< replyArgs.startNodeId << ", destinationNodeId " << replyArgs.destinationNodeId << ", and taxiDriverId "
						<< replyArgs.taxiDriverId << std::endl;
				}
	        }

	        default: break;
	    };
}

bool VehicleController::isNonspatial()
{
	return true;
}
}


