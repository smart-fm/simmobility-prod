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

class WaitTrainActivity;

/**
 * A medium-term WaitTrainActivity behavior facet.
 * \author zhang huai peng
 */
class WaitTrainActivityBehavior: public BehaviorFacet
{
public:
	explicit WaitTrainActivityBehavior();
	virtual ~WaitTrainActivityBehavior();

	//Virtual overrides
	virtual void frame_init(){}
	virtual void frame_tick(){}
	virtual std::string frame_tick_output();
	/**
	 * set parent reference to waiting activity role.
	 * @param parent is pointer to parent waiting activity role
	 */
	void setParent(WaitTrainActivity* parent);

protected:
	WaitTrainActivity* parentWaitTrainActivity;
};

/**
 * A medium-term WaitTrainActivity movement facet.
 * \author zhang huai peng
 */
class WaitTrainActivityMovement: public MovementFacet
{
public:
	explicit WaitTrainActivityMovement();
	virtual ~WaitTrainActivityMovement();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual std::string frame_tick_output();

	TravelMetric & startTravelTimeMetric();
	TravelMetric & finalizeTravelTimeMetric();

	/**
	 * set parent reference to waiting activity role.
	 * @param parent is pointer to parent waiting activity role
	 */
	void setParent(WaitTrainActivity* parent);

protected:
	WaitTrainActivity* parentWaitTrainActivity;
};

}
}
