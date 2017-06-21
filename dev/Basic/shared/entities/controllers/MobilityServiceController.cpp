/*
 * MobilityServiceController.cpp
 *
 *  Created on: 21 Jun 2017
 *      Author: araldo
 */

#include "MobilityServiceController.hpp"
#include "logging/ControllerLog.hpp"

namespace sim_mob {




MobilityServiceController::~MobilityServiceController() {
	// TODO Auto-generated destructor stub
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
	return Entity::UpdateStatus::Continue;
}

void MobilityServiceController::HandleMessage(messaging::Message::MessageType type, const messaging::Message &message)
{

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

	default:
		throw std::runtime_error("Unrecognized message");
	};

}


void MobilityServiceController::subscribeDriver(Person *person)
{
	subscribedDrivers.push_back(person);
}

void MobilityServiceController::unsubscribeDriver(Person *person)
{
	subscribedDrivers.erase(std::remove(subscribedDrivers.begin(),
	                                    subscribedDrivers.end(), person), subscribedDrivers.end());
}


bool MobilityServiceController::isNonspatial()
{
	// A controller is not located in any specific place
	return true;
}

void MobilityServiceController::frame_output(timeslice now)
{
}


} /* namespace sim_mob */
