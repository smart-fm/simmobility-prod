/*
 * WaitTaxiActivityFacets.hpp
 *
 *  Created on: Nov 28, 2016
 *      Author: zhang huai peng
 */

#ifndef WAITTAXIACTIVITYFACETS_HPP_
#define WAITTAXIACTIVITYFACETS_HPP_
#include <string>
#include "entities/conflux/Conflux.hpp"
#include "entities/roles/RoleFacets.hpp"
namespace sim_mob
{
namespace medium
{
class WaitTaxiActivity;

/**
 * A medium-term WaitTaxiActivity behavior facet.
 * \author zhang huai peng
 */
class WaitTaxiActivityBehavior: public BehaviorFacet
{
public:
	explicit WaitTaxiActivityBehavior();
	virtual ~WaitTaxiActivityBehavior();

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
	 * @param waitTaxiActivity is pointer to parent role
	 */
	void setWaitTaxiActivity(WaitTaxiActivity* waitTaxiActivity);

protected:
	WaitTaxiActivity* waitTaxiActivity;
};

/**
 * A medium-term WaitTaxiActivity movement facet.
 * \author zhang huai peng
 */
class WaitTaxiActivityMovement: public MovementFacet
{
public:
	explicit WaitTaxiActivityMovement();
	virtual ~WaitTaxiActivityMovement();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual std::string frame_tick_output();
	virtual Conflux* getStartingConflux() const;

	TravelMetric & startTravelTimeMetric();
	TravelMetric & finalizeTravelTimeMetric();
	/**
	 * set parent reference to waiting activity role.
	 * @param waitTaxiActivity is pointer to parent role
	 */
	void setWaitTaxiActivity(WaitTaxiActivity* waitTaxiActivity);

protected:
	WaitTaxiActivity* waitTaxiActivity;
};
}
}
#endif /* WAITTAXIACTIVITYFACETS_HPP_ */
