//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BusDriver.hpp"
#include "BusDriverFacets.hpp"

#include <vector>
#include <iostream>
#include <cmath>

#include "DriverUpdateParams.hpp"
#include "entities/Person_ST.hpp"
#include "entities/vehicle/BusRoute.hpp"
#include "entities/vehicle/Bus.hpp"
#include "entities/BusController.hpp"
#include "geospatial/network/Point.hpp"
#include "geospatial/network/PT_Stop.hpp"
#include "logging/Log.hpp"
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "entities/roles/waitBusActivityRole/WaitBusActivityRole.hpp"

using namespace sim_mob;

BusDriver::BusDriver(Person_ST *parent, MutexStrategy mtxStrat, BusDriverBehavior *behavior, BusDriverMovement *movement, Role<Person_ST>::Type roleType_) :
Driver(parent, mtxStrat, behavior, movement, roleType_)
{
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

double BusDriver::getPositionX() const
{
	return currPos.getX();
}

double BusDriver::getPositionY() const
{
	return currPos.getY();
}
