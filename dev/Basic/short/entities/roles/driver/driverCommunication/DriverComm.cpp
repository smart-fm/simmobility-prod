//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "DriverComm.hpp"
#include "DriverCommFacets.hpp"
#include "entities/Person.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/commsim/broker/Broker.hpp"

using namespace sim_mob;


sim_mob::DriverComm::DriverComm(Person* parent, sim_mob::MutexStrategy mtxStrat, sim_mob::DriverCommBehavior* behavior, sim_mob::DriverCommMovement* movement) :
	Driver(parent,mtxStrat, behavior, movement)
{
}

sim_mob::DriverComm::~DriverComm()
{
}

Role* sim_mob::DriverComm::clone(Person* parent) const
{
	DriverCommBehavior* behavior = new DriverCommBehavior(parent);
	DriverCommMovement* movement = new DriverCommMovement(parent);
	DriverComm* driver = new DriverComm(parent, parent->getMutexStrategy(), behavior, movement);
	behavior->setParentDriver(driver);
	movement->setParentDriver(driver);

	return driver;
}


