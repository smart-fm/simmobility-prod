/*
 * TrainDriverFacets.hpp
 *
 *  Created on: Feb 17, 2016
 *      Author: zhang huai peng
 */
#include "conf/settings/DisableMPI.h"
#include "entities/roles/RoleFacets.hpp"
#include "TrainPathMover.hpp"

namespace sim_mob {
namespace medium{
class TrainDriver;

class TrainBehavior : public BehaviorFacet
{
public:
	explicit TrainBehavior();
	virtual ~TrainBehavior();

	/**
	 * Virtual overrides
	 */
	virtual void frame_init();
	virtual void frame_tick();
	virtual std::string frame_tick_output();

	TrainDriver* getParentDriver() const;
	void setParentDriver(TrainDriver* parentDriver);

protected:
	/**Pointer to the parent Driver role. */
	TrainDriver* parentDriver;
};

class TrainMovement : public MovementFacet
{
public:
	explicit TrainMovement();
	virtual ~TrainMovement();

	//virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual std::string frame_tick_output();

	TrainDriver* getParentDriver() const;
	void setParentDriver(TrainDriver* parentDriver);

protected:
	virtual TravelMetric& startTravelTimeMetric();
	virtual TravelMetric& finalizeTravelTimeMetric();

private:
	/**Pointer to the parent Driver role.*/
	TrainDriver* parentDriver;
	/**Train path mover*/
	TrainPathMover trainPathMover;
	/**Train platform mover*/
	TrainPlatformMover trainPlatformMover;
private:
	/**
	 * get current speed limit
	 * @return current speed limit
	 */
	double getCurrentSpeedLimit();
	/**
	 * get effective speed
	 * @return effective speed
	 */
	double getEffectiveSpeed();
	/**
	 * get effective moving distance
	 * @return effective distance
	 */
	double getEffectiveMovingDistance();
};

}

} /* namespace sim_mob */


