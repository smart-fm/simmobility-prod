//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * WaitBusActivityFacets.h
 *
 *  Created on: Mar 13, 2014
 *      Author: zhang huai peng
 */

#pragma once

#include "entities/roles/RoleFacets.hpp"
#include "entities/Person.hpp"

namespace sim_mob {
namespace medium
{

class WaitBusActivity;

class WaitBusActivityBehavior: public BehaviorFacet {
public:
	explicit WaitBusActivityBehavior(sim_mob::Person* parentAgent = nullptr);
	virtual ~WaitBusActivityBehavior();

	//Virtual overrides
	virtual void frame_init() {}
	virtual void frame_tick() {}
	virtual void frame_tick_output() {}

	/**
	 * set parent reference to waiting activity role.
	 * @param parentWaitBusActivity is pointer to parent waiting activity role
	 */
	void setParentWaitBusActivity(sim_mob::medium::WaitBusActivity* parentWaitBusActivity);

protected:
	sim_mob::medium::WaitBusActivity* parentWaitBusActivity;
};

class WaitBusActivityMovement: public MovementFacet {
public:
	explicit WaitBusActivityMovement(sim_mob::Person* parentAgent = nullptr);
	virtual ~WaitBusActivityMovement();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual void frame_tick_output();

	/**
	 * set parent reference to waiting activity role.
	 * @param parentWaitBusActivity is pointer to parent waiting activity role
	 */
	void setParentWaitBusActivity(sim_mob::medium::WaitBusActivity* parentWaitBusActivity);

protected:
	sim_mob::medium::WaitBusActivity* parentWaitBusActivity;
	unsigned int totalTimeToCompleteMS;
};
}
}
