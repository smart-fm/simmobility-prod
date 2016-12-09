//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BusDriver.hpp"
#include "BusDriverFacets.hpp"

#include <vector>
#include <iostream>
#include <cmath>

#include "config/ST_Config.hpp"
#include "DriverUpdateParams.hpp"
#include "entities/BusStopAgent.hpp"
#include "entities/Person_ST.hpp"
#include "entities/BusController.hpp"
#include "entities/roles/waitBusActivity/WaitBusActivity.hpp"
#include "geospatial/network/Point.hpp"
#include "geospatial/network/PT_Stop.hpp"
#include "logging/Log.hpp"
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "message/MessageBus.hpp"
#include "message/ST_Message.hpp"

using namespace sim_mob;

BusDriver::BusDriver(Person_ST *parent, MutexStrategy mtxStrat, BusDriverBehavior *behavior, BusDriverMovement *movement, Role<Person_ST>::Type roleType_) :
Driver(parent, mtxStrat, behavior, movement, roleType_), sequenceNum(1), currBoardingTime(0), currAlightingTime(0), currBusStopAgent(nullptr)
{
	isBusDriver = true;
}

Role<Person_ST>* BusDriver::clone(Person_ST* parent) const
{
	BusDriverBehavior *behavior = new BusDriverBehavior();
	BusDriverMovement *movement = new BusDriverMovement();
	BusDriver *busdriver = new BusDriver(parent, parent->getMutexStrategy(), behavior, movement);
	
	behavior->setParentDriver(busdriver);
	movement->setParentDriver(busdriver);
	behavior->setParentBusDriver(busdriver);
	movement->setParentBusDriver(busdriver);
	movement->init();
	
	return busdriver;
}

vector<BufferedBase *> BusDriver::getSubscriptionParams()
{
	return Driver::getSubscriptionParams();
}

DriverRequestParams BusDriver::getDriverRequestParams()
{
	return DriverRequestParams();
}

void BusDriver::HandleParentMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
	switch(type)
	{
	case MSG_ATTEMPT_BOARD_BUS:
	{
		const PersonMessage &personMsg = MSG_CAST(PersonMessage, message);
		tryBoardingPassenger(personMsg.person);
		break;
	}

	case MSG_ALIGHT_BUS:
	{
		const PersonMessage &personMsg = MSG_CAST(PersonMessage, message);
		alightPassenger(personMsg.person);
		break;
	}
	}
}

bool BusDriver::isBusFull()
{
	return (passengerList.size() < ST_Config::getInstance().defaultBusCapacity) ? false : true;
}

void BusDriver::tryBoardingPassenger(Person_ST* passenger)
{
	if(!isBusFull())
	{
		//Add person to the passenger list
		passengerList.push_back(passenger);
		
		//Increment the boarding time by the persons boarding time
		currBoardingTime += passenger->getBoardingCharacteristics();
		
		//Send boarding success message to waiting person
		messaging::MessageBus::PostMessage(passenger, MSG_BOARD_BUS_SUCCESS, messaging::MessageBus::MessagePtr(new BusDriverMessage(this)));
	}
	else
	{
		//Send boarding failed message to the waiting person
		messaging::MessageBus::PostMessage(passenger, MSG_BOARD_BUS_FAIL, messaging::MessageBus::MessagePtr(new BusDriverMessage(this)));
	}	
}

void BusDriver::alightPassenger(Person_ST *passenger)
{
	//Remove person from passenger list
	passengerList.remove(passenger);
	
	currAlightingTime += passenger->getAlightingCharacteristics();
}

double BusDriver::getPositionX() const
{
	return currPos.getX();
}

double BusDriver::getPositionY() const
{
	return currPos.getY();
}

const std::string& BusDriver::getBusLineId() const
{
	return busLineId;
}

void BusDriver::setBusLineId(const std::string& busLine)
{
	busLineId = busLine;
}

BusStopAgent* BusDriver::getCurrBusStopAgent() const
{
	return currBusStopAgent;
}

