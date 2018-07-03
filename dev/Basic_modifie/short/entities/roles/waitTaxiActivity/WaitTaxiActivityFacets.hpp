#ifndef WAITTAXIACTIVITYFACETS_HPP_
#define WAITTAXIACTIVITYFACETS_HPP_
#include <string>
#include "entities/roles/RoleFacets.hpp"
namespace sim_mob
{
class WaitTaxiActivity;
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

class WaitTaxiActivityMovement: public MovementFacet
{
public:
	explicit WaitTaxiActivityMovement();
	virtual ~WaitTaxiActivityMovement();

	//Virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual std::string frame_tick_output();

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
#endif /* WAITTAXIACTIVITYFACETS_HPP_ */
