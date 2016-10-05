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
#include "entities/vehicle/BusRoute.hpp"
#include "entities/vehicle/Bus.hpp"
#include "entities/BusController.hpp"
#include "geospatial/network/Point.hpp"
#include "geospatial/network/PT_Stop.hpp"
#include "logging/Log.hpp"
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "entities/roles/waitBusActivity/WaitBusActivity.hpp"

using namespace sim_mob;

BusDriver::BusDriver(Person_ST *parent, MutexStrategy mtxStrat, BusDriverBehavior *behavior, BusDriverMovement *movement, Role<Person_ST>::Type roleType_) :
Driver(parent, mtxStrat, behavior, movement, roleType_), sequenceNum(1)
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

bool BusDriver::isBusFull()
{
	return (passengerList.size() < ST_Config::getInstance().defaultBusCapacity) ? false : true;
}

void BusDriver::addPassenger(Passenger* passenger)
{
	passengerList.push_back(passenger);
}

double BusDriver::alightPassengers(BusStopAgent *stopAgent)
{
	double alightingTime = 0;
	const BusStop *stop = stopAgent->getBusStop();
	std::list<Passenger*>::iterator itPassenger = passengerList.begin();
	
	if (stop->isVirtualStop())
	{
		stop = stop->getTwinStop();
		
		if (stop->isVirtualStop())
		{
			stringstream msg;
			msg << "Both stops are virtual! Stop code " << stop->getStopCode();
			throw std::runtime_error(msg.str());
		}
		
		stopAgent = BusStopAgent::getBusStopAgentForStop(stop);
	}

	while (itPassenger != passengerList.end())
	{
		(*itPassenger)->makeAlightingDecision(stopAgent->getBusStop());

		if ((*itPassenger)->canAlightVehicle())
		{
			stopAgent->addAlightingPerson(*itPassenger);
			itPassenger = passengerList.erase(itPassenger);
			alightingTime += (*itPassenger)->getParent()->getAlightingCharacteristics();
		}
		else
		{
			itPassenger++;
		}
	}
	
	return alightingTime;
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
