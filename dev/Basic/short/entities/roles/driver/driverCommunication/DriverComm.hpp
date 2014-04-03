//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/roles/driver/Driver.hpp"

namespace sim_mob
{

class Broker;
class DriverCommMovement;
class DriverCommBehavior;

class DriverComm : public Driver
{
	static int totalSendCnt;
	static int totalReceiveCnt;
	int sendCnt,receiveCnt;
public:

	DriverComm(Person* parent/*, Broker* managingBroker*/, sim_mob::MutexStrategy mtxStrat, sim_mob::DriverCommBehavior* behavior = nullptr, sim_mob::DriverCommMovement* movement = nullptr);
	virtual ~DriverComm();

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;

	void receiveModule(timeslice now);
	void sendModule(timeslice now);
	sim_mob::Agent * getParentAgent();
};

}//namspace sim_mob
