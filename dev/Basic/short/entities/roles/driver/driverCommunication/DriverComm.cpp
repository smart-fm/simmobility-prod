//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "DriverComm.hpp"

#include "entities/roles/driver/driverCommunication/DriverCommFacets.hpp"
#include "entities/Person.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/commsim/broker/Broker.hpp"
#include "entities/roles/driver/DriverFacets.hpp"

using namespace sim_mob;


sim_mob::DriverComm::DriverComm(Person* parent, sim_mob::MutexStrategy mtxStrat, sim_mob::DriverBehavior* behavior, sim_mob::DriverCommMovement* movement) :
	Driver(parent,mtxStrat, behavior, movement)
{
}

sim_mob::DriverComm::~DriverComm()
{
}

Role* sim_mob::DriverComm::clone(Person* parent) const
{
	DriverBehavior* behavior = new DriverBehavior(parent);
	DriverCommMovement* movement = new DriverCommMovement(parent);
	DriverComm* driver = new DriverComm(parent, parent->getMutexStrategy(), behavior, movement);
	behavior->setParentDriver(driver);
	movement->setParentDriver(driver);

	return driver;
}


