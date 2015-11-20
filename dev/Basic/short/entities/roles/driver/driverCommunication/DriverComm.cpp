//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "DriverComm.hpp"

#include "entities/roles/driver/driverCommunication/DriverCommFacets.hpp"
#include "entities/Person_ST.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/commsim/broker/Broker.hpp"
#include "entities/roles/driver/DriverFacets.hpp"

using namespace sim_mob;


DriverComm::DriverComm(Person_ST *parent, MutexStrategy mtxStrat, DriverBehavior* behavior, DriverCommMovement* movement) :
	Driver(parent, mtxStrat, behavior, movement)
{
}

DriverComm::~DriverComm()
{
}

Role<Person_ST>* DriverComm::clone(Person_ST *parent) const
{
	DriverBehavior *behavior = new DriverBehavior();
	DriverCommMovement* movement = new DriverCommMovement();
	DriverComm *driver = new DriverComm(parent, parent->getMutexStrategy(), behavior, movement);
	behavior->setParentDriver(driver);
	movement->setParentDriver(driver);

	return driver;
}


