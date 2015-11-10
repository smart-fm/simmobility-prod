//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BusDriver.hpp"
#include "BusDriverFacets.hpp"

#include <vector>
#include <iostream>
#include <cmath>

#include "DriverUpdateParams.hpp"
#include "entities/Person.hpp"
#include "entities/vehicle/BusRoute.hpp"
#include "entities/vehicle/Bus.hpp"
#include "entities/BusController.hpp"
#include "logging/Log.hpp"

#include "geospatial/network/Point.hpp"
#include "geospatial/network/BusStop.hpp"
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "entities/roles/waitBusActivityRole/WaitBusActivityRole.hpp"

using namespace sim_mob;

BusDriver::BusDriver(Person *parent, MutexStrategy mtxStrat, BusDriverBehavior *behavior, BusDriverMovement *movement, Role::type roleType_) :
Driver(parent, mtxStrat, behavior, movement, roleType_)
{
}

Role* BusDriver::clone(Person* parent) const
{
	BusDriverBehavior *behavior = new BusDriverBehavior(parent);
	BusDriverMovement *movement = new BusDriverMovement(parent);
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