/*
 * OnCallController.cpp
 *
 *  Created on: Feb 20, 2017
 *      Author: Akshay Padmanabha
 */

#include "geospatial/network/RoadNetwork.hpp"
#include "logging/ControllerLog.hpp"
#include "message/MessageBus.hpp"
#include "util/GeomHelpers.hpp"
#include "util/Utils.hpp"
#include <algorithm>    // std::sort, next_permutation

#include "OnCallController.hpp"
#include "path/PathSetManager.hpp" // for PrivateTrafficRouteChoice

using namespace sim_mob;
using namespace messaging;

//TODO: These should not be hardcoded
const static double additionalDelayTreshold = std::numeric_limits<double>::max();
const static double waitingTimeTreshold = std::numeric_limits<double>::max();

OnCallController::~OnCallController()
{
	safe_delete_item(rebalancer);
}

void OnCallController::subscribeDriver(Person *driver)
{
	MobilityServiceController::subscribeDriver(driver);
	availableDrivers.push_back(driver);
	driverSchedules.emplace(driver, Schedule() );
}

void OnCallController::unsubscribeDriver(Person *driver)
{
	MobilityServiceController::unsubscribeDriver(driver);

#ifndef NDEBUG
	if (driverSchedules.find(driver) == driverSchedules.end() )
	{
		std::stringstream msg; msg<<"Driver "<< driver->getDatabaseId()<<" has been subscribed but had no "<<
			"schedule associated. This is impossible. It should have had at least an empty schedule";
		throw std::runtime_error(msg.str());
	}

	unsigned scheduleSize = driverSchedules.at(driver).size();
	if (scheduleSize>0 )
	{
		std::stringstream msg; msg<<"Driver "<< driver->getDatabaseId()<<" has a non empty schedule and she sent a message "
		<<"to unsubscribe. This is not admissible";
		throw std::runtime_error(msg.str());
	}

	if (availableDrivers.size()!= driverSchedules.size() )
	{
		std::stringstream msg; msg<<"availableDrivers.size()="<<availableDrivers.size()<<
		", driverSchedules.size()="<<driverSchedules.size()<<". They should be equal";
		throw std::runtime_error(msg.str());
	}
#endif


	driverSchedules.erase(driver);

	//http://en.cppreference.com/w/cpp/algorithm/remove
	availableDrivers.erase(std::remove(availableDrivers.begin(),
	                                   availableDrivers.end(), driver), availableDrivers.end());

#ifndef NDEBUG
	if (availableDrivers.size()!= driverSchedules.size() )
	{
		std::stringstream msg; msg<<"availableDrivers.size()="<<availableDrivers.size()<<
		", driverSchedules.size()="<<driverSchedules.size()<<". They should be equal";
		throw std::runtime_error(msg.str());
	}
#endif
}

void OnCallController::driverAvailable(Person *driver)
{
	availableDrivers.push_back(driver);
	driverSchedules.erase(driver); // The driver has no more schedules
}

void OnCallController::driverUnavailable(Person *person)
{
	availableDrivers.erase(std::remove(availableDrivers.begin(),
	                                   availableDrivers.end(), person), availableDrivers.end());
}


Entity::UpdateStatus OnCallController::frame_tick(timeslice now)
{
	currTick = now;

	if (localTick == scheduleComputationPeriod)
	{
		localTick = 0;

#ifndef NDEBUG
		if (isComputingSchedules)
		{
			throw std::runtime_error("At this point, the controller should not be computing schedules");
		}
		isComputingSchedules = true;
#endif
		computeSchedules();

#ifndef NDEBUG
		isComputingSchedules = false;
#endif

		rebalancer->rebalance(availableDrivers, currTick);
	}
	else
	{
		localTick += 1;
	}

	return Entity::UpdateStatus::Continue;
}

void OnCallController::frame_output(timeslice now)
{
}

void OnCallController::HandleMessage(messaging::Message::MessageType type, const messaging::Message &message)
{
#ifndef NDEBUG
	if (isComputingSchedules)
	{
		throw std::runtime_error("At this point, the controller should not be computing schedules");
	}
#endif

	switch (type)
	{
	case MSG_DRIVER_SUBSCRIBE:
	{
		const DriverSubscribeMessage &subscribeArgs = MSG_CAST(DriverSubscribeMessage, message);
		subscribeDriver(subscribeArgs.person);
		break;
	}

	case MSG_DRIVER_UNSUBSCRIBE:
	{
		const DriverUnsubscribeMessage &unsubscribeArgs = MSG_CAST(DriverUnsubscribeMessage, message);
		ControllerLog() << "Driver " << unsubscribeArgs.person->getDatabaseId() << " unsubscribed " << std::endl;
		unsubscribeDriver(unsubscribeArgs.person);
		break;
	}

	case MSG_DRIVER_AVAILABLE:
	{
		const DriverAvailableMessage &availableArgs = MSG_CAST(DriverAvailableMessage, message);
		driverAvailable(availableArgs.person);
		break;
	}

	case MSG_TRIP_REQUEST:
	{
		const TripRequestMessage &requestArgs = MSG_CAST(TripRequestMessage, message);

		ControllerLog() << "Request received by the controller: "<< requestArgs<<". This request is received at "<<
				currTick<<std::endl;

#ifndef NDEBUG
		if (currTick < requestArgs.timeOfRequest)
			throw std::runtime_error("Request received before it was issed: impossible");
#endif

		/*
		TripRequest r; r.currTick=requestArgs.currTick; r.userId = requestArgs.personId;
		r.startNodeId=requestArgs.startNodeId; r.destinationNodeId=requestArgs.destinationNodeId;
		r.extraTripTimeThreshold=requestArgs.extraTripTimeThreshold;
		*/
		requestQueue.push_back(requestArgs);


		const Node *startNode = RoadNetwork::getInstance()->getMapOfIdvsNodes().at(requestArgs.startNodeId);
		rebalancer->onRequestReceived(startNode);

		break;
	}

	case MSG_SCHEDULE_PROPOSITION_REPLY:
	{
		const SchedulePropositionReplyMessage &replyArgs = MSG_CAST(SchedulePropositionReplyMessage, message);
		if (!replyArgs.success)
		{
			ControllerLog() << "Assignment failure received from " << replyArgs.personId << " at time "
			                << currTick.frame() << ". Message was sent at " << replyArgs.currTick.frame()
			                << " with startNodeId "
			                << replyArgs.startNodeId << ", destinationNodeId " << replyArgs.destinationNodeId
			                << ", and driverId "
			                << replyArgs.driver->getDatabaseId() << std::endl;

			TripRequestMessage r;
			r.timeOfRequest = replyArgs.currTick;
			r.userId = replyArgs.personId;
			r.startNodeId = replyArgs.startNodeId;
			r.destinationNodeId = replyArgs.destinationNodeId;
			r.extraTripTimeThreshold = replyArgs.extraTripTimeThreshold;
			requestQueue.push_back(r);

			driverAvailable(replyArgs.driver);
		}
		else
		{
			ControllerLog() << "Assignment success received from " << replyArgs.personId << " at time "
			                << currTick.frame() << ". Message was sent at " << replyArgs.currTick.frame()
			                << " with startNodeId "
			                << replyArgs.startNodeId << ", destinationNodeId " << replyArgs.destinationNodeId
			                << ", and driverId "
			                << replyArgs.driver->getDatabaseId() << std::endl;

			driverUnavailable(replyArgs.driver);
		}
		break;
	}

	default:
		// If it is not a message specific to this controller, let the generic controller handle it
		MobilityServiceController::HandleMessage(type, message);
	};

}


void OnCallController::assignSchedule(const Person *driver, const Schedule& schedule)
{
	MessageBus::PostMessage((MessageHandler *) driver, MSG_SCHEDULE_PROPOSITION, MessageBus::MessagePtr(
			new SchedulePropositionMessage(currTick, schedule)));

#ifndef NDEBUG
	if (driverSchedules.find(driver) != driverSchedules.end() )
	{
		std::stringstream msg; msg<<"Trying to assign a schedule to driver "<< driver->getDatabaseId() << " who already has one."<<
		" If you are using the greedy controller, this is not possible. Otherwise, please disable this error";
		throw runtime_error(msg.str());
	}
#endif

	driverSchedules[driver]=schedule;
}

bool OnCallController::isCruising(Person *driver) const
{
	MobilityServiceDriver *currDriver = driver->exportServiceDriver();
	if (currDriver)
	{
		if (currDriver->getServiceStatus() == MobilityServiceDriver::SERVICE_FREE)
		{
			return true;
		}
	}
	return false;
}

const Node *OnCallController::getCurrentNode(const Person *driver) const
{
	MobilityServiceDriver *currDriver = driver->exportServiceDriver();
	if (currDriver)
	{
		const Node *currentNode = currDriver->getCurrentNode();
		return currentNode;
	}
	return nullptr;
}

const Person *OnCallController::findClosestDriver(const Node *node) const
{
	double bestDistance = std::numeric_limits<double>::max();
	double bestX, bestY;

	Person *bestDriver = NULL;
	auto driver = availableDrivers.begin();

	while (driver != availableDrivers.end())
	{
		if (isCruising(*driver))
		{
			const Node *driverNode = getCurrentNode(*driver);
			double currDistance = dist(node->getLocation(), driverNode->getLocation());

			if (currDistance < bestDistance)
			{
				bestDriver = *driver;
				bestDistance = currDistance;
				bestX = driverNode->getPosX();
				bestY = driverNode->getPosY();
			}
		}
		driver++;
	}

	if (bestDriver != NULL)
	{
		ControllerLog() << "Closest vehicle is at (" << bestX << ", " << bestY << ")" << std::endl;
	}
	else
	{
		ControllerLog() << "No available driver" << std::endl;
	}

	return bestDriver;
}

//TODO: in the request itself, the user should specify the earliest and latest pickup and dropoff times
double  OnCallController::evaluateSchedule(const Node* initialPosition, const Schedule& schedule,
		double additionalDelayThreshold, double waitingTimeThreshold) const
{
	double scheduleTimeStamp = currTick.ms() / 1000.0; // In seconds
	unsigned latestNodeId = initialPosition->getNodeId();

	// Check that each user is picked up before being dropped off
	std::set<string> dropoffs;
	for (const ScheduleItem& scheduleItem : schedule)
	{
		switch (scheduleItem.scheduleItemType)
		{
			case (ScheduleItemType::DROPOFF):
			{
				dropoffs.insert(scheduleItem.tripRequest.userId);

				unsigned nextNodeId = scheduleItem.tripRequest.destinationNodeId;
				scheduleTimeStamp += PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
						latestNodeId, nextNodeId, DailyTime(currTick.ms()));
				latestNodeId = nextNodeId;

				double earliestPickupTimeStamp = scheduleItem.tripRequest.timeOfRequest.ms()/100.0; // in seconds
				double minimumTravelTime = PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
						scheduleItem.tripRequest.startNodeId, scheduleItem.tripRequest.destinationNodeId,
						DailyTime(currTick.ms()));
				double latestDropoffTimeStamp = earliestPickupTimeStamp + minimumTravelTime + additionalDelayThreshold;
				if (scheduleTimeStamp > latestDropoffTimeStamp)
					return -1;
				break;
			};
			case (ScheduleItemType::PICKUP):
			{
				if ( dropoffs.find(scheduleItem.tripRequest.userId) != dropoffs.end() )
					// Trying to pick up a user who is scheduled to be dropped off before
					return -1;

				unsigned nextNodeId = scheduleItem.tripRequest.startNodeId;
				scheduleTimeStamp += PrivateTrafficRouteChoice::getInstance()->getOD_TravelTime(
						latestNodeId, nextNodeId, DailyTime(currTick.ms()));
				latestNodeId = nextNodeId;

				double earliestPickupTimeStamp = scheduleItem.tripRequest.timeOfRequest.ms()/100.0; // in seconds
				if (scheduleTimeStamp > earliestPickupTimeStamp + waitingTimeThreshold)
					return -1;
				break;

			};
			case (ScheduleItemType::CRUISE):
			{
				throw std::runtime_error("CRUISE is an \"instantaneous\" schedule item, meaning that after the controller sends it, there is no reason to keep memory of it. Therefore, it should not be there anymore");

			};
			default:
			{
				throw runtime_error("Unknown schedule item type");
			}
		}
	}

	double travelTime = scheduleTimeStamp - currTick.ms() / 1000.0;

#ifndef NDEBUG
	if (schedule.size() == 0)
		throw std::runtime_error("You are evaluating a schedule of 0 scheduleItems. Why would you want to do that? Is it an error?");

	if (travelTime <= 1e-5)
	{
		std::stringstream msg; msg<<"The travel time for this schedule of "<< schedule.size()<<" schedule items is 0. Why? Is it an error?";
		throw std::runtime_error(msg.str());
	}
#endif

	return travelTime;
}

double OnCallController::computeOptimalSchedule(const Node* initialNode, const Schedule& currentSchedule,
		const std::vector<TripRequestMessage>& additionalRequests,
		Schedule& newSchedule) const
{
	double travelTime = std::numeric_limits<double>::max();

	//Contruct the required ScheduleItems for the new requests
	std::vector<ScheduleItem> additionalScheduleItems;
	for (const TripRequestMessage& request : additionalRequests)
	{
		additionalScheduleItems.push_back(ScheduleItem(ScheduleItemType::PICKUP, request));
		additionalScheduleItems.push_back(ScheduleItem(ScheduleItemType::DROPOFF, request));
	}

	//https://stackoverflow.com/a/201729/2110769
	Schedule tempSchedule(currentSchedule);
	tempSchedule.insert(tempSchedule.end(), additionalScheduleItems.begin(), additionalScheduleItems.end());

	// sorting is necessary to correctly compute the permutations (see https://www.topcoder.com/community/data-science/data-science-tutorials/power-up-c-with-the-standard-template-library-part-1/)
	std::sort(tempSchedule.begin(), tempSchedule.end());

	do{
		double tempTravelTime = evaluateSchedule(initialNode, tempSchedule, additionalDelayTreshold, waitingTimeTreshold);
		if (tempTravelTime < travelTime)
		{
			travelTime = tempTravelTime; newSchedule = tempSchedule;
		}
	} while (std::next_permutation(tempSchedule.begin(), tempSchedule.end() ) );

#ifndef NDEBUG
	ControllerLog()<<"Current schedule: ";
	for (const ScheduleItem& item : currentSchedule) ControllerLog()<< item<<",";
	ControllerLog()<<". Trying to add requests [";
	for (const TripRequestMessage& request : additionalRequests) ControllerLog()<<request;
	ControllerLog()<<". The optimal schedule is ";
	for (const ScheduleItem& item : newSchedule) ControllerLog()<< item<<",";
	ControllerLog()<<std::endl;
#endif

	return travelTime;
}
