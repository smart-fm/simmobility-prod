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



void MobilityServiceController::subscribeDriver(Person *driver)
{
	ControllerLog()<<"Subscription received by the controller from driver "<< driver->getDatabaseId()
			<<" at time "<< currTick <<std::endl;
#ifndef NDEBUG
                if (!isMobilityServiceDriver(driver) )
                {
                        std::stringstream msg; msg<<"Driver "<<driver->getDatabaseId()<<
                        " is not a MobilityServiceDriver"<< std::endl;
                        throw std::runtime_error(msg.str() );
                }

#endif
	subscribedDrivers.push_back(driver);
}

void MobilityServiceController::unsubscribeDriver(Person *driver)
{
	ControllerLog() << "Unsubscription of driver " << driver->getDatabaseId() <<" at time "<< currTick<< std::endl;
	subscribedDrivers.erase(std::remove(subscribedDrivers.begin(),
	                                    subscribedDrivers.end(), driver), subscribedDrivers.end());
}


bool MobilityServiceController::isNonspatial()
{
	// A controller is not located in any specific place
	return true;
}

void MobilityServiceController::frame_output(timeslice now)
{
}


const std::string fromMobilityServiceControllerTypetoString(MobilityServiceControllerType type)
{
	switch(type)
	{
		case (SERVICE_CONTROLLER_UNKNOWN): { return "SERVICE_CONTROLLER_UNKNOWN"; }
		case (SERVICE_CONTROLLER_GREEDY): { return "SERVICE_CONTROLLER_GREEDY"; }
		case (SERVICE_CONTROLLER_SHARED): { return "SERVICE_CONTROLLER_SHARED"; }
		case (SERVICE_CONTROLLER_ON_HAIL): { return "SERVICE_CONTROLLER_ON_HAIL"; }
		case (SERVICE_CONTROLLER_FRAZZOLI): { return "SERVICE_CONTROLLER_FRAZZOLI"; }
		default:{throw std::runtime_error("type unknown"); }
	};
}

} /* namespace sim_mob */


