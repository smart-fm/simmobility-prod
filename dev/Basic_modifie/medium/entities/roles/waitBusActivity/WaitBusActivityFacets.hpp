//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/conflux/Conflux.hpp"
#include "entities/roles/RoleFacets.hpp"

namespace sim_mob
{
namespace medium
{

class WaitBusActivity;

/**
 * A medium-term WaitBusActivity behavior facet.
 * \author zhang huai peng
 */
class WaitBusActivityBehavior: public BehaviorFacet
{
public:
	explicit WaitBusActivityBehavior();
	virtual ~WaitBusActivityBehavior();

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
	void setParentWaitBusActivity(WaitBusActivity* parentWaitBusActivity);

protected:
	WaitBusActivity* parentWaitBusActivity;
};

/**
 * A medium-term WaitBusActivity movement facet.
 * \author zhang huai peng
 */
class WaitBusActivityMovement: public MovementFacet
{
public:
	explicit WaitBusActivityMovement();
	virtual ~WaitBusActivityMovement();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual std::string frame_tick_output();
	virtual Conflux* getStartingConflux() const;

	TravelMetric & startTravelTimeMetric();
	TravelMetric & finalizeTravelTimeMetric();

	/**
	 * set parent reference to waiting activity role.
	 * @param parentWaitBusActivity is pointer to parent waiting activity role
	 */
	void setParentWaitBusActivity(WaitBusActivity* parentWaitBusActivity);

protected:
	WaitBusActivity* parentWaitBusActivity;
};

}
}
