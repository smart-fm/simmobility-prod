/*
 * MobilityServiceController.cpp
 *
 *  Created on: 21 Jun 2017
 *      Author: araldo
 */

#include "MobilityServiceController.hpp"
#include "logging/ControllerLog.hpp"

#include <string>

namespace sim_mob
{

const unsigned MobilityServiceController::toleratedExtraTime = 150; //seconds


MobilityServiceController::~MobilityServiceController()
{
	// TODO Auto-generated destructor stub
}


Entity::UpdateStatus MobilityServiceController::frame_init(timeslice now)
{
	if (!GetContext())
	{
		messaging::MessageBus::RegisterHandler(this);
	}
#ifndef NDEBUG
	consistencyChecks();
#endif

	currTick = now;

	return Entity::UpdateStatus::Continue;
}

Entity::UpdateStatus MobilityServiceController::frame_tick(timeslice now)
{
#ifndef NDEBUG
	consistencyChecks();
#endif
	currTick = now;
	return Entity::UpdateStatus::Continue;
}

void MobilityServiceController::HandleMessage(messaging::Message::MessageType type, const messaging::Message &message)
{
#ifndef NDEBUG
	consistencyChecks();
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
		const Person *person = unsubscribeArgs.person;
		unsubscribeDriver(unsubscribeArgs.person);
		break;
	}

	case MSG_DRIVER_SHIFT_END:
	{
		const DriverShiftCompleted &scheduleCompletedArgs = MSG_CAST(DriverShiftCompleted, message);
		onDriverShiftEnd(scheduleCompletedArgs.person);
		break;
	}

	case MSG_DRIVER_SCHEDULE_STATUS:
	{
		const DriverScheduleStatusMsg &statusMsgArgs = MSG_CAST(DriverScheduleStatusMsg, message);
		onDriverScheduleStatus(statusMsgArgs.person);
		break;
	}

	default:
		throw std::runtime_error("Unrecognized message");
	};

}


void MobilityServiceController::subscribeDriver(Person *driver)
{

#ifndef NDEBUG
	consistencyChecks();
	if (!isMobilityServiceDriver(driver))
	{
		std::stringstream msg;
		msg << "Driver " << driver->getDatabaseId() <<
		    " is not a MobilityServiceDriver" << std::endl;
		throw std::runtime_error(msg.str());
	}
#endif

	subscribedDrivers.push_back(driver);

	ControllerLog() << "Subscription received by the controller of type " << sim_mob::toString(controllerServiceType)
	                << ", controller info: " << toString() <<
	                ". Subscription from driver " << driver->getDatabaseId()
	                << " at time " << currTick << ". Now subscribed drivers are " << subscribedDrivers.size()
	                << std::endl;
}

void MobilityServiceController::unsubscribeDriver(Person *driver)
{
#ifndef NDEBUG
	if (driver->getDatabaseId().empty() )
	{
		std::stringstream msg; msg<<"The driver with pointer "<< driver<<" has no databaseId and she is asking to unsubscribe";
		throw std::runtime_error(msg.str() );
	}
#endif
	ControllerLog() << "Unsubscription of driver with pointer " << driver << " at time "
	                << currTick << std::endl;
	subscribedDrivers.erase(std::remove(subscribedDrivers.begin(),
	                                    subscribedDrivers.end(), driver), subscribedDrivers.end());
}

bool MobilityServiceController::isNonspatial()
{
#ifndef NDEBUG
	consistencyChecks();
#endif
	// A controller is not located in any specific place
	return true;
}

void MobilityServiceController::frame_output(timeslice now)
{
}

void MobilityServiceController::onRegistrationOnTheMessageBus() const
{
#ifndef NDEBUG
	consistencyChecks();
#endif
	sim_mob::ControllerLog() << "The controller has been successfully registered" << std::endl;
}

void MobilityServiceController::consistencyChecks() const
{
	try
	{ sim_mob::consistencyChecks(controllerServiceType); }
	catch (const std::runtime_error &e)
	{
		std::stringstream msg;
		msg << "Error in controller " << controllerId << ": " << e.what();
		throw std::runtime_error(msg.str());
	}
}

unsigned MobilityServiceController::getControllerId() const
{
	return controllerId;
}

MobilityServiceControllerType MobilityServiceController::getServiceType() const
{
	return controllerServiceType;
}

const std::string MobilityServiceController::toString() const
{
	std::string str;
	str = str + "Controller id:" + std::to_string(getControllerId()) + ",type:" + std::to_string(getServiceType());
	return str;
}

void MobilityServiceController::setToBeRemoved()
{
	Agent::setToBeRemoved();
}

void MobilityServiceController::onDriverShiftEnd(Person *person)
{
}

void MobilityServiceController::onDriverScheduleStatus(Person *person)
{
}

const std::string toString(const MobilityServiceControllerType type)
{

	switch (type)
	{
	case SERVICE_CONTROLLER_GREEDY:
	{
		return "SERVICE_CONTROLLER_GREEDY";
	}
	case SERVICE_CONTROLLER_SHARED:
	{
		return "SERVICE_CONTROLLER_SHARED";
	}
	case SERVICE_CONTROLLER_FRAZZOLI:
	{
		return "SERVICE_CONTROLLER_FRAZZOLI";
	}
	case SERVICE_CONTROLLER_ON_HAIL:
	{
		return "SERVICE_CONTROLLER_ON_HAIL";
	}
	case SERVICE_CONTROLLER_INCREMENTAL:
	{
		return "SERVICE_CONTROLLER_INCREMENTAL";
	}
	default:
	{
		std::stringstream msg;
		msg << "Unrecognized MobilityServiceControllerType " << type;
		throw std::runtime_error(msg.str());
	}
	}
}

void consistencyChecks(const MobilityServiceControllerType type)
{
	switch (type)
	{
	case SERVICE_CONTROLLER_GREEDY:
	case SERVICE_CONTROLLER_SHARED:
	case SERVICE_CONTROLLER_FRAZZOLI:
	case SERVICE_CONTROLLER_ON_HAIL:
	case SERVICE_CONTROLLER_INCREMENTAL:
	{
		break;
	}
	default:
	{
		std::stringstream msg;
		msg << "Unrecognized MobilityServiceControllerType " << type;
		throw std::runtime_error(msg.str());
	}
	}
}

} /* namespace sim_mob */


