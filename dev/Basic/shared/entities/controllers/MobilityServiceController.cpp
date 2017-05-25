/*
 * MobilityServiceController.cpp
 *
 *  Created on: Feb 20, 2017
 *      Author: Akshay Padmanabha
 */

#include "MobilityServiceController.hpp"

#include "entities/Person.hpp"
#include "logging/ControllerLog.hpp"
#include "message/MessageBus.hpp"
#include "message/MobilityServiceControllerMessage.hpp"

namespace sim_mob
{
MobilityServiceController::~MobilityServiceController()
{
}

void MobilityServiceController::subscribeDriver(Person* person)
{
	subscribedDrivers.push_back(person);
	availableDrivers.push_back(person);
}

void MobilityServiceController::unsubscribeDriver(Person* person)
{
	subscribedDrivers.erase(std::remove(subscribedDrivers.begin(),
		subscribedDrivers.end(), person), subscribedDrivers.end());
	availableDrivers.erase(std::remove(availableDrivers.begin(),
		availableDrivers.end(), person), availableDrivers.end());
}

void MobilityServiceController::driverAvailable(Person* person)
{
	availableDrivers.push_back(person);
}

void MobilityServiceController::driverUnavailable(Person* person)
{
	availableDrivers.erase(std::remove(availableDrivers.begin(),
		availableDrivers.end(), person), availableDrivers.end());
}

Entity::UpdateStatus MobilityServiceController::frame_init(timeslice now)
{
	if (!GetContext())
	{
		messaging::MessageBus::RegisterHandler(this);
	}

	currTick = now;
	
	return Entity::UpdateStatus::Continue;
}

Entity::UpdateStatus MobilityServiceController::frame_tick(timeslice now)
{
	currTick = now;

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
		case MSG_DRIVER_SUBSCRIBE:
		{
			const DriverSubscribeMessage& subscribeArgs = MSG_CAST(DriverSubscribeMessage, message);
			subscribeDriver(subscribeArgs.person);
            break;
		}

		case MSG_DRIVER_UNSUBSCRIBE:
		{
			const DriverUnsubscribeMessage& unsubscribeArgs = MSG_CAST(DriverUnsubscribeMessage, message);
			unsubscribeDriver(unsubscribeArgs.person);
            break;
		}

		case MSG_DRIVER_AVAILABLE:
		{
			const DriverAvailableMessage& availableArgs = MSG_CAST(DriverAvailableMessage, message);
			driverAvailable(availableArgs.person);
            break;
		}

        case MSG_TRIP_REQUEST:
        {
			const TripRequestMessage& requestArgs = MSG_CAST(TripRequestMessage, message);

			ControllerLog() << "Request received from " << requestArgs.personId << " at time " << currTick.frame() << ". Message was sent at "
				<< requestArgs.currTick.frame() << " with startNodeId " << requestArgs.startNodeId << ", destinationNodeId "
				<< requestArgs.destinationNodeId << ", and driverId null" << std::endl;

			requestQueue.push_back({requestArgs.currTick, requestArgs.personId, requestArgs.startNodeId,
				requestArgs.destinationNodeId, requestArgs.extraTripTimeThreshold});
            break;
        }

        case MSG_SCHEDULE_PROPOSITION_REPLY:
        {
			const SchedulePropositionReplyMessage& replyArgs = MSG_CAST(SchedulePropositionReplyMessage, message);
			if (!replyArgs.success) {
				ControllerLog() << "Assignment failure received from " << replyArgs.personId << " at time "
					<< currTick.frame() << ". Message was sent at " << replyArgs.currTick.frame() << " with startNodeId "
					<< replyArgs.startNodeId << ", destinationNodeId " << replyArgs.destinationNodeId << ", and driverId "
					<< replyArgs.driver->getDatabaseId() << std::endl;

				requestQueue.push_back({replyArgs.currTick, replyArgs.personId, replyArgs.startNodeId,
					replyArgs.destinationNodeId, replyArgs.extraTripTimeThreshold});
			} else {
				ControllerLog() << "Assignment success received from " << replyArgs.personId << " at time "
					<< currTick.frame() << ". Message was sent at " << replyArgs.currTick.frame() << " with startNodeId "
					<< replyArgs.startNodeId << ", destinationNodeId " << replyArgs.destinationNodeId << ", and driverId "
					<< replyArgs.driver->getDatabaseId() << std::endl;

				driverUnavailable(replyArgs.driver);
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

