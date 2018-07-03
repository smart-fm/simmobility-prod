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

class DriverComm : public Driver
{
public:
	DriverComm(Person_ST *parent, MutexStrategy mtxStrat, DriverBehavior* behavior = nullptr, DriverCommMovement* movement = nullptr);
	virtual ~DriverComm();

	virtual Role<Person_ST>* clone(Person_ST *parent) const;
};

}//namspace sim_mob
