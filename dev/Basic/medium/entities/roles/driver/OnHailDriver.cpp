//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "OnHailDriver.hpp"
#include "entities/controllers/MobilityServiceController.hpp"

using namespace sim_mob;
using namespace medium;
using namespace std;

OnHailDriver::OnHailDriver(Person_MT *parent, const MutexStrategy &mtx, OnHailDriverBehaviour *behaviour,
                           OnHailDriverMovement *movement, string roleName, Type roleType) :
		Driver(parent, behaviour, movement, roleName, roleType), passenger(nullptr), movement(movement)
{
}

OnHailDriver::OnHailDriver(Person_MT *parent) : Driver(parent)
{
}

OnHailDriver::~OnHailDriver()
{
	//The driver should not be destroyed when it has a passenger. If it is being destroyed, then this
	//is an error scenario
#ifndef NDEBUG
	if(passenger)
	{
		stringstream msg;
		msg << "Driver " << parent->getDatabaseId() << " is being destroyed, but it has a passenger.";
		throw runtime_error(msg.str());
	}
#endif
}

Role<Person_MT>* OnHailDriver::clone(Person_MT *person) const
{
#ifndef NDEBUG
	if(person == nullptr)
	{
		return nullptr;
	}
#endif

	OnHailDriverMovement *driverMvt = new OnHailDriverMovement();
	OnHailDriverBehaviour *driverBhvr = new OnHailDriverBehaviour();
	OnHailDriver *driver = new OnHailDriver(person, person->getMutexStrategy(), driverBhvr, driverMvt, "onHailDriver");

	driverBhvr->setParentDriver(driver);
	driverMvt->setParentDriver(driver);
	driverMvt->setOnHailDriver(driver);

	return driver;
}

void OnHailDriver::HandleParentMessage(messaging::Message::MessageType type, const messaging::Message &msg)
{
}

const Node* OnHailDriver::getCurrentNode() const
{
	//return movement->getCurrentNode();
}

const vector<MobilityServiceController *>& OnHailDriver::getSubscribedControllers() const
{
	return subscribedControllers;
}

Schedule OnHailDriver::getAssignedSchedule() const
{
	stringstream msg;
	msg << "Driver " << parent->getDatabaseId() << " is an OnHailDriver. It will never have a schedule, "
	    << "this method should not be called on an OnHailDriver";
	throw runtime_error(msg.str());
}

void OnHailDriver::collectTravelTime()
{
	//Do nothing, as we do not collect travel times for on hail drivers
}

unsigned long OnHailDriver::getPassengerCount() const
{
	return (passenger ? 1 : 0);
}