//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "OnCallDriver.hpp"

using namespace sim_mob;
using namespace medium;
using namespace messaging;
using namespace std;

OnCallDriver::OnCallDriver(Person_MT *parent, const MutexStrategy &mtx, OnCallDriverBehaviour *behaviour,
                           OnCallDriverMovement *movement, string roleName, Type roleType) :
		Driver(parent, behaviour, movement, roleName, roleType), movement(movement), behaviour(behaviour)
{
}

OnCallDriver::OnCallDriver(Person_MT *parent) : Driver(parent)
{
}

OnCallDriver::~OnCallDriver()
{
	//The driver should not be destroyed when it has a passenger. If it is being destroyed, then this
	//is an error scenario
#ifndef NDEBUG
	if(!passengers.empty())
	{
		stringstream msg;
		msg << "OnCallDriver " << parent->getDatabaseId() << " is being destroyed, but it has "
		    << passengers.size() << " passenger(s).";
		throw runtime_error(msg.str());
	}
#endif
}

Role<Person_MT>* OnCallDriver::clone(Person_MT *person) const
{
#ifndef NDEBUG
	if(person == nullptr)
	{
		return nullptr;
	}
#endif

	OnCallDriverMovement *driverMvt = new OnCallDriverMovement();
	OnCallDriverBehaviour *driverBhvr = new OnCallDriverBehaviour();
	OnCallDriver *driver = new OnCallDriver(person, person->getMutexStrategy(), driverBhvr, driverMvt, "OnCallDriver");

	driverBhvr->setParentDriver(driver);
	driverBhvr->setOnCallDriver(driver);
	driverMvt->setParentDriver(driver);
	driverMvt->setOnCallDriver(driver);

	return driver;
}

const Node* OnCallDriver::getCurrentNode() const
{
	return movement->getCurrentNode();
}

const vector<MobilityServiceController *>& OnCallDriver::getSubscribedControllers() const
{
	return subscribedControllers;
}

Schedule OnCallDriver::getAssignedSchedule() const
{
	return driverSchedule.getSchedule();
}

unsigned long OnCallDriver::getPassengerCount() const
{
	return passengers.size();
}

const MobilityServiceDriver* OnCallDriver::exportServiceDriver() const
{
	return this;
}

void OnCallDriver::subscribeToOrIgnoreController(const SvcControllerMap& controllers, MobilityServiceControllerType type)
{
	if (parent->getServiceVehicle().controllerSubscription & type)
	{
		auto range = controllers.equal_range(type);

#ifndef NDEBUG
		if (range.first == range.second)
		{
			std::stringstream msg;
			msg << "OnCallDriver " << parent->getDatabaseId() << " wants to subscribe to type "
			    << toString(type) << ", but no controller of that type is registered";
			throw std::runtime_error(msg.str());
		}
#endif

		for (auto itController = range.first; itController != range.second; ++itController)
		{
			MessageBus::PostMessage(itController->second, MSG_DRIVER_SUBSCRIBE,
			                        MessageBus::MessagePtr(new DriverSubscribeMessage(parent)));

#ifndef NDEBUG
			ControllerLog() << "OnCallDriver " << parent->getDatabaseId()
			                << " sent a subscription to the controller "
			                << itController->second->toString() << " at time " << parent->currTick;
			ControllerLog() << ". parentDriver pointer " << parent << endl;
#endif

			subscribedControllers.push_back(itController->second);
		}
	}
}

void OnCallDriver::endShift()
{
	//Notify the controller(s)
	for(auto ctrlr : subscribedControllers)
	{
		MessageBus::PostMessage(ctrlr, MSG_DRIVER_UNSUBSCRIBE,
		                        MessageBus::MessagePtr(new DriverUnsubscribeMessage(parent)));
	}

	parent->setToBeRemoved();

#ifndef NDEBUG
	ControllerLog() << parent->currTick.ms() << "ms: OnCallDriver "
	                << parent->getDatabaseId() << ": Shift ended"  << endl;
#endif
}