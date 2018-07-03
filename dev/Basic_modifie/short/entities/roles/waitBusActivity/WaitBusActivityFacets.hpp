//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/roles/RoleFacets.hpp"

namespace sim_mob
{

class WaitBusActivity;

class WaitBusActivityBehavior: public BehaviorFacet
{
protected:
	WaitBusActivity *parentWaitBusActivity;
	
public:
	explicit WaitBusActivityBehavior() :
	BehaviorFacet(), parentWaitBusActivity(nullptr)
	{
	}
	
	virtual ~WaitBusActivityBehavior()
	{
	}

	//Virtual overrides
	virtual void frame_init()
	{
	}

	virtual void frame_tick()
	{
	}

	virtual std::string frame_tick_output()
	{
		return std::string();
	}

	/**
	 * set parent reference to waiting activity role.
	 * @param parentWaitBusActivity is pointer to parent waiting activity role
	 */
	void setParentWaitBusActivity(WaitBusActivity *parentWaitBusActivity)
	{
		this->parentWaitBusActivity = parentWaitBusActivity;
	}
};


class WaitBusActivityMovement: public MovementFacet
{
protected:
	WaitBusActivity *parentWaitBusActivity;
	
public:
	explicit WaitBusActivityMovement();
	virtual ~WaitBusActivityMovement();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual std::string frame_tick_output();

	TravelMetric & startTravelTimeMetric();
	TravelMetric & finalizeTravelTimeMetric();

	/**
	 * set parent reference to waiting activity role.
	 * @param parentWaitBusActivity is pointer to parent waiting activity role
	 */
	void setParentWaitBusActivity(WaitBusActivity *parentWaitBusActivity);
};

}
