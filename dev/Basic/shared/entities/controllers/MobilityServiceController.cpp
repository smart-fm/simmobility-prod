/*
 * MobilityServiceController.cpp
 *
 *  Created on: Feb 20, 2017
 *      Author: Akshay Padmanabha
 */

#include "MobilityServiceController.hpp"

#include "message/MessageBus.hpp"
#include "message/MobilityServiceControllerMessage.hpp"

namespace sim_mob
{
MobilityServiceController::~MobilityServiceController()
{
}

void MobilityServiceController::subscribeDriver(Person* person)
{
	drivers.push_back(person);
}

void MobilityServiceController::unsubscribeDriver(Person* person)
{
	drivers.erase(std::remove(drivers.begin(),
		drivers.end(), person), drivers.end());
}

Entity::UpdateStatus MobilityServiceController::frame_init(timeslice now)
{
	if (!GetContext())
	{
		messaging::MessageBus::RegisterHandler(this);
	}
	
	return Entity::UpdateStatus::Continue;
}

Entity::UpdateStatus MobilityServiceController::frame_tick(timeslice now)
{
	currTimeSlice = now;

	if (localTick == scheduleComputationPeriod)
	{
		localTick = 0;

		std::vector<MessageResult> messageResults = computeSchedules();

		std::vector<TripRequest>::iterator request = requestQueue.begin();
		std::vector<MessageResult>::iterator messageResult = messageResults.begin();

		std:std::vector<TripRequest> retryRequestQueue;

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

void MobilityServiceController::frame_output(timeslice now)
{
}

void MobilityServiceController::HandleMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
	switch (type) {
	        case MSG_TRIP_REQUEST:
	        {
				const TripRequestMessage& requestArgs = MSG_CAST(TripRequestMessage, message);

				Print() << "Request received from " << requestArgs.personId << " at time " << currTick.frame() << ". Message was sent at "
					<< requestArgs.currTick.frame() << " with startNodeId " << requestArgs.startNodeId << ", destinationNodeId "
					<< requestArgs.destinationNodeId << ", and driverId null" << std::endl;

				requestQueue.push_back({requestArgs.currTick, requestArgs.personId, requestArgs.startNodeId,
					requestArgs.destinationNodeId, requestArgs.extraTripTimeThreshold});
	            break;
	        }

	        case MSG_TRIP_PROPOSITION_REPLY:
	        {
				const TripPropositionReplyMessage& replyArgs = MSG_CAST(TripPropositionReplyMessage, message);
				if (!replyArgs.success) {
					Print() << "Request received from " << replyArgs.personId << " at time " << currTick.frame() << ". Message was sent at "
						<< replyArgs.currTick.frame() << " with startNodeId " << replyArgs.startNodeId << ", destinationNodeId "
						<< replyArgs.destinationNodeId << ", and driverId null" << std::endl;

					requestQueue.push_back({replyArgs.currTick, replyArgs.personId, replyArgs.startNodeId,
						replyArgs.destinationNodeId, replyArgs.extraTripTimeThreshold});
				} else {
					Print() << "Assignment response received from " << replyArgs.personId << " at time "
						<< currTick.frame() << ". Message was sent at " << replyArgs.currTick.frame() << " with startNodeId "
						<< replyArgs.startNodeId << ", destinationNodeId " << replyArgs.destinationNodeId << ", and driverId "
						<< replyArgs.driverId << std::endl;
				}
	        }

	        default: break;
	    };
}

bool MobilityServiceController::isNonspatial()
{
	return true;
}
}




