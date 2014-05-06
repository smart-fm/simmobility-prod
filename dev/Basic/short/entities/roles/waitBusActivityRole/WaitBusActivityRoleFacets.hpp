//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * WaitBusActivityRoleFacets.hpp
 *
 *  Created on: June 17th, 2013
 *      Author: Yao Jin
 */

#pragma once
#include "conf/settings/DisableMPI.h"
#include "entities/roles/RoleFacets.hpp"
#include "WaitBusActivityRole.hpp"

namespace sim_mob {

class WaitBusActivityRole;
class BusDriver;

class WaitBusActivityRoleBehavior: public sim_mob::BehaviorFacet {
public:
	explicit WaitBusActivityRoleBehavior(sim_mob::Person* parentAgent = nullptr);
	virtual ~WaitBusActivityRoleBehavior();

	WaitBusActivityRole* getParentWaitBusActivityRole() const {
		return parentWaitBusActivityRole;
	}

	void setParentWaitBusActivityRole(WaitBusActivityRole* parentWaitBusActivityRole) {
		this->parentWaitBusActivityRole = parentWaitBusActivityRole;
	}

private:
	WaitBusActivityRole* parentWaitBusActivityRole;

};

class WaitBusActivityRoleMovement: public sim_mob::MovementFacet {
public:
	explicit WaitBusActivityRoleMovement(sim_mob::Person* parentAgent = nullptr, std::string buslineid = "");
	virtual ~WaitBusActivityRoleMovement();

	bool getRegisteredFlag() { return registered; } // get the registered flag
	void setRegisteredFlag(bool registeredFlag) { registered = registeredFlag; } // set the registered flag
	sim_mob::BusStopAgent* getBusStopAgent() { return busStopAgent; }
	BusStop* setBusStopXY(const Node* node);//to find the nearest busstop to a node
	std::string getBuslineID() { return buslineId; }

	//bool isOnCrossing() const;
	WaitBusActivityRole* getParentWaitBusActivityRole() const {
		return parentWaitBusActivityRole;
	}
	void setParentWaitBusActivityRole(WaitBusActivityRole* parentWaitBusActivityRole) {
		this->parentWaitBusActivityRole = parentWaitBusActivityRole;
	}

public:
    //Set by the BusDriver to the MS this Person should board the bus.
	uint32_t boardingMS;
	// indicate whether WaitBusActivityRole is already boarded or not, at initialize stage (isBoarded is false)
	bool isBoarded;
	// tag to indicate the WaitBusActivityRole already determine which bus to take
	bool isTagged;
	//Indicates the BusDriver of the bus we will board when "boarding_Frame" is reached.
	sim_mob::BusDriver* busDriver;
protected:
	//Indicates whether or not this Activity has registered with the appropriate BusStopAgent.
	//An unregistered Activity will not be able to board buses.
	//BusStopAgents will automatically register every WaitBusActivityRole in their vicinity periodically.
	bool registered;
	// indicate at which busStopAgent he is waiting for a bus
	sim_mob::BusStopAgent* busStopAgent;
	//uint32_t TimeOfReachingBusStop;
	std::string buslineId;
	Point2D displayOffset;

public:
	WaitBusActivityRole* parentWaitBusActivityRole;
};
}

