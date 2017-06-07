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

#include "geospatial/network/RoadNetwork.hpp"

namespace sim_mob
{
MobilityServiceController::~MobilityServiceController()
{
	safe_delete_item(rebalancer);
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
	try
	{
	currTick = now;

	if (localTick == scheduleComputationPeriod)
	{
		localTick = 0;

		std::vector<MessageResult> messageResults = computeSchedules();

		std::vector<TripRequestMessage>::iterator request = requestQueue.begin();
		std::vector<MessageResult>::iterator messageResult = messageResults.begin();

#ifndef NDEBUG
		if (requestQueue.size() != messageResults.size() )
		{
			std::stringstream msg; msg<<"requestQueue.size()="<<requestQueue.size()<<", messageResults.size()="<<
				messageResults.size()<<", while they must be the same. Insert the request in the message result";
			throw std::runtime_error(msg.str() );
		}
#endif

		std:std::vector<TripRequestMessage> retryRequestQueue;

		while (request != requestQueue.end())
		{
		    if ((*messageResult) == MessageResult::MESSAGE_ERROR_VEHICLES_UNAVAILABLE)
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
		rebalancer->rebalance(availableDrivers, currTick);

	}
	else
	{
		localTick += 1;
	}

	return Entity::UpdateStatus::Continue;
	}catch (std::exception& e)
	{
		Print()<<"Error in "<<__FILE__<<":"<<__LINE__<< ". The problem is "<< e.what() << std::endl;
#ifndef NDEBUG
		exit(0);
#endif
	}
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

			/*
			TripRequest r; r.currTick=requestArgs.currTick; r.userId = requestArgs.personId;
			r.startNodeId=requestArgs.startNodeId; r.destinationNodeId=requestArgs.destinationNodeId;
			r.extraTripTimeThreshold=requestArgs.extraTripTimeThreshold;
			*/
			requestQueue.push_back(requestArgs);


			const Node* startNode = RoadNetwork::getInstance()->getMapOfIdvsNodes().at(requestArgs.startNodeId);
			rebalancer->onRequestReceived(startNode);

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


				TripRequestMessage r; r.currTick=replyArgs.currTick; r.personId= replyArgs.personId;
				r.startNodeId=replyArgs.startNodeId; r.destinationNodeId=replyArgs.destinationNodeId;
				r.extraTripTimeThreshold=replyArgs.extraTripTimeThreshold;
				requestQueue.push_back(r);
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

void MobilityServiceController::sendScheduleProposition(const Person* driver, Schedule schedule) const
{
	messaging::MessageBus::PostMessage((messaging::MessageHandler*) driver, MSG_SCHEDULE_PROPOSITION,
			messaging::MessageBus::MessagePtr(new SchedulePropositionMessage(currTick, schedule) ) );
}

bool MobilityServiceController::isCruising(Person* driver) const
{
    MobilityServiceDriver* currDriver = driver->exportServiceDriver();
    if (currDriver)
    {
        if (currDriver->getServiceStatus() == MobilityServiceDriver::SERVICE_FREE)
        {
            return true;
        }
    }
    return false;
}

const Node* MobilityServiceController::getCurrentNode(Person* driver) const
{
    MobilityServiceDriver* currDriver = driver->exportServiceDriver();
    if (currDriver)
    {
    	const Node* currentNode =currDriver->getCurrentNode();
        return currentNode;
    }
    return nullptr;
}

const Person* MobilityServiceController::findClosestDriver(const Node* node) const
{
	double bestDistanceSqr = std::numeric_limits<double>::max();
	double bestX, bestY;

	Person* bestDriver = NULL;
	auto driver = availableDrivers.begin();

	while (driver != availableDrivers.end())
	{
		if (isCruising(*driver) )
		{
				const Node* driverNode = getCurrentNode(*driver);
				double currDistanceSqr =
					(node->getPosX() - driverNode->getPosX() )*
					(node->getPosX() - driverNode->getPosX() );
				currDistanceSqr+=
					(node->getPosY() - driverNode->getPosY() )*
					(node->getPosY() - driverNode->getPosY() );

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
	if (bestDriver!=NULL)
		ControllerLog() << "Closest vehicle is at (" << bestX << ", " << bestY << ")" << std::endl;
	else
		ControllerLog()<<"No available driver" << std::endl;
	return bestDriver;
}

}

