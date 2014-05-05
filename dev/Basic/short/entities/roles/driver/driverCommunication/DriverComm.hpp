//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/roles/driver/Driver.hpp"

namespace sim_mob
{

class Broker;
class DriverCommMovement;
class DriverBehavior;

class DriverComm : public Driver {
public:
	DriverComm(Person* parent, sim_mob::MutexStrategy mtxStrat, sim_mob::DriverBehavior* behavior = nullptr, sim_mob::DriverCommMovement* movement = nullptr);
	virtual ~DriverComm();

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;
};

}//namspace sim_mob
